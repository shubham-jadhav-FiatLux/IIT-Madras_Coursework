/*
Author      : Shubham
Date        : 09/04/2026
Description : This program computes the matrix multiplication of two matrices using OpenACC.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 512

void matmul_serial(double A[N][N], double B[N][N], double C[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void matmul_openacc(double A[N][N], double B[N][N], double C[N][N]) {
    #pragma acc data copyin(A[0:N][0:N], B[0:N][0:N]) copyout(C[0:N][0:N])
    {
        #pragma acc parallel loop collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                double sum = 0.0;
                #pragma acc loop seq
                for (int k = 0; k < N; k++) {
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
        }
    }
}

int main() {
    static double A[N][N], B[N][N];
    static double C_serial[N][N], C_acc[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (double)rand() / RAND_MAX;
            B[i][j] = (double)rand() / RAND_MAX;
        }
    }

    printf("Computing with Serial...\n");
    matmul_serial(A, B, C_serial);

    printf("Computing with OpenACC...\n");
    matmul_openacc(A, B, C_acc);

    double max_err = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double err = fabs(C_serial[i][j] - C_acc[i][j]);
            if (err > max_err) max_err = err;
        }
    }

    printf("Max error between OpenACC and Serial results: %e\n", max_err);
    if (max_err < 1e-9) {
        printf("Verification SUCCESS!\n");
    } else {
        printf("Verification FAILED!\n");
    }

    return 0;
}
