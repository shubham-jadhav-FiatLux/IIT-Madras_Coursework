/*
Author    : Shubham Jadhav
Date      : 5 April, 2026
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<mpi.h>

#define PI 3.14159265358979323846

#define C_SPEED 1.0
#define L_DOMAIN 2.0
#define DX 0.002
#define DT 0.0001
#define LEFT_HALO 2
#define RIGHT_HALO 1

double u0(double x)
{
  if(x >= 0.0 && x <= 0.5)
    return sin(4.0 * PI * x);
  else
    return 0.0;
}

double exact_solution(double x, double t)
{
  return u0(x - C_SPEED * t);
}

/*
 * Domain decomposition:
 *   Global grid: i = 0, 1, ..., N  (N+1 points)
 *   Process r owns global indices [gstart, gend]
 *
 * Local array layout (alloc_size = LEFT_HALO + local_n + RIGHT_HALO):
 *   [ghost_L2] [ghost_L1] [data_0] [data_1] ... [data_{n-1}] [ghost_R0]
 *      idx 0      idx 1     idx 2    idx 3        idx n+1       idx n+2
 *
 * Mapping: local index li = LEFT_HALO + (gi - gstart)
 */

/* Exchange halos with neighbors */
void exchange_halos(double *u, int local_n, int left, int right)
{
  MPI_Status status;

  /* Step 1: Fill my LEFT halo (2 pts) from left neighbor's rightmost data */
  /*         Send my rightmost 2 data pts to right neighbor's left halo   */
  MPI_Sendrecv(&u[LEFT_HALO + local_n - 2], 2, MPI_DOUBLE, right, 0,
	       &u[0], 2, MPI_DOUBLE, left, 0,
	       MPI_COMM_WORLD, &status);

  /* Step 2: Fill my RIGHT halo (1 pt) from right neighbor's leftmost data */
  /*         Send my leftmost 1 data pt to left neighbor's right halo      */
  MPI_Sendrecv(&u[LEFT_HALO], 1, MPI_DOUBLE, left, 1,
	       &u[LEFT_HALO + local_n], 1, MPI_DOUBLE, right, 1,
	       MPI_COMM_WORLD, &status);
}

/* Gather solution on rank 0 and write to CSV */
void gather_write(const char *fname, double *u_data, int local_n,
		  int gstart, int N, double t, int rank, int size)
{
  double *u_global = NULL;
  int *counts = NULL, *displs = NULL;
  int i, r;

  if(rank == 0)
    {
      u_global = (double*)calloc(N + 1, sizeof(double));
      counts = (int*)malloc(size * sizeof(int));
      displs = (int*)malloc(size * sizeof(int));
    }

  /* Gather local_n values from each process */
  /* First gather counts */
  MPI_Gather(&local_n, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if(rank == 0)
    {
      displs[0] = 0;
      for(r = 1; r < size; r++)
	displs[r] = displs[r-1] + counts[r-1];
    }

  MPI_Gatherv(u_data, local_n, MPI_DOUBLE,
	      u_global, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if(rank == 0)
    {
      FILE *fp = fopen(fname, "w");
      fprintf(fp, "x,u_numerical,u_exact\n");
      for(i = 0; i <= N; i++)
	fprintf(fp, "%.10f,%.10f,%.10f\n",
		i * DX, u_global[i], exact_solution(i * DX, t));
      fclose(fp);
      free(u_global);
      free(counts);
      free(displs);
    }
}

int main(int argc, char *argv[])
{
  int rank, size;
  int N = (int)(L_DOMAIN / DX);
  int total_pts = N + 1;
  double CFL = C_SPEED * DT / DX;
  int steps_05 = (int)(0.5 / DT);
  int steps_10 = (int)(1.0 / DT);
  int step, i;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /* ---- Domain decomposition ---- */
  int base = total_pts / size;
  int rem  = total_pts % size;
  int gstart, local_n;

  if(rank < rem)
    { local_n = base + 1; gstart = rank * local_n; }
  else
    { local_n = base; gstart = rem * (base + 1) + (rank - rem) * base; }
  

  /* Neighbor ranks */
  int left  = (rank > 0)        ? rank - 1 : MPI_PROC_NULL;
  int right = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;

  /* Local arrays with halo */
  int alloc = LEFT_HALO + local_n + RIGHT_HALO;
  double *u     = (double*)calloc(alloc, sizeof(double));
  double *u_new = (double*)calloc(alloc, sizeof(double));

  if(rank == 0)
    {
      printf("\n==========================================================");
      printf("\n  1D Traveling Wave - MPI Solver (p = %d)", size);
      printf("\n==========================================================");
      printf("\n  c=%.1f  L=%.1f  dx=%f  dt=%f  CFL=%f", C_SPEED, L_DOMAIN, DX, DT, CFL);
      printf("\n  Grid=%d pts, %d pts/proc (approx)", total_pts, local_n);
      printf("\n  Halo: %d left, %d right", LEFT_HALO, RIGHT_HALO);
      printf("\n==========================================================\n");
    }

  /* ================================================================ */
  /*  Run a scheme: 0=upwind, 1=quick                                  */
  /* ================================================================ */
  int scheme;
  for(scheme = 0; scheme <= 1; scheme++)
    {
      const char *sname = (scheme == 0) ? "upwind" : "quick";

      /* Set initial condition */
      for(i = 0; i < local_n; i++)
	u[LEFT_HALO + i] = u0((gstart + i) * DX);

      /* Zero halos */
      u[0] = 0.0; u[1] = 0.0;
      u[LEFT_HALO + local_n] = 0.0;

      /* Write t = 0 */
      {
	char fname[256];
	sprintf(fname, "mpi_p%d_%s_t0.0.csv", size, sname);
	gather_write(fname, &u[LEFT_HALO], local_n, gstart, N, 0.0, rank, size);
	if(rank == 0) printf("\n  --- %s (p=%d) ---\n  t=0.0: %s\n",
			     scheme == 0 ? "Upwind" : "QUICK", size, fname);
      }

      /* Time-stepping loop */
      for(step = 1; step <= steps_10; step++)
	{
	  /* Exchange halos */
	  exchange_halos(u, local_n, left, right);

	  /* Apply BCs to halos at domain boundaries */
	  if(rank == 0)
	    { u[0] = 0.0; u[1] = 0.0; } /* left boundary ghosts */
	  if(rank == size - 1)
	    u[LEFT_HALO + local_n] = 0.0; /* right boundary ghost */

	  /* Update local points */
	  for(i = 0; i < local_n; i++)
	    {
	      int gi = gstart + i;
	      int li = LEFT_HALO + i;

	      if(gi == 0 || gi == N)
		{
		  u_new[li] = 0.0; /* boundary condition */
		}
	      else if(scheme == 0 || gi == 1)
		{
		  /* Upwind scheme (also used at gi=1 for QUICK) */
		  u_new[li] = u[li] - CFL * (u[li] - u[li - 1]);
		}
	      else
		{
		  /* QUICK scheme: needs li-2, li-1, li, li+1 */
		  u_new[li] = u[li]
		    - CFL * (  3.0/8.0 * u[li]
			     - 7.0/8.0 * u[li - 1]
			     + 1.0/8.0 * u[li - 2]
			     + 3.0/8.0 * u[li + 1] );
		}
	    }

	  /* Copy */
	  memcpy(&u[LEFT_HALO], &u_new[LEFT_HALO], local_n * sizeof(double));

	  /* Output at t = 0.5 */
	  if(step == steps_05)
	    {
	      char fname[256];
	      sprintf(fname, "mpi_p%d_%s_t0.5.csv", size, sname);
	      gather_write(fname, &u[LEFT_HALO], local_n, gstart, N, 0.5, rank, size);
	      if(rank == 0) printf("  t=0.5: %s\n", fname);
	    }
	}

      /* Output at t = 1.0 */
      {
	char fname[256];
	sprintf(fname, "mpi_p%d_%s_t1.0.csv", size, sname);
	gather_write(fname, &u[LEFT_HALO], local_n, gstart, N, 1.0, rank, size);
	if(rank == 0) printf("  t=1.0: %s\n", fname);
      }
    }

  if(rank == 0)
    printf("\n==========================================================\n\n");

  free(u);
  free(u_new);
  MPI_Finalize();
  return 0;
}
