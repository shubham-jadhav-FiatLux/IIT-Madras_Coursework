/*
Author      : Shubham
Date        : 19/03/2026
Description : This program demonstrates the use of advanced collective communication operations in MPI.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = MPI_Wtime();
    double tick = MPI_Wtick();
    if (rank == 0) printf("Clock tick resolution: %e\n", tick);

    // Scatterv / Gatherv setup
    int *sendcounts = NULL;
    int *displs = NULL;
    int *sendbuf = NULL;
    int total_elements = 0;

    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            sendcounts[i] = i + 1;
            displs[i] = total_elements;
            total_elements += (i + 1);
        }
        sendbuf = (int*)malloc(total_elements * sizeof(int));
        for (int i = 0; i < total_elements; i++) sendbuf[i] = i * 10;
    }

    int my_count = rank + 1;
    int *recvbuf = (int*)malloc(my_count * sizeof(int));

    MPI_Scatterv(sendbuf, sendcounts, displs, MPI_INT, recvbuf, my_count, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Rank %d received %d elements from Scatterv\n", rank, my_count);

    // Modified data
    for (int i = 0; i < my_count; i++) recvbuf[i] += 1;

    MPI_Gatherv(recvbuf, my_count, MPI_INT, sendbuf, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Gathered array at rank 0: ");
        for (int i = 0; i < total_elements; i++) printf("%d ", sendbuf[i]);
        printf("\n");
        free(sendcounts); free(displs); free(sendbuf);
    }
    free(recvbuf);

    // Alltoall
    int *alltoall_send = (int*)malloc(size * sizeof(int));
    int *alltoall_recv = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) alltoall_send[i] = rank * 100 + i;

    MPI_Alltoall(alltoall_send, 1, MPI_INT, alltoall_recv, 1, MPI_INT, MPI_COMM_WORLD);

    printf("Rank %d Alltoall received: ", rank);
    for (int i = 0; i < size; i++) printf("%d ", alltoall_recv[i]);
    printf("\n");

    free(alltoall_send); free(alltoall_recv);

    double end_time = MPI_Wtime();
    printf("Rank %d execution time: %f seconds\n", rank, end_time - start_time);

    MPI_Finalize();
    return 0;
}
