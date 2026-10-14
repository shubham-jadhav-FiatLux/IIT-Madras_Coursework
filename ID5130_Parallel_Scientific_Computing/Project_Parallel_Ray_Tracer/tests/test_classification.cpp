#include "box.h"
#include "cylinder.h"
#include "hittable.h"
#include "plane.h"
#include "sphere.h"

#include "check.h"

using namespace rt;

static int test_sphere() {
    Sphere s(Vec3{0, 0, 0}, 1.0);
    const double eps = 1e-6;

    CHECK(s.classify(Vec3{0, 0, 0},    eps) == Classification::Inside);
    CHECK(s.classify(Vec3{0.5, 0, 0},  eps) == Classification::Inside);
    CHECK(s.classify(Vec3{1, 0, 0},    eps) == Classification::On);
    CHECK(s.classify(Vec3{0, -1, 0},   eps) == Classification::On);
    CHECK(s.classify(Vec3{2, 0, 0},    eps) == Classification::Outside);
    CHECK(s.classify(Vec3{0, 1.1, 0},  eps) == Classification::Outside);

    // Epsilon boundary: 0.5*eps inside → On; 2*eps inside → Inside.
    CHECK(s.classify(Vec3{1.0 - 0.5 * eps, 0, 0}, eps) == Classification::On);
    CHECK(s.classify(Vec3{1.0 - 2.0 * eps, 0, 0}, eps) == Classification::Inside);
    CHECK(s.classify(Vec3{1.0 + 0.5 * eps, 0, 0}, eps) == Classification::On);
    CHECK(s.classify(Vec3{1.0 + 2.0 * eps, 0, 0}, eps) == Classification::Outside);

    return 0;
}

static int test_plane() {
    Plane pl(Vec3{0, 0, 0}, Vec3{0, 1, 0});  // normal up; inside is y < 0
    const double eps = 1e-6;

    CHECK(pl.classify(Vec3{0, -1, 0},  eps) == Classification::Inside);
    CHECK(pl.classify(Vec3{5, -0.5, 3}, eps) == Classification::Inside);
    CHECK(pl.classify(Vec3{0, 0, 0},    eps) == Classification::On);
    CHECK(pl.classify(Vec3{1, 0, -2},   eps) == Classification::On);
    CHECK(pl.classify(Vec3{0, 1, 0},    eps) == Classification::Outside);

    CHECK(pl.classify(Vec3{0, 0.5 * eps, 0},  eps) == Classification::On);
    CHECK(pl.classify(Vec3{0, -0.5 * eps, 0}, eps) == Classification::On);
    CHECK(pl.classify(Vec3{0, 2.0 * eps, 0},  eps) == Classification::Outside);
    CHECK(pl.classify(Vec3{0, -2.0 * eps, 0}, eps) == Classification::Inside);

    return 0;
}

static int test_box() {
    Box b(Vec3{-1, -1, -1}, Vec3{1, 1, 1});
    const double eps = 1e-6;

    CHECK(b.classify(Vec3{0, 0, 0},      eps) == Classification::Inside);
    CHECK(b.classify(Vec3{0.5, 0.5, 0.5}, eps) == Classification::Inside);
    CHECK(b.classify(Vec3{1, 0, 0},      eps) == Classification::On);     // face
    CHECK(b.classify(Vec3{1, 1, 0},      eps) == Classification::On);     // edge
    CHECK(b.classify(Vec3{1, 1, 1},      eps) == Classification::On);     // corner
    CHECK(b.classify(Vec3{2, 0, 0},      eps) == Classification::Outside);
    CHECK(b.classify(Vec3{0, 0, -2},     eps) == Classification::Outside);
    CHECK(b.classify(Vec3{2, 2, 2},      eps) == Classification::Outside); // diagonal

    // Epsilon boundary on face.
    CHECK(b.classify(Vec3{1 - 0.5 * eps, 0, 0}, eps) == Classification::On);
    CHECK(b.classify(Vec3{1 - 2.0 * eps, 0, 0}, eps) == Classification::Inside);
    CHECK(b.classify(Vec3{1 + 0.5 * eps, 0, 0}, eps) == Classification::On);
    CHECK(b.classify(Vec3{1 + 2.0 * eps, 0, 0}, eps) == Classification::Outside);

    return 0;
}

static int test_cylinder() {
    Cylinder c(Vec3{0, 0, 0}, 1.0, 2.0);  // unit radius, y in [0, 2]
    const double eps = 1e-6;

    CHECK(c.classify(Vec3{0, 1, 0},    eps) == Classification::Inside);
    CHECK(c.classify(Vec3{0.5, 1, 0},  eps) == Classification::Inside);
    CHECK(c.classify(Vec3{1, 1, 0},    eps) == Classification::On);        // side
    CHECK(c.classify(Vec3{0, 0, 0},    eps) == Classification::On);        // bottom-center
    CHECK(c.classify(Vec3{0, 2, 0},    eps) == Classification::On);        // top-center
    CHECK(c.classify(Vec3{1, 2, 0},    eps) == Classification::On);        // top-rim
    CHECK(c.classify(Vec3{0.5, 0, 0},  eps) == Classification::On);        // on bottom cap disk
    CHECK(c.classify(Vec3{2, 1, 0},    eps) == Classification::Outside);   // outside radially
    CHECK(c.classify(Vec3{0, 3, 0},    eps) == Classification::Outside);   // above top
    CHECK(c.classify(Vec3{0, -1, 0},   eps) == Classification::Outside);   // below bottom

    // Epsilon boundary at side surface.
    CHECK(c.classify(Vec3{1 - 0.5 * eps, 1, 0}, eps) == Classification::On);
    CHECK(c.classify(Vec3{1 - 2.0 * eps, 1, 0}, eps) == Classification::Inside);
    CHECK(c.classify(Vec3{1 + 0.5 * eps, 1, 0}, eps) == Classification::On);
    CHECK(c.classify(Vec3{1 + 2.0 * eps, 1, 0}, eps) == Classification::Outside);

    return 0;
}

static int test_list() {
    HittableList world;
    world.add(std::make_unique<Sphere>(Vec3{ 2, 0, 0}, 1.0));
    world.add(std::make_unique<Sphere>(Vec3{-2, 0, 0}, 1.0));
    const double eps = 1e-6;

    CHECK(world.classify(Vec3{ 2, 0, 0}, eps) == Classification::Inside);
    CHECK(world.classify(Vec3{-2, 0, 0}, eps) == Classification::Inside);
    CHECK(world.classify(Vec3{ 3, 0, 0}, eps) == Classification::On);
    CHECK(world.classify(Vec3{ 0, 0, 0}, eps) == Classification::Outside);  // between spheres
    return 0;
}

int main() {
    if (test_sphere())   return 1;
    if (test_plane())    return 1;
    if (test_box())      return 1;
    if (test_cylinder()) return 1;
    if (test_list())     return 1;
    std::cout << "all classification tests passed\n";
    return 0;
}
