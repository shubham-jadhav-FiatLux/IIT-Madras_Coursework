#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank, size;
    int n = 8; // matrix size n x n
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (n % size != 0 && rank == 0) {
        printf("Matrix size must be divisible by number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int local_n = n / size;
    double *local_A = (double*)malloc(local_n * n * sizeof(double));
    double *x = (double*)malloc(n * sizeof(double));
    double *local_y = (double*)malloc(local_n * sizeof(double));

    double *A = NULL;
    double *y = NULL;

    if (rank == 0) {
        A = (double*)malloc(n * n * sizeof(double));
        y = (double*)malloc(n * sizeof(double));
        // Initialize A and x
        for (int i = 0; i < n * n; i++) A[i] = i + 1;
        for (int i = 0; i < n; i++) x[i] = 1.0;
    }

    // Scatter A
    MPI_Scatter(A, local_n * n, MPI_DOUBLE, local_A, local_n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // Broadcast x
    MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Compute local matrix-vector multiplication
    for (int i = 0; i < local_n; i++) {
        local_y[i] = 0.0;
        for (int j = 0; j < n; j++) {
            local_y[i] += local_A[i * n + j] * x[j];
        }
    }

    // Gather y
    MPI_Gather(local_y, local_n, MPI_DOUBLE, y, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result y:\n");
        for (int i = 0; i < n; i++) {
            printf("%f\n", y[i]);
        }
        free(A);
        free(y);
    }

    free(local_A);
    free(x);
    free(local_y);
    MPI_Finalize();
    return 0;
}
