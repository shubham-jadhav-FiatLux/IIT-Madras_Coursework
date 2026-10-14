/* ==========================================================================
 *  CS6023: GPU Programming  --  Assignment 1 (2026 offering)  --  SOLUTION
 *  "The Wizard's Lens": An Image Preprocessing Pipeline on the GPU
 *
 *  FILE: pipeline.cu
 *
 *  Worked solution. It keeps the exact structure of the released
 *  boilerplate (same kernel signatures, same parser, same printer) and
 *  fills in the nine TODO regions.
 *
 *  Notes on the choices made here are collected at the bottom of the file
 *  under "PERFORMANCE NOTES".
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>

#define CEIL_DIV(n, b) (((n) + (b) - (int)1) / (b))

#define W_RED   0.299f
#define W_GREEN 0.587f
#define W_BLUE  0.114f

/* 256 threads/block is a good default: 8 full warps, low register pressure. */
#define TPB_1D  256
#define TPB_2D  16


/* ==========================================================================
 *  STAGE 1  --  GRAYSCALE      gray = 0.299R + 0.587G + 0.114B
 * ========================================================================== */
__global__ void grayscaleKernel(const unsigned char *__restrict__ d_rgb,
                                float *__restrict__ d_gray,
                                int H, int W)
{
    const int idx = blockIdx.x * blockDim.x + threadIdx.x;
    const int n   = H * W;
    if (idx >= n) return;                    /* boundary guard */

    const int base = 3 * idx;                /* RGB is interleaved */
    const float r = (float)d_rgb[base + 0];
    const float g = (float)d_rgb[base + 1];
    const float b = (float)d_rgb[base + 2];

    /* fmaf keeps the dot product in one fused multiply-add chain: fewer
     * instructions and one less rounding step. */
    d_gray[idx] = fmaf(W_RED, r, fmaf(W_GREEN, g, W_BLUE * b));
}


/* ==========================================================================
 *  STAGE 2  --  BILINEAR RESIZE, align-corners      (H x W) -> (Hr x Wr)
 * ========================================================================== */
__global__ void resizeKernel(const float *__restrict__ d_gray,
                             float *__restrict__ d_resized,
                             int H, int W,
                             int Hr, int Wr)
{
    const int ox = blockIdx.x * blockDim.x + threadIdx.x;   /* column */
    const int oy = blockIdx.y * blockDim.y + threadIdx.y;   /* row    */
    if (ox >= Wr || oy >= Hr) return;

    const float scaleY = (Hr > 1) ? (float)(H - 1) / (float)(Hr - 1) : 0.0f;
    const float scaleX = (Wr > 1) ? (float)(W - 1) / (float)(Wr - 1) : 0.0f;

    const float fy = (float)oy * scaleY;
    const float fx = (float)ox * scaleX;

    const int y0 = (int)floorf(fy);
    const int x0 = (int)floorf(fx);
    const int y1 = min(y0 + 1, H - 1);       /* clamp at bottom/right edge */
    const int x1 = min(x0 + 1, W - 1);

    const float wy = fy - (float)y0;
    const float wx = fx - (float)x0;

    const float *row0 = d_gray + (size_t)y0 * (size_t)W;
    const float *row1 = d_gray + (size_t)y1 * (size_t)W;

    /* The lerp form  a + (b - a)*w  is one FMA; the naive
     * a*(1-w) + b*w  is two multiplies, a subtract and an add. */
    const float top = fmaf(row0[x1] - row0[x0], wx, row0[x0]);
    const float bot = fmaf(row1[x1] - row1[x0], wx, row1[x0]);

    d_resized[(size_t)oy * (size_t)Wr + (size_t)ox] = fmaf(bot - top, wy, top);
}


/* ==========================================================================
 *  STAGE 3  --  CENTER CROP     (Hr x Wr) -> (Hc x Wc)
 * ========================================================================== */
__global__ void cropKernel(const float *__restrict__ d_resized,
                           float *__restrict__ d_cropped,
                           int Hr, int Wr,
                           int Hc, int Wc)
{
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= Wc || y >= Hc) return;

    const int offY = (Hr - Hc) / 2;
    const int offX = (Wr - Wc) / 2;

    d_cropped[(size_t)y * (size_t)Wc + (size_t)x] =
        d_resized[(size_t)(y + offY) * (size_t)Wr + (size_t)(x + offX)];
}


/* ==========================================================================
 *  STAGE 4  --  NORMALIZE       out = (in/255 - mean) / std
 * ========================================================================== */
__global__ void normalizeKernel(const float *__restrict__ d_cropped,
                                float *__restrict__ d_out,
                                int Hc, int Wc,
                                float mean, float stdv)
{
    const int idx = blockIdx.x * blockDim.x + threadIdx.x;
    const int n   = Hc * Wc;
    if (idx >= n) return;

    /* One reciprocal, then a multiply: division is far more expensive. */
    const float invStd = 1.0f / stdv;
    const float scaled = d_cropped[idx] * (1.0f / 255.0f);
    d_out[idx] = (scaled - mean) * invStd;
}


/* ==========================================================================
 *                                  MAIN
 * ========================================================================== */
int main(int argc, char **argv)
{
    /* ------------------------- INPUT PARSING ------------------------- */
    FILE *fin = stdin;
    if (argc >= 2) {
        fin = fopen(argv[1], "r");
        if (!fin) {
            fprintf(stderr, "ERROR: cannot open input file '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    int H, W, Hr, Wr, Hc, Wc;
    float mean, stdv;

    if (fscanf(fin, "%d %d", &H, &W) != 2) {
        fprintf(stderr, "ERROR: malformed input (image dimensions)\n");
        return EXIT_FAILURE;
    }
    if (fscanf(fin, "%d %d", &Hr, &Wr) != 2) {
        fprintf(stderr, "ERROR: malformed input (resize dimensions)\n");
        return EXIT_FAILURE;
    }
    if (fscanf(fin, "%d %d", &Hc, &Wc) != 2) {
        fprintf(stderr, "ERROR: malformed input (crop dimensions)\n");
        return EXIT_FAILURE;
    }
    if (fscanf(fin, "%f %f", &mean, &stdv) != 2) {
        fprintf(stderr, "ERROR: malformed input (mean / std)\n");
        return EXIT_FAILURE;
    }

    if (H <= 0 || W <= 0 || Hr <= 0 || Wr <= 0 || Hc <= 0 || Wc <= 0) {
        fprintf(stderr, "ERROR: dimensions must be positive\n");
        return EXIT_FAILURE;
    }
    if (Hc > Hr || Wc > Wr) {
        fprintf(stderr, "ERROR: crop size must not exceed resize size\n");
        return EXIT_FAILURE;
    }

    const size_t nPixIn   = (size_t)H  * (size_t)W;
    const size_t nRgb     = nPixIn * 3u;
    const size_t nPixRes  = (size_t)Hr * (size_t)Wr;
    const size_t nPixCrop = (size_t)Hc * (size_t)Wc;

    unsigned char *h_rgb = (unsigned char *)malloc(nRgb * sizeof(unsigned char));
    float         *h_out = (float *)calloc(nPixCrop, sizeof(float));
    if (!h_rgb || !h_out) {
        fprintf(stderr, "ERROR: host allocation failed\n");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < nRgb; ++i) {
        int v;
        if (fscanf(fin, "%d", &v) != 1) {
            fprintf(stderr, "ERROR: expected %zu pixel values, got %zu\n", nRgb, i);
            return EXIT_FAILURE;
        }
        h_rgb[i] = (unsigned char)v;
    }
    if (fin != stdin) fclose(fin);
    /* ----------------------- END INPUT PARSING ----------------------- */

    unsigned char *d_rgb     = NULL;
    float         *d_gray    = NULL;
    float         *d_resized = NULL;
    float         *d_cropped = NULL;
    float         *d_out     = NULL;

    /* ------------------------- TODO 5 : ALLOCATE --------------------- */
    cudaMalloc((void **)&d_rgb,     nRgb     * sizeof(unsigned char));
    cudaMalloc((void **)&d_gray,    nPixIn   * sizeof(float));
    cudaMalloc((void **)&d_resized, nPixRes  * sizeof(float));
    cudaMalloc((void **)&d_cropped, nPixCrop * sizeof(float));
    cudaMalloc((void **)&d_out,     nPixCrop * sizeof(float));

    /* ------------------------- TODO 6 : H -> D ----------------------- */
    cudaMemcpy(d_rgb, h_rgb, nRgb * sizeof(unsigned char),
               cudaMemcpyHostToDevice);

    /* ------------------------- TODO 7 : LAUNCH ----------------------- */
    {
        /* Stage 1 : element-wise over H*W pixels -> 1-D grid. */
        const int blocks1 = CEIL_DIV((int)nPixIn, TPB_1D);
        grayscaleKernel<<<blocks1, TPB_1D>>>(d_rgb, d_gray, H, W);

        /* Stage 2 : 2-D output image -> 2-D grid, 16x16 = 256 threads. */
        const dim3 block2(TPB_2D, TPB_2D);
        const dim3 gridResize(CEIL_DIV(Wr, TPB_2D), CEIL_DIV(Hr, TPB_2D));
        resizeKernel<<<gridResize, block2>>>(d_gray, d_resized, H, W, Hr, Wr);

        /* Stage 3 : 2-D again, sized to the cropped output. */
        const dim3 gridCrop(CEIL_DIV(Wc, TPB_2D), CEIL_DIV(Hc, TPB_2D));
        cropKernel<<<gridCrop, block2>>>(d_resized, d_cropped, Hr, Wr, Hc, Wc);

        /* Stage 4 : element-wise again. */
        const int blocks4 = CEIL_DIV((int)nPixCrop, TPB_1D);
        normalizeKernel<<<blocks4, TPB_1D>>>(d_cropped, d_out, Hc, Wc, mean, stdv);
    }

    /* ------------------------- TODO 8 : D -> H ----------------------- *
     * cudaMemcpy is synchronous with respect to the default stream, so it
     * also acts as the barrier that waits for the kernels to finish.     */
    cudaMemcpy(h_out, d_out, nPixCrop * sizeof(float), cudaMemcpyDeviceToHost);

    /* ------------------------ OUTPUT PRINTING ------------------------ */
    printf("%d %d\n", Hc, Wc);
    for (int y = 0; y < Hc; ++y) {
        for (int x = 0; x < Wc; ++x) {
            printf("%.6f%c", h_out[(size_t)y * (size_t)Wc + (size_t)x],
                   (x == Wc - 1) ? '\n' : ' ');
        }
    }
    fflush(stdout);

    /* --------------------------- TODO 9 : FREE ----------------------- */
    cudaFree(d_rgb);
    cudaFree(d_gray);
    cudaFree(d_resized);
    cudaFree(d_cropped);
    cudaFree(d_out);

    free(h_rgb);
    free(h_out);
    return EXIT_SUCCESS;
}

/* ==========================================================================
 *  PERFORMANCE NOTES  (for the post-deadline walkthrough)
 * --------------------------------------------------------------------------
 *  1. Thread mapping. Every kernel maps one thread to one *output* element
 *     (a "gather" pattern). Each output is written exactly once by exactly
 *     one thread, so no atomics and no race conditions, and the writes are
 *     perfectly coalesced.
 *
 *  2. Boundary guards. H*W is almost never a multiple of 256, so the last
 *     block always contains idle threads. The `if (idx >= n) return;` guard
 *     is not optional - without it you write past the end of the buffer.
 *     This is the single most common bug in this assignment.
 *
 *  3. Coalescing. In the 2-D kernels, threadIdx.x maps to the *column*.
 *     Consecutive threads in a warp then touch consecutive addresses in a
 *     row, which the memory controller merges into a few wide transactions.
 *     Swapping x and y here typically costs a 5-10x slowdown.
 *
 *  4. __restrict__ and const. Promising the compiler that input and output
 *     do not alias lets it keep values in registers and, on sm_35+, route
 *     read-only loads through the texture cache.
 *
 *  5. Error checking. Omitted here to keep the first assignment readable.
 *     Production code should check every CUDA call: cudaMalloc can fail,
 *     and a kernel launch reports errors only through cudaGetLastError().
 *     A silent launch failure looks exactly like a page of zeros.
 *
 *  6. What we deliberately did NOT do (it belongs to later assignments):
 *       - Fusing all four stages into one kernel. That would remove two full
 *         round trips to global memory. It is easy here because crop and
 *         normalize are pure element-wise maps: a single kernel over the
 *         Hc*Wc output could compute its own source coordinates, sample the
 *         RGB image, convert, interpolate and normalize in registers.
 *         Expect roughly a 2-3x end-to-end speedup on large images.
 *       - Pinned host memory + cudaMemcpyAsync + streams, so that the H2D
 *         copy overlaps with computation.
 *       - Reading the RGB bytes as uchar4 for wider loads.
 *     For any realistic image size, the wall-clock time of this program is
 *     dominated by fscanf() parsing the text input, not by the GPU at all -
 *     itself a useful lesson about where bottlenecks really live.
 * ========================================================================== */
