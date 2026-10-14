/*
Author      : Shubham
Date        : 19/03/2026
Description : This program computes the vector addition of two vectors using OpenACC.
*/

// Compile with: pgcc -acc -Minfo=accel q3_openacc.c
#include <stdio.h>

int main() {
    int N = 100000;
    int a[N], b[N], c[N];

    // Initialize
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }

    #pragma acc parallel loop
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }

    printf("OpenACC Vector Addition Done. c[10] = %d\n", c[10]);
    return 0;
}
