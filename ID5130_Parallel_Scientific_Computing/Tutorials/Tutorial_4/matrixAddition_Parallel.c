#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAXSIZE 10000

double matrix1[10000][10000];
double matrix2[10000][10000];
double addition[10000][10000];

int thread_count=8;

void additionMatrix(double matrix1[MAXSIZE][MAXSIZE], double matrix2[MAXSIZE][MAXSIZE], double addition[MAXSIZE][MAXSIZE], int);
void additionMatrixParallel(double matrix1[MAXSIZE][MAXSIZE], double matrix2[MAXSIZE][MAXSIZE], double addition[MAXSIZE][MAXSIZE], int);

int main(int argc, char* argv[]){

	if (argc == 2)
		thread_count = strtol(argv[1], NULL, 10);
	int N;
	printf("Enter dimention of matrix: ");
	scanf("%d", &N);

	//int matrix1[MAXSIZE][MAXSIZE];	// Overflows stackframe memory assigned to main()
	//int matrix2[MAXSIZE][MAXSIZE];
	//int addition[MAXSIZE][MAXSIZE];

	// Initialize Matrix and Vector with random values
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			matrix1[ii][jj] = (double)rand()/RAND_MAX;
			matrix2[ii][jj] = (double)rand()/RAND_MAX;
		}
	}

	clock_t start = clock();

void additionMatrix(double matrix1[MAXSIZE][MAXSIZE], double matrix2[MAXSIZE][MAXSIZE], double addition[MAXSIZE][MAXSIZE], int);
	// Add matrix1 and matrix2
	additionMatrix(matrix1, matrix2, addition, N);

	clock_t end = clock();

	double total_time1 = ((double)(end - start))/CLOCKS_PER_SEC;
	
	start = clock();

	// Add matrix1 and matrix2
	additionMatrixParallel(matrix1, matrix2, addition, N);

	end = clock();

	double total_time2 = ((double)(end - start))/CLOCKS_PER_SEC;

	// Print Matrix
	//for (int ii = 0; ii < N; ii++){
		//for (int jj = 0; jj < N; jj++){
			//printf("%.2f ",addition[ii][jj]);
		//}
		//printf("\n");
	//}


	printf("Serial	: For N = %d, Time taken in sec = %f sec\n", N, total_time1);
	printf("Parallel: For N = %d, Time taken in sec = %f sec\n", N, total_time2);

}

void additionMatrix(double matrix1[MAXSIZE][MAXSIZE], double matrix2[MAXSIZE][MAXSIZE], double addition[MAXSIZE][MAXSIZE], int N){

	//#pragma omp parallel for num_threads(8)
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){	
			addition[ii][jj] = matrix1[ii][jj] + matrix2[ii][jj];
		}
	}
}

void additionMatrixParallel(double matrix1[MAXSIZE][MAXSIZE], double matrix2[MAXSIZE][MAXSIZE], double addition[MAXSIZE][MAXSIZE], int N){

	#pragma omp parallel for num_threads(thread_count)
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){	
			addition[ii][jj] = matrix1[ii][jj] + matrix2[ii][jj];
		}
	}
}
