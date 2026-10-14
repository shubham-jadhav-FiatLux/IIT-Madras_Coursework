/*
Author    : Shubham Jadhav
Date      : 5 April, 2026
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#define PI 3.14159265358979323846

/* Problem parameters */
#define C_SPEED 1.0       /* wave speed */
#define L_DOMAIN 2.0      /* domain length */
#define DX 0.002          /* grid spacing */
#define DT 0.0001         /* time step */

/* Initial condition: u0(x) = sin(4*pi*x) for 0<=x<=0.5, else 0 */
double u0(double x)
{
  if(x >= 0.0 && x <= 0.5)
    return sin(4.0 * PI * x);
  else
    return 0.0;
}

/* Exact solution: u(x,t) = u0(x - c*t) */
double exact_solution(double x, double t)
{
  double xi = x - C_SPEED * t;
  /* Handle periodicity if needed; for this problem, u0 is zero outside [0,0.5] */
  /* The wave moves right, so xi can be negative; u0 returns 0 for xi < 0 */
  return u0(xi);
}

/* Write solution to CSV file */
void write_solution(const char *filename, double *u, int N, double t)
{
  FILE *fp = fopen(filename, "w");
  int i;
  double x, u_exact;

  if(fp == NULL)
    {
      printf("Error: cannot open %s\n", filename);
      return;
    }

  fprintf(fp, "x,u_numerical,u_exact\n");
  for(i = 0; i <= N; i++)
    {
      x = i * DX;
      u_exact = exact_solution(x, t);
      fprintf(fp, "%.10f,%.10f,%.10f\n", x, u[i], u_exact);
    }
  fclose(fp);
}

int main()
{
  int N = (int)(L_DOMAIN / DX); /* number of intervals = 1000 */
  int total_pts = N + 1;        /* grid points 0 to N */
  double *u, *u_new;
  double CFL = C_SPEED * DT / DX;
  int i, step;
  int steps_05, steps_10;


  printf("\n==========================================================");
  printf("\n  1D Traveling Wave Equation - Serial Solver");
  printf("\n==========================================================");
  printf("\n  c = %.1f, L = %.1f, dx = %f, dt = %f", C_SPEED, L_DOMAIN, DX, DT);
  printf("\n  CFL = c*dt/dx = %f", CFL);
  printf("\n  Grid points = %d, Total intervals = %d", total_pts, N);
  printf("\n==========================================================\n");

  steps_05 = (int)(0.5 / DT);   /* 5000 steps to t=0.5 */
  steps_10 = (int)(1.0 / DT);   /* 10000 steps to t=1.0 */

  u     = (double*)calloc(total_pts, sizeof(double));
  u_new = (double*)calloc(total_pts, sizeof(double));

  /* ================================================================ */
  /*  UPWIND SCHEME                                                    */
  /* ================================================================ */
  printf("\n  --- Upwind Scheme ---\n");

  /* Set initial condition */
  for(i = 0; i <= N; i++)
    u[i] = u0(i * DX);

  /* Write t = 0 */
  write_solution("serial_upwind_t0.0.csv", u, N, 0.0);
  printf("  t = 0.0: written to serial_upwind_t0.0.csv\n");

  /* Time-stepping */
  for(step = 1; step <= steps_10; step++)
    {


      /* Boundary conditions */
      u_new[0] = 0.0;   /* u(x=0, t) = 0 */
      u_new[N] = 0.0;   /* u(x=L, t) = 0 */

      /* Interior points: upwind scheme */
      /* u_i^{n+1} = u_i^n - CFL * (u_i^n - u_{i-1}^n) */
      for(i = 1; i < N; i++)
	{
	  u_new[i] = u[i] - CFL * (u[i] - u[i-1]);
	}

      /* Swap pointers */
      memcpy(u, u_new, total_pts * sizeof(double));

      /* Write at t = 0.5 */
      if(step == steps_05)
	{
	  write_solution("serial_upwind_t0.5.csv", u, N, 0.5);
	  printf("  t = 0.5: written to serial_upwind_t0.5.csv\n");
	}
    }

  /* Write at t = 1.0 */
  write_solution("serial_upwind_t1.0.csv", u, N, 1.0);
  printf("  t = 1.0: written to serial_upwind_t1.0.csv\n");

  /* ================================================================ */
  /*  QUICK SCHEME                                                     */
  /* ================================================================ */
  printf("\n  --- QUICK Scheme ---\n");

  /* Reset initial condition */
  for(i = 0; i <= N; i++)
    u[i] = u0(i * DX);

  write_solution("serial_quick_t0.0.csv", u, N, 0.0);
  printf("  t = 0.0: written to serial_quick_t0.0.csv\n");

  /* Time-stepping */
  for(step = 1; step <= steps_10; step++)
    {


      /* Boundary conditions */
      u_new[0] = 0.0;
      u_new[N] = 0.0;

      /* i = 1: use upwind (QUICK needs i-2 which doesn't exist) */
      u_new[1] = u[1] - CFL * (u[1] - u[0]);

      /* Interior points i = 2 to N-1: QUICK scheme */
      /* du/dx = (1/dx) * (3/8*u_i - 7/8*u_{i-1} + 1/8*u_{i-2} + 3/8*u_{i+1}) */
      /* u_i^{n+1} = u_i^n - CFL * (3/8*u_i - 7/8*u_{i-1} + 1/8*u_{i-2} + 3/8*u_{i+1}) */
      for(i = 2; i < N; i++)
	{
	  u_new[i] = u[i] - CFL * (  3.0/8.0 * u[i]
				    - 7.0/8.0 * u[i-1]
				    + 1.0/8.0 * u[i-2]
				    + 3.0/8.0 * u[i+1] );
	}

      memcpy(u, u_new, total_pts * sizeof(double));

      if(step == steps_05)
	{
	  write_solution("serial_quick_t0.5.csv", u, N, 0.5);
	  printf("  t = 0.5: written to serial_quick_t0.5.csv\n");
	}
    }

  write_solution("serial_quick_t1.0.csv", u, N, 1.0);
  printf("  t = 1.0: written to serial_quick_t1.0.csv\n");

  printf("\n==========================================================\n\n");

  free(u);
  free(u_new);

  return 0;
}
