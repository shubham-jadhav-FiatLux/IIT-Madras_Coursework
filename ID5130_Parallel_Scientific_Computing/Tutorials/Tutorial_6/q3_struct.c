#include <mpi.h>
#include <stdio.h>

typedef struct {
    double a[3];
    double b[2];
    int n;
} MyData;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MyData data;

    if (rank == 0) {
        data.a[0] = 12.67; data.a[1] = 56.45; data.a[2] = 25.32;
        data.b[0] = 56.79; data.b[1] = 98.26;
        data.n = 10;
    }

    MPI_Datatype my_mpi_type;
    int blocklengths[3] = {3, 2, 1};
    MPI_Aint displacements[3];
    MPI_Datatype types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT};

    // Calculate displacements
    MPI_Aint base_address;
    MPI_Get_address(&data, &base_address);
    MPI_Get_address(&data.a, &displacements[0]);
    MPI_Get_address(&data.b, &displacements[1]);
    MPI_Get_address(&data.n, &displacements[2]);

    displacements[0] -= base_address;
    displacements[1] -= base_address;
    displacements[2] -= base_address;

    MPI_Type_create_struct(3, blocklengths, displacements, types, &my_mpi_type);
    MPI_Type_commit(&my_mpi_type);

    MPI_Bcast(&data, 1, my_mpi_type, 0, MPI_COMM_WORLD);

    printf("Rank %d: a = {%f, %f, %f}, b = {%f, %f}, n = %d\n",
           rank, data.a[0], data.a[1], data.a[2], data.b[0], data.b[1], data.n);

    MPI_Type_free(&my_mpi_type);
    MPI_Finalize();
    return 0;
}
