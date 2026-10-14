#pragma once
#include "vec3.h"

namespace rt {

struct Ray {
    Vec3 origin;
    Vec3 direction;
    Vec3 inv_dir;

    Ray(const Vec3& o, const Vec3& d)
        : origin(o),
          direction(d),
          inv_dir{1.0 / d.x, 1.0 / d.y, 1.0 / d.z} {}

    Vec3 at(double t) const { return origin + direction * t; }
};

} // namespace rt
