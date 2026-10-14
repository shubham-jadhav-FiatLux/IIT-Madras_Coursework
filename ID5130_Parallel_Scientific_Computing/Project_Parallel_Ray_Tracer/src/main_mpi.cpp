#include "camera.h"
#include "path.h"
#include "ppm.h"
#include "ray.h"
#include "scene.h"
#include "vec3.h"
#include "whitted.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <mpi.h>
#include <omp.h>

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
// Tile infrastructure
// ---------------------------------------------------------------------------

struct Tile {
    int x0, y0, x1, y1;
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

// LPT (Longest Processing Time) assignment: tiles sorted desc by cost,
// greedily assigned to the least-loaded rank.
std::vector<std::vector<int>> lpt_assign(const std::vector<Tile>& tiles, int nranks) {
    std::vector<std::vector<int>> asgn(nranks);
    std::vector<double> load(nranks, 0.0);
    for (int ti = 0; ti < static_cast<int>(tiles.size()); ++ti) {
        const int r = static_cast<int>(
            std::min_element(load.begin(), load.end()) - load.begin());
        asgn[r].push_back(ti);
        load[r] += tiles[ti].cost;
    }
    return asgn;
}

// Row-strip assignment (uniform, no preview needed).
std::vector<std::vector<int>> strip_assign(const std::vector<Tile>& tiles,
                                            int /*W*/, int H, int nranks) {
    std::vector<std::vector<int>> asgn(nranks);
    for (int ti = 0; ti < static_cast<int>(tiles.size()); ++ti) {
        const int r = std::min((tiles[ti].y0 * nranks) / H, nranks - 1);
        asgn[r].push_back(ti);
    }
    return asgn;
}

// Whitted preview cost: skeleton ray-tree traversal count, no shading.
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

// PT preview: render preview_spp samples per pixel, compute per-tile luminance
// variance (Welford's online algorithm). Variance is the right cost proxy for
// MC convergence: a tile with high pixel-color variance after preview_spp
// samples needs more SPP to converge. Cost = variance + epsilon (epsilon
// floor lets zero-variance tiles still be ranked).
void preview_pt_variance(std::vector<Tile>& tiles,
                         const rt::Camera& cam, const rt::PathScene& ps,
                         int W, int H, int preview_spp = 4) {
    const int ntiles = static_cast<int>(tiles.size());
    #pragma omp parallel for schedule(static)
    for (int ti = 0; ti < ntiles; ++ti) {
        auto& t = tiles[ti];
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
        t.cost = variance + 1e-3;
    }
}

} // namespace

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = 0, nranks = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);

    const std::string scene_path = (argc > 1) ? argv[1] : "scenes/whitted_showcase.scene";
    const std::string out_path   = (argc > 2) ? argv[2] : derive_output_path(scene_path);
    const bool adaptive = (argc > 3 && std::string(argv[3]) == "--adaptive");

    int exit_code = 0;
    try {
        rt::Scene s = rt::load_scene(scene_path);

        const int W = s.W;
        const int H = s.H;
        const double aspect = static_cast<double>(W) / H;
        const bool is_pt    = (s.integrator == rt::Integrator::PathTrace);

        rt::WhittedScene ws;
        ws.world      = &s.world;
        ws.lights     = s.lights;
        ws.ambient    = s.ambient;
        ws.background = s.background;
        ws.max_depth  = s.max_depth;

        rt::PathScene ps;
        if (is_pt) {
            ps.world       = &s.world;
            ps.area_lights = s.area_lights;
            ps.background  = s.background;
            ps.max_depth   = s.max_depth;
        }
        const int spp = s.spp;

        rt::Camera cam(s.camera_pos, s.camera_lookat, s.camera_up, s.vfov, aspect);

        auto tiles = make_tiles(W, H);
        const int ntiles = static_cast<int>(tiles.size());

        std::vector<int> tile_owner(ntiles, 0);

        // --- ADAPTIVE PREVIEW (rank 0 only) ----------------------------
        // Rank 0 runs the preview pass in parallel (OMP), computes per-tile
        // cost (Whitted: traversal cost; PT: pixel-color variance), sorts
        // descending, then assigns tiles to ranks via LPT. Other ranks idle
        // during preview — preview is ~1–4% of total work, so the serial
        // bottleneck is small. Tile order and ownership broadcast below.
        if (adaptive) {
            if (rank == 0) {
                if (is_pt) preview_pt_variance(tiles, cam, ps, W, H);
                else       preview_whitted_cost(tiles, cam, ws, W, H);
                std::sort(tiles.begin(), tiles.end(),
                          [](const Tile& a, const Tile& b){ return a.cost > b.cost; });
                const auto asgn = lpt_assign(tiles, nranks);
                for (int r = 0; r < nranks; ++r)
                    for (int ti : asgn[r])
                        tile_owner[ti] = r;
            }
        } else {
            if (rank == 0) {
                const auto asgn = strip_assign(tiles, W, H, nranks);
                for (int r = 0; r < nranks; ++r)
                    for (int ti : asgn[r])
                        tile_owner[ti] = r;
            }
        }

        // Broadcast tile order (after sort) and ownership to all ranks.
        std::vector<int> tile_data(ntiles * 4);
        if (rank == 0)
            for (int ti = 0; ti < ntiles; ++ti) {
                tile_data[ti * 4 + 0] = tiles[ti].x0;
                tile_data[ti * 4 + 1] = tiles[ti].y0;
                tile_data[ti * 4 + 2] = tiles[ti].x1;
                tile_data[ti * 4 + 3] = tiles[ti].y1;
            }
        MPI_Bcast(tile_data.data(),  ntiles * 4, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(tile_owner.data(), ntiles,     MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0)
            for (int ti = 0; ti < ntiles; ++ti) {
                tiles[ti].x0 = tile_data[ti * 4 + 0];
                tiles[ti].y0 = tile_data[ti * 4 + 1];
                tiles[ti].x1 = tile_data[ti * 4 + 2];
                tiles[ti].y1 = tile_data[ti * 4 + 3];
            }

        // Collect this rank's tiles.
        std::vector<int> my_tiles;
        for (int ti = 0; ti < ntiles; ++ti)
            if (tile_owner[ti] == rank)
                my_tiles.push_back(ti);
        const int my_ntiles = static_cast<int>(my_tiles.size());

        // Allocate packed pixel buffer and precompute per-tile byte offsets.
        // Use size_t internally to handle 4K+ images; cast to int at the MPI
        // boundary with an explicit overflow guard (MPI_Gatherv counts are int).
        std::vector<size_t> tile_off(my_ntiles);
        size_t total_bytes = 0;
        {
            size_t off = 0;
            for (int k = 0; k < my_ntiles; ++k) {
                tile_off[k] = off;
                const auto& t = tiles[my_tiles[k]];
                off += static_cast<size_t>((t.x1 - t.x0) * (t.y1 - t.y0)) * 3;
            }
            total_bytes = off;
        }
        if (total_bytes > static_cast<size_t>(INT_MAX)) {
            std::cerr << "rank " << rank << ": local buffer "
                      << total_bytes << " B exceeds INT_MAX; need MPI custom datatype\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        const int local_bytes = static_cast<int>(total_bytes);
        std::vector<uint8_t> local_buf(total_bytes);

        // First-touch local buffer in tile order so each tile's bytes land on
        // the rendering thread's local NUMA node (helpful on 2-socket nodes
        // when 1 rank spans both sockets, e.g. np=1 t=40 hybrid).
        #pragma omp parallel for schedule(static)
        for (int k = 0; k < my_ntiles; ++k) {
            const auto& t = tiles[my_tiles[k]];
            const size_t bytes_in_tile =
                static_cast<size_t>((t.x1 - t.x0) * (t.y1 - t.y0)) * 3;
            std::fill_n(local_buf.data() + tile_off[k], bytes_in_tile, uint8_t{0});
        }

        // Tile metadata (x0,y0,x1,y1) for rank-0 reassembly.
        std::vector<int> my_meta(my_ntiles * 4);
        for (int k = 0; k < my_ntiles; ++k) {
            const auto& t = tiles[my_tiles[k]];
            my_meta[k * 4 + 0] = t.x0;
            my_meta[k * 4 + 1] = t.y0;
            my_meta[k * 4 + 2] = t.x1;
            my_meta[k * 4 + 3] = t.y1;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        const auto t0 = std::chrono::steady_clock::now();

        // Render: each thread gets a tile; integrator selected once per rank.
        #pragma omp parallel for schedule(dynamic, 1)
        for (int k = 0; k < my_ntiles; ++k) {
            const auto& t   = tiles[my_tiles[k]];
            size_t      off = tile_off[k];

            for (int y = t.y0; y < t.y1; ++y)
                for (int x = t.x0; x < t.x1; ++x) {
                    rt::Vec3 c;
                    if (is_pt) {
                        rt::Vec3 accum{0, 0, 0};
                        for (int si = 0; si < spp; ++si) {
                            rt::RNG rng(rt::pixel_seed(x, y, W, si));
                            double dx, dy;
                            rt::stratified_offset(si, spp, rng, x, y, W, dx, dy);
                            accum += rt::path_radiance(
                                cam.ray(x + dx, y + dy, W, H), ps, rng);
                        }
                        c = rt::reinhard(accum * (1.0 / spp));
                    } else {
                        c = rt::whitted_radiance(cam.ray(x, y, W, H), ws, ws.max_depth);
                    }
                    local_buf[off + 0] = to_byte(c.x);
                    local_buf[off + 1] = to_byte(c.y);
                    local_buf[off + 2] = to_byte(c.z);
                    off += 3;
                }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        const auto t1 = std::chrono::steady_clock::now();
        const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        // Communication-time measurement: separate timer for the gather step
        // so we can report compute vs communication breakdown.
        const auto tg0 = std::chrono::steady_clock::now();

        std::vector<int> all_bytes(nranks), all_meta_counts(nranks);
        MPI_Gather(&local_bytes, 1, MPI_INT, all_bytes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
        int my_meta_count = my_ntiles * 4;
        MPI_Gather(&my_meta_count, 1, MPI_INT, all_meta_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<uint8_t> all_pixels;
        std::vector<int>     all_meta;
        std::vector<int> byte_displs(nranks, 0), meta_displs(nranks, 0);

        if (rank == 0) {
            for (int r = 1; r < nranks; ++r) {
                byte_displs[r] = byte_displs[r - 1] + all_bytes[r - 1];
                meta_displs[r] = meta_displs[r - 1] + all_meta_counts[r - 1];
            }
            all_pixels.resize(byte_displs[nranks - 1] + all_bytes[nranks - 1]);
            all_meta.resize(meta_displs[nranks - 1] + all_meta_counts[nranks - 1]);
        }

        MPI_Gatherv(local_buf.data(), local_bytes, MPI_UNSIGNED_CHAR,
                    all_pixels.data(), all_bytes.data(), byte_displs.data(),
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
        MPI_Gatherv(my_meta.data(), my_meta_count, MPI_INT,
                    all_meta.data(), all_meta_counts.data(), meta_displs.data(),
                    MPI_INT, 0, MPI_COMM_WORLD);

        const auto tg1 = std::chrono::steady_clock::now();
        const double gather_ms = std::chrono::duration<double, std::milli>(tg1 - tg0).count();

        if (rank == 0) {
            std::vector<uint8_t> image(static_cast<size_t>(W) * H * 3);
            size_t src_off = 0;
            const int total_tiles = static_cast<int>(all_meta.size()) / 4;
            for (int k = 0; k < total_tiles; ++k) {
                const int tx0 = all_meta[k * 4 + 0];
                const int ty0 = all_meta[k * 4 + 1];
                const int tx1 = all_meta[k * 4 + 2];
                const int ty1 = all_meta[k * 4 + 3];
                for (int y = ty0; y < ty1; ++y)
                    for (int x = tx0; x < tx1; ++x) {
                        const size_t dst = (static_cast<size_t>(y) * W + x) * 3;
                        image[dst + 0] = all_pixels[src_off + 0];
                        image[dst + 1] = all_pixels[src_off + 1];
                        image[dst + 2] = all_pixels[src_off + 2];
                        src_off += 3;
                    }
            }

            if (!rt::write_ppm(out_path, W, H, image)) {
                std::cerr << "failed to write " << out_path << "\n";
                exit_code = 1;
            } else {
                std::string mode = is_pt
                    ? ("path_trace spp " + std::to_string(spp))
                    : std::string("whitted");
                std::cout << "wrote " << out_path
                          << " (" << W << "x" << H << ")"
                          << " ranks "   << nranks
                          << " threads " << omp_get_max_threads()
                          << " "         << mode
                          << (adaptive ? " adaptive" : " standard")
                          << " render "  << ms
                          << " gather "  << gather_ms
                          << " ms\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "rank " << rank << " error: " << e.what() << "\n";
        exit_code = 1;
    }

    MPI_Finalize();
    return exit_code;
}
