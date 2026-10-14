// Author  : Shubham Jadhav
// Date    : 18 April, 2026
// Cholesky Decomposition: Serial + OpenACC V1 (Column-Oriented)

/*==========================================================================
 V1 strategy:
   For each column jj (sequential):
     Step A: GPU reduction to compute diagonal L(jj,jj)
     Step B: gang-parallel loop over rows ii > jj, each computing L(ii,jj)
   Matrix transferred once in / once out via acc data region.
============================================================================*/

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

static double wall_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

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
 * OpenACC V1: Column-oriented
 *   - Matrix transferred once in/out via acc data region
 *   - Step A: scalar reduction on GPU for diagonal
 *   - Step B: gang-vector parallel over rows (independent per row)
 *   - Kernel launches: 2 * N total
 * ================================================================ */
void cholesky_acc_v1(TYPE *a, int n)
{
    int ii, jj, kk;
    TYPE s, diag, inv_diag;
    int ngangs = (n < 256) ? 32 : 256;

#pragma acc data copy(a[0:n*n])
    {
        for (jj = 0; jj < n; ++jj) {
            /* Step A: diagonal reduction */
            s = 0.0f;
#pragma acc parallel loop reduction(+:s) present(a[0:n*n]) \
    num_gangs(ngangs) vector_length(128)
            for (kk = 0; kk < jj; ++kk)
                s += A(jj,kk) * A(jj,kk);

            diag = sqrtf(A(jj,jj) - s);
            A(jj,jj) = diag;
            inv_diag = (diag > SMALLVAL ? 1.0f / diag : 1.0f);

#pragma acc update device(a[jj*n + jj : 1])

            /* Step B: below-diagonal column, parallel over rows */
#pragma acc parallel loop gang vector present(a[0:n*n]) \
    num_gangs(ngangs) vector_length(128) private(kk)
            for (ii = jj + 1; ii < n; ++ii) {
                TYPE local_s = 0.0f;
#pragma acc loop seq
                for (kk = 0; kk < jj; ++kk)
                    local_s += A(ii,kk) * A(jj,kk);
                A(ii,jj) = (A(ii,jj) - local_s) * inv_diag;
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
    printf("  Cholesky: Serial vs OpenACC V1 (Column-Oriented)\n");
    printf("======================================================\n");

#ifdef _OPENACC
    printf("  OpenACC enabled. Device type: %d\n", acc_get_device_type());
    printf("  Number of devices: %d\n\n", acc_get_num_devices(acc_get_device_type()));
#else
    printf("  Serial only (no OpenACC)\n\n");
#endif

    csv = fopen("cholesky_v1_timing.csv", "w");
    fprintf(csv, "N,serial_time,v1_time\n");

    for (s = 0; s < ns; s++) {
        int n = sizes[s];
        long nn = (long)n * n;

        TYPE *a_serial = (TYPE*)malloc(nn * sizeof(TYPE));
        TYPE *a_v1     = (TYPE*)malloc(nn * sizeof(TYPE));

        if (!a_serial || !a_v1) {
            printf("  ERROR: malloc failed for N=%d\n", n);
            free(a_serial); free(a_v1);
            continue;
        }

        initmult(a_serial, n);
        memcpy(a_v1, a_serial, nn * sizeof(TYPE));

        printf("------------------------------------------------------\n");
        printf("  N = %d  (%.1f MB)\n", n, nn * sizeof(TYPE) / (1024.0 * 1024.0));
        printf("------------------------------------------------------\n");

        double t0 = wall_time();
        cholesky_serial(a_serial, n);
        double t_serial = wall_time() - t0;
        printf("  Serial     = %10.6f s\n", t_serial);

        t0 = wall_time();
        cholesky_acc_v1(a_v1, n);
        double t_v1 = wall_time() - t0;
        printf("  OpenACC V1 = %10.6f s  (speedup: %.2fx)\n", t_v1, t_serial / t_v1);

        fprintf(csv, "%d,%.6f,%.6f\n", n, t_serial, t_v1);

        if (n <= 2000) {
            float md = max_diff(a_serial, a_v1, n);
            printf("\n  Verification: Max |serial - V1| = %.2e  %s\n",
                   md, md < 1e-4f ? "[PASS]" : "[FAIL]");

            if (n == 10) {
                printf("\n  Serial lower-triangular L:\n");
                printLower(a_serial, n);
                printf("\n  V1 lower-triangular L:\n");
                printLower(a_v1, n);
            }
        }
        printf("\n");
        free(a_serial);
        free(a_v1);
    }

    fclose(csv);
    printf("======================================================\n");
    printf("  Timing written to cholesky_v1_timing.csv\n");
    printf("======================================================\n\n");
    return 0;
}
