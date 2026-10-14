#pragma once
#include "aabb.h"
#include "ray.h"
#include "vec3.h"

#include <memory>
#include <vector>

namespace rt {

struct Material;

struct HitRecord {
    double t;
    Vec3 p;
    Vec3 n;            // outward-facing normal relative to the ray (flipped if inside)
    bool front_face;   // true if ray hit the outside surface
    const Material* material = nullptr;
};

enum class Classification { Outside, On, Inside };

// A segment along a ray where the ray is inside a solid primitive.
// t_enter/t_exit may be ±infinity for half-spaces.
struct Interval {
    double t_enter;
    double t_exit;
    Vec3   n_enter;  // outward normal of the solid at t_enter
    Vec3   n_exit;   // outward normal of the solid at t_exit
};

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const = 0;
    virtual Classification classify(const Vec3& p, double eps = 1e-6) const = 0;

    // Append "inside" intervals of this solid along ray r. Default: none.
    // Primitives override. Required for CSG participation.
    virtual void intervals(const Ray& r, std::vector<Interval>& out) const {
        (void)r; (void)out;
    }

    // Conservative AABB. Return false if the object is unbounded
    // (e.g. a plane). Default: unbounded.
    virtual bool bounds(AABB& out) const {
        (void)out;
        return false;
    }

    // Any-hit query for shadow rays: does *any* intersection exist in
    // [tmin, tmax]? Aggregates override to early-exit on the first hit.
    virtual bool occluded(const Ray& r, double tmin, double tmax) const {
        HitRecord rec;
        return hit(r, tmin, tmax, rec);
    }
};

inline void set_face_normal(const Ray& r, const Vec3& outward, HitRecord& rec) {
    rec.front_face = dot(r.direction, outward) <= 0;
    rec.n = rec.front_face ? outward : -outward;
}

// Given a CSG-combined interval set, return the nearest boundary in [tmin, tmax].
inline bool hit_from_intervals(const std::vector<Interval>& ivs, const Ray& r,
                               double tmin, double tmax, HitRecord& rec) {
    double best_t = tmax;
    Vec3   best_n;
    bool   found = false;
    for (const auto& iv : ivs) {
        if (iv.t_enter >= tmin && iv.t_enter <= best_t) {
            best_t = iv.t_enter; best_n = iv.n_enter; found = true;
        }
        if (iv.t_exit >= tmin && iv.t_exit <= best_t) {
            best_t = iv.t_exit;  best_n = iv.n_exit;  found = true;
        }
    }
    if (!found) return false;
    rec.t = best_t;
    rec.p = r.at(best_t);
    set_face_normal(r, best_n, rec);
    return true;
}

class HittableList : public Hittable {
public:
    std::vector<std::unique_ptr<Hittable>> items;

    void add(std::unique_ptr<Hittable> h) { items.push_back(std::move(h)); }
    void clear() { items.clear(); }

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        HitRecord temp;
        bool any = false;
        double closest = tmax;
        for (const auto& h : items) {
            if (h->hit(r, tmin, closest, temp)) {
                any = true;
                closest = temp.t;
                rec = temp;
            }
        }
        return any;
    }

    bool occluded(const Ray& r, double tmin, double tmax) const override {
        for (const auto& h : items) {
            if (h->occluded(r, tmin, tmax)) return true;
        }
        return false;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        Classification best = Classification::Outside;
        for (const auto& h : items) {
            switch (h->classify(p, eps)) {
                case Classification::Inside:  return Classification::Inside;
                case Classification::On:      best = Classification::On; break;
                case Classification::Outside: break;
            }
        }
        return best;
    }
};

} // namespace rt
