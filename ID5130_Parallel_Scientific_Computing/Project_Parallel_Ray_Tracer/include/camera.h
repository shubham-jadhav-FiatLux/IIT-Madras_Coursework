#pragma once
#include "ray.h"
#include "vec3.h"
#include <cmath>

namespace rt {

inline constexpr double PI = 3.141592653589793;

class Camera {
public:
    Camera(const Vec3& eye, const Vec3& look_at, const Vec3& up_hint,
           double vfov_degrees, double aspect)
        : eye_(eye) {
        const double vfov  = vfov_degrees * PI / 180.0;
        const double half_h = std::tan(vfov * 0.5);
        const double half_w = aspect * half_h;
        const Vec3 forward = normalize(look_at - eye);
        const Vec3 right   = normalize(cross(forward, up_hint));
        const Vec3 up      = cross(right, forward);
        lower_left_ = forward - right * half_w - up * half_h;
        horizontal_ = right * (2.0 * half_w);
        vertical_   = up    * (2.0 * half_h);
    }

    Ray ray(int px, int py, int W, int H) const {
        const double u = (px + 0.5) / W;
        const double v = 1.0 - (py + 0.5) / H;
        const Vec3 dir = normalize(lower_left_ + horizontal_ * u + vertical_ * v);
        return Ray{eye_, dir};
    }

    // Sub-pixel ray: cx,cy are continuous pixel coords (e.g. px+jitter)
    Ray ray(double cx, double cy, int W, int H) const {
        const double u = cx / W;
        const double v = 1.0 - cy / H;
        const Vec3 dir = normalize(lower_left_ + horizontal_ * u + vertical_ * v);
        return Ray{eye_, dir};
    }

private:
    Vec3 eye_;
    Vec3 lower_left_;
    Vec3 horizontal_;
    Vec3 vertical_;
};

} // namespace rt
