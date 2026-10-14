#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXSIZE 1000

void multiplyMatrix(int matrix[][MAXSIZE], int vector[][1], int product[][1], int);

int main(){

	int N;
	printf("Enter dimention of matrix: ");
	scanf("%d", &N);

	int matrix[MAXSIZE][MAXSIZE];
	int vector[MAXSIZE][1];
	int product[MAXSIZE][1];

	//clock_t start = clock();
	// Initialize Matrix and Vector with random values
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			matrix[ii][jj] = rand() % 10 + 1;
		}
		vector[ii][0] = rand() % 10 + 1;
	}

	clock_t start = clock();

	// Multiply matrix and vector
	multiplyMatrix(matrix, vector, product, N);

	clock_t end = clock();

	double total_time = ((double)(end - start))/CLOCKS_PER_SEC;

	// Print Matrix
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < 1; jj++){
			printf("%d ",product[ii][jj]);
		}
		printf("\n");
	}

	printf("For N = %d, Time taken in sec = %f sec\n", N, total_time);

}

void multiplyMatrix(int matrix[][MAXSIZE], int vector[][1], int product[][1], int N){

	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < 1; jj++){
		
			for (int k = 0; k < N; k++){
			
				product[ii][0] += matrix[ii][k]*vector[k][0];
			}
		}
	}
}
