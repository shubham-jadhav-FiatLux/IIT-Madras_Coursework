/*
Author      : Shubham
Date        : 26/03/2026
*/

#include <stdio.h>

#define N 100

int main(){

	int mynumber = 10;
	int numbers[N] = {80,70,60,50,10};
	int flag = -1;
	int i = 0;

//#pragma acc parallel
#pragma acc parallel loop
    	for (i = 0; i < N; i++)
        	//if (numbers[i] == mynumber) break;
		if (numbers[i] == mynumber) flag = i;

	//if (i != N) printf("Number not found \n");
	//else printf("Number found at location %d \n", i);

	if (flag == -1) printf("Number not found \n");
	else printf("Number found at location %d \n", flag);

	return 0;
}
