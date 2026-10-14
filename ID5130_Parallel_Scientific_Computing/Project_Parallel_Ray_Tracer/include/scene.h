#pragma once
#include "box.h"
#include "bvh.h"
#include "csg.h"
#include "cylinder.h"
#include "hittable.h"
#include "light.h"
#include "material.h"
#include "plane.h"
#include "sphere.h"
#include "vec3.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rt {

enum class Integrator { Whitted, PathTrace };

// Parsed scene. Owns materials (stable pointers) and primitives (via world).
struct Scene {
    int    W{800}, H{450};
    Vec3   camera_pos{0, 1.2, 3.0};
    Vec3   camera_lookat{0, 0.4, -0.8};
    Vec3   camera_up{0, 1, 0};
    double vfov{45.0};
    Vec3   background{0.6, 0.75, 0.95};
    Vec3   ambient{0.04, 0.04, 0.05};
    int    max_depth{10};
    Integrator integrator{Integrator::Whitted};
    int    spp{1};
    std::vector<PointLight> lights;
    std::vector<AreaLight>  area_lights;

    std::vector<std::unique_ptr<Material>>        material_storage;
    std::unordered_map<std::string, const Material*> materials_by_name;

    HittableList world;
};

namespace detail {

struct Line {
    std::vector<std::string> toks;
    bool opens_block{false};
    bool closes_block{false};
    int  line_no{0};
};

inline std::vector<Line> tokenize_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open scene file: " + path);
    std::vector<Line> out;
    std::string raw;
    int line_no = 0;
    while (std::getline(in, raw)) {
        ++line_no;
        const auto hash = raw.find('#');
        if (hash != std::string::npos) raw.erase(hash);
        std::istringstream iss(raw);
        Line L;
        L.line_no = line_no;
        std::string tok;
        while (iss >> tok) L.toks.push_back(tok);
        if (L.toks.empty()) continue;
        if (L.toks.size() == 1 && L.toks[0] == "}") {
            L.closes_block = true;
            out.push_back(std::move(L));
            continue;
        }
        if (L.toks.back() == "{") {
            L.toks.pop_back();
            L.opens_block = true;
        }
        out.push_back(std::move(L));
    }
    return out;
}

struct Cursor {
    const std::vector<Line>& lines;
    size_t i{0};
    bool done() const { return i >= lines.size(); }
    const Line& peek() const { return lines[i]; }
    const Line& eat() { return lines[i++]; }
};

inline double to_num(const std::string& s, int line_no) {
    try { return std::stod(s); }
    catch (...) { throw std::runtime_error("line " + std::to_string(line_no) + ": not a number: " + s); }
}

struct Attrs {
    std::unordered_map<std::string, std::vector<std::string>> kv;
};

inline Attrs parse_attrs(const std::vector<std::string>& toks, size_t start,
                         const std::unordered_map<std::string, int>& arity,
                         int line_no) {
    Attrs a;
    size_t i = start;
    while (i < toks.size()) {
        const std::string key = toks[i++];
        const auto it = arity.find(key);
        if (it == arity.end())
            throw std::runtime_error("line " + std::to_string(line_no) + ": unknown attribute: " + key);
        std::vector<std::string> vals;
        for (int k = 0; k < it->second; ++k) {
            if (i >= toks.size())
                throw std::runtime_error("line " + std::to_string(line_no) + ": missing value for " + key);
            vals.push_back(toks[i++]);
        }
        a.kv[key] = std::move(vals);
    }
    return a;
}

inline Vec3 as_vec3(const std::vector<std::string>& v, int line_no) {
    if (v.size() != 3) throw std::runtime_error("line " + std::to_string(line_no) + ": expected 3 numbers");
    return { to_num(v[0], line_no), to_num(v[1], line_no), to_num(v[2], line_no) };
}

inline double as_num(const std::vector<std::string>& v, int line_no) {
    if (v.size() != 1) throw std::runtime_error("line " + std::to_string(line_no) + ": expected 1 number");
    return to_num(v[0], line_no);
}

inline const Material* lookup_material(const Scene& s, const std::string& name, int line_no) {
    const auto it = s.materials_by_name.find(name);
    if (it == s.materials_by_name.end())
        throw std::runtime_error("line " + std::to_string(line_no) + ": undefined material: " + name);
    return it->second;
}

inline std::unique_ptr<Hittable> parse_primitive(const Line& L, const Scene& scene,
                                                 bool allow_material) {
    const auto& t = L.toks;
    const std::string& kw = t[0];
    const int ln = L.line_no;

    auto maybe_material = [&](const Attrs& a) -> const Material* {
        const auto it = a.kv.find("material");
        if (it == a.kv.end()) return nullptr;
        if (!allow_material)
            throw std::runtime_error("line " + std::to_string(ln) + ": material not allowed on csg operand");
        return lookup_material(scene, it->second[0], ln);
    };

    if (kw == "sphere") {
        const std::unordered_map<std::string, int> ar = {{"center", 3}, {"radius", 1}, {"material", 1}};
        const auto a = parse_attrs(t, 1, ar, ln);
        return std::make_unique<Sphere>(as_vec3(a.kv.at("center"), ln),
                                        as_num (a.kv.at("radius"), ln),
                                        maybe_material(a));
    }
    if (kw == "plane") {
        const std::unordered_map<std::string, int> ar = {
            {"point", 3}, {"normal", 3}, {"material", 1}, {"clip", 6}};
        const auto a = parse_attrs(t, 1, ar, ln);
        auto pl = std::make_unique<Plane>(as_vec3(a.kv.at("point"),  ln),
                                          as_vec3(a.kv.at("normal"), ln),
                                          maybe_material(a));
        if (a.kv.count("clip")) {
            const auto& cv = a.kv.at("clip");
            pl->clip_lo = as_vec3({cv[0], cv[1], cv[2]}, ln);
            pl->clip_hi = as_vec3({cv[3], cv[4], cv[5]}, ln);
        }
        return pl;
    }
    if (kw == "box") {
        const std::unordered_map<std::string, int> ar = {{"min", 3}, {"max", 3}, {"material", 1}};
        const auto a = parse_attrs(t, 1, ar, ln);
        return std::make_unique<Box>(as_vec3(a.kv.at("min"), ln),
                                     as_vec3(a.kv.at("max"), ln),
                                     maybe_material(a));
    }
    if (kw == "cylinder") {
        const std::unordered_map<std::string, int> ar = {
            {"base", 3}, {"radius", 1}, {"height", 1}, {"material", 1}};
        const auto a = parse_attrs(t, 1, ar, ln);
        return std::make_unique<Cylinder>(as_vec3(a.kv.at("base"),   ln),
                                          as_num (a.kv.at("radius"), ln),
                                          as_num (a.kv.at("height"), ln),
                                          maybe_material(a));
    }
    throw std::runtime_error("line " + std::to_string(ln) + ": unknown primitive: " + kw);
}

inline std::unique_ptr<Hittable> parse_csg_block(Cursor& cur, const Line& header, Scene& scene) {
    const auto& t = header.toks;
    const int ln = header.line_no;
    if (t.size() < 2) throw std::runtime_error("line " + std::to_string(ln) + ": csg missing op");

    CSGOp op;
    if      (t[1] == "union")        op = CSGOp::Union;
    else if (t[1] == "intersection") op = CSGOp::Intersection;
    else if (t[1] == "difference")   op = CSGOp::Difference;
    else throw std::runtime_error("line " + std::to_string(ln) + ": bad csg op: " + t[1]);

    const std::unordered_map<std::string, int> ar = {{"material", 1}};
    const auto a = parse_attrs(t, 2, ar, ln);
    const Material* mat = nullptr;
    if (const auto it = a.kv.find("material"); it != a.kv.end())
        mat = lookup_material(scene, it->second[0], ln);

    if (!header.opens_block)
        throw std::runtime_error("line " + std::to_string(ln) + ": csg must open a { block");

    std::vector<std::unique_ptr<Hittable>> operands;
    while (!cur.done()) {
        const Line& nxt = cur.peek();
        if (nxt.closes_block) { cur.eat(); break; }
        cur.eat();
        if (nxt.toks[0] == "csg")
            operands.push_back(parse_csg_block(cur, nxt, scene));
        else
            operands.push_back(parse_primitive(nxt, scene, /*allow_material=*/false));
    }
    if (operands.size() != 2)
        throw std::runtime_error("line " + std::to_string(ln) + ": csg needs exactly 2 operands, got "
                                 + std::to_string(operands.size()));
    return std::make_unique<CSGNode>(op, std::move(operands[0]), std::move(operands[1]), mat);
}

} // namespace detail

inline Scene load_scene(const std::string& path) {
    const auto lines = detail::tokenize_file(path);
    detail::Cursor cur{lines};
    Scene scene;

    while (!cur.done()) {
        const detail::Line& L = cur.eat();
        const auto& t = L.toks;
        const int ln = L.line_no;
        const std::string& kw = t[0];

        if (kw == "image") {
            if (t.size() != 3) throw std::runtime_error("line " + std::to_string(ln) + ": image W H");
            scene.W = static_cast<int>(detail::to_num(t[1], ln));
            scene.H = static_cast<int>(detail::to_num(t[2], ln));
        } else if (kw == "camera") {
            const std::unordered_map<std::string, int> ar = {
                {"pos", 3}, {"lookat", 3}, {"up", 3}, {"vfov", 1}};
            const auto a = detail::parse_attrs(t, 1, ar, ln);
            if (a.kv.count("pos"))    scene.camera_pos    = detail::as_vec3(a.kv.at("pos"),    ln);
            if (a.kv.count("lookat")) scene.camera_lookat = detail::as_vec3(a.kv.at("lookat"), ln);
            if (a.kv.count("up"))     scene.camera_up     = detail::as_vec3(a.kv.at("up"),     ln);
            if (a.kv.count("vfov"))   scene.vfov          = detail::as_num (a.kv.at("vfov"),   ln);
        } else if (kw == "background") {
            if (t.size() != 4) throw std::runtime_error("line " + std::to_string(ln) + ": background R G B");
            scene.background = {detail::to_num(t[1], ln), detail::to_num(t[2], ln), detail::to_num(t[3], ln)};
        } else if (kw == "ambient") {
            if (t.size() != 4) throw std::runtime_error("line " + std::to_string(ln) + ": ambient R G B");
            scene.ambient = {detail::to_num(t[1], ln), detail::to_num(t[2], ln), detail::to_num(t[3], ln)};
        } else if (kw == "max_depth") {
            if (t.size() != 2) throw std::runtime_error("line " + std::to_string(ln) + ": max_depth N");
            scene.max_depth = static_cast<int>(detail::to_num(t[1], ln));
        } else if (kw == "material") {
            if (t.size() < 2) throw std::runtime_error("line " + std::to_string(ln) + ": material NAME ...");
            const std::string name = t[1];
            const std::unordered_map<std::string, int> ar = {
                {"albedo", 3}, {"specular", 3}, {"shininess", 1},
                {"reflectivity", 1}, {"ior", 1}, {"emission", 3}};
            const auto a = detail::parse_attrs(t, 2, ar, ln);
            auto m = std::make_unique<Material>();
            if (a.kv.count("albedo"))       m->albedo       = detail::as_vec3(a.kv.at("albedo"),       ln);
            if (a.kv.count("specular"))     m->specular     = detail::as_vec3(a.kv.at("specular"),     ln);
            if (a.kv.count("shininess"))    m->shininess    = detail::as_num (a.kv.at("shininess"),    ln);
            if (a.kv.count("reflectivity")) m->reflectivity = detail::as_num (a.kv.at("reflectivity"), ln);
            if (a.kv.count("ior"))          m->ior          = detail::as_num (a.kv.at("ior"),          ln);
            if (a.kv.count("emission"))     m->emission     = detail::as_vec3(a.kv.at("emission"),     ln);
            scene.materials_by_name[name] = m.get();
            scene.material_storage.push_back(std::move(m));
        } else if (kw == "light") {
            if (t.size() < 2 || t[1] != "point")
                throw std::runtime_error("line " + std::to_string(ln) + ": only point lights supported");
            const std::unordered_map<std::string, int> ar = {{"pos", 3}, {"intensity", 3}};
            const auto a = detail::parse_attrs(t, 2, ar, ln);
            PointLight pl;
            pl.position  = detail::as_vec3(a.kv.at("pos"),       ln);
            pl.intensity = detail::as_vec3(a.kv.at("intensity"), ln);
            scene.lights.push_back(pl);
        } else if (kw == "integrator") {
            if (t.size() < 2) throw std::runtime_error("line " + std::to_string(ln) + ": integrator whitted|path_trace");
            if      (t[1] == "whitted")     scene.integrator = Integrator::Whitted;
            else if (t[1] == "path_trace")  scene.integrator = Integrator::PathTrace;
            else throw std::runtime_error("line " + std::to_string(ln) + ": unknown integrator: " + t[1]);
        } else if (kw == "spp") {
            if (t.size() != 2) throw std::runtime_error("line " + std::to_string(ln) + ": spp N");
            scene.spp = static_cast<int>(detail::to_num(t[1], ln));
        } else if (kw == "area_light") {
            const std::unordered_map<std::string, int> ar = {
                {"corner", 3}, {"u", 3}, {"v", 3}, {"emission", 3}};
            const auto a = detail::parse_attrs(t, 1, ar, ln);
            scene.area_lights.push_back(
                make_area_light(detail::as_vec3(a.kv.at("corner"),   ln),
                                detail::as_vec3(a.kv.at("u"),        ln),
                                detail::as_vec3(a.kv.at("v"),        ln),
                                detail::as_vec3(a.kv.at("emission"), ln)));
        } else if (kw == "csg") {
            scene.world.add(detail::parse_csg_block(cur, L, scene));
        } else if (kw == "sphere" || kw == "plane" || kw == "box" || kw == "cylinder") {
            scene.world.add(detail::parse_primitive(L, scene, /*allow_material=*/true));
        } else {
            throw std::runtime_error("line " + std::to_string(ln) + ": unknown directive: " + kw);
        }
    }

    scene.world.items = build_world_with_bvh(std::move(scene.world.items));
    return scene;
}

} // namespace rt
