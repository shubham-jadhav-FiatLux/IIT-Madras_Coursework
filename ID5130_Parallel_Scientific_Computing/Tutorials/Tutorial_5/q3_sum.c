/*
Author      : Shubham
Date        : 26/03/2026
Description : This program computes the sum of all values in a vector using MPI.
*/

#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    float my_value = (float)(rank + 1) * 1.5f; // Each process has a distinct value
    float sum = 0.0f;

    if (rank == 0) {
        sum += my_value;
        for (int i = 1; i < size; i++) {
            float recv_val;
            MPI_Recv(&recv_val, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += recv_val;
        }
        printf("The sum of all values is %f\n", sum);
    } else {
        MPI_Send(&my_value, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
