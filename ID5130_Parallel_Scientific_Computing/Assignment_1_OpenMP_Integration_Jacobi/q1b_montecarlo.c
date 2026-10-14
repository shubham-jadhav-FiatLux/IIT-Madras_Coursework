// Question 1(b): Triple integral using Monte Carlo Method - Serial & OpenMP Parallel 

// Author:  Shubham Jadhav
// Date:    10 March, 2026

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

#ifdef _OPENMP
#include<omp.h>
#endif

/* The integrand function */
double func(double x, double y, double z)
{
  return (4.0*x*x*x + x*y*y + 5.0*y + y*z + 6.0*z);
}

/* Generate a random double in [low, high] using rand_r for thread safety */
double random_double(double low, double high, unsigned int *seed)
{
  return low + (high - low) * ((double)rand_r(seed) / (double)RAND_MAX);
}

int main(int argc, char* argv[])
{
  double a_x, b_x, a_y, b_y, a_z, b_z; /* integration limits */
  double Volume;
  double analytical = 2040.0;
  int thread_count = 1;
  int N_values[] = {100, 1000, 10000, 100000, 1000000};
  int num_tests = 5;
  int t;

  if (argc == 2)
    {
      thread_count = strtol(argv[1], NULL, 10);
    }
  else
    {
      printf("\n A command line argument other than name of the executable is required...exiting the program..\n");
      return 1;
    }

  /* Integration limits */
  a_x = 0.0; b_x = 4.0;
  a_y = 0.0; b_y = 3.0;
  a_z = 0.0; b_z = 2.0;
  Volume = (b_x - a_x) * (b_y - a_y) * (b_z - a_z); /* V = 4*3*2 = 24 */

  printf("\n==========================================================");
  printf("\n  Monte Carlo Integration of Triple Integral");
  printf("\n==========================================================");
  printf("\n  Volume V = %.1f", Volume);
  printf("\n  Analytical result = %f", analytical);
  printf("\n  Number of threads for parallel = %d", thread_count);
  printf("\n==========================================================\n");

  printf("\n  %10s | %16s %12s %12s %12s | %16s %12s %12s %12s | %10s",
	 "N", "Serial Result", "Error(%)", "V*f_rms", "Time(s)",
	 "Parallel Result", "Error(%)", "V*f_rms", "Time(s)", "Speedup");
  printf("\n  ----------+----------------------------------------------------------+----------------------------------------------------------+----------");

  for(t = 0; t < num_tests; t++)
    {
      long N = N_values[t];
      long l;
      double x, y, z, fval;
      double f_sum, f2_sum;
      double f_bar, f2_bar, f_rms;
      double serial_result, serial_error_est;
      double parallel_result, parallel_error_est;
      double serial_time = 0.0, parallel_time = 0.0;

      /* ---- SERIAL VERSION ---- */
      f_sum = 0.0;
      f2_sum = 0.0;
      unsigned int seed = 42; /* fixed seed for reproducibility */

#ifdef _OPENMP
      serial_time = omp_get_wtime();
#endif

      for(l = 0; l < N; l++)
	{
	  x = random_double(a_x, b_x, &seed);
	  y = random_double(a_y, b_y, &seed);
	  z = random_double(a_z, b_z, &seed);
	  fval = func(x, y, z);
	  f_sum += fval;
	  f2_sum += fval * fval;
	}

      f_bar = f_sum / N;
      f2_bar = f2_sum / N;
      f_rms = sqrt((f2_bar - f_bar * f_bar) / N);

      serial_result = Volume * f_bar;
      serial_error_est = Volume * f_rms;  /* V * f_rms from Eq. (3) */

#ifdef _OPENMP
      serial_time = omp_get_wtime() - serial_time;
#endif

      /* ---- PARALLEL VERSION ---- */
      f_sum = 0.0;
      f2_sum = 0.0;

#ifdef _OPENMP
      parallel_time = omp_get_wtime();
#endif

#pragma omp parallel num_threads(thread_count) \
  private(l, x, y, z, fval, seed) reduction(+:f_sum, f2_sum)
      {
#ifdef _OPENMP
	/* Each thread gets a unique seed based on its rank */
	seed = 42 + omp_get_thread_num() * 1000;
#else
	seed = 42;
#endif

#pragma omp for
	for(l = 0; l < N; l++)
	  {
	    x = random_double(a_x, b_x, &seed);
	    y = random_double(a_y, b_y, &seed);
	    z = random_double(a_z, b_z, &seed);
	    fval = func(x, y, z);
	    f_sum += fval;
	    f2_sum += fval * fval;
	  }
      }

      f_bar = f_sum / N;
      f2_bar = f2_sum / N;
      f_rms = sqrt((f2_bar - f_bar * f_bar) / N);

      parallel_result = Volume * f_bar;
      parallel_error_est = Volume * f_rms;  /* V * f_rms from Eq. (3) */

#ifdef _OPENMP
      parallel_time = omp_get_wtime() - parallel_time;
#endif

      double speedup = 0.0;
#ifdef _OPENMP
      if(parallel_time > 0.0)
	speedup = serial_time / parallel_time;
#endif

      printf("\n  %10ld | %16.6f %11.4f%% %12.6f %12.6f | %16.6f %11.4f%% %12.6f %12.6f | %10.3f",
	     N,
	     serial_result,
	     100.0 * fabs(serial_result - analytical) / analytical,
	     serial_error_est,
	     serial_time,
	     parallel_result,
	     100.0 * fabs(parallel_result - analytical) / analytical,
	     parallel_error_est,
	     parallel_time,
	     speedup);
    }

  printf("\n\n  Monte Carlo result: I = V*f_bar +/- V*f_rms  [Eq. (3)]");
  printf("\n  V*f_rms is the statistical error estimate; it decreases as 1/sqrt(N).");
  printf("\n  Parallel uses different random sequences per thread,");
  printf("\n  so serial and parallel results differ slightly but both converge.\n");
  printf("\n==========================================================\n\n");

  return 0;
}
