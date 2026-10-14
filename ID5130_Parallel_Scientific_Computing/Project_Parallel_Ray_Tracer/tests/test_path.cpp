#include "check.h"
#include "path.h"
#include "vec3.h"

#include <cmath>

using namespace rt;

// --- RNG ---

static int test_rng() {
    RNG rng(42);
    for (int i = 0; i < 1000; ++i) {
        const double v = rng.next();
        CHECK(v >= 0.0 && v < 1.0);
    }
    // Same seed → same sequence.
    RNG a(7), b(7);
    for (int i = 0; i < 20; ++i)
        CHECK(a.next_u32() == b.next_u32());
    // Different seeds → different first value.
    RNG c(1), d(2);
    bool differs = false;
    for (int i = 0; i < 10 && !differs; ++i)
        differs = (c.next_u32() != d.next_u32());
    CHECK(differs);
    return 0;
}

// --- pixel_seed ---

static int test_pixel_seed() {
    // Different pixels get different seeds.
    CHECK(pixel_seed(0, 0, 512, 0) != pixel_seed(1, 0, 512, 0));
    CHECK(pixel_seed(0, 0, 512, 0) != pixel_seed(0, 1, 512, 0));
    CHECK(pixel_seed(0, 0, 512, 0) != pixel_seed(0, 0, 512, 1));
    return 0;
}

// --- cosine_sample ---

static int test_cosine_sample() {
    // Output should be in the hemisphere of n and normalized.
    const Vec3 n{0, 1, 0};
    RNG rng(123);
    for (int i = 0; i < 500; ++i) {
        const Vec3 s = cosine_sample(n, rng);
        CHECK_NEAR(length(s), 1.0, 1e-9);
        CHECK(dot(s, n) >= 0.0);
    }
    // Non-axis normal.
    const Vec3 n2 = normalize(Vec3{1, 1, 1});
    RNG rng2(99);
    for (int i = 0; i < 200; ++i) {
        const Vec3 s = cosine_sample(n2, rng2);
        CHECK_NEAR(length(s), 1.0, 1e-9);
        CHECK(dot(s, n2) >= -1e-9);
    }
    return 0;
}

// --- stratified_offset ---

static int test_stratified_offset() {
    // Perfect square spp=4 (2×2 grid) + per-pixel Cranley-Patterson rotation.
    // CP rotation is a toroidal shift: applied uniformly to all spp samples
    // for a given pixel, so the points remain a sqrt(spp) × sqrt(spp) jittered
    // grid (preserving variance-reduction properties of stratification), but
    // their absolute positions in [0,1)² shift to decorrelate from neighbour
    // pixels. The integer-cell membership is *not* preserved by CP rotation —
    // a sample originally at 0.45 with rotation 0.6 wraps to 0.05.
    const int spp = 4;
    const int W = 512, px = 10, py = 10;
    double dx, dy;
    double xs[4], ys[4];
    for (int si = 0; si < spp; ++si) {
        RNG rng(pixel_seed(px, py, W, si));
        stratified_offset(si, spp, rng, px, py, W, dx, dy);
        CHECK(dx >= 0.0 && dx < 1.0);
        CHECK(dy >= 0.0 && dy < 1.0);
        xs[si] = dx;
        ys[si] = dy;
    }
    // The 4 samples should still be distinct (non-degenerate stratification).
    for (int i = 0; i < spp; ++i)
        for (int j = i + 1; j < spp; ++j) {
            CHECK(std::abs(xs[i] - xs[j]) > 1e-9 || std::abs(ys[i] - ys[j]) > 1e-9);
        }
    // Non-square spp=3: fallback to pure jitter, still in [0,1)^2.
    for (int si = 0; si < 3; ++si) {
        RNG rng(pixel_seed(5, 5, W, si));
        stratified_offset(si, 3, rng, 5, 5, W, dx, dy);
        CHECK(dx >= 0.0 && dx < 1.0);
        CHECK(dy >= 0.0 && dy < 1.0);
    }
    // Determinism: same pixel + same si → same offset across calls.
    {
        RNG a(pixel_seed(7, 11, W, 2)), b(pixel_seed(7, 11, W, 2));
        double ax, ay, bx, by;
        stratified_offset(2, spp, a, 7, 11, W, ax, ay);
        stratified_offset(2, spp, b, 7, 11, W, bx, by);
        CHECK_NEAR(ax, bx, 1e-15);
        CHECK_NEAR(ay, by, 1e-15);
    }
    // Decorrelation: different pixels yield different offsets at sample 0.
    {
        RNG ra(pixel_seed(0, 0, W, 0)), rb(pixel_seed(1, 0, W, 0));
        double ax, ay, bx, by;
        stratified_offset(0, spp, ra, 0, 0, W, ax, ay);
        stratified_offset(0, spp, rb, 1, 0, W, bx, by);
        CHECK(std::abs(ax - bx) > 1e-9 || std::abs(ay - by) > 1e-9);
    }
    return 0;
}

// --- reinhard ---

static int test_reinhard() {
    // 0 maps to 0.
    const Vec3 r0 = reinhard({0, 0, 0});
    CHECK_NEAR(r0.x, 0.0, 1e-12);
    // Finite positive maps below input.
    const Vec3 r1 = reinhard({1, 2, 4});
    CHECK(r1.x < 1.0 && r1.x > 0.0);
    CHECK_NEAR(r1.x, 0.5, 1e-9);
    CHECK_NEAR(r1.y, 2.0 / 3.0, 1e-9);
    CHECK_NEAR(r1.z, 4.0 / 5.0, 1e-9);
    return 0;
}

// --- power2 ---

static int test_power2() {
    CHECK_NEAR(power2(1.0, 0.0), 1.0, 1e-12);
    CHECK_NEAR(power2(0.0, 1.0), 0.0, 1e-12);
    CHECK_NEAR(power2(1.0, 1.0), 0.5, 1e-12);
    // power2 + power2(b,a) == 1.
    const double a = 0.3, b = 0.7;
    CHECK_NEAR(power2(a, b) + power2(b, a), 1.0, 1e-12);
    return 0;
}

int main() {
    if (test_rng())               return 1;
    if (test_pixel_seed())        return 1;
    if (test_cosine_sample())     return 1;
    if (test_stratified_offset()) return 1;
    if (test_reinhard())          return 1;
    if (test_power2())            return 1;
    std::cout << "all path tests passed\n";
    return 0;
}
