/*
Author      : Shubham
Date        : 19/03/2026
Description : This program computes the derivative of a function using MPI.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double u(double x) {
    return x*x*x - sin(5.0 * x);
}

double du_exact(double x) {
    return 3.0*x*x - 5.0 * cos(5.0 * x);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double dx = 0.01; // can change to 0.001
    double L = 3.0;
    int N = (int)(L / dx) + 1;
    
    int local_N = N / size;
    int remainder = N % size;
    if (rank < remainder) local_N++;
    
    int start_idx = 0;
    for (int i = 0; i < rank; i++) {
        start_idx += (N / size) + (i < remainder ? 1 : 0);
    }
    
    // Allocate memory with 2 ghost points on each side for 4th order
    double *f = (double*)malloc((local_N + 4) * sizeof(double));
    double *df1 = (double*)malloc(local_N * sizeof(double)); // 1st order
    double *df2 = (double*)malloc(local_N * sizeof(double)); // 2nd order
    double *df4 = (double*)malloc(local_N * sizeof(double)); // 4th order
    
    // Evaluate f
    for (int i = 0; i < local_N; i++) {
        double x = (start_idx + i) * dx;
        f[i + 2] = u(x); // Offset by 2 for ghost points
    }
    
    // Communicate ghost points
    int left_neighbor = (rank == 0) ? MPI_PROC_NULL : rank - 1;
    int right_neighbor = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;
    
    // Send to left, receive from right
    MPI_Sendrecv(&f[2], 2, MPI_DOUBLE, left_neighbor, 0,
                 &f[local_N + 2], 2, MPI_DOUBLE, right_neighbor, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                 
    // Send to right, receive from left
    MPI_Sendrecv(&f[local_N], 2, MPI_DOUBLE, right_neighbor, 1,
                 &f[0], 2, MPI_DOUBLE, left_neighbor, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                 
    // Fill boundary ghost points where neighbors don't exist
    if (rank == 0) {
        // extrapolated or exact for boundary ghost points
        f[1] = u((start_idx - 1) * dx);
        f[0] = u((start_idx - 2) * dx);
    }
    if (rank == size - 1) {
        f[local_N + 2] = u((start_idx + local_N) * dx);
        f[local_N + 3] = u((start_idx + local_N + 1) * dx);
    }
    
    // Calculate derivatives
    for (int i = 0; i < local_N; i++) {
        int global_i = start_idx + i;
        
        // Forward difference for last element, backward if global_i == N-1
        if (global_i < N - 1) {
            df1[i] = (f[i + 3] - f[i + 2]) / dx; // 1st order forward
        } else {
            df1[i] = (f[i + 2] - f[i + 1]) / dx; // 1st order backward
        }
        
        // 2nd and 4th order central; the ghost layer holds u(x) beyond the
        // domain at both physical ends, so no boundary fallback is needed
        df2[i] = (f[i + 3] - f[i + 1]) / (2.0 * dx);
        df4[i] = (f[i] - 8.0 * f[i + 1] + 8.0 * f[i + 3] - f[i + 4]) / (12.0 * dx);
    }
    
    // Max error of each scheme against the exact derivative
    double local_err[3] = {0.0, 0.0, 0.0}, global_err[3];
    for (int i = 0; i < local_N; i++) {
        double exact = du_exact((start_idx + i) * dx);
        double e1 = fabs(df1[i] - exact);
        double e2 = fabs(df2[i] - exact);
        double e4 = fabs(df4[i] - exact);
        if (e1 > local_err[0]) local_err[0] = e1;
        if (e2 > local_err[1]) local_err[1] = e2;
        if (e4 > local_err[2]) local_err[2] = e4;
    }
    MPI_Reduce(local_err, global_err, 3, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("dx = %.3f\n", dx);
        printf("Max error, 1st order : %e\n", global_err[0]);
        printf("Max error, 2nd order : %e\n", global_err[1]);
        printf("Max error, 4th order : %e\n", global_err[2]);
    }
    
    free(f); free(df1); free(df2); free(df4);
    MPI_Finalize();
    return 0;
}
