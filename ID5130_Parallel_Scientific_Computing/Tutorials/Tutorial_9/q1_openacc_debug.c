/*
Author      : Shubham
Date        : 09/04/2026
Description : This program computes the percentage of infected people in each city.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N    100
#define PROBZERO 33

int main() {
   int a[N][N];     // cities X localities
   int x[N],        // sum of infected people for all the localities of a city
       y[N];        // number of localities in a city

   srand(time(NULL));

   // Initialization should be done sequentially since rand() is not thread-safe on devices
   // #pragma acc parallel
   for (int ii = 0; ii < N; ++ii) {
       x[ii] = 0;
       y[ii] = 0;
       for (int jj = 0; jj < N; ++jj) {
           a[ii][jj] = rand() % N; 
           if (rand() % 100 < PROBZERO) a[ii][jj] = 0; 
       }
   }

   // #pragma acc parallel collapse(2) reduction   
   #pragma acc data copyin(a) copyout(x, y)
   {
       #pragma acc parallel loop
       for (int ii = 0; ii < N; ++ii) {
           int loc_sum = 0;
           int loc_count = 0;

           for (int jj = 0; jj < N; ++jj) {
               loc_sum += a[ii][jj];
               if (a[ii][jj] > 0) loc_count++;
           }

           x[ii] = loc_sum;
           y[ii] = loc_count;
       }
   }

   for (int ii = 0; ii < N; ++ii) {
        if (y[ii] > 0)				     		// check for non zero denominator
            printf("%.0f ", x[ii] * 100.0 / (y[ii] * N));
        else 
            printf("0 ");
   }
   printf("\n");

   return 0;
}
