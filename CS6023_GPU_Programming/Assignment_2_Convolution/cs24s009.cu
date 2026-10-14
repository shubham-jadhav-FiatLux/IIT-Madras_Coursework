#include <chrono>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>

using namespace std;

using std::cin;
using std::cout;

typedef long long ll;

#define TILE 32
#define MAX_FILTER_ELEMS 4096                       // K*R*S*C <= 4096 (assignment constraint)
#define MAX_DYN_SMEM 48000                          // default per-block shared-memory budget in bytes

__constant__ long int cFilter[MAX_FILTER_ELEMS];    // 32 KB, broadcast to the whole warp on read


// Tiled convolution: one 32x32 block computes a 32x32 patch of one output map (blockIdx.z).
// For every input channel the block cooperatively loads the (32+R-1)x(32+S-1) halo tile into
// shared memory with coalesced row-wise loads, then every thread reads its R*S taps from it.
// No thread returns before a barrier; out-of-range threads only skip their final store.
__global__ void dkernel(const long int *matrix, long int *result, int h, int w, int c, int r, int s)
{
    extern __shared__ long int tile[];

    const int row = blockIdx.y * TILE + threadIdx.y;
    const int col = blockIdx.x * TILE + threadIdx.x;
    const int filterNumber = blockIdx.z;

    const int padR = r / 2, padS = s / 2;
    const int tileH = TILE + r - 1, tileW = TILE + s - 1;
    const int tid = threadIdx.y * TILE + threadIdx.x;

    long int sum = 0;

    for (int ch = 0; ch < c; ch++) {

        const long int *plane = matrix + (long)ch * h * w;

        for (int idx = tid; idx < tileH * tileW; idx += TILE * TILE) {
            int ty = idx / tileW, tx = idx % tileW;
            int gy = blockIdx.y * TILE + ty - padR;
            int gx = blockIdx.x * TILE + tx - padS;
            tile[idx] = (gy >= 0 && gy < h && gx >= 0 && gx < w) ? plane[(long)gy * w + gx] : 0;
        }
        __syncthreads();

        const long int *f = cFilter + (filterNumber * c + ch) * r * s;
        for (int i = 0; i < r; i++)
            for (int j = 0; j < s; j++)
                sum += tile[(threadIdx.y + i) * tileW + threadIdx.x + j] * f[i * s + j];

        __syncthreads();                            // tile is reused by the next channel
    }

    if (row < h && col < w)
        result[(long)filterNumber * h * w + (long)row * w + col] = sum;
}


// Fallback for filters so large that the halo tile does not fit in shared memory:
// same arithmetic, taps read straight from global memory (still coalesced along x).
__global__ void dkernelLarge(const long int *matrix, long int *result, int h, int w, int c, int r, int s)
{
    const int row = blockIdx.y * TILE + threadIdx.y;
    const int col = blockIdx.x * TILE + threadIdx.x;
    if (row >= h || col >= w) return;

    const int filterNumber = blockIdx.z;
    long int sum = 0;
    for (int ch = 0; ch < c; ch++) {
        const long int *f = cFilter + (filterNumber * c + ch) * r * s;
        for (int i = 0; i < r; i++)
            for (int j = 0; j < s; j++) {
                int y = row + i - r / 2, x = col + j - s / 2;
                if (y >= 0 && y < h && x >= 0 && x < w)
                    sum += matrix[(long)ch * h * w + (long)y * w + x] * f[i * s + j];
            }
    }
    result[(long)filterNumber * h * w + (long)row * w + col] = sum;
}


int main(int argc, char **argv)
{
    int h, w, c;
    cin >> h >> w >> c;
    long int *h_mat = new long int[h * w * c];
    for (long int i = 0; i < h * w * c; i++)
    {
        cin >> h_mat[i];
    }

    int cf, r, s, k;
    cin >> cf >> r >> s >> k;

    long int *h_filter = new long int[r * s * c * k];
    for (long int i = 0; i < r * s * c * k; i++)
    {
        cin >> h_filter[i];
    }
    long int *h_ans = new long int[h * w * k];

    /**
     *
     * DO NOT CHANGE ANYTHING ABOVE THIS LINE
     *
     **/

    auto start = std::chrono::high_resolution_clock::now(); // keep it just before the kernel launch

    /****************************************************Start Here***********************************************************/

        
   // ----------------------------------------------------------------------------------------------------------------------------------------------------

    // Device Allocations
    long int *gpuMat, *gpuAns;

    cudaMalloc(&gpuMat, h * w * c * sizeof(long int));
    cudaMalloc(&gpuAns, h * w * k * sizeof(long int));

    // Host to Device Copy (filters go to constant memory)
    cudaMemcpy(gpuMat, h_mat, h * w * c * sizeof(long int), cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(cFilter, h_filter, r * s * c * k * sizeof(long int));

    dim3 blockDim(TILE, TILE);
    dim3 gridDim((w + TILE - 1) / TILE, (h + TILE - 1) / TILE, k);
    size_t smemBytes = (size_t)(TILE + r - 1) * (TILE + s - 1) * sizeof(long int);

    // Kernel Launch
    if (smemBytes <= MAX_DYN_SMEM)
        dkernel<<< gridDim, blockDim, smemBytes >>>(gpuMat, gpuAns, h, w, c, r, s);
    else
        dkernelLarge<<< gridDim, blockDim >>>(gpuMat, gpuAns, h, w, c, r, s);

    // Device to Host Copy
    cudaMemcpy(h_ans, gpuAns, h * w * k * sizeof(long int), cudaMemcpyDeviceToHost);

    // cudaFree to free up the allocated memory on GPU
    cudaFree(gpuAns);
    cudaFree(gpuMat);
    
    
   // ----------------------------------------------------------------------------------------------------------------------------------------------------

    /*$$$$$$$$$$$$$$$$$$$$$$$$Make sure your final output from the device is stored in h_ans.$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
    auto end = std::chrono::high_resolution_clock::now(); // keep it just after the kernel launch
    std::chrono::duration<double> elapsed1 = end - start;
    /**
     *
     * DO NOT CHANGE ANYTHING BELOW THIS LINE
     *
     */

    cudaDeviceSynchronize();
    std::ofstream file("cuda.out");
    if (file.is_open())
    {
        for (long int i = 0; i < h * k; i++)
        {
            for (long int j = 0; j < w; j++)
            {
                file << h_ans[i * w + j] << " ";
            }
            file << "\n";
        }
        file.close();
    }
    else
    {
        std::cout << "Unable to open file";
    }

    std::ofstream file2("cuda_timing.out");
    if (file2.is_open())
    {
        file2 << elapsed1.count() << "\n";
        file2.close();
    }
    else
    {
        std::cout << "Unable to open file";
    }

    return 0;
}
