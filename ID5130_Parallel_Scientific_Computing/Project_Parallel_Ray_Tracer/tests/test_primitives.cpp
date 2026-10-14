#include "box.h"
#include "cylinder.h"
#include "hittable.h"
#include "plane.h"
#include "sphere.h"

#include "check.h"

using namespace rt;

static int test_sphere() {
    Sphere s(Vec3{0, 0, 0}, 1.0);
    HitRecord rec;

    // Head-on hit from +z.
    CHECK(s.hit(Ray{Vec3{0, 0, 3}, Vec3{0, 0, -1}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 2.0, 1e-9);
    CHECK_NEAR(rec.n.z, 1.0, 1e-9);
    CHECK(rec.front_face);

    // Miss: ray parallel to +x, passing beside the sphere.
    CHECK(!s.hit(Ray{Vec3{0, 3, 0}, Vec3{1, 0, 0}}, 1e-4, 1e9, rec));

    // Grazing hit: ray tangent to sphere at (0,1,0).
    CHECK(s.hit(Ray{Vec3{-3, 1, 0}, Vec3{1, 0, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 3.0, 1e-6);
    CHECK_NEAR(rec.n.y, 1.0, 1e-6);

    // Ray origin inside sphere: returns exit hit, normal flipped (front_face=false).
    CHECK(s.hit(Ray{Vec3{0, 0, 0}, Vec3{0, 0, 1}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 1.0, 1e-9);
    CHECK(!rec.front_face);
    CHECK_NEAR(rec.n.z, -1.0, 1e-9);

    // Behind-ray sphere: miss.
    CHECK(!s.hit(Ray{Vec3{0, 0, 3}, Vec3{0, 0, 1}}, 1e-4, 1e9, rec));

    return 0;
}

static int test_plane() {
    Plane p(Vec3{0, 0, 0}, Vec3{0, 1, 0});
    HitRecord rec;

    CHECK(p.hit(Ray{Vec3{0, 2, 0}, Vec3{0, -1, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 2.0, 1e-9);
    CHECK_NEAR(rec.n.y, 1.0, 1e-9);
    CHECK(rec.front_face);

    // Parallel ray: miss.
    CHECK(!p.hit(Ray{Vec3{0, 2, 0}, Vec3{1, 0, 0}}, 1e-4, 1e9, rec));

    // Ray shooting up away from plane: miss (t negative).
    CHECK(!p.hit(Ray{Vec3{0, 2, 0}, Vec3{0, 1, 0}}, 1e-4, 1e9, rec));

    // Hit from below: normal flipped.
    CHECK(p.hit(Ray{Vec3{0, -2, 0}, Vec3{0, 1, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.n.y, -1.0, 1e-9);
    CHECK(!rec.front_face);

    return 0;
}

static int test_box() {
    Box b(Vec3{-1, -1, -1}, Vec3{1, 1, 1});
    HitRecord rec;

    // Hit front face from +z.
    CHECK(b.hit(Ray{Vec3{0, 0, 3}, Vec3{0, 0, -1}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 2.0, 1e-9);
    CHECK_NEAR(rec.n.z, 1.0, 1e-9);
    CHECK(rec.front_face);

    // Hit top face from above.
    CHECK(b.hit(Ray{Vec3{0, 3, 0}, Vec3{0, -1, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.n.y, 1.0, 1e-9);

    // Miss: ray passing beside.
    CHECK(!b.hit(Ray{Vec3{5, 5, 5}, Vec3{0, 0, -1}}, 1e-4, 1e9, rec));

    // Origin inside box: hit exit face.
    CHECK(b.hit(Ray{Vec3{0, 0, 0}, Vec3{1, 0, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 1.0, 1e-9);
    CHECK(!rec.front_face);

    return 0;
}

static int test_cylinder() {
    Cylinder c(Vec3{0, 0, 0}, 1.0, 2.0);  // unit radius, height 2, from y=0 to y=2
    HitRecord rec;

    // Hit side from +x.
    CHECK(c.hit(Ray{Vec3{3, 1, 0}, Vec3{-1, 0, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 2.0, 1e-9);
    CHECK_NEAR(rec.n.x, 1.0, 1e-9);
    CHECK(rec.front_face);

    // Hit top cap from above.
    CHECK(c.hit(Ray{Vec3{0, 5, 0}, Vec3{0, -1, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 3.0, 1e-9);
    CHECK_NEAR(rec.n.y, 1.0, 1e-9);

    // Miss: ray above cylinder, going sideways.
    CHECK(!c.hit(Ray{Vec3{3, 5, 0}, Vec3{-1, 0, 0}}, 1e-4, 1e9, rec));

    // Ray along axis through both caps from inside: hits top cap.
    CHECK(c.hit(Ray{Vec3{0, 1, 0}, Vec3{0, 1, 0}}, 1e-4, 1e9, rec));
    CHECK_NEAR(rec.t, 1.0, 1e-9);
    CHECK(!rec.front_face);

    // Parallel side ray that misses (offset beyond radius).
    CHECK(!c.hit(Ray{Vec3{3, 1, 2}, Vec3{-1, 0, 0}}, 1e-4, 1e9, rec));

    return 0;
}

int main() {
    if (test_sphere())   return 1;
    if (test_plane())    return 1;
    if (test_box())      return 1;
    if (test_cylinder()) return 1;
    std::cout << "all primitive tests passed\n";
    return 0;
}
