#pragma once
#include "hittable.h"

#include <cmath>
#include <limits>

namespace rt {

// Finite capped cylinder aligned with +y axis.
// base = center of bottom cap; extends to base + (0, height, 0).
class Cylinder : public Hittable {
public:
    Vec3 base;
    double radius;
    double height;
    const Material* material{nullptr};

    Cylinder(const Vec3& b, double r, double h, const Material* m = nullptr)
        : base(b), radius(r), height(h), material(m) {}

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        const double ox = r.origin.x - base.x;
        const double oz = r.origin.z - base.z;
        const double dx = r.direction.x;
        const double dz = r.direction.z;

        double best_t = tmax;
        Vec3   best_outward;
        bool   found = false;

        const double a      = dx * dx + dz * dz;
        if (a > 1e-12) {
            const double half_b = ox * dx + oz * dz;
            const double c      = ox * ox + oz * oz - radius * radius;
            const double disc   = half_b * half_b - a * c;
            if (disc >= 0.0) {
                const double sq = std::sqrt(disc);
                const double roots[2] = { (-half_b - sq) / a, (-half_b + sq) / a };
                for (double t : roots) {
                    if (t < tmin || t > best_t) continue;
                    const double y_local = r.origin.y + t * r.direction.y - base.y;
                    if (y_local < 0.0 || y_local > height) continue;
                    const Vec3 p = r.at(t);
                    best_t = t;
                    best_outward = Vec3{(p.x - base.x) / radius, 0.0, (p.z - base.z) / radius};
                    found = true;
                }
            }
        }

        if (std::abs(r.direction.y) > 1e-12) {
            const double y_caps[2] = { base.y, base.y + height };
            const double signs[2]  = { -1.0, 1.0 };
            for (int i = 0; i < 2; ++i) {
                const double t = (y_caps[i] - r.origin.y) / r.direction.y;
                if (t < tmin || t > best_t) continue;
                const double px = r.origin.x + t * r.direction.x - base.x;
                const double pz = r.origin.z + t * r.direction.z - base.z;
                if (px * px + pz * pz <= radius * radius) {
                    best_t = t;
                    best_outward = Vec3{0.0, signs[i], 0.0};
                    found = true;
                }
            }
        }

        if (!found) return false;
        rec.t = best_t;
        rec.p = r.at(best_t);
        set_face_normal(r, best_outward, rec);
        rec.material = material;
        return true;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        const double dx = p.x - base.x;
        const double dz = p.z - base.z;
        const double r_p = std::sqrt(dx * dx + dz * dz);
        const double y   = p.y - base.y;

        const double d_radial = r_p - radius;    // >0 outside tube
        const double d_bottom = -y;               // >0 below base
        const double d_top    = y - height;       // >0 above top

        if (d_radial > eps || d_bottom > eps || d_top > eps) {
            return Classification::Outside;
        }
        if (std::abs(d_radial) <= eps ||
            std::abs(d_bottom) <= eps ||
            std::abs(d_top)    <= eps) {
            return Classification::On;
        }
        return Classification::Inside;
    }

    bool bounds(AABB& out) const override {
        const Vec3 lo{ base.x - radius, base.y,          base.z - radius };
        const Vec3 hi{ base.x + radius, base.y + height, base.z + radius };
        out = AABB{ lo, hi };
        return true;
    }

    void intervals(const Ray& r, std::vector<Interval>& out) const override {
        const double inf = std::numeric_limits<double>::infinity();

        // Side: infinite cylinder along y. Inside when r_p <= radius.
        const double ox = r.origin.x - base.x;
        const double oz = r.origin.z - base.z;
        const double dx = r.direction.x;
        const double dz = r.direction.z;
        const double a      = dx * dx + dz * dz;
        const double half_b = ox * dx + oz * dz;
        const double c      = ox * ox + oz * oz - radius * radius;

        double t_s_enter, t_s_exit;
        Vec3   n_s_enter, n_s_exit;
        bool   side_bounded;
        if (a < 1e-30) {
            if (c > 0.0) return;         // ray outside tube, parallel to axis
            t_s_enter = -inf; t_s_exit = +inf;
            side_bounded = false;
        } else {
            const double disc = half_b * half_b - a * c;
            if (disc < 0.0) return;
            const double sq = std::sqrt(disc);
            t_s_enter = (-half_b - sq) / a;
            t_s_exit  = (-half_b + sq) / a;
            const Vec3 p0 = r.at(t_s_enter);
            const Vec3 p1 = r.at(t_s_exit);
            n_s_enter = Vec3{(p0.x - base.x) / radius, 0.0, (p0.z - base.z) / radius};
            n_s_exit  = Vec3{(p1.x - base.x) / radius, 0.0, (p1.z - base.z) / radius};
            side_bounded = true;
        }

        // Slab: 0 <= y_local <= height.
        const double dy = r.direction.y;
        const double y0 = r.origin.y - base.y;
        double t_y_enter, t_y_exit;
        Vec3   n_y_enter{0,-1,0}, n_y_exit{0,1,0};
        if (std::abs(dy) < 1e-30) {
            if (y0 < 0.0 || y0 > height) return;
            t_y_enter = -inf; t_y_exit = +inf;
        } else {
            double t0 = (0.0    - y0) / dy;
            double t1 = (height - y0) / dy;
            Vec3   n0{0,-1,0}, n1{0,1,0};
            if (t0 > t1) { std::swap(t0, t1); std::swap(n0, n1); }
            t_y_enter = t0; t_y_exit = t1;
            n_y_enter = n0; n_y_exit = n1;
        }

        // Intersect side and slab.
        double t_enter; Vec3 n_enter;
        double t_exit;  Vec3 n_exit;
        if (t_s_enter > t_y_enter) { t_enter = t_s_enter; n_enter = n_s_enter; }
        else                        { t_enter = t_y_enter; n_enter = n_y_enter; }
        if (t_s_exit  < t_y_exit)  { t_exit  = t_s_exit;  n_exit  = n_s_exit; }
        else                        { t_exit  = t_y_exit;  n_exit  = n_y_exit; }
        if (t_enter >= t_exit) return;
        (void)side_bounded;

        Interval iv;
        iv.t_enter = t_enter; iv.t_exit = t_exit;
        iv.n_enter = n_enter; iv.n_exit = n_exit;
        out.push_back(iv);
    }
};

} // namespace rt
