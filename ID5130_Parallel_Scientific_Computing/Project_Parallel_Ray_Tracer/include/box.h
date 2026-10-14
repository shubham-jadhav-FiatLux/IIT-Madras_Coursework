#pragma once
#include "hittable.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rt {

// Axis-aligned bounding box (slab method).
class Box : public Hittable {
public:
    Vec3 pmin, pmax;
    const Material* material{nullptr};

    Box(const Vec3& lo, const Vec3& hi, const Material* m = nullptr)
        : pmin(lo), pmax(hi), material(m) {}

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        double t_enter = -std::numeric_limits<double>::infinity();
        double t_exit  =  std::numeric_limits<double>::infinity();
        int    enter_axis = 0, exit_axis = 0;
        double enter_sign = 0.0, exit_sign = 0.0;

        for (int a = 0; a < 3; ++a) {
            if (std::abs(r.direction[a]) < 1e-30) {
                if (r.origin[a] < pmin[a] || r.origin[a] > pmax[a]) return false;
                continue;
            }
            const double inv_d = r.inv_dir[a];
            double t0 = (pmin[a] - r.origin[a]) * inv_d;
            double t1 = (pmax[a] - r.origin[a]) * inv_d;
            double s0 = -1.0, s1 = 1.0;
            if (inv_d < 0.0) { std::swap(t0, t1); std::swap(s0, s1); }
            if (t0 > t_enter) { t_enter = t0; enter_axis = a; enter_sign = s0; }
            if (t1 < t_exit)  { t_exit  = t1; exit_axis  = a; exit_sign  = s1; }
            if (t_enter > t_exit) return false;
        }

        double t;
        int    axis;
        double sign;
        if (t_enter >= tmin && t_enter <= tmax) {
            t = t_enter; axis = enter_axis; sign = enter_sign;
        } else if (t_exit >= tmin && t_exit <= tmax) {
            t = t_exit;  axis = exit_axis;  sign = exit_sign;
        } else {
            return false;
        }

        rec.t = t;
        rec.p = r.at(t);
        Vec3 outward{0, 0, 0};
        outward[axis] = sign;
        set_face_normal(r, outward, rec);
        rec.material = material;
        return true;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        Vec3 q{ std::max(pmin.x, std::min(p.x, pmax.x)),
                std::max(pmin.y, std::min(p.y, pmax.y)),
                std::max(pmin.z, std::min(p.z, pmax.z)) };
        const double d_out = length(p - q);
        if (d_out > eps) return Classification::Outside;

        double d_in = std::numeric_limits<double>::infinity();
        for (int a = 0; a < 3; ++a) {
            d_in = std::min({d_in, p[a] - pmin[a], pmax[a] - p[a]});
        }
        if (d_in <= eps) return Classification::On;
        return Classification::Inside;
    }

    bool bounds(AABB& out) const override {
        out = AABB{ pmin, pmax };
        return true;
    }

    void intervals(const Ray& r, std::vector<Interval>& out) const override {
        double t_enter = -std::numeric_limits<double>::infinity();
        double t_exit  =  std::numeric_limits<double>::infinity();
        int    enter_axis = 0, exit_axis = 0;
        double enter_sign = 0.0, exit_sign = 0.0;

        for (int a = 0; a < 3; ++a) {
            if (std::abs(r.direction[a]) < 1e-30) {
                if (r.origin[a] < pmin[a] || r.origin[a] > pmax[a]) return;
                continue;
            }
            const double inv_d = r.inv_dir[a];
            double t0 = (pmin[a] - r.origin[a]) * inv_d;
            double t1 = (pmax[a] - r.origin[a]) * inv_d;
            double s0 = -1.0, s1 = 1.0;
            if (inv_d < 0.0) { std::swap(t0, t1); std::swap(s0, s1); }
            if (t0 > t_enter) { t_enter = t0; enter_axis = a; enter_sign = s0; }
            if (t1 < t_exit)  { t_exit  = t1; exit_axis  = a; exit_sign  = s1; }
            if (t_enter > t_exit) return;
        }
        Interval iv;
        iv.t_enter = t_enter;
        iv.t_exit  = t_exit;
        Vec3 n_e{0,0,0}; n_e[enter_axis] = enter_sign; iv.n_enter = n_e;
        Vec3 n_x{0,0,0}; n_x[exit_axis]  = exit_sign;  iv.n_exit  = n_x;
        out.push_back(iv);
    }
};

} // namespace rt
