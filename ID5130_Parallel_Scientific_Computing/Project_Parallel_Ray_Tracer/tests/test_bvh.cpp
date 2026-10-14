#include "aabb.h"
#include "box.h"
#include "bvh.h"
#include "csg.h"
#include "cylinder.h"
#include "hittable.h"
#include "plane.h"
#include "sphere.h"

#include "check.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

using namespace rt;

static int test_aabb_slab() {
    AABB b{ Vec3{-1,-1,-1}, Vec3{1,1,1} };
    CHECK( b.hit(Ray{Vec3{0,0,3},   Vec3{0,0,-1}}, 0, 1e9));
    CHECK(!b.hit(Ray{Vec3{0,0,3},   Vec3{0,0, 1}}, 0, 1e9));    // away from box
    CHECK(!b.hit(Ray{Vec3{5,5,5},   Vec3{0,0,-1}}, 0, 1e9));    // misses
    CHECK( b.hit(Ray{Vec3{0,0,0},   Vec3{1,0, 0}}, 0, 1e9));    // origin inside
    // Direction parallel to an axis but origin outside the slab in that axis.
    CHECK(!b.hit(Ray{Vec3{0,5,0},   Vec3{0,0,-1}}, 0, 1e9));
    return 0;
}

static int test_primitive_bounds() {
    AABB b;

    Sphere s(Vec3{1, 2, 3}, 0.5);
    CHECK(s.bounds(b));
    CHECK_NEAR(b.pmin.x, 0.5, 1e-12); CHECK_NEAR(b.pmax.x, 1.5, 1e-12);
    CHECK_NEAR(b.pmin.y, 1.5, 1e-12); CHECK_NEAR(b.pmax.y, 2.5, 1e-12);

    Box bx(Vec3{-1,-2,-3}, Vec3{4,5,6});
    CHECK(bx.bounds(b));
    CHECK_NEAR(b.pmin.x, -1.0, 1e-12); CHECK_NEAR(b.pmax.z, 6.0, 1e-12);

    Cylinder c(Vec3{0, 1, 0}, 2.0, 5.0);
    CHECK(c.bounds(b));
    CHECK_NEAR(b.pmin.x, -2.0, 1e-12); CHECK_NEAR(b.pmax.x, 2.0, 1e-12);
    CHECK_NEAR(b.pmin.y,  1.0, 1e-12); CHECK_NEAR(b.pmax.y, 6.0, 1e-12);

    Plane p(Vec3{0,0,0}, Vec3{0,1,0});
    CHECK(!p.bounds(b));     // unbounded
    return 0;
}

static int test_csg_bounds() {
    AABB b;

    // Union of two unit spheres at x=-1 and x=+1.
    auto sa = std::make_unique<Sphere>(Vec3{-1, 0, 0}, 1.0);
    auto sb = std::make_unique<Sphere>(Vec3{ 1, 0, 0}, 1.0);
    CSGNode u(CSGOp::Union, std::move(sa), std::move(sb));
    CHECK(u.bounds(b));
    CHECK_NEAR(b.pmin.x, -2.0, 1e-12);
    CHECK_NEAR(b.pmax.x,  2.0, 1e-12);

    // Difference: bounds of A only.
    auto da = std::make_unique<Box>(Vec3{0,0,0}, Vec3{1,1,1});
    auto db = std::make_unique<Sphere>(Vec3{0.5, 0.5, 0.5}, 100.0);  // huge B
    CSGNode d(CSGOp::Difference, std::move(da), std::move(db));
    CHECK(d.bounds(b));
    CHECK_NEAR(b.pmin.x, 0.0, 1e-12);
    CHECK_NEAR(b.pmax.x, 1.0, 1e-12);

    // Difference with unbounded A: unbounded.
    auto pa = std::make_unique<Plane>(Vec3{0,0,0}, Vec3{0,1,0});
    auto pb = std::make_unique<Sphere>(Vec3{0,0,0}, 1.0);
    CSGNode d2(CSGOp::Difference, std::move(pa), std::move(pb));
    CHECK(!d2.bounds(b));

    return 0;
}

static int test_bvh_empty_world() {
    // build_world_with_bvh on an all-unbounded set gives back just the planes.
    std::vector<std::unique_ptr<Hittable>> items;
    items.push_back(std::make_unique<Plane>(Vec3{0,0,0}, Vec3{0,1,0}));
    auto out = build_world_with_bvh(std::move(items));
    CHECK(out.size() == 1);
    AABB b;
    CHECK(!out[0]->bounds(b));   // still unbounded (the plane)
    return 0;
}

static int test_bvh_matches_linear() {
    // Generate a deterministic cloud of spheres, build BVH and a linear list,
    // and verify that closest-hit results are identical for many random rays.
    std::mt19937 rng(0xC0FFEE);
    std::uniform_real_distribution<double> pos(-5.0, 5.0);
    std::uniform_real_distribution<double> rad( 0.05, 0.4);

    constexpr int N = 64;
    HittableList linear;
    std::vector<std::unique_ptr<Hittable>> bvh_items;
    for (int i = 0; i < N; ++i) {
        const Vec3 c{ pos(rng), pos(rng), pos(rng) };
        const double r = rad(rng);
        linear.add(std::make_unique<Sphere>(c, r));
        bvh_items.push_back(std::make_unique<Sphere>(c, r));
    }
    BVHNode bvh(std::move(bvh_items));

    // Aim rays from outside the cluster toward random points inside it,
    // so a meaningful fraction actually hit something.
    std::uniform_real_distribution<double> shell(8.0, 12.0);
    std::uniform_real_distribution<double> dir(-1.0, 1.0);
    constexpr int RAYS = 5000;
    int hits_lin = 0, hits_bvh = 0;
    for (int i = 0; i < RAYS; ++i) {
        const Vec3 dir_to_origin{ dir(rng), dir(rng), dir(rng) };
        if (length_squared(dir_to_origin) < 1e-9) continue;
        const Vec3 o = normalize(dir_to_origin) * shell(rng);
        const Vec3 target{ pos(rng), pos(rng), pos(rng) };
        Vec3 d = target - o;
        if (length_squared(d) < 1e-9) continue;
        d = normalize(d);
        Ray r{o, d};

        HitRecord hl, hb;
        const bool ok_l = linear.hit(r, 1e-4, 1e9, hl);
        const bool ok_b = bvh.hit   (r, 1e-4, 1e9, hb);
        CHECK(ok_l == ok_b);
        if (ok_l) {
            ++hits_lin; ++hits_bvh;
            // Bit-exact: same primitive, same intersection math, deterministic min.
            CHECK(hl.t == hb.t);
            CHECK(hl.n.x == hb.n.x && hl.n.y == hb.n.y && hl.n.z == hb.n.z);
            CHECK(hl.front_face == hb.front_face);
        }
    }
    CHECK(hits_lin > 100);   // sanity: scene actually got hit a lot
    return 0;
}

int main() {
    if (test_aabb_slab())          return 1;
    if (test_primitive_bounds())   return 1;
    if (test_csg_bounds())         return 1;
    if (test_bvh_empty_world())    return 1;
    if (test_bvh_matches_linear()) return 1;
    std::cout << "all BVH tests passed\n";
    return 0;
}
