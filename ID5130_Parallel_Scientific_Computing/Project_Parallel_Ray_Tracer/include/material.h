#pragma once
#include "vec3.h"

namespace rt {

struct Material {
    Vec3   albedo{0.8, 0.8, 0.8};  // diffuse color
    Vec3   specular{0.0, 0.0, 0.0};
    double shininess{0.0};          // Blinn-Phong exponent
    double reflectivity{0.0};       // [0,1] blend with mirror reflection
    double ior{0.0};                // 0 = opaque; >0 = dielectric (glass)
    Vec3   emission{0.0, 0.0, 0.0}; // Le for area lights / emissive surfaces
};

} // namespace rt
