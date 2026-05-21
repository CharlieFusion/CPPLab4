#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define Max(a,b) ((a)>(b)?(a):(b))

// Глобальные переменные
int rank, size;          // rank — номер процесса, size — количество процессов
int N;                   // Размер глобальной сетки
int itmax;               // Максимальное число итераций
double maxeps = 0.1e-7;  // Требуемая точность
double eps;              // Текущая погрешность

void init(double* local_A, int local_n, int n);
void relax(double* local_A, double* local_B, int local_n, int n, int rank, int size, MPI_Comm comm);
void resid(double* local_A, double* local_B, int local_n, int n, double* eps, int rank, int size, MPI_Comm comm);
void verify(double* local_A, int local_n, int n, int rank, int size, MPI_Comm comm);
void exchange_halos(double* local_A, int local_n, int n, int rank, int size, MPI_Comm comm);

int main(int argc, char** argv) {
    int it;
    double time_start, time_end;

    // Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            printf("Usage: %s <N> [itmax]\n", argv[0]);
            printf("Example: %s 137 11\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    N = atoi(argv[1]);
    itmax = (argc > 2) ? atoi(argv[2]) : 11;

    if (rank == 0) {
        printf("========================================\n");
        printf("MPI Jacobi Relaxation\n");
        printf("Grid size: %d x %d x %d\n", N, N, N);
        printf("Max iterations: %d\n", itmax);
        printf("MPI processes: %d\n", size);
        printf("========================================\n\n");
    }

    // Определяем количество локальных строк для каждого процесса
    int base_local_n = (N - 2) / size;      // Внутренние строки на процесс
    int remainder = (N - 2) % size;         // Остаток (добавляется первым процессам)

    // Локальное количество внутренних строк
    int local_internal_n;
    if (rank < remainder) {
        local_internal_n = base_local_n + 1;
    } else {
        local_internal_n = base_local_n;
    }

    // Полное количество строк в локальном блоке
    int local_n = local_internal_n + 2;

    double* local_A = (double*)malloc(local_n * N * sizeof(double));
    double* local_B = (double*)malloc(local_n * N * sizeof(double));

    if (local_A == NULL || local_B == NULL) {
        printf("Process %d: Memory allocation failed!\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    init(local_A, local_n, N);

    // Барьер для синхронизации перед замером времени
    MPI_Barrier(MPI_COMM_WORLD);
    time_start = MPI_Wtime();

    for (it = 1; it <= itmax; it++) {
        eps = 0.0;
        relax(local_A, local_B, local_n, N, rank, size, MPI_COMM_WORLD);
        resid(local_A, local_B, local_n, N, &eps, rank, size, MPI_COMM_WORLD);

        if (rank == 0 && it % 10 == 0) {
            printf("it=%4d eps=%f\n", it, eps);
        }

        if (eps < maxeps) break;
    }

    time_end = MPI_Wtime();

    if (rank == 0) {
        printf("\nTime in seconds = %g\n", time_end - time_start);
    }

    verify(local_A, local_n, N, rank, size, MPI_COMM_WORLD);

    free(local_A);
    free(local_B);

    MPI_Finalize();
    return 0;
}

void init(double* local_A, int local_n, int n) {
    for (int i = 0; i < local_n; i++) {
        int gi = i;
        // Здесь для простоты предполагаем, что процесс 0 начинает с i=0
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                int idx = (i * n + j) * n + k;
                if (gi == 0 || gi == n-1 || j == 0 || j == n-1 || k == 0 || k == n-1) {
                    local_A[idx] = 0.0;
                } else {
                    local_A[idx] = 4.0 + gi + j + k;
                }
            }
        }
    }
}

// Обмен граничными строками между соседними процессами
void exchange_halos(double* local_A, int local_n, int n, int rank, int size, MPI_Comm comm) {
    int prev = (rank - 1 + size) % size;
    int next = (rank + 1) % size;

    // Отправка верхней граничной строки вверх, получение от вышестоящего процесса
    MPI_Sendrecv(&local_A[1 * n], n, MPI_DOUBLE, prev, 0,
                 &local_A[0 * n], n, MPI_DOUBLE, prev, 0,
                 comm, MPI_STATUS_IGNORE);

    // Отправка нижней граничной строки вниз, получение от нижестоящего процесса
    MPI_Sendrecv(&local_A[(local_n - 2) * n], n, MPI_DOUBLE, next, 1,
                 &local_A[(local_n - 1) * n], n, MPI_DOUBLE, next, 1,
                 comm, MPI_STATUS_IGNORE);
}

void relax(double* local_A, double* local_B, int local_n, int n, int rank, int size, MPI_Comm comm) {
    exchange_halos(local_A, local_n, n, rank, size, comm);

    for (int i = 1; i <= local_n - 2; i++) {
        for (int j = 1; j <= n - 2; j++) {
            for (int k = 1; k <= n - 2; k++) {
                int idx = (i * n + j) * n + k;
                int idx_up = ((i-1) * n + j) * n + k;
                int idx_down = ((i+1) * n + j) * n + k;
                int idx_left = (i * n + (j-1)) * n + k;
                int idx_right = (i * n + (j+1)) * n + k;
                int idx_front = (i * n + j) * n + (k-1);
                int idx_back = (i * n + j) * n + (k+1);

                local_B[idx] = (local_A[idx_up] + local_A[idx_down] +
                                local_A[idx_left] + local_A[idx_right] +
                                local_A[idx_front] + local_A[idx_back]) / 6.0;
            }
        }
    }
}

void resid(double* local_A, double* local_B, int local_n, int n, double* eps,
           int rank, int size, MPI_Comm comm) {
    double local_eps = 0.0;

    for (int i = 1; i <= local_n - 2; i++) {
        for (int j = 1; j <= n - 2; j++) {
            for (int k = 1; k <= n - 2; k++) {
                int idx = (i * n + j) * n + k;
                double e = fabs(local_A[idx] - local_B[idx]);
                local_A[idx] = local_B[idx];
                if (e > local_eps) local_eps = e;
            }
        }
    }

    MPI_Allreduce(&local_eps, eps, 1, MPI_DOUBLE, MPI_MAX, comm);
}

// Верификация результата
void verify(double* local_A, int local_n, int n, int rank, int size, MPI_Comm comm) {
    double local_sum = 0.0;
    double global_sum = 0.0;

    for (int i = 1; i <= local_n - 2; i++) {
        int gi = i;
        for (int j = 1; j <= n - 2; j++) {
            for (int k = 1; k <= n - 2; k++) {
                int idx = (i * n + j) * n + k;
                local_sum += local_A[idx] * (gi + 1) * (j + 1) * (k + 1) / (n * n * n);
            }
        }
    }

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, comm);

    if (rank == 0) {
        printf("Verification sum S = %f\n", global_sum);
    }
}