cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)

find_package(OpenMP REQUIRED)

if(OpenMP_C_FOUND)
    get_target_property(OpenMP_FLAGS OpenMP::OpenMP_C INTERFACE_COMPILE_OPTIONS)
    if(NOT OpenMP_FLAGS)
        set(OpenMP_FLAGS ${OpenMP_C_FLAGS})
    endif()

    message(STATUS "OpenMP found, flags: ${OpenMP_FLAGS}")
    message(STATUS "OpenMP libraries: ${OpenMP_C_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

set(N_VALUES 137 201 233 265 297)

foreach(N ${N_VALUES})
    add_executable(jacobi_s_N${N} src/jacobi_static.cpp)
    target_compile_definitions(jacobi_s_N${N} PRIVATE N_VALUE=${N})

    add_executable(jacobi_t_N${N} src/jacobi_threads.cpp)
    target_compile_definitions(jacobi_t_N${N} PRIVATE N_VALUE=${N})

    if(MINGW)
        target_compile_options(jacobi_t_N${N} PRIVATE -fopenmp)
        target_link_options(jacobi_t_N${N} PRIVATE -fopenmp)
    else()
        target_link_libraries(jacobi_t_N${N} OpenMP::OpenMP_C)
    endif()

    if(MINGW)
        target_compile_options(jacobi_s_N${N} PRIVATE -U_OPENMP)
    endif()
endforeach()