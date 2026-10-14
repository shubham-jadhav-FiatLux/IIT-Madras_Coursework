#pragma once
#include "hittable.h"
#include "light.h"
#include "material.h"
#include "ray.h"
#include "vec3.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace rt {

inline Vec3 reflect(const Vec3& I, const Vec3& N) {
    return I - N * (2.0 * dot(I, N));
}

// eta = n_incoming / n_transmitted. N is the surface normal on the incoming side.
// Returns false on total internal reflection.
inline bool refract_dir(const Vec3& I, const Vec3& N, double eta, Vec3& T) {
    const double cos_i = -dot(I, N);
    const double sin2_t = eta * eta * std::max(0.0, 1.0 - cos_i * cos_i);
    if (sin2_t > 1.0) return false;
    const double cos_t = std::sqrt(1.0 - sin2_t);
    T = I * eta + N * (eta * cos_i - cos_t);
    return true;
}

inline double schlick(double cos_theta, double ior) {
    double r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * std::pow(1.0 - cos_theta, 5);
}

inline Vec3 direct_lighting(const HitRecord& rec, const Vec3& view_dir,
                            const Hittable& world,
                            const std::vector<PointLight>& lights,
                            const Vec3& ambient) {
    const Material& m = *rec.material;
    Vec3 L = m.albedo * ambient;
    for (const auto& light : lights) {
        const Vec3 to_light = light.position - rec.p;
        const double dist = length(to_light);
        if (dist < 1e-6) continue;
        const Vec3 Ldir = to_light / dist;
        if (world.occluded(Ray{rec.p, Ldir}, 1e-4, dist - 1e-4)) continue;
        const double ndotl = std::max(0.0, dot(rec.n, Ldir));
        L += (m.albedo * light.intensity) * ndotl;
        if (m.shininess > 0.0 && ndotl > 0.0) {
            const Vec3 H = normalize(Ldir + view_dir);
            const double ndoth = std::max(0.0, dot(rec.n, H));
            L += (m.specular * light.intensity) * std::pow(ndoth, m.shininess);
        }
    }
    return L;
}

struct WhittedScene {
    const Hittable* world{nullptr};
    std::vector<PointLight> lights;
    Vec3 ambient{0.05, 0.05, 0.05};
    Vec3 background{0.6, 0.75, 0.95};
    int  max_depth{10};
};

// Estimates the ray-tree cost for a pixel without computing any shading.
// Returns a dimensionless integer: 0 = background miss, higher = more
// recursive work. Used by the adaptive preview pass to build a tile cost map
// for LPT scheduling and NUMA warm-up (first-touch of BVH nodes).
inline int whitted_cost(const Ray& r, const WhittedScene& scene, int depth) {
    if (depth <= 0) return 0;
    HitRecord rec;
    if (!scene.world->hit(r, 1e-4, 1e9, rec)) return 0;
    if (!rec.material) return 1;
    const Material& m = *rec.material;
    const int shadow_cost = static_cast<int>(scene.lights.size());
    if (m.ior > 0.0) {
        const Vec3 R = reflect(r.direction, rec.n);
        const int refl = whitted_cost(Ray{rec.p, R}, scene, depth - 1);
        return 1 + shadow_cost + 2 * refl; // approximate refract ≈ reflect cost
    }
    if (m.reflectivity > 0.0) {
        const Vec3 R = reflect(r.direction, rec.n);
        return 1 + shadow_cost + whitted_cost(Ray{rec.p, R}, scene, depth - 1);
    }
    return 1 + shadow_cost;
}

inline Vec3 whitted_radiance(const Ray& r, const WhittedScene& scene, int depth) {
    if (depth <= 0) return Vec3{0, 0, 0};

    HitRecord rec;
    if (!scene.world->hit(r, 1e-4, 1e9, rec)) return scene.background;
    if (rec.material == nullptr) return Vec3{1, 0, 1};  // magenta = unassigned

    const Material& m = *rec.material;
    const Vec3 view = -r.direction;

    // Dielectric (glass): Fresnel-weighted reflect + refract, no direct lighting.
    if (m.ior > 0.0) {
        const double cos_theta = std::min(1.0, -dot(r.direction, rec.n));
        const double eta = rec.front_face ? (1.0 / m.ior) : m.ior;
        Vec3 T;
        const bool transmits = refract_dir(r.direction, rec.n, eta, T);
        const double fr = transmits ? schlick(cos_theta, m.ior) : 1.0;
        const Vec3 R = reflect(r.direction, rec.n);

        Vec3 color = whitted_radiance(Ray{rec.p, R}, scene, depth - 1) * fr;
        if (transmits) {
            color += whitted_radiance(Ray{rec.p, T}, scene, depth - 1) * (1.0 - fr);
        }
        return color;
    }

    Vec3 direct = direct_lighting(rec, view, *scene.world, scene.lights, scene.ambient);

    if (m.reflectivity > 0.0) {
        const Vec3 R = reflect(r.direction, rec.n);
        const Vec3 refl = whitted_radiance(Ray{rec.p, R}, scene, depth - 1);
        direct = direct * (1.0 - m.reflectivity) + refl * m.reflectivity;
    }
    return direct;
}

} // namespace rt
