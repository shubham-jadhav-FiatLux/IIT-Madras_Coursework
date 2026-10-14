#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define M 1500
#define N 1000

// Using column-major access A(row, col) = A[col * M + row]
#define A_idx(row, col) ((col) * M + (row))
#define R_idx(row, col) ((col) * N + (row))

void init_matrix(float *A) {
    for (int i = 0; i < M * N; i++) {
        A[i] = (float)rand() / RAND_MAX;
    }
}

int main() {
    float *A = (float*)malloc(M * N * sizeof(float));
    float *Q = (float*)malloc(M * N * sizeof(float));
    float *R = (float*)calloc(N * N, sizeof(float));

    init_matrix(A);

    // Copy A to Q
    for (int i = 0; i < M * N; i++) {
        Q[i] = A[i];
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma acc data copyin(A[0:M*N]) copy(Q[0:M*N]) copy(R[0:N*N])
    {
        for (int i = 0; i < N; i++) {
            // R[i, i] = ||Q[:, i]||
            float norm_sq = 0.0f;
            #pragma acc parallel loop reduction(+:norm_sq)
            for (int k = 0; k < M; k++) {
                float val = Q[A_idx(k, i)];
                norm_sq += val * val;
            }
            float norm = sqrtf(norm_sq);
            R[R_idx(i, i)] = norm;

            // Q[:, i] = Q[:, i] / R[i, i]
            #pragma acc parallel loop
            for (int k = 0; k < M; k++) {
                Q[A_idx(k, i)] /= norm;
            }

            // for j = i+1 ... N-1
            #pragma acc parallel loop
            for (int j = i + 1; j < N; j++) {
                float dot = 0.0f;
                // Inner product R[i, j] = Q[:, j]^T Q[:, i]
                #pragma acc loop reduction(+:dot)
                for (int k = 0; k < M; k++) {
                    dot += Q[A_idx(k, j)] * Q[A_idx(k, i)];
                }
                R[R_idx(i, j)] = dot;

                // Q[:, j] -= R[i, j] * Q[:, i]
                #pragma acc loop
                for (int k = 0; k < M; k++) {
                    Q[A_idx(k, j)] -= dot * Q[A_idx(k, i)];
                }
            }
        }
    }

    gettimeofday(&end, NULL);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

    printf("QR Decomposition (M=%d, N=%d) completed in %f seconds\n", M, N, time_taken);

    free(A);
    free(Q);
    free(R);
    return 0;
}
