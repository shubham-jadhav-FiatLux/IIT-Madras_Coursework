/*
Author    : Shubham Jadhav
Date      : 7 April, 2026
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#define PI 3.14159265358979323846

/* Source term */
double q_func(double x, double y)
{
  return x * x + y * y;
}

int main(int argc, char *argv[])
{
  double delta = 0.1;       /* default grid spacing */
  if(argc >= 2) delta = atof(argv[1]);

  int N = (int)(2.0 / delta); /* number of intervals in each direction */
  int Np = N + 1;              /* grid points per direction */
  double tol = 1e-4;
  int max_iter = 500000;

  printf("\n==========================================================");
  printf("\n  Poisson Equation - Serial Jacobi Solver");
  printf("\n==========================================================");
  printf("\n  Delta = %f, Grid = %d x %d", delta, Np, Np);
  printf("\n  Convergence: max|phi^{k+1} - phi^k| < %e", tol);
  printf("\n==========================================================\n");

  /* Allocate 2D arrays as contiguous 1D */
  /* Index: phi[i*Np + j], where i -> x, j -> y */
  /* x_i = -1 + i*delta,  y_j = -1 + j*delta */
  double *phi_old = (double*)calloc(Np * Np, sizeof(double));
  double *phi_new = (double*)calloc(Np * Np, sizeof(double));

  int i, j, iter;
  double x, y;

  /* Pre-compute source term */
  double *q_arr = (double*)malloc(Np * Np * sizeof(double));
  double d2_over_4 = (delta * delta) / 4.0;
  for(i = 0; i < Np; i++)
    for(j = 0; j < Np; j++)
      {
	x = -1.0 + i * delta;
	y = -1.0 + j * delta;
	q_arr[i * Np + j] = d2_over_4 * q_func(x, y);
      }

  /* Apply Dirichlet BCs to both arrays */
  for(j = 0; j < Np; j++)
    {
      y = -1.0 + j * delta;
      /* Left BC: x=-1 (i=0): phi = sin(2*pi*y) */
      phi_old[0 * Np + j] = sin(2.0 * PI * y);
      phi_new[0 * Np + j] = sin(2.0 * PI * y);
    }
  /* Top (j=N) and Bottom (j=0) BCs: phi = 0  (already zero from calloc) */
  /* Right BC (i=N): Neumann, handled during iteration */

  /* ---- Jacobi iteration ---- */
  double max_diff;
  for(iter = 1; iter <= max_iter; iter++)
    {
      /* Update interior points: i=1..N-1, j=1..N-1 */
      for(i = 1; i < N; i++)
	{
	  for(j = 1; j < N; j++)
	    {
	      phi_new[i * Np + j] =
		0.25 * (phi_old[(i+1)*Np + j] + phi_old[(i-1)*Np + j]
			+ phi_old[i*Np + (j+1)] + phi_old[i*Np + (j-1)])
		+ q_arr[i * Np + j];
	    }
	}

      /* Neumann BC at x=1 (i=N): phi_N = (4*phi_{N-1} - phi_{N-2}) / 3 */
      for(j = 1; j < N; j++)
	{
	  phi_new[N * Np + j] = (4.0 * phi_new[(N-1)*Np + j]
				 - phi_new[(N-2)*Np + j]) / 3.0;
	}

      /* Check convergence: max |phi_new - phi_old| */
      max_diff = 0.0;
      for(i = 1; i <= N; i++) /* include Neumann boundary */
	for(j = 1; j < N; j++)
	  {
	    double diff = fabs(phi_new[i*Np + j] - phi_old[i*Np + j]);
	    if(diff > max_diff) max_diff = diff;
	  }

      /* Swap */
      double *tmp = phi_old;
      phi_old = phi_new;
      phi_new = tmp;

      if(max_diff < tol)
	{
	  printf("\n  Converged at iteration %d (max_diff = %e)\n", iter, max_diff);
	  break;
	}

      if(iter % 5000 == 0)
	printf("  iter %d: max_diff = %e\n", iter, max_diff);
    }

  if(iter > max_iter)
    printf("\n  WARNING: Did not converge in %d iterations (max_diff=%e)\n",
	   max_iter, max_diff);

  /* Result is in phi_old (after last swap) */

  /* Write phi vs x at y=0 */
  {
    int j0 = N / 2; /* j index for y = 0 */
    char fname[256];
    sprintf(fname, "serial_phi_vs_x_y0_d%.4f.csv", delta);
    FILE *fp = fopen(fname, "w");
    fprintf(fp, "x,phi\n");
    for(i = 0; i < Np; i++)
      {
	x = -1.0 + i * delta;
	fprintf(fp, "%.10f,%.10f\n", x, phi_old[i * Np + j0]);
      }
    fclose(fp);
    printf("  Written: %s\n", fname);
  }

  /* Write phi vs y at x=0 */
  {
    int i0 = N / 2; /* i index for x = 0 */
    char fname[256];
    sprintf(fname, "serial_phi_vs_y_x0_d%.4f.csv", delta);
    FILE *fp = fopen(fname, "w");
    fprintf(fp, "y,phi\n");
    for(j = 0; j < Np; j++)
      {
	y = -1.0 + j * delta;
	fprintf(fp, "%.10f,%.10f\n", y, phi_old[i0 * Np + j]);
      }
    fclose(fp);
    printf("  Written: %s\n", fname);
  }

  printf("\n==========================================================\n\n");

  free(phi_old);
  free(phi_new);
  free(q_arr);

  return 0;
}
