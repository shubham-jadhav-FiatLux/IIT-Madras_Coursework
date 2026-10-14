// Author  : Shubham Jadhav
// Date    : 18 April, 2026
// Cholesky Decomposition: Serial + OpenACC V2 (Blocked/Tiled)

/*========================================================================
 V2 strategy (LAPACK-style blocked Cholesky):
   For each block column jb of width NB:
     Phase 1 (POTRF): factor NB×NB diagonal block serially on device
     Phase 2 (TRSM):  solve off-diagonal panel, gang-parallel rows
     Phase 3 (SYRK):  update trailing submatrix, gang+vector parallel
   Only 3 * ceil(N/NB) kernel launches; SYRK is GEMM-scale compute.
===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#ifdef _OPENACC
#include <openacc.h>
#endif

#define TYPE     float
#define SMALLVAL 0.001f
#define A(i,j)   a[(i)*(n) + (j)]

/* Block size tuned for Maxwell cc50: 64 balances SYRK work vs register pressure */
#define NB 64

static double wall_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static inline int imin(int a, int b) { return a < b ? a : b; }

void initmult(TYPE *a, int n)
{
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < n && j < i; j++) {
            A(i,j) = (i + j) / (float)n / (float)n;
            A(j,i) = (i + j) / (float)n / (float)n;
        }
    for (i = 0; i < n; i++)
        A(i,i) = 1.0f;
}

void printLower(TYPE *a, int n)
{
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j <= i; j++)
            printf("%8.4f ", A(i,j));
        printf("\n");
    }
}

/* ================================================================
 * SERIAL Cholesky (column-oriented)
 * ================================================================ */
void cholesky_serial(TYPE *a, int n)
{
    int ii, jj, kk;
    TYPE s;

    for (jj = 0; jj < n; ++jj) {
        s = 0.0f;
        for (kk = 0; kk < jj; ++kk)
            s += A(jj,kk) * A(jj,kk);
        A(jj,jj) = sqrtf(A(jj,jj) - s);

        TYPE inv_diag = (A(jj,jj) > SMALLVAL ? 1.0f / A(jj,jj) : 1.0f);
        for (ii = jj + 1; ii < n; ++ii) {
            s = 0.0f;
            for (kk = 0; kk < jj; ++kk)
                s += A(ii,kk) * A(jj,kk);
            A(ii,jj) = (A(ii,jj) - s) * inv_diag;
        }
    }
}

/* ================================================================
 * OpenACC V2: Blocked / Tiled Cholesky
 *
 * For each block column jb = 0, NB, 2*NB, ...
 *   Phase 1 (POTRF): serial NB×NB Cholesky on device
 *   Phase 2 (TRSM):  gang-parallel rows solve off-diagonal panel
 *   Phase 3 (SYRK):  gang+vector update of trailing submatrix
 *
 * All data stays on device within the acc data region.
 * ================================================================ */
void cholesky_acc_v2(TYPE *restrict a, int n)
{
    int jb, nb;

#pragma acc data copy(a[0:n*n])
    {
        for (jb = 0; jb < n; jb += NB) {
            nb = imin(NB, n - jb);

            /* Phase 1: POTRF — factor diagonal block serially on device */
#pragma acc serial present(a[0:n*n])
            {
                int j, i, k;
                for (j = 0; j < nb; ++j) {
                    TYPE s = 0.0f;
                    for (k = 0; k < j; ++k)
                        s += a[(jb+j)*n + (jb+k)] * a[(jb+j)*n + (jb+k)];
                    TYPE diag = sqrtf(a[(jb+j)*n + (jb+j)] - s);
                    a[(jb+j)*n + (jb+j)] = diag;

                    TYPE inv_d = (diag > SMALLVAL ? 1.0f / diag : 1.0f);
                    for (i = j + 1; i < nb; ++i) {
                        s = 0.0f;
                        for (k = 0; k < j; ++k)
                            s += a[(jb+i)*n + (jb+k)] * a[(jb+j)*n + (jb+k)];
                        a[(jb+i)*n + (jb+j)] = (a[(jb+i)*n + (jb+j)] - s) * inv_d;
                    }
                }
            }

            if (jb + nb >= n) continue;

            /* Phase 2: TRSM — solve off-diagonal panel
             * Rows ii in [jb+nb, n) are gang-parallel (independent).
             * Columns j in [0, nb) are sequential (dependency within block). */
#pragma acc parallel loop gang present(a[0:n*n])
            for (int ii = jb + nb; ii < n; ++ii) {
                int j, k;
                for (j = 0; j < nb; ++j) {
                    TYPE s = 0.0f;
#pragma acc loop seq
                    for (k = 0; k < j; ++k)
                        s += a[ii*n + (jb+k)] * a[(jb+j)*n + (jb+k)];
                    TYPE diag = a[(jb+j)*n + (jb+j)];
                    TYPE inv_d = (diag > SMALLVAL ? 1.0f / diag : 1.0f);
                    a[ii*n + (jb+j)] = (a[ii*n + (jb+j)] - s) * inv_d;
                }
            }

            /* Phase 3: SYRK — update trailing submatrix
             * A[i,j] -= sum_{k=0}^{nb-1} L[i,jb+k] * L[j,jb+k]
             * Outer i-loop: gang-parallel; inner j-loop: vector-parallel. */
#pragma acc parallel loop gang present(a[0:n*n])
            for (int i = jb + nb; i < n; ++i) {
#pragma acc loop vector
                for (int j = jb + nb; j <= i; ++j) {
                    TYPE s = 0.0f;
                    for (int k = 0; k < nb; ++k)
                        s += a[i*n + (jb+k)] * a[j*n + (jb+k)];
                    a[i*n + j] -= s;
                }
            }
        }
    }
}

float max_diff(TYPE *a, TYPE *b, int n)
{
    float mx = 0.0f;
    int k;
    for (k = 0; k < n * n; k++) {
        float d = fabsf(a[k] - b[k]);
        if (d > mx) mx = d;
    }
    return mx;
}

int main(void)
{
    int sizes[] = {10, 100, 1000, 2000, 5000};
    int ns = 5;
    int s;
    FILE *csv;

    printf("\n======================================================\n");
    printf("  Cholesky: Serial vs OpenACC V2 (Blocked/Tiled)\n");
    printf("  Block size NB = %d\n", NB);
    printf("======================================================\n");

#ifdef _OPENACC
    printf("  OpenACC enabled. Device type: %d\n", acc_get_device_type());
    printf("  Number of devices: %d\n\n", acc_get_num_devices(acc_get_device_type()));
#else
    printf("  Serial only (no OpenACC)\n\n");
#endif

    csv = fopen("cholesky_v2_timing.csv", "w");
    fprintf(csv, "N,serial_time,v2_time\n");

    for (s = 0; s < ns; s++) {
        int n = sizes[s];
        long nn = (long)n * n;

        TYPE *a_serial = (TYPE*)malloc(nn * sizeof(TYPE));
        TYPE *a_v2     = (TYPE*)malloc(nn * sizeof(TYPE));

        if (!a_serial || !a_v2) {
            printf("  ERROR: malloc failed for N=%d\n", n);
            free(a_serial); free(a_v2);
            continue;
        }

        initmult(a_serial, n);
        memcpy(a_v2, a_serial, nn * sizeof(TYPE));

        printf("------------------------------------------------------\n");
        printf("  N = %d  (%.1f MB)\n", n, nn * sizeof(TYPE) / (1024.0 * 1024.0));
        printf("------------------------------------------------------\n");

        double t0 = wall_time();
        cholesky_serial(a_serial, n);
        double t_serial = wall_time() - t0;
        printf("  Serial     = %10.6f s\n", t_serial);

        t0 = wall_time();
        cholesky_acc_v2(a_v2, n);
        double t_v2 = wall_time() - t0;
        printf("  OpenACC V2 = %10.6f s  (speedup: %.2fx)\n", t_v2, t_serial / t_v2);

        fprintf(csv, "%d,%.6f,%.6f\n", n, t_serial, t_v2);

        if (n <= 2000) {
            float md = max_diff(a_serial, a_v2, n);
            printf("\n  Verification: Max |serial - V2| = %.2e  %s\n",
                   md, md < 1e-4f ? "[PASS]" : "[FAIL]");

            if (n == 10) {
                printf("\n  Serial lower-triangular L:\n");
                printLower(a_serial, n);
                printf("\n  V2 lower-triangular L:\n");
                printLower(a_v2, n);
            }
        }
        printf("\n");
        free(a_serial);
        free(a_v2);
    }

    fclose(csv);
    printf("======================================================\n");
    printf("  Timing written to cholesky_v2_timing.csv\n");
    printf("======================================================\n\n");
    return 0;
}
