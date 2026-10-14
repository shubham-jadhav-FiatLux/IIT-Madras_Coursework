#pragma once
#include "vec3.h"
#include <cmath>

namespace rt {

struct PointLight {
    Vec3 position;
    Vec3 intensity;  // radiant intensity (pre-multiplied color * power)
};

// Parallelogram area light: points p = corner + s*u + t*v, s,t∈[0,1].
struct AreaLight {
    Vec3   corner;
    Vec3   u, v;      // edge vectors
    Vec3   emission;  // spectral radiance Le
    double area;      // |cross(u,v)|, precomputed
    Vec3   normal;    // normalize(cross(u,v)), pointing into the scene
};

inline AreaLight make_area_light(const Vec3& corner, const Vec3& u, const Vec3& v,
                                  const Vec3& emission) {
    const Vec3   cx   = cross(u, v);
    const double area = std::sqrt(dot(cx, cx));
    return {corner, u, v, emission, area, cx / area};
}

} // namespace rt
