/*
Author      : Shubham
Date        : 26/03/2026
Description : This program computes the sum of all values in a vector using OpenACC.
*/

#include <stdio.h>

#define N 10

int main(){

	int a[N] = {0,1,2,3,4,5,6,7,8,9};
	int sum = 0;

	#pragma acc parallel loop private(sum) firstprivate(a)
    	for (int i = 0 ; i < 10; ++i)
       		sum += a[i];

	printf("Sum = %d\n", sum);

	return 0;
}
