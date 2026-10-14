#include <stdio.h>
#include <stdlib.h>

int main(){

	int N;
	printf("Enter size N of matrix: ");
	scanf("%d", &N);

	int A[N][N];
	int ATranspose[N][N];
	int A_plus_ATranspose[N][N];

	// Initialize matrix A with random values
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			A[ii][jj] = rand() % 100;
		}
	}

	// Create A Transpose
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			ATranspose[ii][jj] = A[jj][ii];
		}
	}

	// Add A and A transpose
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			A_plus_ATranspose[ii][jj] = A[ii][jj] + ATranspose[ii][jj]; 
		}
	}

	// Print A plus A Transpose
	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			printf("%3d ", A_plus_ATranspose[ii][jj]);
		}
		printf("\n");
	}

	//  Check for Symmetry
	int isSymmetric = 1;

	for (int ii = 0; ii < N; ii++){
		for (int jj = 0; jj < N; jj++){
			if (A_plus_ATranspose[ii][jj] == A_plus_ATranspose[jj][ii])
				continue;
			else{
				isSymmetric = 0;
				printf("Not Symmetric\n");
			}	
		}
	}
	if (isSymmetric == 1)
		printf("A plus A Transpose is symmetric\n");
	

	return 0;
}
