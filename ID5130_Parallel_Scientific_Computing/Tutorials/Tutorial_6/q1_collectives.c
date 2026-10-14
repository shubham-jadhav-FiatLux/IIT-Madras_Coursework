#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int my_val = rank + 1;
    int sum_reduce = 0;
    int sum_allreduce = 0;

    // 1. MPI_Reduce
    MPI_Reduce(&my_val, &sum_reduce, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("MPI_Reduce on rank 0: sum = %d\n", sum_reduce);
    }

    // 2. MPI_Allreduce
    MPI_Allreduce(&my_val, &sum_allreduce, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    printf("MPI_Allreduce on rank %d: sum = %d\n", rank, sum_allreduce);

    // 3. MPI_Scatter
    int *sendbuf = NULL;
    int recv_scatter;
    if (rank == 0) {
        sendbuf = (int*)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            sendbuf[i] = (i + 1) * 10;
        }
    }
    MPI_Scatter(sendbuf, 1, MPI_INT, &recv_scatter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("MPI_Scatter on rank %d: received %d\n", rank, recv_scatter);

    // 4. MPI_Gather
    int *gatherbuf = NULL;
    if (rank == 0) {
        gatherbuf = (int*)malloc(size * sizeof(int));
    }
    int gather_send = rank * 5;
    MPI_Gather(&gather_send, 1, MPI_INT, gatherbuf, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("MPI_Gather on rank 0: ");
        for (int i = 0; i < size; i++) printf("%d ", gatherbuf[i]);
        printf("\n");
        free(gatherbuf);
        free(sendbuf);
    }

    // 5. MPI_Allgather
    int *allgatherbuf = (int*)malloc(size * sizeof(int));
    int allgather_send = rank * 2;
    MPI_Allgather(&allgather_send, 1, MPI_INT, allgatherbuf, 1, MPI_INT, MPI_COMM_WORLD);
    printf("MPI_Allgather on rank %d: ", rank);
    for (int i = 0; i < size; i++) printf("%d ", allgatherbuf[i]);
    printf("\n");
    free(allgatherbuf);

    MPI_Finalize();
    return 0;
}
