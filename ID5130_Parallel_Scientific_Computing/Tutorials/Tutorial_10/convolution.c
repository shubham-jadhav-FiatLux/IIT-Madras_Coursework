// Author	:	Shubham Jadhav
// Date		:	20 April, 2026


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define K 5

void run_convolution(int N) {
    int *A = (int*)malloc(N * sizeof(int));
    int *A_out = (int*)malloc(N * sizeof(int));
    int F[K] = {3, 4, 5, 4, 3};
    int half_K = K / 2;

    for (int i = 0; i < N; i++) {
        A[i] = i + 1; 			// Example inputs: 1, 2, 3, 4, ...
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma acc data copy(A[0:N]) create(A_out[0:N]) copyin(F[0:K])
    {
        #pragma acc parallel loop
        for (int i = 0; i < N; i++) {
            int sum = 0;
            #pragma acc loop seq
            for (int j = -half_K; j <= half_K; j++) {
                int a_idx = i + j;
                int a_val = (a_idx >= 0 && a_idx < N) ? A[a_idx] : 0;
                sum += a_val * F[j + half_K];
            }
            A_out[i] = sum;
        }

        #pragma acc parallel loop
        for (int i = 0; i < N; i++) {
            A[i] = A_out[i];
        }
    }

    gettimeofday(&end, NULL);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

    printf("Convolution (N=%d) completed in %f seconds\n", N, time_taken);

    // Print first 8 elements as example check to match tutorial example
    printf("Output for N=%d (first 8 elements): ", N);
    for (int i = 0; i < 8 && i < N; i++) {
        printf("%d ", A[i]);
    }
    printf("\n\n");

    free(A);
    free(A_out);
}

int main() {
    printf("Testing Convolution Filter\n");
    
    // As per problem statement
    run_convolution(8);
    
    // Test sizes given in problem
    run_convolution(1000);
    run_convolution(2000);
    run_convolution(3000);

    return 0;
}
