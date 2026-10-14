// Author   :   Shubham Balaso Jadhav
// Date     :   20/03/2025
// Roll No  :   CS24S009

// Header Files
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <climits>
#include <cuda.h>
#include <cuda_runtime.h>

#define MOD 1000000007

// A candidate edge is packed as (weight << IDX_BITS) | edge index so that one atomicMin picks the
// lightest edge and breaks ties by index. 26 index bits allow 67M edges, leaving 38 bits for the weight.
#define IDX_BITS 26
#define IDX_MASK ((1ULL << IDX_BITS) - 1)

struct Edge {
    int src, dest;
    unsigned long long weight;   // already multiplied by the terrain factor (can exceed 32 bits)
};

// Device Functions
__device__ int sbj_find(int *parent, int u) {
    int root = u;
    while (root != parent[root])
        root = parent[root];
    
    while (u != root) {
        int next = parent[u];
        parent[u] = root;
        u = next;
    }
    
    return root;
}

// Kernels

__global__ void sbj_init_parent(int *parent, int V) {

    int id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id < V)
        parent[id] = id;
}


__global__ void sbj_init_min_edge(unsigned long long *min_edge_pair, int V) {

    int id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id < V) {
        min_edge_pair[id] = ~0ULL;   // no candidate edge yet
    }
}


__global__ void sbj_find_min_edges(unsigned long long *min_edge_pair, Edge *edges, int *parent, int E) {

    int id = blockIdx.x * blockDim.x + threadIdx.x;

    if (id < E) {

        Edge e = edges[id];
        int u = sbj_find(parent, e.src);
        int v = sbj_find(parent, e.dest);
        
        if (u == v) return;
        
        unsigned long long pair = (e.weight << IDX_BITS) | (unsigned long long)id;
        
        atomicMin(&min_edge_pair[u], pair);
    
        atomicMin(&min_edge_pair[v], pair);
    }
}


__global__ void sbj_merge_components(unsigned long long *min_edge_pair, Edge *edges, int *parent, unsigned long long *mst_cost, int *changes, int V) {

    int id = blockIdx.x * blockDim.x + threadIdx.x;

    if (id < V) {

        unsigned long long packed = min_edge_pair[id];
        unsigned long long weight = packed >> IDX_BITS;
        unsigned int idx = (unsigned int)(packed & IDX_MASK);
    
        if (packed == ~0ULL) return;
        
        Edge e = edges[idx];
        int u = sbj_find(parent, e.src);
        int v = sbj_find(parent, e.dest);
        
        if (u == v) return;
        
        int high = max(u, v);
        int low = min(u, v);
        
        if (atomicCAS(&parent[high], high, low) == high) {
            __threadfence(); // Ensure parent update visibility
            atomicAdd(mst_cost, weight);
            atomicAdd(changes, 1);   
        }
    }
}

int main() {

    int V, E;
    std::cin >> V >> E;
    
    std::vector<Edge> h_edges;
    for (int i = 0; i < E; i++) {

        int u, v;
        long long wt;
        std::string s = "normal";
        std::cin >> u >> v >> wt;
        
        if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
            std::cin >> s;
        }
        
        int multiplier = (s == "green") ? 2 : (s == "traffic") ? 5 : (s == "dept") ? 3 : 1;
        h_edges.push_back({u, v, (unsigned long long)(wt * multiplier)});
    }
    
    Edge *d_edges;
    int *d_parent, *d_changes;
    unsigned long long *d_mst_cost;
    unsigned long long *d_min_edge_pair;

    cudaMalloc(&d_edges, E * sizeof(Edge));
    cudaMalloc(&d_parent, V * sizeof(int));
    cudaMalloc(&d_min_edge_pair, V * sizeof(unsigned long long));
    cudaMalloc(&d_mst_cost, sizeof(unsigned long long));
    cudaMalloc(&d_changes, sizeof(int));
    
    cudaMemcpy(d_edges, h_edges.data(), E * sizeof(Edge), cudaMemcpyHostToDevice);
    
    cudaMemset(d_mst_cost, 0, sizeof(unsigned long long));
    

//--------------------------------------------Start timing---------------------------------------------

    auto start = std::chrono::high_resolution_clock::now();
    
    int block_size = 256;
    int grid_size = (V + block_size - 1) / block_size;

    sbj_init_parent<<<grid_size, block_size>>>(d_parent, V);
    
    bool done = false;

    while (!done) {
        
        cudaMemset(d_changes, 0, sizeof(int));

        sbj_init_min_edge<<<grid_size, block_size>>>(d_min_edge_pair, V);
        
        grid_size = (E + block_size - 1) / block_size;
        sbj_find_min_edges<<<grid_size, block_size>>>(d_min_edge_pair, d_edges, d_parent, E);
        
        grid_size = (V + block_size - 1) / block_size;
        sbj_merge_components<<<grid_size, block_size>>>(d_min_edge_pair, d_edges, d_parent, d_mst_cost, d_changes, V);
        
        int h_changes;
        cudaMemcpy(&h_changes, d_changes, sizeof(int), cudaMemcpyDeviceToHost);
        if (h_changes == 0) {
            done = true;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();

//-------------------------------------------- End timing------------------------------------------------------

    std::chrono::duration<double> elapsed = end - start;
    //std::cout << std::endl;
    //std::cout << elapsed.count() << " s\n\n";
    

    unsigned long long h_mst_cost;
    cudaMemcpy(&h_mst_cost, d_mst_cost, sizeof(unsigned long long), cudaMemcpyDeviceToHost);
    std::cout << h_mst_cost % MOD << std::endl;

    // Free device memory
    cudaFree(d_edges);
    cudaFree(d_parent);
    cudaFree(d_min_edge_pair);
    cudaFree(d_mst_cost);
    cudaFree(d_changes);
    
    return 0;
}
