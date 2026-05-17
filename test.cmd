@echo off

echo ESC[33m STATIC RESULTS:
for %%N in (137 201 233 265 297) do (
        echo ESC[36m N=%%N
        echo ESC[32m
        cmake-build-debug\bin\jacobi_s_N%%N
        echo
)
echo ESC[33m PARALLEL 1 CORE:
for %%N in (137 201 233 265 297) do (
        echo ESC[36m N=%%N
        echo ESC[32m
        set OMP_NUM_THREADS=1
        cmake-build-debug\bin\jacobi_t_N%%N
        echo
)
echo ESC[33m PARALLEL 2 CORE:
for %%N in (137 201 233 265 297) do (
        echo ESC[36m N=%%N
        echo ESC[32m
        set OMP_NUM_THREADS=2
        cmake-build-debug\bin\jacobi_t_N%%N
        echo
)
echo ESC[0m