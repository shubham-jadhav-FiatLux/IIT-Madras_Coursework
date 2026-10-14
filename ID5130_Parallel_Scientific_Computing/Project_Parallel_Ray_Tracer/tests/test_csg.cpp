#include "box.h"
#include "csg.h"
#include "cylinder.h"
#include "hittable.h"
#include "plane.h"
#include "sphere.h"

#include "check.h"

#include <memory>

using namespace rt;

static int test_intervals_basic() {
    Sphere s(Vec3{0, 0, 0}, 1.0);
    std::vector<Interval> ivs;
    s.intervals(Ray{Vec3{0, 0, 3}, Vec3{0, 0, -1}}, ivs);
    CHECK(ivs.size() == 1);
    CHECK_NEAR(ivs[0].t_enter, 2.0, 1e-9);
    CHECK_NEAR(ivs[0].t_exit,  4.0, 1e-9);
    CHECK_NEAR(ivs[0].n_enter.z, 1.0, 1e-9);
    CHECK_NEAR(ivs[0].n_exit.z, -1.0, 1e-9);

    // Miss sphere
    ivs.clear();
    s.intervals(Ray{Vec3{0, 3, 0}, Vec3{1, 0, 0}}, ivs);
    CHECK(ivs.empty());
    return 0;
}

static int test_plane_halfspace_intervals() {
    Plane p(Vec3{0, 0, 0}, Vec3{0, 1, 0});  // inside = y < 0
    std::vector<Interval> ivs;
    // Ray from y=2 heading -y: starts outside, enters half-space at y=0.
    p.intervals(Ray{Vec3{0, 2, 0}, Vec3{0, -1, 0}}, ivs);
    CHECK(ivs.size() == 1);
    CHECK_NEAR(ivs[0].t_enter, 2.0, 1e-9);
    CHECK(std::isinf(ivs[0].t_exit));

    // Ray from y=-1 heading +y: starts inside, exits at y=0.
    ivs.clear();
    p.intervals(Ray{Vec3{0, -1, 0}, Vec3{0, 1, 0}}, ivs);
    CHECK(ivs.size() == 1);
    CHECK(std::isinf(ivs[0].t_enter) && ivs[0].t_enter < 0);
    CHECK_NEAR(ivs[0].t_exit, 1.0, 1e-9);
    return 0;
}

static int test_union() {
    auto s1 = std::make_unique<Sphere>(Vec3{-0.5, 0, 0}, 1.0);  // [-1.5, 0.5]
    auto s2 = std::make_unique<Sphere>(Vec3{ 0.5, 0, 0}, 1.0);  // [-0.5, 1.5]
    CSGNode u(CSGOp::Union, std::move(s1), std::move(s2));

    // Ray along +x hits merged union at x=-1.5, exits at x=1.5.
    HitRecord rec;
    CHECK(u.hit(Ray{Vec3{-3, 0, 0}, Vec3{1, 0, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 1.5, 1e-9);
    CHECK(rec.front_face);

    std::vector<Interval> ivs;
    u.intervals(Ray{Vec3{-3, 0, 0}, Vec3{1, 0, 0}}, ivs);
    CHECK(ivs.size() == 1);  // overlap merged into one
    CHECK_NEAR(ivs[0].t_enter, 1.5, 1e-9);
    CHECK_NEAR(ivs[0].t_exit,  4.5, 1e-9);

    // Classify
    CHECK(u.classify(Vec3{-0.5, 0, 0}, 1e-6) == Classification::Inside);  // inside s1
    CHECK(u.classify(Vec3{ 0.5, 0, 0}, 1e-6) == Classification::Inside);  // inside s2
    CHECK(u.classify(Vec3{ 0,   0, 0}, 1e-6) == Classification::Inside);  // inside both
    CHECK(u.classify(Vec3{ 3,   0, 0}, 1e-6) == Classification::Outside);
    return 0;
}

static int test_intersection() {
    auto s1 = std::make_unique<Sphere>(Vec3{-0.5, 0, 0}, 1.0);
    auto s2 = std::make_unique<Sphere>(Vec3{ 0.5, 0, 0}, 1.0);
    CSGNode i(CSGOp::Intersection, std::move(s1), std::move(s2));

    // Along +x from (-3,0,0): the lens is in x ∈ [-0.5, 0.5].
    std::vector<Interval> ivs;
    i.intervals(Ray{Vec3{-3, 0, 0}, Vec3{1, 0, 0}}, ivs);
    CHECK(ivs.size() == 1);
    CHECK_NEAR(ivs[0].t_enter, 2.5, 1e-9);  // reach x=-0.5
    CHECK_NEAR(ivs[0].t_exit,  3.5, 1e-9);  // reach x=+0.5

    // Classify
    CHECK(i.classify(Vec3{ 0, 0, 0},  1e-6) == Classification::Inside);   // in both
    CHECK(i.classify(Vec3{-1, 0, 0},  1e-6) == Classification::Outside);  // only in s1
    CHECK(i.classify(Vec3{ 1, 0, 0},  1e-6) == Classification::Outside);  // only in s2
    return 0;
}

static int test_difference() {
    // Big sphere at origin minus small sphere at (+0.6, 0, 0).
    auto outer = std::make_unique<Sphere>(Vec3{0, 0, 0}, 1.0);
    auto bite  = std::make_unique<Sphere>(Vec3{0.6, 0, 0}, 0.5);  // spans x in [0.1, 1.1]
    CSGNode d(CSGOp::Difference, std::move(outer), std::move(bite));

    // Ray along +x from (-3,0,0). Enters outer at x=-1, exits outer at x=1.
    // The "bite" sphere cuts a hole from x=0.1 to x=1.1. Inside outer-bite is:
    //   [x=-1, x=0.1] and [x=1.1, ?]. But 1.1 > 1 (outer exit), so only [-1, 0.1].
    std::vector<Interval> ivs;
    d.intervals(Ray{Vec3{-3, 0, 0}, Vec3{1, 0, 0}}, ivs);
    CHECK(ivs.size() == 1);
    CHECK_NEAR(ivs[0].t_enter, 2.0, 1e-9);   // x=-1: outer enter
    CHECK_NEAR(ivs[0].t_exit,  3.1, 1e-9);   // x=0.1: bite enter (flipped)

    // At exit, normal should be the flipped bite normal.
    // At x=0.1, bite center is (0.6,0,0), outward normal = (0.1-0.6, 0, 0)/0.5 = (-1,0,0).
    // Flipped = (+1, 0, 0).
    CHECK_NEAR(ivs[0].n_exit.x, 1.0, 1e-9);

    HitRecord rec;
    CHECK(d.hit(Ray{Vec3{-3, 0, 0}, Vec3{1, 0, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 2.0, 1e-9);  // first hit is outer sphere's front

    // Ray passing through the "hole" from -x direction, starting beyond outer:
    // Actually a ray that enters the hole region should produce different intervals.
    // Ray from (0.6, 0, 3) direction (0, 0, -1): goes down through the cavity.
    // Outer sphere: enters at z=+sqrt(1-0.36)≈0.8, exits at z=-0.8.
    // Bite sphere (center (0.6,0,0), r=0.5): enters at z=+0.5, exits at z=-0.5.
    // Difference: [outer_enter, bite_enter] ∪ [bite_exit, outer_exit]
    //           = [0.8 -> 0.5] ∪ [-0.5 -> -0.8] (in ray param)
    // Ray origin (0.6,0,3), direction (0,0,-1): t of outer_enter = 3 - 0.8 = 2.2.
    // t of bite_enter = 3 - 0.5 = 2.5. t of bite_exit = 3 + 0.5 = 3.5. t of outer_exit = 3 + 0.8 = 3.8.
    std::vector<Interval> ivs2;
    d.intervals(Ray{Vec3{0.6, 0, 3}, Vec3{0, 0, -1}}, ivs2);
    CHECK(ivs2.size() == 2);
    CHECK_NEAR(ivs2[0].t_enter, 2.2, 1e-6);
    CHECK_NEAR(ivs2[0].t_exit,  2.5, 1e-6);
    CHECK_NEAR(ivs2[1].t_enter, 3.5, 1e-6);
    CHECK_NEAR(ivs2[1].t_exit,  3.8, 1e-6);

    // Classify
    CHECK(d.classify(Vec3{-0.5, 0, 0}, 1e-6) == Classification::Inside);   // in outer, not bite
    CHECK(d.classify(Vec3{ 0.6, 0, 0}, 1e-6) == Classification::Outside);  // in bite → removed
    CHECK(d.classify(Vec3{ 2,   0, 0}, 1e-6) == Classification::Outside);  // outside outer
    return 0;
}

static int test_difference_with_halfspace() {
    // Cylinder clipped by a plane (half-space) from above.
    auto cyl   = std::make_unique<Cylinder>(Vec3{0, 0, 0}, 1.0, 2.0);  // y in [0,2]
    auto plane = std::make_unique<Plane>(Vec3{0, 1.5, 0}, Vec3{0, -1, 0});
    // Plane normal points down; "inside" half-space = above y=1.5.
    // Cylinder - half-space removes the portion of cylinder ABOVE y=1.5.
    CSGNode d(CSGOp::Difference, std::move(cyl), std::move(plane));

    // Ray straight down through center: cylinder caps from (0, 5, 0) dir (0,-1,0).
    // Expected hit: top at y=1.5 (the cut), because cylinder - above_half = [y=0, y=1.5].
    HitRecord rec;
    CHECK(d.hit(Ray{Vec3{0, 5, 0}, Vec3{0, -1, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 3.5, 1e-6);  // 5 - 1.5 = 3.5

    CHECK(d.classify(Vec3{0, 1, 0},  1e-6) == Classification::Inside);   // in clipped part
    CHECK(d.classify(Vec3{0, 1.8, 0}, 1e-6) == Classification::Outside); // in removed part
    CHECK(d.classify(Vec3{0, 1.5, 0}, 1e-6) == Classification::On);      // on cut plane
    return 0;
}

int main() {
    if (test_intervals_basic())          return 1;
    if (test_plane_halfspace_intervals()) return 1;
    if (test_union())                     return 1;
    if (test_intersection())              return 1;
    if (test_difference())                return 1;
    if (test_difference_with_halfspace()) return 1;
    std::cout << "all CSG tests passed\n";
    return 0;
}
