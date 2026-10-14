#pragma once
#include "hittable.h"

#include <cmath>
#include <limits>

namespace rt {

// Plane as a half-space: normal points to the "outside"; "inside" is the
// half-space behind the plane (dot(p - point, normal) < 0).
class Plane : public Hittable {
public:
    Vec3 point;
    Vec3 normal; // unit
    const Material* material{nullptr};
    // Optional finite extents: hit points outside [clip_lo, clip_hi] are rejected.
    Vec3 clip_lo{-1e30, -1e30, -1e30};
    Vec3 clip_hi{ 1e30,  1e30,  1e30};

    Plane(const Vec3& p, const Vec3& n, const Material* m = nullptr)
        : point(p), normal(normalize(n)), material(m) {}

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        const double denom = dot(r.direction, normal);
        if (std::abs(denom) < 1e-12) return false;
        const double t = dot(point - r.origin, normal) / denom;
        if (t < tmin || t > tmax) return false;
        const Vec3 p = r.at(t);
        if (p.x < clip_lo.x || p.x > clip_hi.x ||
            p.y < clip_lo.y || p.y > clip_hi.y ||
            p.z < clip_lo.z || p.z > clip_hi.z) return false;
        rec.t = t;
        rec.p = p;
        set_face_normal(r, normal, rec);
        rec.material = material;
        return true;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        const double d = dot(p - point, normal);
        if (std::abs(d) <= eps) return Classification::On;
        return d < 0.0 ? Classification::Inside : Classification::Outside;
    }

    void intervals(const Ray& r, std::vector<Interval>& out) const override {
        const double inf = std::numeric_limits<double>::infinity();
        const double denom = dot(r.direction, normal);
        const double f0    = dot(r.origin - point, normal);
        Interval iv;
        if (std::abs(denom) < 1e-12) {
            if (f0 < 0.0) {
                iv.t_enter = -inf; iv.t_exit = +inf;
                iv.n_enter = normal; iv.n_exit = normal;
                out.push_back(iv);
            }
            return;
        }
        const double t_cross = -f0 / denom;
        if (denom > 0.0) {
            iv.t_enter = -inf;    iv.t_exit = t_cross;
        } else {
            iv.t_enter = t_cross; iv.t_exit = +inf;
        }
        iv.n_enter = normal;
        iv.n_exit  = normal;
        out.push_back(iv);
    }
};

} // namespace rt
