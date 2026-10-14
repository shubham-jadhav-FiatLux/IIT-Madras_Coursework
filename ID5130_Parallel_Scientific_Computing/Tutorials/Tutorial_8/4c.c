/*
Author      : Shubham
Date        : 26/03/2026
Description : This program demonstrates the use of kernels in OpenACC.
*/

#include <stdio.h>

#define N 10

int main(){

	int a[N];

	#pragma acc kernels
	{
    		printf("One \n");
    		for (int i = 0 ; i < 10; ++i)
    		{
        		a[i] = i;
        		printf("first loop %d \n", i);
    		}
    		printf("Two \n");
    		for (int i = 0; i < 10; ++i)
    		{
        		a[i] *= 2;
        		printf("second loop %d \n", i);
    		}
    		printf("Three \n");
	}

	return 0;
}
