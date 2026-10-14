#pragma once
#include "hittable.h"

#include <algorithm>
#include <deque>
#include <memory>
#include <vector>

namespace rt {

enum class CSGOp { Union, Intersection, Difference };

namespace detail {

struct IntervalEvent {
    double t;
    int    delta;     // +1 enter, -1 exit
    Vec3   n;         // outward normal at this event
    bool   from_a;    // true if from operand A, else B
};

// Reusable event scratch. Leaf-level: one buffer per thread is safe because
// the detail functions don't recurse into each other.
inline std::vector<IntervalEvent>& event_scratch() {
    static thread_local std::vector<IntervalEvent> ev;
    return ev;
}

inline void push_events(const std::vector<Interval>& A, const std::vector<Interval>& B,
                        std::vector<IntervalEvent>& ev) {
    ev.clear();
    ev.reserve(2 * (A.size() + B.size()));
    for (const auto& iv : A) {
        ev.push_back({iv.t_enter, +1, iv.n_enter, true});
        ev.push_back({iv.t_exit,  -1, iv.n_exit,  true});
    }
    for (const auto& iv : B) {
        ev.push_back({iv.t_enter, +1, iv.n_enter, false});
        ev.push_back({iv.t_exit,  -1, iv.n_exit,  false});
    }
}

// Union of interval lists. Touching intervals (a.exit == b.enter) merge
// (no spurious zero-length boundary).
inline void interval_union(const std::vector<Interval>& A,
                           const std::vector<Interval>& B,
                           std::vector<Interval>& out) {
    auto& ev = event_scratch();
    push_events(A, B, ev);
    std::sort(ev.begin(), ev.end(), [](const IntervalEvent& x, const IntervalEvent& y) {
        if (x.t != y.t) return x.t < y.t;
        return x.delta > y.delta;   // enters before exits when tied
    });

    out.clear();
    int depth = 0;
    Interval cur{};
    for (const auto& e : ev) {
        const int nd = depth + e.delta;
        if (depth == 0 && nd == 1) { cur.t_enter = e.t; cur.n_enter = e.n; }
        else if (depth >= 1 && nd == 0) { cur.t_exit = e.t; cur.n_exit = e.n; out.push_back(cur); }
        depth = nd;
    }
}

// Intersection: result inside when both A and B inside.
inline void interval_intersection(const std::vector<Interval>& A,
                                  const std::vector<Interval>& B,
                                  std::vector<Interval>& out) {
    auto& ev = event_scratch();
    push_events(A, B, ev);
    std::sort(ev.begin(), ev.end(), [](const IntervalEvent& x, const IntervalEvent& y) {
        if (x.t != y.t) return x.t < y.t;
        return x.delta < y.delta;   // exits before enters when tied (no zero-length overlap)
    });

    out.clear();
    int da = 0, db = 0;
    Interval cur{};
    bool inside_prev = false;
    for (const auto& e : ev) {
        if (e.from_a) da += e.delta; else db += e.delta;
        const bool inside_now = (da > 0) && (db > 0);
        if (!inside_prev && inside_now)      { cur.t_enter = e.t; cur.n_enter = e.n; }
        else if (inside_prev && !inside_now) { cur.t_exit  = e.t; cur.n_exit  = e.n; out.push_back(cur); }
        inside_prev = inside_now;
    }
}

// Difference A - B: inside when inside A and outside B.
// B's normals are flipped when they become boundary of the result.
inline void interval_difference(const std::vector<Interval>& A,
                                const std::vector<Interval>& B,
                                std::vector<Interval>& out) {
    auto& ev = event_scratch();
    push_events(A, B, ev);
    std::sort(ev.begin(), ev.end(), [](const IntervalEvent& x, const IntervalEvent& y) {
        if (x.t != y.t) return x.t < y.t;
        return x.delta < y.delta;
    });

    out.clear();
    int da = 0, db = 0;
    Interval cur{};
    bool inside_prev = false;
    for (const auto& e : ev) {
        if (e.from_a) da += e.delta; else db += e.delta;
        const bool inside_now = (da > 0) && (db == 0);
        if (!inside_prev && inside_now) {
            cur.t_enter = e.t;
            cur.n_enter = e.from_a ? e.n : (Vec3{} - e.n);  // flip B normal
        } else if (inside_prev && !inside_now) {
            cur.t_exit = e.t;
            cur.n_exit = e.from_a ? e.n : (Vec3{} - e.n);
            out.push_back(cur);
        }
        inside_prev = inside_now;
    }
}

} // namespace detail

class CSGNode : public Hittable {
public:
    CSGOp op;
    std::unique_ptr<Hittable> a;
    std::unique_ptr<Hittable> b;
    const Material* material{nullptr};

    CSGNode(CSGOp o, std::unique_ptr<Hittable> A, std::unique_ptr<Hittable> B,
            const Material* m = nullptr)
        : op(o), a(std::move(A)), b(std::move(B)), material(m) {}

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        // One thread_local result buffer per thread; reused across calls.
        // Safe: intervals() returns before any recursive whitted call reuses it.
        static thread_local std::vector<Interval> ivs;
        ivs.clear();
        intervals(r, ivs);
        if (!hit_from_intervals(ivs, r, tmin, tmax, rec)) return false;
        rec.material = material;
        return true;
    }

    Classification classify(const Vec3& p, double eps = 1e-6) const override {
        const auto ca = a->classify(p, eps);
        const auto cb = b->classify(p, eps);
        switch (op) {
            case CSGOp::Union:
                if (ca == Classification::Inside  || cb == Classification::Inside)  return Classification::Inside;
                if (ca == Classification::Outside && cb == Classification::Outside) return Classification::Outside;
                return Classification::On;
            case CSGOp::Intersection:
                if (ca == Classification::Outside || cb == Classification::Outside) return Classification::Outside;
                if (ca == Classification::Inside  && cb == Classification::Inside)  return Classification::Inside;
                return Classification::On;
            case CSGOp::Difference:
                if (ca == Classification::Outside) return Classification::Outside;
                if (cb == Classification::Inside)  return Classification::Outside;
                if (ca == Classification::On)      return Classification::On;
                if (cb == Classification::On)      return Classification::On;
                return Classification::Inside;
        }
        return Classification::Outside;
    }

    void intervals(const Ray& r, std::vector<Interval>& out) const override {
        // Thread-local pool indexed by nesting depth so sibling and parent CSG
        // nodes on the same thread don't overwrite each other's operand buffers.
        // Pool grows on demand to support arbitrarily deep CSG trees.
        //
        // std::deque is required (not std::vector) for reference stability:
        // a recursive intervals() call at deeper nesting may grow the pool,
        // and we hold references to pool_a[depth] and pool_b[depth] across
        // those recursive calls. std::deque's push_back/emplace_back do not
        // invalidate references to existing elements; std::vector's may.
        static thread_local int depth = 0;
        static thread_local std::deque<std::vector<Interval>> pool_a, pool_b;
        while (static_cast<size_t>(depth) >= pool_a.size()) {
            pool_a.emplace_back();
            pool_b.emplace_back();
        }

        auto& sa = pool_a[depth];
        auto& sb = pool_b[depth];
        sa.clear();
        sb.clear();
        ++depth;
        a->intervals(r, sa);
        b->intervals(r, sb);
        --depth;

        switch (op) {
            case CSGOp::Union:        detail::interval_union       (sa, sb, out); break;
            case CSGOp::Intersection: detail::interval_intersection(sa, sb, out); break;
            case CSGOp::Difference:   detail::interval_difference  (sa, sb, out); break;
        }
    }

    bool bounds(AABB& out) const override {
        AABB ba, bb;
        const bool ha = a->bounds(ba);
        const bool hb = b->bounds(bb);
        switch (op) {
            case CSGOp::Union:
                if (!ha || !hb) return false;
                out = ba; out.enclose(bb); return true;
            case CSGOp::Intersection:
                if (!ha && !hb) return false;
                if (!ha) { out = bb; return true; }
                if (!hb) { out = ba; return true; }
                out = AABB{
                    Vec3{ std::max(ba.pmin.x, bb.pmin.x),
                          std::max(ba.pmin.y, bb.pmin.y),
                          std::max(ba.pmin.z, bb.pmin.z) },
                    Vec3{ std::min(ba.pmax.x, bb.pmax.x),
                          std::min(ba.pmax.y, bb.pmax.y),
                          std::min(ba.pmax.z, bb.pmax.z) } };
                return true;
            case CSGOp::Difference:
                if (!ha) return false;
                out = ba; return true;
        }
        return false;
    }
};

} // namespace rt
