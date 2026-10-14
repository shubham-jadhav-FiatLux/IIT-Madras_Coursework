#pragma once
#include <cmath>

namespace rt {

struct Vec3 {
    double x, y, z;

    constexpr Vec3() : x(0), y(0), z(0) {}
    constexpr Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    constexpr Vec3 operator+(const Vec3& b) const { return {x+b.x, y+b.y, z+b.z}; }
    constexpr Vec3 operator-(const Vec3& b) const { return {x-b.x, y-b.y, z-b.z}; }
    constexpr Vec3 operator*(const Vec3& b) const { return {x*b.x, y*b.y, z*b.z}; }
    constexpr Vec3 operator*(double s)      const { return {x*s, y*s, z*s}; }
    constexpr Vec3 operator/(double s)      const { return {x/s, y/s, z/s}; }
    constexpr Vec3 operator-()              const { return {-x, -y, -z}; }

    Vec3& operator+=(const Vec3& b) { x+=b.x; y+=b.y; z+=b.z; return *this; }
    Vec3& operator-=(const Vec3& b) { x-=b.x; y-=b.y; z-=b.z; return *this; }
    Vec3& operator*=(double s)      { x*=s; y*=s; z*=s;       return *this; }

    double  operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
    double& operator[](int i)       { return i == 0 ? x : (i == 1 ? y : z); }
};

constexpr Vec3 operator*(double s, const Vec3& v) { return v * s; }

constexpr double dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

constexpr Vec3 cross(const Vec3& a, const Vec3& b) {
    return { a.y*b.z - a.z*b.y,
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}

inline double length_squared(const Vec3& v) { return dot(v, v); }
inline double length(const Vec3& v)         { return std::sqrt(length_squared(v)); }
inline Vec3   normalize(const Vec3& v)      { return v / length(v); }

} // namespace rt
