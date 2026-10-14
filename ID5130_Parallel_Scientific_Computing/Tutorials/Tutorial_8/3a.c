/*
Author      : Shubham
Date        : 26/03/2026
*/

#include <stdio.h>

#define N 10

int main(){

	int a[N] = {0,1};

	#pragma acc serial
    	for (int i = 2 ; i < 10; ++i)
       		a[i] = a[i-1] + a[i-2];

	#pragma acc parallel loop
	for (int i = 0; i < 10; ++i)
		printf("%d\t", a[i]);

	return 0;
}
