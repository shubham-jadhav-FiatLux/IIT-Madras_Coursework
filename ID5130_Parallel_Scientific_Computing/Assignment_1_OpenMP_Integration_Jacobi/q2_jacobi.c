// Question 2: Poisson Equation Solver using Jacobi Method - Serial & OpenMP Parallel 

// Author:  Shubham Jadhav
// Date:    10 March, 2026


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#ifdef _OPENMP
#include<omp.h>
#endif

/* Macro for 2D indexing into 1D contiguous array */
#define IDX(i,j,cols) ((i)*(cols) + (j))

/* Source term */
double q_func(double x, double y)
{
  return 2.0 * (2.0 - x*x - y*y);
}

/* Exact solution */
double exact_solution(double x, double y)
{
  return (x*x - 1.0) * (y*y - 1.0);
}

/* Allocate contiguous 1D array for (rows x cols) grid, zero-initialized */
double* allocate_grid(int rows, int cols)
{
  double *arr = (double*)calloc(rows * cols, sizeof(double));
  return arr;
}

/* Check convergence: max relative error < tolerance (1%) */
int check_convergence(double *phi, double delta, int N, double tol)
{
  int i, j;
  int cols = N + 1;
  double x, y, exact_val, rel_err, max_rel_err;

  max_rel_err = 0.0;
  for(i = 0; i <= N; i++)
    {
      x = -1.0 + i * delta;
      for(j = 0; j <= N; j++)
	{
	  y = -1.0 + j * delta;
	  exact_val = exact_solution(x, y);

	  if(fabs(exact_val) > 1e-10)
	    rel_err = fabs(phi[IDX(i,j,cols)] - exact_val) / fabs(exact_val);
	  else
	    rel_err = fabs(phi[IDX(i,j,cols)] - exact_val);

	  if(rel_err > max_rel_err)
	    max_rel_err = rel_err;
	}
    }
  return (max_rel_err < tol) ? 1 : 0;
}

/* Pre-compute source term array: delta^2/4 * q(x,y) for all interior points */
double* precompute_source(double delta, int N)
{
  int i, j;
  int cols = N + 1;
  double x, y;
  double delta_sq_over_4 = (delta * delta) / 4.0;
  double *q_arr = allocate_grid(N+1, N+1);

  for(i = 1; i < N; i++)
    {
      x = -1.0 + i * delta;
      for(j = 1; j < N; j++)
	{
	  y = -1.0 + j * delta;
	  q_arr[IDX(i,j,cols)] = delta_sq_over_4 * q_func(x, y);
	}
    }
  return q_arr;
}

/*=========================================================================
 * Serial Jacobi solver
 * Uses pointer swap (no copy) and pre-computed source term.
 * Returns number of iterations.
 *=========================================================================*/
int jacobi_serial(double delta, int N, double *phi_final, int max_iter)
{
  int i, j, iter;
  int cols = N + 1;
  double *phi_old, *phi_new, *temp;
  double *q_arr;

  phi_old = allocate_grid(N+1, N+1);
  phi_new = allocate_grid(N+1, N+1);
  q_arr = precompute_source(delta, N);

  for(iter = 0; iter < max_iter; iter++)
    {
      for(i = 1; i < N; i++)
	{
	  for(j = 1; j < N; j++)
	    {
	      phi_new[IDX(i,j,cols)] =
		0.25 * (phi_old[IDX(i+1,j,cols)] + phi_old[IDX(i-1,j,cols)]
			+ phi_old[IDX(i,j+1,cols)] + phi_old[IDX(i,j-1,cols)])
		+ q_arr[IDX(i,j,cols)];
	    }
	}

      /* Pointer swap - O(1) instead of O(N^2) copy */
      temp = phi_old;
      phi_old = phi_new;
      phi_new = temp;

      if((iter + 1) % 100 == 0)
	{
	  if(check_convergence(phi_old, delta, N, 0.01))
	    {
	      iter++;
	      break;
	    }
	}
    }

  /* Result is in phi_old after the last swap */
  if(phi_final != NULL)
    memcpy(phi_final, phi_old, (N+1)*(N+1)*sizeof(double));

  free(phi_old);
  free(phi_new);
  free(q_arr);

  return iter;
}

/*=========================================================================
 * Parallel Jacobi solver using OpenMP
 *
 * OPTIMIZATIONS:
 *   1. Single parallel region: threads forked ONCE, reused across all
 *      iterations. Only #pragma omp for (worksharing) inside.
 *
 *   2. Parity-based double buffering: phi[0] and phi[1] arrays alternate
 *      as read/write buffers based on iteration parity (even/odd).
 *      This ELIMINATES the need for #pragma omp single + pointer swap
 *      and its extra barrier per iteration.
 *
 *   3. Contiguous 1D memory: better cache line utilization vs. 2D
 *      pointer-of-pointers.
 *
 *   4. Pre-computed source term: avoids repeated q_func() calls.
 *
 * Synchronization: only 1 implicit barrier per iteration (from omp for).
 *=========================================================================*/
int jacobi_parallel(double delta, int N, double *phi_final, int max_iter, int thread_count)
{
  int i, j;
  int cols = N + 1;
  double *phi[2]; /* double-buffered: phi[0] and phi[1] */
  double *q_arr;
  int converged = 0;
  int final_iter = max_iter;

  phi[0] = allocate_grid(N+1, N+1);
  phi[1] = allocate_grid(N+1, N+1);
  q_arr = precompute_source(delta, N);

  /* ONE parallel region for entire solve */
#pragma omp parallel num_threads(thread_count) private(i, j)
  {
    int iter;
    int rd, wr; /* read and write buffer indices */

    for(iter = 0; iter < max_iter; iter++)
      {
	/* Parity indexing: no pointer swap needed, no omp single needed */
	rd = iter % 2;       /* read from this buffer */
	wr = 1 - rd;         /* write to this buffer */

	/* Jacobi update - workshared across threads */
	/* Only ONE implicit barrier here (end of omp for) */
#pragma omp for schedule(static)
	for(i = 1; i < N; i++)
	  {
	    for(j = 1; j < N; j++)
	      {
		phi[wr][IDX(i,j,cols)] =
		  0.25 * (phi[rd][IDX(i+1,j,cols)] + phi[rd][IDX(i-1,j,cols)]
			  + phi[rd][IDX(i,j+1,cols)] + phi[rd][IDX(i,j-1,cols)])
		  + q_arr[IDX(i,j,cols)];
	      }
	  }
	/* implicit barrier ensures all threads done before next iteration */

	/* Convergence check every 100 iterations */
	if((iter + 1) % 100 == 0)
	  {
#pragma omp single
	    {
	      if(check_convergence(phi[wr], delta, N, 0.01))
		{
		  converged = 1;
		  final_iter = iter + 1;
		}
	    }
	    /* implicit barrier */
	    if(converged) break;
	  }
      }

#pragma omp single
    {
      if(!converged)
	final_iter = max_iter;
    }
  }
  /* END parallel region */

  /* Determine which buffer has the final result */
  {
    int last_wr = 1 - ((final_iter - 1) % 2); /* wr index of last completed iteration */
    if(phi_final != NULL)
      memcpy(phi_final, phi[last_wr], (N+1)*(N+1)*sizeof(double));
  }

  free(phi[0]);
  free(phi[1]);
  free(q_arr);

  return final_iter;
}

/* Write phi vs x at a given y to CSV file */
void write_profile(const char* filename, double *phi, double delta, int N, double y_target)
{
  int i, j_target;
  int cols = N + 1;
  double x, exact_val;
  FILE *fp;

  j_target = (int)round((y_target - (-1.0)) / delta);

  fp = fopen(filename, "w");
  if(fp == NULL)
    {
      printf("\n Error: Cannot open file %s for writing!\n", filename);
      return;
    }

  fprintf(fp, "x,phi_numerical,phi_exact\n");
  for(i = 0; i <= N; i++)
    {
      x = -1.0 + i * delta;
      exact_val = exact_solution(x, y_target);
      fprintf(fp, "%.10f,%.10f,%.10f\n", x, phi[IDX(i,j_target,cols)], exact_val);
    }
  fclose(fp);
}

int main(int argc, char* argv[])
{
  int thread_count = 1;
  int max_iter = 200000;
  int N, iter_serial, iter_parallel;
  double delta;
  double serial_time, parallel_time;
  double *phi_serial, *phi_parallel;

  if (argc >= 2)
    {
      thread_count = strtol(argv[1], NULL, 10);
    }
  else
    {
      printf("\n Usage: %s <num_threads> [delta]", argv[0]);
      printf("\n A command line argument other than name of the executable is required...exiting the program..\n");
      return 1;
    }

  delta = 0.1;
  if(argc >= 3)
    delta = atof(argv[2]);

  N = (int)round(2.0 / delta);

  printf("\n==========================================================");
  printf("\n  Jacobi Method for Poisson Equation");
  printf("\n==========================================================");
  printf("\n  Delta = %f, Grid = %d x %d (%d interior points)", delta, N+1, N+1, (N-1)*(N-1));
  printf("\n  Convergence criterion: within 1%% of exact solution");
  printf("\n  Exact solution: phi = (x^2 - 1)(y^2 - 1)");
  printf("\n==========================================================\n");

  phi_serial   = allocate_grid(N+1, N+1);
  phi_parallel = allocate_grid(N+1, N+1);

  /* ---- SERIAL JACOBI ---- */
  printf("\n  Running serial Jacobi...\n");

#ifdef _OPENMP
  serial_time = omp_get_wtime();
#endif

  iter_serial = jacobi_serial(delta, N, phi_serial, max_iter);

#ifdef _OPENMP
  serial_time = omp_get_wtime() - serial_time;
#endif

  printf("  [SERIAL] Iterations = %d", iter_serial);
#ifdef _OPENMP
  printf("\n  [SERIAL] Time       = %f seconds", serial_time);
#endif
  printf("\n");

  {
    char fname[256];
    sprintf(fname, "jacobi_serial_delta%.4f_y0.5.csv", delta);
    write_profile(fname, phi_serial, delta, N, 0.5);
    printf("  [SERIAL] Profile written to %s\n", fname);
  }

  /* ---- PARALLEL JACOBI ---- */
  printf("\n  Running parallel Jacobi with %d threads...\n", thread_count);

#ifdef _OPENMP
  parallel_time = omp_get_wtime();
#endif

  iter_parallel = jacobi_parallel(delta, N, phi_parallel, max_iter, thread_count);

#ifdef _OPENMP
  parallel_time = omp_get_wtime() - parallel_time;
#endif

  printf("  [PARALLEL, p=%d] Iterations = %d", thread_count, iter_parallel);
#ifdef _OPENMP
  printf("\n  [PARALLEL, p=%d] Time       = %f seconds", thread_count, parallel_time);
  if(parallel_time > 0.0)
    printf("\n  Speedup                     = %f", serial_time / parallel_time);
#endif
  printf("\n");

  {
    char fname[256];
    sprintf(fname, "jacobi_parallel_p%d_delta%.4f_y0.5.csv", thread_count, delta);
    write_profile(fname, phi_parallel, delta, N, 0.5);
    printf("  [PARALLEL] Profile written to %s\n", fname);
  }

  /* Verify serial == parallel */
  {
    int i;
    double max_diff = 0.0;
    int total = (N+1)*(N+1);
    for(i = 0; i < total; i++)
      {
	double diff = fabs(phi_serial[i] - phi_parallel[i]);
	if(diff > max_diff)
	  max_diff = diff;
      }
    printf("\n  Max difference (serial vs parallel) = %.15e", max_diff);
    if(max_diff < 1e-10)
      printf("\n  VERIFICATION: Serial and Parallel produce the SAME output.\n");
    else
      printf("\n  Note: Slight numerical differences due to floating-point ordering.\n");
  }

  printf("\n==========================================================\n\n");

  free(phi_serial);
  free(phi_parallel);

  return 0;
}
