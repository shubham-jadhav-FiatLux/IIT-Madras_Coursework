#pragma once
#include "ray.h"
#include "vec3.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rt {

struct AABB {
    Vec3 pmin;
    Vec3 pmax;

    static AABB empty() {
        const double inf = std::numeric_limits<double>::infinity();
        return { Vec3{ inf,  inf,  inf}, Vec3{-inf, -inf, -inf} };
    }

    static AABB from_point(const Vec3& p) { return { p, p }; }

    Vec3 centroid() const {
        return Vec3{ 0.5 * (pmin.x + pmax.x),
                     0.5 * (pmin.y + pmax.y),
                     0.5 * (pmin.z + pmax.z) };
    }

    int longest_axis() const {
        const double dx = pmax.x - pmin.x;
        const double dy = pmax.y - pmin.y;
        const double dz = pmax.z - pmin.z;
        if (dx >= dy && dx >= dz) return 0;
        return (dy >= dz) ? 1 : 2;
    }

    void enclose(const AABB& o) {
        pmin = Vec3{ std::min(pmin.x, o.pmin.x),
                     std::min(pmin.y, o.pmin.y),
                     std::min(pmin.z, o.pmin.z) };
        pmax = Vec3{ std::max(pmax.x, o.pmax.x),
                     std::max(pmax.y, o.pmax.y),
                     std::max(pmax.z, o.pmax.z) };
    }

    void enclose_point(const Vec3& p) {
        pmin = Vec3{ std::min(pmin.x, p.x), std::min(pmin.y, p.y), std::min(pmin.z, p.z) };
        pmax = Vec3{ std::max(pmax.x, p.x), std::max(pmax.y, p.y), std::max(pmax.z, p.z) };
    }

    // Slab test. Returns true if [tmin, tmax] overlaps the ray's box-entry interval.
    // Uses the ray's cached inv_dir; guards direction[a] near zero so inf
    // never reaches the multiplication.
    bool hit(const Ray& r, double tmin, double tmax) const {
        for (int a = 0; a < 3; ++a) {
            if (std::abs(r.direction[a]) < 1e-30) {
                if (r.origin[a] < pmin[a] || r.origin[a] > pmax[a]) return false;
                continue;
            }
            const double inv_d = r.inv_dir[a];
            double t0 = (pmin[a] - r.origin[a]) * inv_d;
            double t1 = (pmax[a] - r.origin[a]) * inv_d;
            if (inv_d < 0.0) std::swap(t0, t1);
            if (t0 > tmin) tmin = t0;
            if (t1 < tmax) tmax = t1;
            if (tmax < tmin) return false;
        }
        return true;
    }
};

inline AABB enclose(const AABB& a, const AABB& b) {
    AABB r = a;
    r.enclose(b);
    return r;
}

inline double surface_area(const AABB& b) {
    const Vec3 d = b.pmax - b.pmin;
    return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
}

} // namespace rt
