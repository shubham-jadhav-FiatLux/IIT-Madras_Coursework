#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAXSIZE 10000

int matrix1[10000][10000];
int matrix2[10000][10000];
int addition[10000][10000];

void additionMatrix(int matrix1[MAXSIZE][MAXSIZE], int matrix2[MAXSIZE][MAXSIZE], int addition[MAXSIZE][MAXSIZE], int);

int main(){

	int N;
	printf("Enter dimention of matrix: ");
	scanf("%d", &N);

	//int matrix1[MAXSIZE][MAXSIZE];
	//int matrix2[MAXSIZE][MAXSIZE];
	//int addition[MAXSIZE][MAXSIZE];

	// Initialize Matrix and Vector with random values
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			matrix1[ii][jj] = rand() % 10 + 1;
			matrix2[ii][jj] = rand() % 10 + 1;
		}
	}

	clock_t start = clock();

	// Add matrix1 and matrix2
	additionMatrix(matrix1, matrix2, addition, N);

	clock_t end = clock();

	double total_time = ((double)(end - start))/CLOCKS_PER_SEC;

	// Print Matrix
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			printf("%d ",addition[ii][jj]);
		}
		printf("\n");
	}

	printf("For N = %d, Time taken in sec = %f sec\n", N, total_time);

}

void additionMatrix(int matrix1[MAXSIZE][MAXSIZE], int matrix2[MAXSIZE][MAXSIZE], int addition[MAXSIZE][MAXSIZE], int N){

	#pragma omp parallel for num_threads(8)
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){	
			addition[ii][jj] = matrix1[ii][jj] + matrix2[ii][jj];
		}
	}
}
