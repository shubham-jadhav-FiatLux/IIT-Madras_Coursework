/*
Author      : Shubham
Date        : 26/03/2026
Description : This program demonstrates the use of gangs in OpenACC.
*/

#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
    int ngangs = 1;

    if (argc == 2)
        ngangs = atoi(argv[1]);
    else
    {
        printf(" Run the program as ./a.out 10");
        return 1;
    }

    printf("\n ngangs = %d", ngangs);

    #pragma acc parallel num_gangs(ngangs)
    {
        printf(" Hello world \n");
        printf(" Bye world \n");
    }

    printf("Host \n");

    #pragma acc parallel num_gangs(ngangs/2)
        printf("Second pragma \n");

    return 0;
}
