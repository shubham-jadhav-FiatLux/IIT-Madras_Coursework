#pragma once
#include "hittable.h"
#include "light.h"
#include "material.h"
#include "ray.h"
#include "vec3.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace rt {

inline constexpr double kPI = 3.141592653589793;

// ---------------------------------------------------------------------------
// PCG32 RNG — seeded per-pixel so output is identical regardless of thread count
// ---------------------------------------------------------------------------
struct RNG {
    uint64_t state{0};
    uint64_t inc{1};

    explicit RNG(uint64_t seed, uint64_t stream = 1) {
        inc   = (stream << 1u) | 1u;
        next_u32();
        state += seed;
        next_u32();
    }

    uint32_t next_u32() {
        const uint64_t old = state;
        state = old * 6364136223846793005ULL + inc;
        const uint32_t xsh = static_cast<uint32_t>(((old >> 18u) ^ old) >> 27u);
        const uint32_t rot = static_cast<uint32_t>(old >> 59u);
        return (xsh >> rot) | (xsh << (uint32_t(-rot) & 31u));
    }

    double next() { return (next_u32() >> 8) * (1.0 / (1u << 24)); }
};

// Unique seed for pixel (px,py), sample index si, in image of width W
inline uint64_t pixel_seed(int px, int py, int W, int si = 0) {
    return static_cast<uint64_t>(py) * static_cast<uint64_t>(W)
         + static_cast<uint64_t>(px)
         + static_cast<uint64_t>(si) * 1000003ULL;
}

// ---------------------------------------------------------------------------
// Sampling helpers
// ---------------------------------------------------------------------------

inline void build_onb(const Vec3& n, Vec3& t, Vec3& b) {
    const Vec3 ref = (std::abs(n.x) < 0.9) ? Vec3{1, 0, 0} : Vec3{0, 1, 0};
    t = normalize(cross(ref, n));
    b = cross(n, t);
}

// Cosine-weighted hemisphere sample about n (Malley's method).
// Implicit pdf = cos(theta)/kPI; Lambertian throughput update is simply *= albedo.
inline Vec3 cosine_sample(const Vec3& n, RNG& rng) {
    const double r1  = rng.next();
    const double r2  = rng.next();
    const double phi = 2.0 * kPI * r1;
    const double sq  = std::sqrt(r2);
    const double lx  = std::cos(phi) * sq;
    const double lz  = std::sin(phi) * sq;
    const double ly  = std::sqrt(std::max(0.0, 1.0 - r2));
    Vec3 t, b;
    build_onb(n, t, b);
    return normalize(t * lx + n * ly + b * lz);
}

// ---------------------------------------------------------------------------
// Path scene
// ---------------------------------------------------------------------------

struct PathScene {
    const Hittable*        world{nullptr};
    std::vector<AreaLight> area_lights;
    Vec3                   background{0, 0, 0};
    int                    max_depth{8};
};

// Power heuristic (β=2): w_i = p_i² / Σ p_j²
inline double power2(double a, double b) {
    return (a * a) / (a * a + b * b);
}

// Solid-angle PDF that the area-light sampling strategy would assign to a BSDF
// ray that arrived at rec.p from direction ray.direction, for MIS.
inline double light_pdf_sa(const std::vector<AreaLight>& lights,
                            const HitRecord& rec, const Ray& ray) {
    if (lights.empty()) return 0.0;
    const double dist2 = length_squared(rec.p - ray.origin);
    double sum = 0.0;
    for (const auto& lt : lights) {
        const double cos_lt = std::abs(dot(lt.normal, ray.direction));
        if (cos_lt < 1e-9) continue;
        sum += dist2 / (lt.area * cos_lt);
    }
    return sum / static_cast<double>(lights.size());
}

// ---------------------------------------------------------------------------
// Iterative path tracer with NEE + MIS (balance heuristic)
// ---------------------------------------------------------------------------

inline Vec3 path_radiance(const Ray& r0, const PathScene& scene, RNG& rng) {
    Vec3   L{0, 0, 0};
    Vec3   beta{1, 1, 1};
    Ray    ray        = r0;
    bool   count_Le   = true;  // include emission (camera ray or after specular)
    double prev_pdf   = 0.0;   // BSDF pdf of the ray that generated current ray

    for (int bounce = 0; bounce <= scene.max_depth; ++bounce) {
        HitRecord rec;
        if (!scene.world->hit(ray, 1e-4, 1e9, rec)) {
            L += beta * scene.background;
            break;
        }
        const Material& m = *rec.material;

        // Emission — MIS-weighted on indirect hits
        const bool emissive = dot(m.emission, m.emission) > 0.0;
        if (emissive && count_Le) {
            double w = 1.0;
            if (bounce > 0 && !scene.area_lights.empty()) {
                const double pdf_lt = light_pdf_sa(scene.area_lights, rec, ray);
                w = (pdf_lt + prev_pdf > 0.0) ? power2(prev_pdf, pdf_lt) : 1.0;
            }
            L += beta * m.emission * w;
        }
        if (emissive) break;

        // NEE — direct sample each area light
        for (const auto& lt : scene.area_lights) {
            const Vec3   lp   = lt.corner + lt.u * rng.next() + lt.v * rng.next();
            const Vec3   to_l = lp - rec.p;
            const double dist = length(to_l);
            if (dist < 1e-6) continue;
            const Vec3 ldir = to_l / dist;

            const double cos_s = dot(rec.n, ldir);   // cosine at shading point
            if (cos_s <= 0.0) continue;

            const double cos_l = -dot(lt.normal, ldir);  // cosine at light face
            if (cos_l <= 0.0) continue;

            if (scene.world->occluded(Ray{rec.p, ldir}, 1e-4, dist - 1e-4)) continue;

            // pdf of light sample in solid-angle: (1/area) * dist^2 / cos_l
            const double pdf_lt   = (dist * dist) / (lt.area * cos_l);
            const double pdf_bsdf = cos_s / kPI;
            const double w_lt     = power2(pdf_lt, pdf_bsdf);  // power heuristic β=2

            // (albedo/kPI) * cos_s / pdf_lt * w_lt * Le
            L += beta * m.albedo * (cos_s / (kPI * pdf_lt)) * w_lt * lt.emission;
        }

        // BSDF sample — cosine-weighted hemisphere (Lambertian)
        const Vec3   wi    = cosine_sample(rec.n, rng);
        const double cos_i = dot(rec.n, wi);
        prev_pdf = cos_i / kPI;

        // Throughput: (albedo/kPI) * cos_i / (cos_i/kPI) = albedo
        beta = beta * m.albedo;

        // Russian roulette — start at bounce 5 (preserve more indirect paths)
        if (bounce >= 5) {
            const double rr = std::min(std::max({beta.x, beta.y, beta.z}), 0.95);
            if (rng.next() >= rr) break;
            beta = beta / rr;
        }

        // count_Le=true is correct here only because the BSDF is Lambertian
        // and MIS (power heuristic) handles double-counting between NEE and the
        // BSDF-hits-light path. If specular/glossy BSDFs are added, set
        // count_Le=false after diffuse bounces (NEE owns direct), true after
        // specular bounces (no NEE was performed).
        count_Le = true;
        ray      = Ray{rec.p, wi};
    }
    return L;
}

// Reinhard tone mapping (per-channel)
inline Vec3 reinhard(const Vec3& c) {
    return {c.x / (1.0 + c.x), c.y / (1.0 + c.y), c.z / (1.0 + c.z)};
}

// ---------------------------------------------------------------------------
// Stratified pixel sampling
// ---------------------------------------------------------------------------
// When spp is a perfect square N², divides the pixel into an N×N grid and
// places sample si in stratum (si/N, si%N), jittered within the cell.
// Otherwise falls back to pure jitter (uniform [0,1)² per sample).
// Deterministic: seeded per (pixel, sample index) regardless of thread count.

inline bool is_perfect_square(int n, int& root) {
    root = static_cast<int>(std::sqrt(static_cast<double>(n)) + 0.5);
    return root * root == n;
}

// Returns (dx, dy) in [0,1)² for sample si of spp total samples at pixel (px,py).
// rng must already be seeded; calling code advances rng for path tracing.
//
// Stratified sampling on an N×N grid when spp = N². Cranley-Patterson rotation
// (per-pixel toroidal shift) is applied so adjacent pixels do not share the
// same sub-pixel pattern. The rotation is identical for all spp samples within
// a given pixel, so the per-pixel stratification property (one sample per
// strata cell) is preserved — only the position within [0,1)² shifts.
inline void stratified_offset(int si, int spp, RNG& rng,
                              int px, int py, int W,
                              double& dx, double& dy) {
    int sqspp;
    if (is_perfect_square(spp, sqspp)) {
        const int sy = si / sqspp;
        const int sx = si % sqspp;
        dx = (sx + rng.next()) / sqspp;
        dy = (sy + rng.next()) / sqspp;
    } else {
        dx = rng.next();
        dy = rng.next();
    }
    // Cranley-Patterson rotation seeded by pixel coordinates.
    // Multiplied by a large odd constant to spread the bits before extraction.
    const uint64_t h = pixel_seed(px, py, W, 0) * 0x9E3779B97F4A7C15ULL;
    const double rx = static_cast<double>((h >>  0) & 0xFFFFFFu) * (1.0 / (1u << 24));
    const double ry = static_cast<double>((h >> 32) & 0xFFFFFFu) * (1.0 / (1u << 24));
    dx += rx; if (dx >= 1.0) dx -= 1.0;
    dy += ry; if (dy >= 1.0) dy -= 1.0;
}

} // namespace rt
