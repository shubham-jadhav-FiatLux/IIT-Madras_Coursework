#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 15 // 15 equally spaced points

double f(double x) {
    return sin(5.0 * x);
}

double df_exact(double x) {
    return 5.0 * cos(5.0 * x);
}

// TDMA Serial algorithm
void tdma(double a[], double b[], double c[], double d[], double df[], int n) {
    c[0] = c[0] / b[0];
    d[0] = d[0] / b[0];
    
    for (int i = 1; i < n; i++) {
        double denom = b[i] - a[i] * c[i - 1];
        c[i] = c[i] / denom;
        d[i] = (d[i] - a[i] * d[i - 1]) / denom;
    }
    
    df[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        df[i] = d[i] - c[i] * df[i + 1];
    }
}

int main() {
    double L = 3.0; // x in [0, 3]
    double dx = L / (N - 1);
    double x[N];
    double y[N];
    double exact[N];
    
    for (int i = 0; i < N; i++) {
        x[i] = i * dx;
        y[i] = f(x[i]);
        exact[i] = df_exact(x[i]);
    }
    
    double a[N], b[N], c[N], d[N], df_pade[N];
    
    // Pade 4th order internal + 3rd order boundary definitions
    
    // Left boundary: df_0 + 2 df_1 = (-5 f_0 + 4 f_1 + f_2) / (2 dx)
    b[0] = 1.0;
    c[0] = 2.0;
    d[0] = (-5.0 * y[0] + 4.0 * y[1] + y[2]) / (2.0 * dx);
    a[0] = 0.0;
    
    // Internal points: 1/4 df_{i-1} + df_i + 1/4 df_{i+1} = 3/4 * (f_{i+1} - f_{i-1}) / dx
    for (int i = 1; i < N - 1; i++) {
        a[i] = 0.25;
        b[i] = 1.0;
        c[i] = 0.25;
        d[i] = 0.75 * (y[i + 1] - y[i - 1]) / dx;
    }
    
    // Right boundary: 2 df_{N-2} + df_{N-1} = (5 f_{N-1} - 4 f_{N-2} - f_{N-3}) / (2 dx)
    a[N - 1] = 2.0;
    b[N - 1] = 1.0;
    c[N - 1] = 0.0;
    d[N - 1] = (5.0 * y[N - 1] - 4.0 * y[N - 2] - y[N - 3]) / (2.0 * dx);
    
    // Solve with TDMA algorithm
    tdma(a, b, c, d, df_pade, N);
    
    // Compute CDS2 + FD1/BD1 for comparison
    double df_cds[N];
    df_cds[0] = (y[1] - y[0]) / dx; // FD1
    for (int i = 1; i < N - 1; i++) {
        df_cds[i] = (y[i + 1] - y[i - 1]) / (2.0 * dx); // CDS2
    }
    df_cds[N - 1] = (y[N - 1] - y[N - 2]) / dx; // BD1
    
    // Output comparison
    printf(" x      | Exact      | Pade 4th   | CDS 2nd    | Err Pade   | Err CDS\n");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < N; i++) {
        double err_pade = fabs(df_pade[i] - exact[i]);
        double err_cds = fabs(df_cds[i] - exact[i]);
        printf("%6.3f | %10.5f | %10.5f | %10.5f | %10.5f | %10.5f\n",
               x[i], exact[i], df_pade[i], df_cds[i], err_pade, err_cds);
    }
    
    return 0;
}
