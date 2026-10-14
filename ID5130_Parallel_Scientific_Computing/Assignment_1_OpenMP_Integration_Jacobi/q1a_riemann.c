// Question 1(a): Triple integral using Riemann Integration - Serial & OpenMP Parallel 

// Author:  Shubham Jadhav
// Date:    10 March, 2026

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#ifdef _OPENMP
#include<omp.h>
#endif

/* The integrand function */
double func(double x, double y, double z)
{
  return (4.0*x*x*x + x*y*y + 5.0*y + y*z + 6.0*z);
}

int main(int argc, char* argv[])
{
  int Nx, Ny, Nz;
  double a_x, b_x, a_y, b_y, a_z, b_z; /* integration limits */
  double dx, dy, dz, dV;
  double x, y, z;
  double serial_result, parallel_result;
  double serial_time, parallel_time;
  double analytical = 2040.0;
  int thread_count = 1;
  int i, j, k;

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

  /* Number of subdivisions as specified */
  Nx = 48; Ny = 36; Nz = 24;

  dx = (b_x - a_x) / Nx;
  dy = (b_y - a_y) / Ny;
  dz = (b_z - a_z) / Nz;
  dV = dx * dy * dz;

  printf("\n==========================================================");
  printf("\n  Riemann Integration of Triple Integral");
  printf("\n==========================================================");
  printf("\n  Nx = %d, Ny = %d, Nz = %d", Nx, Ny, Nz);
  printf("\n  dx = %f, dy = %f, dz = %f", dx, dy, dz);
  printf("\n  Analytical result = %f", analytical);
  printf("\n==========================================================\n");

  /* ---- SERIAL VERSION ---- */
  serial_result = 0.0;

#ifdef _OPENMP
  serial_time = omp_get_wtime();
#endif

  for(i = 0; i < Nx; i++)
    {
      for(j = 0; j < Ny; j++)
	{
	  for(k = 0; k < Nz; k++)
	    {
	      /* Using midpoint rule for better accuracy */
	      x = a_x + (i + 0.5) * dx;
	      y = a_y + (j + 0.5) * dy;
	      z = a_z + (k + 0.5) * dz;
	      serial_result += func(x, y, z) * dV;
	    }
	}
    }

#ifdef _OPENMP
  serial_time = omp_get_wtime() - serial_time;
#endif

  printf("\n  [SERIAL] Riemann Sum       = %.10f", serial_result);
  printf("\n  [SERIAL] Absolute Error    = %.10f", fabs(serial_result - analytical));
  printf("\n  [SERIAL] Relative Error    = %.6f %%", 100.0 * fabs(serial_result - analytical) / analytical);
#ifdef _OPENMP
  printf("\n  [SERIAL] Time              = %f seconds", serial_time);
#endif
  printf("\n");

  /* ---- PARALLEL VERSION ---- */
  parallel_result = 0.0;

#ifdef _OPENMP
  parallel_time = omp_get_wtime();
#endif

#pragma omp parallel for collapse(3) reduction(+:parallel_result) \
  private(i, j, k, x, y, z) num_threads(thread_count)
  for(i = 0; i < Nx; i++)
    {
      for(j = 0; j < Ny; j++)
	{
	  for(k = 0; k < Nz; k++)
	    {
	      x = a_x + (i + 0.5) * dx;
	      y = a_y + (j + 0.5) * dy;
	      z = a_z + (k + 0.5) * dz;
	      parallel_result += func(x, y, z) * dV;
	    }
	}
    }

#ifdef _OPENMP
  parallel_time = omp_get_wtime() - parallel_time;
#endif

  printf("\n  [PARALLEL, p=%d] Riemann Sum    = %.10f", thread_count, parallel_result);
  printf("\n  [PARALLEL, p=%d] Absolute Error = %.10f", thread_count, fabs(parallel_result - analytical));
  printf("\n  [PARALLEL, p=%d] Relative Error = %.6f %%", thread_count, 100.0 * fabs(parallel_result - analytical) / analytical);
#ifdef _OPENMP
  printf("\n  [PARALLEL, p=%d] Time           = %f seconds", thread_count, parallel_time);
  printf("\n  Speedup                         = %f", serial_time / parallel_time);
#endif

  /* Verify serial and parallel produce same result */
  printf("\n\n  Difference (Serial - Parallel) = %.15e", fabs(serial_result - parallel_result));
  if(fabs(serial_result - parallel_result) < 1e-10)
    printf("\n  VERIFICATION: Serial and Parallel produce the SAME output.\n");
  else
    printf("\n  WARNING: Serial and Parallel results differ!\n");

  printf("\n==========================================================\n\n");

  return 0;
}
