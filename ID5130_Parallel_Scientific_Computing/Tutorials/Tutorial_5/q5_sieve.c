#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 1000;
    int local_n = n / size;
    int local_a = rank * local_n + 1; // 1-based indexing
    if (local_a == 1) local_a = 2; // start from 2
    int local_b = (rank + 1) * local_n;
    if (rank == size - 1) local_b = n; // Handle remainder

    int local_size = local_b - local_a + 1;
    char *marked = (char *)calloc(local_size, sizeof(char));

    int prime = 2;
    int first;
    
    // Quick prime finding array for root
    char *root_primes = NULL;
    if (rank == 0) {
        root_primes = (char *)calloc(sqrt(n) + 2, sizeof(char));
        for (int j = 4; j <= sqrt(n) + 1; j += 2)   // multiples of the first prime
            root_primes[j] = 1;
    }

    do {
        if (prime * prime > local_a) {
            first = prime * prime - local_a;
        } else {
            if (!(local_a % prime))
                first = 0;
            else
                first = prime - (local_a % prime);
        }

        for (int i = first; i < local_size; i += prime) {
            marked[i] = 1;
        }
        
        if (rank == 0) {
            // Find next prime using root_primes; n + 1 ends the loop when none is left
            int next = n + 1;
            for (int i = prime + 1; i <= sqrt(n) + 1; i++) {
                if (!root_primes[i]) {
                    next = i;
                    for (int j = i * i; j <= sqrt(n) + 1; j += i) {
                        root_primes[j] = 1;
                    }
                    break;
                }
            }
            prime = next;
        }
        MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } while (prime * prime <= n);

    int count = 0;
    for (int i = 0; i < local_size; i++) {
        if (!marked[i]) count++;
    }

    int global_count;
    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("There are %d primes less than or equal to %d\n", global_count, n);
        free(root_primes);
    }

    free(marked);
    MPI_Finalize();
    return 0;
}
