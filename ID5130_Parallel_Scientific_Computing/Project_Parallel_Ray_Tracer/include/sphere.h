#pragma once
#include "hittable.h"

#include <cmath>

namespace rt {

class Sphere : public Hittable {
public:
    Vec3 center;
    double radius;
    const Material* material{nullptr};

    Sphere(const Vec3& c, double r, const Material* m = nullptr)
        : center(c), radius(r), material(m) {}

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        const Vec3 oc    = r.origin - center;
        const double a      = dot(r.direction, r.direction);
        const double half_b = dot(oc, r.direction);
        const double c      = dot(oc, oc) - radius * radius;
        const double disc   = half_b * half_b - a * c;
        if (disc < 0) return false;
        const double sq = std::sqrt(disc);

        double t = (-half_b - sq) / a;
        if (t < tmin || t > tmax) {
            t = (-half_b + sq) / a;
            if (t < tmin || t > tmax) return false;
        }
        rec.t = t;
        rec.p = r.at(t);
        set_face_normal(r, (rec.p - center) / radius, rec);
        rec.material = material;
        return true;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        const double d = length(p - center) - radius;
        if (std::abs(d) <= eps) return Classification::On;
        return d < 0.0 ? Classification::Inside : Classification::Outside;
    }

    bool bounds(AABB& out) const override {
        const Vec3 r3{radius, radius, radius};
        out = AABB{ center - r3, center + r3 };
        return true;
    }

    void intervals(const Ray& r, std::vector<Interval>& out) const override {
        const Vec3   oc     = r.origin - center;
        const double a      = dot(r.direction, r.direction);
        const double half_b = dot(oc, r.direction);
        const double c      = dot(oc, oc) - radius * radius;
        const double disc   = half_b * half_b - a * c;
        if (disc < 0.0) return;
        const double sq = std::sqrt(disc);
        const double t0 = (-half_b - sq) / a;
        const double t1 = (-half_b + sq) / a;
        if (t0 >= t1) return;
        Interval iv;
        iv.t_enter = t0;
        iv.t_exit  = t1;
        iv.n_enter = (r.at(t0) - center) / radius;
        iv.n_exit  = (r.at(t1) - center) / radius;
        out.push_back(iv);
    }
};

} // namespace rt
