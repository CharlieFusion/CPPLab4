#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <omp.h>

#define Max(a,b) ((a)>(b)?(a):(b))

#ifndef N_VALUE
#define N_VALUE 137
#endif

#define N N_VALUE

double maxeps = 0.1e-7;
int itmax = 11;
int i, j, k;
double eps;
double A[N][N][N], B[N][N][N];

void relax();
void resid();
void init();
void verify();
double wtime();

int main(int an, char **as) {
    int it;
    double time0, time1;

    init();
    time0 = omp_get_wtime();

    for(it = 1; it <= itmax; it++) {
        eps = 0.0;
        relax();
        resid();
        printf("it=%4i eps=%f\n", it, eps);
        if(eps < maxeps) break;
    }

    time1 = omp_get_wtime();
    printf("Time in seconds=%g\n", time1 - time0);
    verify();

    return 0;
}

void init() {
#pragma omp parallel for private(j,k) shared(A)
    for(i = 0; i <= N-1; i++)
        for(j = 0; j <= N-1; j++)
            for(k = 0; k <= N-1; k++) {
                if(i==0 || i==N-1 || j==0 || j==N-1 || k==0 || k==N-1)
                    A[i][j][k] = 0;
                else
                    A[i][j][k] = (4. + i + j + k);
            }
}

void relax() {
#pragma omp parallel for private(j,k) shared(A,B)
    for(i = 1; i <= N-2; i++)
        for(j = 1; j <= N-2; j++)
            for(k = 1; k <= N-2; k++) {
                B[i][j][k] = (A[i-1][j][k] + A[i+1][j][k] +
                              A[i][j-1][k] + A[i][j+1][k] +
                              A[i][j][k-1] + A[i][j][k+1]) / 6.0;
            }
}

void resid() {
    double local_eps = 0.0;

#pragma omp parallel for private(j,k) reduction(max:local_eps) shared(A,B)
    for(i = 1; i <= N-2; i++)
        for(j = 1; j <= N-2; j++)
            for(k = 1; k <= N-2; k++) {
                double e = fabs(A[i][j][k] - B[i][j][k]);
                A[i][j][k] = B[i][j][k];
                if(e > local_eps) local_eps = e;
            }

#pragma omp critical
    {
        if(local_eps > eps) eps = local_eps;
    }
}

void verify() {
    double s = 0.0;

#pragma omp parallel for private(j,k) reduction(+:s) shared(A)
    for(i = 0; i <= N-1; i++)
        for(j = 0; j <= N-1; j++)
            for(k = 0; k <= N-1; k++) {
                s += A[i][j][k] * (i+1) * (j+1) * (k+1) / (N*N*N);
            }

    printf("S = %f\n", s);
}