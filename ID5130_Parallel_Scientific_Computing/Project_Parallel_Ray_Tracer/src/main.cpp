#include "camera.h"
#include "path.h"
#include "ppm.h"
#include "ray.h"
#include "scene.h"
#include "vec3.h"
#include "whitted.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>

namespace {

uint8_t to_byte(double c) {
    const double g = std::pow(std::clamp(c, 0.0, 1.0), 1.0 / 2.2);
    return static_cast<uint8_t>(g * 255.0 + 0.5);
}

std::string derive_output_path(const std::string& scene_path) {
    const auto slash = scene_path.find_last_of('/');
    const auto stem_start = (slash == std::string::npos) ? 0 : slash + 1;
    const auto dot = scene_path.find_last_of('.');
    const auto stem_end = (dot == std::string::npos || dot < stem_start) ? scene_path.size() : dot;
    return "output/" + scene_path.substr(stem_start, stem_end - stem_start) + ".ppm";
}

// ---------------------------------------------------------------------------
// Tile infrastructure — shared between Whitted and Path Trace
// ---------------------------------------------------------------------------

struct Tile {
    int    x0, y0, x1, y1;
    double cost{0.0};
};

std::vector<Tile> make_tiles(int W, int H, int tw = 32, int th = 32) {
    std::vector<Tile> tiles;
    tiles.reserve(((W + tw - 1) / tw) * ((H + th - 1) / th));
    for (int y0 = 0; y0 < H; y0 += th)
        for (int x0 = 0; x0 < W; x0 += tw)
            tiles.push_back({x0, y0, std::min(x0 + tw, W), std::min(y0 + th, H)});
    return tiles;
}

// ---------------------------------------------------------------------------
// Pixel buffer NUMA first-touch
// ---------------------------------------------------------------------------
// Parallel zero-init using schedule(static) so the OS first-touch policy
// places each tile's output bytes on the rendering thread's local NUMA node.
// On 2-socket Xeon systems this avoids cross-NUMA writes during the render
// loop. Cost is one memory-bandwidth-bound pass over the buffer (~10ms for
// 1024² × 3 bytes), negligible vs render time.
void parallel_init_pixels(std::vector<uint8_t>& pixels,
                          const std::vector<Tile>& tiles, int W) {
    const int ntiles = static_cast<int>(tiles.size());
    #pragma omp parallel for schedule(static)
    for (int ti = 0; ti < ntiles; ++ti) {
        const auto& t = tiles[ti];
        for (int y = t.y0; y < t.y1; ++y)
            for (int x = t.x0; x < t.x1; ++x) {
                const size_t i = (static_cast<size_t>(y) * W + x) * 3;
                pixels[i + 0] = 0;
                pixels[i + 1] = 0;
                pixels[i + 2] = 0;
            }
    }
}

// ---------------------------------------------------------------------------
// Adaptive preview pass — assigns a per-tile cost for LPT scheduling
// ---------------------------------------------------------------------------
// Whitted preview: skeleton ray-tree traversal cost via whitted_cost().
// PathTrace preview: render preview_spp samples, compute color variance per tile.
// Variance correlates with convergence difficulty: shadow regions, caustic
// areas, CSG silhouettes need more samples to resolve.
//
// Both preview variants use schedule(static) so that BVH pages first-touched
// during the preview land on the local NUMA node of each thread — providing
// implicit NUMA locality during the main render too (when threads stay on
// the same cores via OMP_PROC_BIND=close).

void preview_whitted_cost(std::vector<Tile>& tiles,
                          const rt::Camera& cam, const rt::WhittedScene& ws,
                          int W, int H) {
    const int ntiles = static_cast<int>(tiles.size());
    #pragma omp parallel for schedule(static)
    for (int ti = 0; ti < ntiles; ++ti) {
        auto& t = tiles[ti];
        double cost = 0.0;
        for (int y = t.y0; y < t.y1; y += 4)
            for (int x = t.x0; x < t.x1; x += 4)
                cost += static_cast<double>(
                    rt::whitted_cost(cam.ray(x, y, W, H), ws, ws.max_depth));
        t.cost = cost;
    }
}

// Variance-weighted PT preview. Renders preview_spp samples per pixel,
// computes per-tile RGB variance. Cost = (variance + epsilon) so empty
// (low-variance) tiles still get some budget for completeness.
void preview_pt_variance(std::vector<Tile>& tiles,
                         const rt::Camera& cam, const rt::PathScene& ps,
                         int W, int H, int preview_spp = 4) {
    const int ntiles = static_cast<int>(tiles.size());
    #pragma omp parallel for schedule(static)
    for (int ti = 0; ti < ntiles; ++ti) {
        auto& t = tiles[ti];
        // Single-pass mean+variance via Welford's online algorithm on luminance.
        double mean = 0.0, M2 = 0.0;
        long long n = 0;
        for (int y = t.y0; y < t.y1; ++y)
            for (int x = t.x0; x < t.x1; ++x) {
                rt::Vec3 accum{0, 0, 0};
                for (int si = 0; si < preview_spp; ++si) {
                    rt::RNG rng(rt::pixel_seed(x, y, W, si));
                    double dx, dy;
                    rt::stratified_offset(si, preview_spp, rng, x, y, W, dx, dy);
                    accum += rt::path_radiance(cam.ray(x + dx, y + dy, W, H), ps, rng);
                }
                const rt::Vec3 c = accum * (1.0 / preview_spp);
                const double L = 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z;
                ++n;
                const double delta = L - mean;
                mean += delta / static_cast<double>(n);
                M2   += delta * (L - mean);
            }
        const double variance = (n > 1) ? (M2 / static_cast<double>(n - 1)) : 0.0;
        // Floor with small constant so zero-variance tiles still get ranked.
        t.cost = variance + 1e-3;
    }
}

void sort_lpt(std::vector<Tile>& tiles) {
    std::sort(tiles.begin(), tiles.end(),
              [](const Tile& a, const Tile& b){ return a.cost > b.cost; });
}

} // namespace

int main(int argc, char** argv) {
    const std::string scene_path = (argc > 1) ? argv[1] : "scenes/whitted_showcase.scene";
    const std::string out_path   = (argc > 2) ? argv[2] : derive_output_path(scene_path);
    const bool adaptive = (argc > 3 && std::string(argv[3]) == "--adaptive");

    try {
        rt::Scene s = rt::load_scene(scene_path);

        const int W = s.W;
        const int H = s.H;
        const double aspect = static_cast<double>(W) / H;
        const bool is_pt = (s.integrator == rt::Integrator::PathTrace);

        rt::Camera cam(s.camera_pos, s.camera_lookat, s.camera_up, s.vfov, aspect);

        rt::WhittedScene ws;
        ws.world      = &s.world;
        ws.lights     = s.lights;
        ws.ambient    = s.ambient;
        ws.background = s.background;
        ws.max_depth  = s.max_depth;

        rt::PathScene ps;
        ps.world       = &s.world;
        ps.area_lights = s.area_lights;
        ps.background  = s.background;
        ps.max_depth   = s.max_depth;
        const int spp = s.spp;

        std::vector<uint8_t> pixels(static_cast<size_t>(W) * H * 3);
        const int nthreads = omp_get_max_threads();

        // Build tile list. Two roles:
        //   1. Provides the unit-of-work for the parallel render loop.
        //   2. Drives parallel pixel-buffer first-touch (NUMA locality).
        auto tiles = make_tiles(W, H);

        // First-touch the pixel buffer so each tile's output bytes are placed
        // on the local NUMA node of the thread that will render that tile.
        // Uses schedule(static) — same as the preview pass — so the mapping
        // is consistent when threads are bound (OMP_PROC_BIND=close).
        parallel_init_pixels(pixels, tiles, W);

        const auto t0 = std::chrono::steady_clock::now();

        if (adaptive) {
            // Preview pass: cost-estimate each tile, then sort descending so
            // the dynamic-schedule render loop pulls heaviest tiles first
            // (LPT — Longest Processing Time first — load balancing).
            if (is_pt) preview_pt_variance(tiles, cam, ps, W, H);
            else       preview_whitted_cost(tiles, cam, ws, W, H);
            sort_lpt(tiles);
        }

        const int ntiles = static_cast<int>(tiles.size());
        #pragma omp parallel for schedule(dynamic, 1)
        for (int ti = 0; ti < ntiles; ++ti) {
            const auto& t = tiles[ti];
            for (int y = t.y0; y < t.y1; ++y)
                for (int x = t.x0; x < t.x1; ++x) {
                    rt::Vec3 c;
                    if (is_pt) {
                        rt::Vec3 accum{0, 0, 0};
                        for (int si = 0; si < spp; ++si) {
                            rt::RNG rng(rt::pixel_seed(x, y, W, si));
                            double dx, dy;
                            rt::stratified_offset(si, spp, rng, x, y, W, dx, dy);
                            accum += rt::path_radiance(cam.ray(x + dx, y + dy, W, H), ps, rng);
                        }
                        c = rt::reinhard(accum * (1.0 / spp));
                    } else {
                        c = rt::whitted_radiance(cam.ray(x, y, W, H), ws, ws.max_depth);
                    }
                    const size_t i = (static_cast<size_t>(y) * W + x) * 3;
                    pixels[i + 0] = to_byte(c.x);
                    pixels[i + 1] = to_byte(c.y);
                    pixels[i + 2] = to_byte(c.z);
                }
        }

        const auto t1 = std::chrono::steady_clock::now();
        const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (!rt::write_ppm(out_path, W, H, pixels)) {
            std::cerr << "failed to write " << out_path << "\n";
            return 1;
        }
        std::cout << "wrote " << out_path << " (" << W << "x" << H << ") "
                  << "threads " << nthreads << " "
                  << (is_pt ? "path_trace spp " + std::to_string(spp)
                            : std::string("whitted"))
                  << (adaptive ? " adaptive" : " standard")
                  << " render " << ms << " ms\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
