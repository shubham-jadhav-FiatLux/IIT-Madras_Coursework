/*
Author    : Shubham Jadhav
Date      : 7 April, 2026
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<mpi.h>

#define PI 3.14159265358979323846

double q_func(double x, double y)
{
  return x * x + y * y;
}

int main(int argc, char *argv[])
{
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double delta = 0.01;
  if(argc >= 2) delta = atof(argv[1]);

  int N = (int)(2.0 / delta);
  int Np = N + 1;
  double tol = 1e-4;
  int max_iter = 500000;
  double d2_over_4 = (delta * delta) / 4.0;

  /* ================================================================ */
  /* Row-wise 1D decomposition along x (i-direction).                  */
  /* Process r owns rows i = [istart, iend] of the global grid.        */
  /* Local array has local_ni rows + 2 halo rows (1 top, 1 bottom).    */
  /* Each row has Np columns (full y-direction).                        */
  /* ================================================================ */
  int base = Np / size;
  int rem  = Np % size;
  int istart, local_ni;

  if(rank < rem)
    { local_ni = base + 1; istart = rank * local_ni; }
  else
    { local_ni = base; istart = rem * (base + 1) + (rank - rem) * base; }
  int iend = istart + local_ni - 1;

  int left  = (rank > 0)        ? rank - 1 : MPI_PROC_NULL;
  int right = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;

  /* Local arrays: (local_ni + 2) rows x Np columns */
  /* Row 0 = left halo, rows 1..local_ni = data, row local_ni+1 = right halo */
  int alloc_rows = local_ni + 2;
  double *phi_old = (double*)calloc(alloc_rows * Np, sizeof(double));
  double *phi_new = (double*)calloc(alloc_rows * Np, sizeof(double));

  /* Pre-compute source */
  double *q_arr = (double*)malloc(alloc_rows * Np * sizeof(double));
  int i, j, li;
  double x, y;

  for(li = 1; li <= local_ni; li++)
    {
      int gi = istart + li - 1; /* global i */
      x = -1.0 + gi * delta;
      for(j = 0; j < Np; j++)
	{
	  y = -1.0 + j * delta;
	  q_arr[li * Np + j] = d2_over_4 * q_func(x, y);
	}
    }

  /* Apply Dirichlet BCs */
  /* Left boundary x=-1 (global i=0): phi = sin(2*pi*y) */
  if(istart == 0)
    {
      for(j = 0; j < Np; j++)
	{
	  y = -1.0 + j * delta;
	  phi_old[1 * Np + j] = sin(2.0 * PI * y); /* local row 1 = global i=0 */
	  phi_new[1 * Np + j] = sin(2.0 * PI * y);
	}
    }
  /* Top (j=N) and bottom (j=0) BCs: phi=0 (already zero) */

  if(rank == 0)
    {
      printf("\n==========================================================");
      printf("\n  Poisson Equation - MPI Jacobi (p = %d)", size);
      printf("\n==========================================================");
      printf("\n  Delta = %f, Grid = %d x %d", delta, Np, Np);
      printf("\n  Convergence: max|phi^{k+1} - phi^k| < %e", tol);
      printf("\n==========================================================\n");
    }

  /* ---- Jacobi iteration ---- */
  int iter;
  double local_max_diff, global_max_diff;

  for(iter = 1; iter <= max_iter; iter++)
    {
      /* Halo exchange: send/recv entire rows (Np doubles) */
      /* Send my first data row to left, recv from left into my left halo */
      MPI_Sendrecv(&phi_old[1 * Np], Np, MPI_DOUBLE, left, 0,
		   &phi_old[(local_ni + 1) * Np], Np, MPI_DOUBLE, right, 0,
		   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      /* Send my last data row to right, recv from right into my right halo */
      MPI_Sendrecv(&phi_old[local_ni * Np], Np, MPI_DOUBLE, right, 1,
		   &phi_old[0], Np, MPI_DOUBLE, left, 1,
		   MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      /* Update interior points */
      for(li = 1; li <= local_ni; li++)
	{
	  int gi = istart + li - 1;

	  /* Skip Dirichlet boundaries (i=0: left BC already set) */
	  if(gi == 0) continue;

	  /* Skip Neumann boundary i=N (handled separately below) */
	  if(gi == N) continue;

	  for(j = 1; j < N; j++) /* skip j=0 and j=N (Dirichlet phi=0) */
	    {
	      phi_new[li * Np + j] =
		0.25 * (phi_old[(li+1)*Np + j] + phi_old[(li-1)*Np + j]
			+ phi_old[li*Np + (j+1)] + phi_old[li*Np + (j-1)])
		+ q_arr[li * Np + j];
	    }
	}

      /* Neumann BC at x=1 (global i=N): phi_N = (4*phi_{N-1} - phi_{N-2})/3 */
      if(iend == N)
	{
	  int li_N = local_ni;     /* local row for global i=N */
	  int li_Nm1 = local_ni - 1; /* for global i=N-1 */
	  int li_Nm2 = local_ni - 2; /* for global i=N-2 */

	  /* If li_Nm2 < 1, it's in the halo (li=0), which is fine */
	  for(j = 1; j < N; j++)
	    {
	      phi_new[li_N * Np + j] =
		(4.0 * phi_new[li_Nm1 * Np + j]
		 - phi_new[li_Nm2 * Np + j]) / 3.0;
	    }
	}

      /* Convergence check */
      local_max_diff = 0.0;
      for(li = 1; li <= local_ni; li++)
	{
	  int gi = istart + li - 1;
	  if(gi == 0) continue; /* skip fixed Dirichlet */
	  for(j = 1; j < N; j++)
	    {
	      double diff = fabs(phi_new[li*Np + j] - phi_old[li*Np + j]);
	      if(diff > local_max_diff) local_max_diff = diff;
	    }
	}

      MPI_Allreduce(&local_max_diff, &global_max_diff, 1,
		    MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

      /* Swap */
      double *tmp = phi_old;
      phi_old = phi_new;
      phi_new = tmp;

      /* Re-apply Dirichlet BCs after swap (left boundary stays fixed) */
      if(istart == 0)
	for(j = 0; j < Np; j++)
	  {
	    y = -1.0 + j * delta;
	    phi_old[1 * Np + j] = sin(2.0 * PI * y);
	  }

      if(global_max_diff < tol)
	{
	  if(rank == 0)
	    printf("\n  Converged at iteration %d (max_diff = %e)\n", iter, global_max_diff);
	  break;
	}

      if(rank == 0 && iter % 10000 == 0)
	printf("  iter %d: max_diff = %e\n", iter, global_max_diff);
    }

  if(iter > max_iter && rank == 0)
    printf("\n  WARNING: Did not converge in %d iterations\n", max_iter);

  /* ---- Gather and write output ---- */
  /* Gather full phi on rank 0 */
  double *phi_global = NULL;
  int *counts = NULL, *displs = NULL;

  if(rank == 0)
    {
      phi_global = (double*)calloc(Np * Np, sizeof(double));
      counts = (int*)malloc(size * sizeof(int));
      displs = (int*)malloc(size * sizeof(int));
    }

  /* Each process sends local_ni * Np doubles (data rows only, no halo) */
  int send_count = local_ni * Np;
  MPI_Gather(&send_count, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if(rank == 0)
    {
      displs[0] = 0;
      for(i = 1; i < size; i++)
	displs[i] = displs[i-1] + counts[i-1];
    }

  MPI_Gatherv(&phi_old[1 * Np], send_count, MPI_DOUBLE,
	      phi_global, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  /* Write CSV files */
  if(rank == 0)
    {
      char fname[256];

      /* phi vs x at y=0 */
      int j0 = N / 2;
      sprintf(fname, "mpi_p%d_phi_vs_x_y0_d%.4f.csv", size, delta);
      FILE *fp = fopen(fname, "w");
      fprintf(fp, "x,phi\n");
      for(i = 0; i < Np; i++)
	fprintf(fp, "%.10f,%.10f\n", -1.0 + i * delta, phi_global[i * Np + j0]);
      fclose(fp);
      printf("  Written: %s\n", fname);

      /* phi vs y at x=0 */
      int i0 = N / 2;
      sprintf(fname, "mpi_p%d_phi_vs_y_x0_d%.4f.csv", size, delta);
      fp = fopen(fname, "w");
      fprintf(fp, "y,phi\n");
      for(j = 0; j < Np; j++)
	fprintf(fp, "%.10f,%.10f\n", -1.0 + j * delta, phi_global[i0 * Np + j]);
      fclose(fp);
      printf("  Written: %s\n", fname);

      free(phi_global);
      free(counts);
      free(displs);
    }

  if(rank == 0)
    printf("\n==========================================================\n\n");

  free(phi_old);
  free(phi_new);
  free(q_arr);

  MPI_Finalize();
  return 0;
}
