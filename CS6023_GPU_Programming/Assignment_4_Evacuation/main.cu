// CS6023 Assignment 4 -- Attack on Titans: Evacuation Arc
//
// Everything that is computation runs on the GPU:
//   1. dedupe roads (the evaluator always uses the FIRST road between two cities) and build a CSR graph,
//   2. one Dijkstra per populated city (thread per city) with the evaluator's own cost model, i.e. a road of
//      length L and capacity C costs L*ceil(prime/C) minutes/12, which is also what an elderly person "walks",
//   3. capacity-aware assignment of people to shelters in parallel rounds (atomic reservation, never over-fills),
//   4. a second Dijkstra per city to rebuild the tree and emit the walk and the drop list.
// The host only parses the input and prints the result.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <climits>
#include <thrust/sort.h>
#include <thrust/scan.h>
#include <thrust/device_ptr.h>

typedef long long ll;
static const ll INF = (1LL << 60);
static const int KMAX = 512;                 // nearest shelters examined per city

#define CK(call) do { cudaError_t e_ = (call); if (e_ != cudaSuccess) { \
    fprintf(stderr, "CUDA error %s at %s:%d\n", cudaGetErrorString(e_), __FILE__, __LINE__); exit(1); } } while (0)

template <class T> static T* dalloc(size_t n) { T* p = nullptr; CK(cudaMalloc(&p, (n ? n : 1) * sizeof(T))); return p; }
template <class T> static thrust::device_ptr<T> dp(T* p) { return thrust::device_pointer_cast(p); }

// ------------------------------------------------------------------------------------------------
// Graph building
// ------------------------------------------------------------------------------------------------
__global__ void k_edge_keys(int m, int n, const int* eu, const int* ev, ll* key, int* idx)
{
    int e = blockIdx.x * blockDim.x + threadIdx.x;
    if (e >= m) return;
    int lo = min(eu[e], ev[e]), hi = max(eu[e], ev[e]);
    key[e] = (lo == hi) ? LLONG_MAX : (ll)lo * n + hi;     // self loops are dropped
    idx[e] = e;
}

// after a stable sort, the first element of every key group is the first road in input order
__global__ void k_keep_first(int m, const ll* key, const int* idx, int* keep)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= m) return;
    keep[idx[i]] = (key[i] != LLONG_MAX && (i == 0 || key[i] != key[i - 1])) ? 1 : 0;
}

__global__ void k_directed(int m, int n, const int* keep, const int* pos, const int* eu, const int* ev,
                           const int* el, const int* ec, ll* key2, int* len2, int* cap2, int* perm)
{
    int e = blockIdx.x * blockDim.x + threadIdx.x;
    if (e >= m || !keep[e]) return;
    int p = pos[e] * 2;
    key2[p]     = (ll)eu[e] * n + ev[e]; len2[p]     = el[e]; cap2[p]     = ec[e]; perm[p]     = p;
    key2[p + 1] = (ll)ev[e] * n + eu[e]; len2[p + 1] = el[e]; cap2[p + 1] = ec[e]; perm[p + 1] = p + 1;
}

__global__ void k_csr_fill(int m2, int n, const ll* key2, const int* perm, const int* len2, const int* cap2,
                           int* rowCnt, int* adjTo, int* adjLen, int* adjCap)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= m2) return;
    ll k = key2[i];
    int src = (int)(k / n), dst = (int)(k % n);
    atomicAdd(&rowCnt[src], 1);
    adjTo[i] = dst; adjLen[i] = len2[perm[i]]; adjCap[i] = cap2[perm[i]];
}

// ------------------------------------------------------------------------------------------------
// Dijkstra with an indexed binary heap, one thread per populated city
// ------------------------------------------------------------------------------------------------
struct Graph {
    int n;
    const int *rowPtr, *adjTo, *adjLen, *adjCap;
    const signed char* isSh;
};

__device__ __forceinline__ bool nodeLess(const ll* d, int a, int b) { return d[a] < d[b] || (d[a] == d[b] && a < b); }

__device__ void sift_up(int* h, int* pos, const ll* d, int i)
{
    int v = h[i];
    while (i > 0) {
        int p = (i - 1) >> 1;
        if (!nodeLess(d, v, h[p])) break;
        h[i] = h[p]; pos[h[i]] = i; i = p;
    }
    h[i] = v; pos[v] = i;
}

__device__ void sift_down(int* h, int* pos, const ll* d, int hs, int i)
{
    int v = h[i];
    for (;;) {
        int c = 2 * i + 1;
        if (c >= hs) break;
        if (c + 1 < hs && nodeLess(d, h[c + 1], h[c])) c++;
        if (!nodeLess(d, h[c], v)) break;
        h[i] = h[c]; pos[h[i]] = i; i = c;
    }
    h[i] = v; pos[v] = i;
}

// Shared Dijkstra. Stops when stopAfterShelters shelters were settled (and, if needNonShelter, the nearest
// non-shelter city was settled too). Settling order is deterministic, so both passes see the same order.
template <bool TREE>
__device__ void dijkstra(const Graph& g, int src, int prime, int emax, int stopAfterShelters, bool needNonShelter,
                         ll* d, int* h, int* pos, int* par, int* dep,
                         int* outSh, ll* outD, ll* outW, int* outCnt, int* nonShelter)
{
    const int n = g.n;
    for (int i = 0; i < n; i++) { d[i] = INF; pos[i] = -1; }
    int hs = 0, shelters = 0, firstNon = -1, prevSh = -1;
    ll cum = 0;
    d[src] = 0;
    if (TREE) { par[src] = -1; dep[src] = 0; }
    h[hs] = src; pos[src] = hs; hs++;
    while (hs > 0) {
        int u = h[0];
        hs--;
        if (hs > 0) { h[0] = h[hs]; pos[h[0]] = 0; sift_down(h, pos, d, hs, 0); }
        pos[u] = -2;
        if (g.isSh[u]) {
            if (outSh) {
                outSh[shelters] = u; outD[shelters] = d[u];
                // Elderly chain: the shelters an elderly person can still reach if the group walks, in order,
                // through the shortest-path tree from one chain member to the next (full-group costs, so this is
                // an upper bound on what the evaluator will really count). outW = walked ticks, or -1 if out.
                ll cand;
                if (prevSh < 0) cand = d[u];
                else {
                    int x = prevSh, y = u;
                    while (dep[x] > dep[y]) x = par[x];
                    while (dep[y] > dep[x]) y = par[y];
                    while (x != y) { x = par[x]; y = par[y]; }
                    cand = cum + d[prevSh] + d[u] - 2 * d[x];
                }
                if (cand <= emax) { cum = cand; prevSh = u; outW[shelters] = cand; }
                else outW[shelters] = -1;
            }
            shelters++;
        } else if (firstNon < 0) firstNon = u;
        if (shelters >= stopAfterShelters && (!needNonShelter || firstNon >= 0)) break;
        for (int e = g.rowPtr[u]; e < g.rowPtr[u + 1]; e++) {
            int v = g.adjTo[e];
            if (pos[v] == -2) continue;
            ll w = (ll)g.adjLen[e] * max(1, (prime + g.adjCap[e] - 1) / g.adjCap[e]);
            ll nd = d[u] + w;
            if (nd < d[v]) {
                d[v] = nd;
                if (TREE) { par[v] = u; dep[v] = dep[u] + 1; }
                if (pos[v] == -1) { h[hs] = v; pos[v] = hs; hs++; sift_up(h, pos, d, hs - 1); }
                else sift_up(h, pos, d, pos[v]);
            }
        }
    }
    if (outCnt) *outCnt = shelters;
    if (nonShelter) *nonShelter = firstNon;
}

__global__ void k_klist(Graph g, int K, int emax, int chunkStart, int chunkSize, const int* popCity, const int* popP,
                        ll* dist, int* heap, int* pos, int* parA, int* depA, int* klSh, ll* klD, ll* klW, int* klCnt)
{
    int t = blockIdx.x * blockDim.x + threadIdx.x;
    if (t >= chunkSize) return;
    int ci = chunkStart + t;
    size_t o = (size_t)t * g.n;
    int cnt = 0;
    dijkstra<true>(g, popCity[ci], popP[ci], emax, K, false, dist + o, heap + o, pos + o, parA + o, depA + o,
                   klSh + (size_t)ci * K, klD + (size_t)ci * K, klW + (size_t)ci * K, &cnt, nullptr);
    klCnt[ci] = cnt;
}

// ------------------------------------------------------------------------------------------------
// Capacity-aware assignment, one round = every unfinished city reserves capacity at its next shelter.
// Elderly are served first (only where the evaluator would still count them), then prime-age.
// ------------------------------------------------------------------------------------------------
__global__ void k_scarcity(int numPop, int K, const int* popE, const ll* klW, const int* klSh, const int* klCnt, const int* shCap, ll* key, int* order)
{
    int ci = blockIdx.x * blockDim.x + threadIdx.x;
    if (ci >= numPop) return;
    ll cap = 0;
    for (int i = 0; i < klCnt[ci]; i++) if (klW[(size_t)ci * K + i] >= 0) cap += shCap[klSh[(size_t)ci * K + i]];
    // elderly per unit of reachable capacity: the scarcest cities get served first
    key[ci] = popE[ci] == 0 ? LLONG_MAX : (ll)(1000000.0 * (double)popE[ci] / (double)(cap + 1));
    key[ci] = -key[ci];
    order[ci] = ci;
}

__global__ void k_prime_key(int numPop, const int* popP, ll* key, int* order)
{
    int ci = blockIdx.x * blockDim.x + threadIdx.x;
    if (ci >= numPop) return;
    key[ci] = -(ll)popP[ci];                     // biggest groups first: they have the longest walks
    order[ci] = ci;
}

__global__ void k_assign(int phase, int numPop, int first, int count, const int* order, int K, int emax, const int* klSh, const ll* klD, const ll* klW, const int* klCnt,
                         const int* shCap, int* used, int* remP, int* remE, int* ptr, int* gP, int* gE, int* active)
{
    int t = blockIdx.x * blockDim.x + threadIdx.x;
    if (t >= count) return;
    int ci = order[first + t];
    // phase 0 places elderly (only possible near home), phase 1 places prime-age (can go anywhere)
    int need = phase == 0 ? remE[ci] : remP[ci];
    int i = ptr[ci], cnt = klCnt[ci];
    const int* sh = klSh + (size_t)ci * K;
    const ll* dd = klD + (size_t)ci * K;
    const ll* ww = klW + (size_t)ci * K;
    while (i < cnt) {
        if (need == 0 || (phase == 0 && dd[i] > emax)) { i = cnt; break; }   // nothing more can be placed
        if ((phase == 1 || ww[i] >= 0) && used[sh[i]] < shCap[sh[i]]) break; // usable and still has room
        i++;
    }
    if (i >= cnt) { ptr[ci] = cnt; return; }
    int s = sh[i];
    int old = atomicAdd(&used[s], need);
    int room = old < shCap[s] ? shCap[s] - old : 0;
    int granted = min(need, room);
    if (phase == 0) { gE[(size_t)ci * K + i] += granted; remE[ci] = need - granted; }
    else            { gP[(size_t)ci * K + i] += granted; remP[ci] = need - granted; }
    ptr[ci] = i + 1;
    atomicAdd(active, 1);
}

// ------------------------------------------------------------------------------------------------
// Walk construction. Stops are visited in this order: the "dump" stop (a non-shelter city that takes whoever
// could not be placed), then the shelters in increasing distance. Between stops the walk follows the
// shortest-path tree (up to the lowest common ancestor and down again). The walk ends at the stop where the
// last prime-age person is dropped, so nobody is left on board once the evaluator considers the city done.
// ------------------------------------------------------------------------------------------------
// ticks needed to move a group with `prem` prime-age people along the road u-v (evaluator cost model)
__device__ ll road_ticks(const Graph& g, int u, int v, int prem)
{
    int lo = g.rowPtr[u], hi = g.rowPtr[u + 1] - 1;           // adjacency is sorted by destination
    while (lo < hi) { int mid = (lo + hi) >> 1; if (g.adjTo[mid] < v) lo = mid + 1; else hi = mid; }
    int cap = g.adjCap[lo];
    int batches = prem <= 0 ? 1 : (prem + cap - 1) / cap;
    return (ll)g.adjLen[lo] * batches;
}

struct StopCtx {
    int ci, K, cnt, home, dump;                 // dump >= 0: non-shelter city that takes the leftovers
    int leftP, leftE, last, mergeSlot;          // last: last used slot in walk order (-1 only dump, -2 home, -3 none)
    bool dumpEntry, dumpFirst, merge;           // merge: leftovers are added to slot mergeSlot's drop
};

// Stops are visited in this order: the dump stop (a non-shelter city that takes whoever could not be placed),
// then every shelter that takes elderly (nearest first, all inside the elderly chain), then the shelters that
// only take prime-age people. Between stops the walk follows the shortest-path tree (up to the lowest common
// ancestor and down again). The walk ends at the last stop that drops anybody.
template <class F>
__device__ void for_each_stop(const StopCtx& c, const int* klSh, const int* gP, const int* gE, F stop)
{
    if (c.last == -3) return;
    if (c.last == -2) { stop(c.home, c.leftP, c.leftE); return; }
    if (c.dumpEntry && c.dumpFirst) stop(c.dump, c.leftP, c.leftE);
    for (int pass = 0; pass < 2; pass++)
        for (int i = 0; i < c.cnt; i++) {
            int p = gP[(size_t)c.ci * c.K + i], e = gE[(size_t)c.ci * c.K + i];
            if (p + e == 0 || (pass == 0) != (e > 0)) continue;
            if (c.merge && i == c.mergeSlot) { p += c.leftP; e += c.leftE; }
            stop(klSh[(size_t)c.ci * c.K + i], p, e);
        }
    if (c.dumpEntry && !c.dumpFirst) stop(c.dump, c.leftP, c.leftE);
}

// Replays the plan the way the evaluator will: travel ticks per road, the elderly limit, the rule that a drop is only
// executed while somebody who still counts is on board, and the rule that a city is finished (for good) as soon as
// nobody who counts is left on board. Returns false if the plan would hang the evaluator or leave people behind.
__device__ bool plan_valid(const Graph& g, const int* par, const int* dep, const StopCtx& c, const int* klSh,
                           const int* gP, const int* gE, int emax, int prime0, int elderly0)
{
    ll dist = 0; int prem = prime0, eld = elderly0, cur = c.home;
    bool ok = true, done = false;
    for_each_stop(c, klSh, gP, gE, [&](int node, int p, int e) {
        if (!ok) return;
        if (done) { ok = false; return; }
        if (node != cur) {
            int x = cur, y = node;
            while (dep[x] > dep[y]) { dist += road_ticks(g, x, par[x], prem); x = par[x]; }
            while (dep[y] > dep[x]) { dist += road_ticks(g, y, par[y], prem); y = par[y]; }
            while (x != y) { dist += road_ticks(g, x, par[x], prem) + road_ticks(g, y, par[y], prem); x = par[x]; y = par[y]; }
        }
        cur = node;
        if (prem + (dist <= emax ? eld : 0) <= 0) { ok = false; return; }   // drop would not be executed
        prem -= p; eld -= e;
        if (prem < 0 || eld < 0) { ok = false; return; }
        if (prem + (dist <= emax ? eld : 0) <= 0) done = true;
    });
    return ok && prem == 0 && eld == 0;
}

__device__ StopCtx make_ctx(const Graph& g, const int* par, const int* dep, int ci, int K, int emax,
                            const int* klSh, const int* klCnt, const int* popCity, const int* popP, const int* popE,
                            const int* gP, const int* gE, const int* remP, const int* remE, int nonShelter)
{
    const signed char* isSh = g.isSh;
    StopCtx c;
    c.ci = ci; c.K = K; c.cnt = klCnt[ci]; c.home = popCity[ci];
    c.leftP = remP[ci]; c.leftE = remE[ci];
    c.dump = -1; c.dumpEntry = false; c.dumpFirst = false; c.merge = false; c.mergeSlot = -1;
    bool left = (c.leftP + c.leftE) > 0;
    // last / first used slot in walk order: prime-only slots come after the elderly ones
    int lastPrimeOnly = -1, lastElderly = -1, firstElderly = -1, firstPrimeOnly = -1;
    for (int i = 0; i < c.cnt; i++) {
        int p = gP[(size_t)ci * K + i], e = gE[(size_t)ci * K + i];
        if (e > 0) { lastElderly = i; if (firstElderly < 0) firstElderly = i; }
        else if (p > 0) { lastPrimeOnly = i; if (firstPrimeOnly < 0) firstPrimeOnly = i; }
    }
    int lastSlot = lastPrimeOnly >= 0 ? lastPrimeOnly : lastElderly;
    int firstSlot = firstElderly >= 0 ? firstElderly : firstPrimeOnly;
    c.last = lastSlot >= 0 ? lastSlot : -3;
    if (!left) return c;

    if (!isSh[c.home]) {                         // home is not a shelter: leave the leftovers there at t = 0
        c.dump = c.home; c.dumpEntry = true; c.dumpFirst = true;
        if (lastSlot < 0) c.last = -1;
        return c;
    }
    // Home is a shelter. Try, in order of preference, until the replay says the evaluator will accept the plan:
    //  1 only elderly left over and the last stop is already beyond their limit: drop them there (not counted)
    //  2 take them to the nearest non-shelter city (first if no prime-age people are left over, otherwise last)
    //  3 overflow the last stop      4 overflow the first stop
    for (int option = 1; option <= 4; option++) {
        StopCtx t = c;
        if (option == 1) { if (c.leftP != 0 || lastSlot < 0) continue; t.merge = true; t.mergeSlot = lastSlot; }
        else if (option == 2) {
            if (nonShelter < 0) continue;
            t.dump = nonShelter; t.dumpEntry = true; t.dumpFirst = (c.leftP == 0);
            if (lastSlot < 0) t.last = -1;
        }
        else if (option == 3) { t.merge = true; t.mergeSlot = lastSlot; if (lastSlot < 0) t.last = -2; }
        else { t.merge = true; t.mergeSlot = firstSlot; if (firstSlot < 0) t.last = -2; }
        if (option >= 3 && t.last == -2) return t;
        if (plan_valid(g, par, dep, t, klSh, gP, gE, emax, popP[ci], popE[ci])) return t;
        if (option == 4) return t;
    }
    return c;
}

__device__ int lca_leg(const int* par, const int* dep, int a, int b, int* upCnt, int* downCnt)
{
    int x = a, y = b;
    while (dep[x] > dep[y]) x = par[x];
    while (dep[y] > dep[x]) y = par[y];
    while (x != y) { x = par[x]; y = par[y]; }
    *upCnt = dep[a] - dep[x]; *downCnt = dep[b] - dep[x];
    return x;
}

template <bool WRITE>
__global__ void k_walk(Graph g, int K, int emax, int chunkStart, int chunkSize, const int* popCity, const int* popP, const int* popE,
                       ll* dist, int* heap, int* pos, int* parA, int* depA,
                       const int* klSh, const int* klCnt, const int* gP, const int* gE,
                       const int* remP, const int* remE,
                       int* pathLen, int* dropCnt, int* nonShelterArr,
                       const int* pathOff, const int* dropOff, int* walk, int* drops)
{
    int t = blockIdx.x * blockDim.x + threadIdx.x;
    if (t >= chunkSize) return;
    int ci = chunkStart + t;
    size_t o = (size_t)t * g.n;
    int* par = parA + o; int* dep = depA + o;

    if (!WRITE) {
        // pass 1 of the walk: rebuild the tree far enough to reach every stop, then measure the walk
        int lastUsed = -1; bool left = (remP[ci] + remE[ci]) > 0;
        for (int i = 0; i < klCnt[ci]; i++) if (gP[(size_t)ci * K + i] + gE[(size_t)ci * K + i] > 0) lastUsed = i;
        bool needNon = left && g.isSh[popCity[ci]];
        int stopAt = lastUsed + 1;
        int nonShelter = -1;
        dijkstra<true>(g, popCity[ci], popP[ci], emax, stopAt > 0 ? stopAt : 0, needNon, dist + o, heap + o, pos + o,
                       par, dep, nullptr, nullptr, nullptr, nullptr, &nonShelter);
        nonShelterArr[ci] = nonShelter;
    }
    int nonShelter = nonShelterArr[ci];
    StopCtx c = make_ctx(g, par, dep, ci, K, emax, klSh, klCnt, popCity, popP, popE, gP, gE, remP, remE, nonShelter);

    if (!WRITE) {
        int len = 1, nd = 0, cur = c.home;
        for_each_stop(c, klSh, gP, gE, [&](int node, int p, int e) {
            if (node != cur) { int up, down; lca_leg(par, dep, cur, node, &up, &down); len += up + down; }
            cur = node; nd++;
        });
        pathLen[ci] = len; dropCnt[ci] = nd;
    } else {
        int* w = walk + pathOff[ci];
        int* dr = drops + (size_t)dropOff[ci] * 3;
        int wp = 0, dn = 0, cur = c.home;
        w[wp++] = c.home;
        for_each_stop(c, klSh, gP, gE, [&](int node, int p, int e) {
            if (node != cur) {
                int up, down;
                int l = lca_leg(par, dep, cur, node, &up, &down);
                int v = cur;
                for (int k = 0; k < up; k++) { v = par[v]; w[wp++] = v; }
                int x = node;
                for (int k = down - 1; k >= 0; k--) { w[wp + k] = x; x = par[x]; }
                wp += down; (void)l;
            }
            cur = node;
            dr[dn * 3] = node; dr[dn * 3 + 1] = p; dr[dn * 3 + 2] = e; dn++;
        });
    }
}

// ------------------------------------------------------------------------------------------------
// Host
// ------------------------------------------------------------------------------------------------
static std::vector<int> readInts(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    std::string buf(sz, '\0');
    if (fread(&buf[0], 1, sz, f) != (size_t)sz) { fprintf(stderr, "read error\n"); exit(1); }
    fclose(f);
    std::vector<int> v; v.reserve(sz / 4);
    const char* p = buf.c_str(); const char* end = p + sz;
    while (p < end) {
        while (p < end && (*p < '0' || *p > '9') && *p != '-') p++;
        if (p >= end) break;
        bool neg = (*p == '-'); if (neg) p++;
        long x = 0;
        while (p < end && *p >= '0' && *p <= '9') x = x * 10 + (*p++ - '0');
        v.push_back((int)(neg ? -x : x));
    }
    return v;
}

int main(int argc, char** argv)
{
    if (argc != 3) { printf("Usage: %s <input_file> <output_file>\n", argv[0]); return 1; }
    std::vector<int> in = readInts(argv[1]);
    size_t q = 0;
    int n = in[q++], m = in[q++];
    std::vector<int> eu(m), ev(m), el(m), ec(m);
    for (int i = 0; i < m; i++) { eu[i] = in[q++]; ev[i] = in[q++]; el[i] = in[q++]; ec[i] = in[q++]; }
    int ns = in[q++];
    std::vector<int> shCapH(n, 0); std::vector<signed char> isShH(n, 0);
    for (int i = 0; i < ns; i++) { int c = in[q++], cp = in[q++]; isShH[c] = 1; shCapH[c] = cp; }
    int np = in[q++];
    std::vector<int> pCity(np), pP(np), pE(np);
    for (int i = 0; i < np; i++) { pCity[i] = in[q++]; pP[i] = in[q++]; pE[i] = in[q++]; }
    int emax = in[q++];

    const int TB = 256;
    auto grid = [&](size_t c) { return (int)((c + TB - 1) / TB); };

    // ---- upload
    int *d_eu = dalloc<int>(m), *d_ev = dalloc<int>(m), *d_el = dalloc<int>(m), *d_ec = dalloc<int>(m);
    CK(cudaMemcpy(d_eu, eu.data(), m * sizeof(int), cudaMemcpyHostToDevice));
    CK(cudaMemcpy(d_ev, ev.data(), m * sizeof(int), cudaMemcpyHostToDevice));
    CK(cudaMemcpy(d_el, el.data(), m * sizeof(int), cudaMemcpyHostToDevice));
    CK(cudaMemcpy(d_ec, ec.data(), m * sizeof(int), cudaMemcpyHostToDevice));
    int* d_shCap = dalloc<int>(n); signed char* d_isSh = dalloc<signed char>(n);
    CK(cudaMemcpy(d_shCap, shCapH.data(), n * sizeof(int), cudaMemcpyHostToDevice));
    CK(cudaMemcpy(d_isSh, isShH.data(), n, cudaMemcpyHostToDevice));
    int *d_popCity = dalloc<int>(np), *d_popP = dalloc<int>(np), *d_popE = dalloc<int>(np);
    CK(cudaMemcpy(d_popCity, pCity.data(), np * sizeof(int), cudaMemcpyHostToDevice));
    CK(cudaMemcpy(d_popP, pP.data(), np * sizeof(int), cudaMemcpyHostToDevice));
    CK(cudaMemcpy(d_popE, pE.data(), np * sizeof(int), cudaMemcpyHostToDevice));

    // ---- graph: keep the first road of every city pair, directed CSR sorted by (src,dst)
    ll* d_key = dalloc<ll>(m); int* d_idx = dalloc<int>(m); int* d_keep = dalloc<int>(m); int* d_pos = dalloc<int>(m);
    k_edge_keys<<<grid(m), TB>>>(m, n, d_eu, d_ev, d_key, d_idx);
    thrust::stable_sort_by_key(dp(d_key), dp(d_key) + m, dp(d_idx));
    k_keep_first<<<grid(m), TB>>>(m, d_key, d_idx, d_keep);
    thrust::exclusive_scan(dp(d_keep), dp(d_keep) + m, dp(d_pos));
    int lastPos = 0, lastKeep = 0;
    if (m > 0) { CK(cudaMemcpy(&lastPos, d_pos + m - 1, sizeof(int), cudaMemcpyDeviceToHost));
                 CK(cudaMemcpy(&lastKeep, d_keep + m - 1, sizeof(int), cudaMemcpyDeviceToHost)); }
    int mk = lastPos + lastKeep, m2 = 2 * mk;
    ll* d_key2 = dalloc<ll>(m2); int *d_len2 = dalloc<int>(m2), *d_cap2 = dalloc<int>(m2), *d_perm = dalloc<int>(m2);
    k_directed<<<grid(m), TB>>>(m, n, d_keep, d_pos, d_eu, d_ev, d_el, d_ec, d_key2, d_len2, d_cap2, d_perm);
    thrust::stable_sort_by_key(dp(d_key2), dp(d_key2) + m2, dp(d_perm));
    int *d_rowPtr = dalloc<int>(n + 1), *d_adjTo = dalloc<int>(m2), *d_adjLen = dalloc<int>(m2), *d_adjCap = dalloc<int>(m2);
    CK(cudaMemset(d_rowPtr, 0, (n + 1) * sizeof(int)));
    k_csr_fill<<<grid(m2), TB>>>(m2, n, d_key2, d_perm, d_len2, d_cap2, d_rowPtr, d_adjTo, d_adjLen, d_adjCap);
    thrust::exclusive_scan(dp(d_rowPtr), dp(d_rowPtr) + n + 1, dp(d_rowPtr));
    Graph g{n, d_rowPtr, d_adjTo, d_adjLen, d_adjCap, d_isSh};
    cudaFree(d_eu); cudaFree(d_ev); cudaFree(d_el); cudaFree(d_ec); cudaFree(d_key); cudaFree(d_idx);
    cudaFree(d_keep); cudaFree(d_pos); cudaFree(d_key2); cudaFree(d_len2); cudaFree(d_cap2); cudaFree(d_perm);

    // ---- chunked scratch for the per-city Dijkstra runs
    size_t perThread = (size_t)n * (sizeof(ll) + 4 * sizeof(int));
    size_t freeB = 0, totalB = 0; CK(cudaMemGetInfo(&freeB, &totalB));
    size_t budget = std::min<size_t>((size_t)1200000000, freeB / 2);          // scratch for the Dijkstra runs
    size_t chunk = budget / perThread;
    chunk = std::max<size_t>(64, std::min<size_t>(chunk, 8192));
    chunk = std::min<size_t>(chunk, np);
    ll* d_dist = dalloc<ll>(chunk * n); int* d_heap = dalloc<int>(chunk * n); int* d_hpos = dalloc<int>(chunk * n);
    int* d_par = dalloc<int>(chunk * n); int* d_dep = dalloc<int>(chunk * n);

    int K = std::min(KMAX, std::max(ns, 1));
    int* d_klSh = dalloc<int>((size_t)np * K); ll* d_klD = dalloc<ll>((size_t)np * K); ll* d_klW = dalloc<ll>((size_t)np * K); int* d_klCnt = dalloc<int>(np);
    CK(cudaMemset(d_klCnt, 0, np * sizeof(int)));

    // ---- stage A: nearest shelters of every city
    for (size_t s = 0; s < (size_t)np; s += chunk) {
        int cs = (int)std::min(chunk, np - s);
        k_klist<<<grid(cs), TB>>>(g, K, emax, (int)s, cs, d_popCity, d_popP, d_dist, d_heap, d_hpos, d_par, d_dep, d_klSh, d_klD, d_klW, d_klCnt);
    }
    CK(cudaDeviceSynchronize());

    // ---- stage B: capacity-aware assignment in parallel rounds
    int *d_used = dalloc<int>(n), *d_remP = dalloc<int>(np), *d_remE = dalloc<int>(np), *d_ptr = dalloc<int>(np);
    int *d_gP = dalloc<int>((size_t)np * K), *d_gE = dalloc<int>((size_t)np * K), *d_active = dalloc<int>(1);
    CK(cudaMemset(d_used, 0, n * sizeof(int))); CK(cudaMemset(d_ptr, 0, np * sizeof(int)));
    CK(cudaMemset(d_gP, 0, (size_t)np * K * sizeof(int))); CK(cudaMemset(d_gE, 0, (size_t)np * K * sizeof(int)));
    CK(cudaMemcpy(d_remP, d_popP, np * sizeof(int), cudaMemcpyDeviceToDevice));
    CK(cudaMemcpy(d_remE, d_popE, np * sizeof(int), cudaMemcpyDeviceToDevice));
    ll* d_skey = dalloc<ll>(np); int* d_order = dalloc<int>(np);
    k_scarcity<<<grid(np), TB>>>(np, K, d_popE, d_klW, d_klSh, d_klCnt, d_shCap, d_skey, d_order);
    thrust::stable_sort_by_key(dp(d_skey), dp(d_skey) + np, dp(d_order));   // most elderly per reachable capacity first
    ll* d_pkey = dalloc<ll>(np); int* d_order2 = dalloc<int>(np);
    k_prime_key<<<grid(np), TB>>>(np, d_popP, d_pkey, d_order2);
    thrust::stable_sort_by_key(dp(d_pkey), dp(d_pkey) + np, dp(d_order2));
    for (int phase = 0; phase < 2; phase++) {
        CK(cudaMemset(d_ptr, 0, np * sizeof(int)));
        const int* ord = phase == 0 ? d_order : d_order2;
        int batch = std::max(1, np / 32);                                   // priority batches
        for (int first = 0; first < np; first += batch) {
            int count = std::min(batch, np - first);
            for (int round = 0; round < K + 2; round++) {
                CK(cudaMemset(d_active, 0, sizeof(int)));
                k_assign<<<grid(count), TB>>>(phase, np, first, count, ord, K, emax, d_klSh, d_klD, d_klW, d_klCnt,
                                              d_shCap, d_used, d_remP, d_remE, d_ptr, d_gP, d_gE, d_active);
                int act = 0; CK(cudaMemcpy(&act, d_active, sizeof(int), cudaMemcpyDeviceToHost));
                if (act == 0) break;
            }
        }
    }

    // ---- stage C: walks and drops, chunk by chunk
    int *d_pathLen = dalloc<int>(np), *d_dropCnt = dalloc<int>(np), *d_nonSh = dalloc<int>(np);
    int *d_pathOff = dalloc<int>(np), *d_dropOff = dalloc<int>(np);
    std::vector<int> pathOffH(np + 1, 0), dropOffH(np + 1, 0);
    size_t walkCap = 1 << 20, dropCap = 1 << 18;
    int* d_walk = dalloc<int>(walkCap); int* d_drops = dalloc<int>(dropCap * 3);
    size_t walkTot = 0, dropTot = 0;
    for (size_t s = 0; s < (size_t)np; s += chunk) {
        int cs = (int)std::min(chunk, np - s);
        k_walk<false><<<grid(cs), TB>>>(g, K, emax, (int)s, cs, d_popCity, d_popP, d_popE, d_dist, d_heap, d_hpos, d_par, d_dep,
                                        d_klSh, d_klCnt, d_gP, d_gE, d_remP, d_remE, d_pathLen, d_dropCnt, d_nonSh,
                                        d_pathOff, d_dropOff, d_walk, d_drops);
        CK(cudaDeviceSynchronize());
        std::vector<int> pl(cs), dc(cs);
        CK(cudaMemcpy(pl.data(), d_pathLen + s, cs * sizeof(int), cudaMemcpyDeviceToHost));
        CK(cudaMemcpy(dc.data(), d_dropCnt + s, cs * sizeof(int), cudaMemcpyDeviceToHost));
        size_t wBase = walkTot, dBase = dropTot;
        for (int i = 0; i < cs; i++) {
            pathOffH[s + i] = (int)walkTot; walkTot += pl[i];
            dropOffH[s + i] = (int)dropTot; dropTot += dc[i];
        }
        if (walkTot > walkCap) { size_t nc = std::max(walkTot, walkCap * 2); int* nb = dalloc<int>(nc);
            CK(cudaMemcpy(nb, d_walk, wBase * sizeof(int), cudaMemcpyDeviceToDevice)); cudaFree(d_walk); d_walk = nb; walkCap = nc; }
        if (dropTot > dropCap) { size_t nc = std::max(dropTot, dropCap * 2); int* nb = dalloc<int>(nc * 3);
            CK(cudaMemcpy(nb, d_drops, dBase * 3 * sizeof(int), cudaMemcpyDeviceToDevice)); cudaFree(d_drops); d_drops = nb; dropCap = nc; }
        CK(cudaMemcpy(d_pathOff + s, pathOffH.data() + s, cs * sizeof(int), cudaMemcpyHostToDevice));
        CK(cudaMemcpy(d_dropOff + s, dropOffH.data() + s, cs * sizeof(int), cudaMemcpyHostToDevice));
        k_walk<true><<<grid(cs), TB>>>(g, K, emax, (int)s, cs, d_popCity, d_popP, d_popE, d_dist, d_heap, d_hpos, d_par, d_dep,
                                       d_klSh, d_klCnt, d_gP, d_gE, d_remP, d_remE, d_pathLen, d_dropCnt, d_nonSh,
                                       d_pathOff, d_dropOff, d_walk, d_drops);
        CK(cudaDeviceSynchronize());
    }
    pathOffH[np] = (int)walkTot; dropOffH[np] = (int)dropTot;

    // ---- output: np lines of paths, then np lines of drops
    std::vector<int> walkH(walkTot), dropsH(dropTot * 3);
    if (walkTot) CK(cudaMemcpy(walkH.data(), d_walk, walkTot * sizeof(int), cudaMemcpyDeviceToHost));
    if (dropTot) CK(cudaMemcpy(dropsH.data(), d_drops, dropTot * 3 * sizeof(int), cudaMemcpyDeviceToHost));
    std::string out; out.reserve((walkTot + dropTot * 3) * 7 + np * 2);
    char tmp[16];
    auto put = [&](int x, char sep) { int l = snprintf(tmp, sizeof tmp, "%d", x); out.append(tmp, l); out.push_back(sep); };
    for (int i = 0; i < np; i++)
        for (int j = pathOffH[i]; j < pathOffH[i + 1]; j++) put(walkH[j], j + 1 < pathOffH[i + 1] ? ' ' : '\n');
    for (int i = 0; i < np; i++) {
        int a = dropOffH[i], b = dropOffH[i + 1];
        if (a == b) out.push_back('\n');
        for (int j = a; j < b; j++) {
            put(dropsH[j * 3], ' '); put(dropsH[j * 3 + 1], ' '); put(dropsH[j * 3 + 2], j + 1 < b ? ' ' : '\n');
        }
    }
    FILE* fo = fopen(argv[2], "w");
    fwrite(out.data(), 1, out.size(), fo);
    fclose(fo);
    return 0;
}
