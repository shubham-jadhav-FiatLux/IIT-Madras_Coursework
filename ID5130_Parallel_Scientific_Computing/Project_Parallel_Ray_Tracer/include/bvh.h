#pragma once
#include "aabb.h"
#include "hittable.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace rt {

// Top-down median-split BVH over a list of Hittables that all have finite
// bounds. Constructor consumes the vector. Nodes are binary; leaves hold one
// primitive in `left` (and `right == nullptr`).
class BVHNode : public Hittable {
public:
    AABB box;
    std::unique_ptr<Hittable> left;
    std::unique_ptr<Hittable> right;

    explicit BVHNode(std::vector<std::unique_ptr<Hittable>> items) {
        build(std::move(items));
    }

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        if (!box.hit(r, tmin, tmax)) return false;
        if (!right) return left->hit(r, tmin, tmax, rec);

        HitRecord temp;
        bool any = false;
        double closest = tmax;
        if (left->hit(r, tmin, closest, temp)) {
            any = true; closest = temp.t; rec = temp;
        }
        if (right->hit(r, tmin, closest, temp)) {
            any = true; closest = temp.t; rec = temp;
        }
        return any;
    }

    bool occluded(const Ray& r, double tmin, double tmax) const override {
        if (!box.hit(r, tmin, tmax)) return false;
        if (left  && left->occluded (r, tmin, tmax)) return true;
        if (right && right->occluded(r, tmin, tmax)) return true;
        return false;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        Classification best = Classification::Outside;
        if (left) {
            switch (left->classify(p, eps)) {
                case Classification::Inside:  return Classification::Inside;
                case Classification::On:      best = Classification::On; break;
                case Classification::Outside: break;
            }
        }
        if (right) {
            switch (right->classify(p, eps)) {
                case Classification::Inside:  return Classification::Inside;
                case Classification::On:      best = Classification::On; break;
                case Classification::Outside: break;
            }
        }
        return best;
    }

    bool bounds(AABB& out) const override {
        out = box;
        return true;
    }

private:
    // Binned SAH BVH construction.
    // C_trav = 1, C_isect = 1.5; 12 bins per axis.
    // Falls back to equal-count split when all centroids are coincident.
    void build(std::vector<std::unique_ptr<Hittable>> items) {
        const size_t N = items.size();

        // Cache bounds of every item (avoids re-querying virtual dispatch).
        std::vector<AABB> item_box(N);
        AABB root_box = AABB::empty();
        AABB centroid_box = AABB::empty();
        for (size_t i = 0; i < N; ++i) {
            items[i]->bounds(item_box[i]);
            root_box.enclose(item_box[i]);
            centroid_box.enclose_point(item_box[i].centroid());
        }
        box = root_box;

        if (N == 1) { left = std::move(items[0]); return; }
        if (N == 2) {
            left  = std::move(items[0]);
            right = std::move(items[1]);
            return;
        }

        static constexpr int   B       = 12;
        static constexpr double C_TRAV  = 1.0;
        static constexpr double C_ISECT = 1.5;
        const double root_sa = surface_area(root_box);

        int    best_axis  = -1;
        int    best_split = 0;            // split before this bin index
        double best_cost  = C_ISECT * static_cast<double>(N);  // leaf cost

        struct Bin { AABB bounds; int count{0}; };

        for (int axis = 0; axis < 3; ++axis) {
            const double span = centroid_box.pmax[axis] - centroid_box.pmin[axis];
            if (span < 1e-10) continue;

            Bin bins[B];
            for (int b = 0; b < B; ++b) bins[b].bounds = AABB::empty();

            for (size_t i = 0; i < N; ++i) {
                const double c = item_box[i].centroid()[axis];
                int b = static_cast<int>(B * (c - centroid_box.pmin[axis]) / span);
                b = std::clamp(b, 0, B - 1);
                bins[b].bounds.enclose(item_box[i]);
                ++bins[b].count;
            }

            // Prefix scan: left-side bounds and counts for splits 1..B-1.
            AABB prefix[B]; int cnt_l[B];
            prefix[0] = bins[0].bounds; cnt_l[0] = bins[0].count;
            for (int b = 1; b < B; ++b) {
                prefix[b] = prefix[b - 1];
                if (bins[b].count > 0) prefix[b].enclose(bins[b].bounds);
                cnt_l[b] = cnt_l[b - 1] + bins[b].count;
            }

            // Suffix scan: evaluate each split plane.
            AABB suffix = AABB::empty(); int cnt_r = 0;
            for (int b = B - 1; b >= 1; --b) {
                if (bins[b].count > 0) suffix.enclose(bins[b].bounds);
                cnt_r += bins[b].count;
                if (cnt_l[b - 1] == 0 || cnt_r == 0) continue;
                const double cost =
                    C_TRAV + C_ISECT *
                    (surface_area(prefix[b - 1]) * cnt_l[b - 1] +
                     surface_area(suffix)        * cnt_r) / root_sa;
                if (cost < best_cost) {
                    best_cost = cost; best_axis = axis; best_split = b;
                }
            }
        }

        // Partition items at the chosen split (or median if SAH found no gain).
        auto pivot = items.end();
        if (best_axis >= 0) {
            const double span =
                centroid_box.pmax[best_axis] - centroid_box.pmin[best_axis];
            // stable_partition preserves relative order within each half.
            size_t idx = 0;
            pivot = std::stable_partition(
                items.begin(), items.end(),
                [&](const std::unique_ptr<Hittable>&) {
                    int b = static_cast<int>(
                        B * (item_box[idx].centroid()[best_axis] -
                             centroid_box.pmin[best_axis]) / span);
                    b = std::clamp(b, 0, B - 1);
                    return b < best_split ? (++idx, true) : (++idx, false);
                });
        }
        // Degenerate partition or SAH leaf → equal-count median split.
        if (pivot == items.begin() || pivot == items.end())
            pivot = items.begin() + N / 2;

        std::vector<std::unique_ptr<Hittable>> lhs(
            std::make_move_iterator(items.begin()),
            std::make_move_iterator(pivot));
        std::vector<std::unique_ptr<Hittable>> rhs(
            std::make_move_iterator(pivot),
            std::make_move_iterator(items.end()));

        left  = std::make_unique<BVHNode>(std::move(lhs));
        right = std::make_unique<BVHNode>(std::move(rhs));
    }
};

// Partition `items` into bounded vs unbounded, build a BVH over the bounded
// subset, and return [bvh, ...unbounded] (BVH first so it's tried before
// the linear unbounded list in HittableList::hit). If there are no bounded
// items, returns just the unbounded ones.
inline std::vector<std::unique_ptr<Hittable>>
build_world_with_bvh(std::vector<std::unique_ptr<Hittable>> items) {
    std::vector<std::unique_ptr<Hittable>> bounded, unbounded;
    for (auto& it : items) {
        AABB b;
        if (it->bounds(b)) bounded.push_back(std::move(it));
        else               unbounded.push_back(std::move(it));
    }
    std::vector<std::unique_ptr<Hittable>> out;
    if (!bounded.empty())
        out.push_back(std::make_unique<BVHNode>(std::move(bounded)));
    for (auto& it : unbounded) out.push_back(std::move(it));
    return out;
}

} // namespace rt
