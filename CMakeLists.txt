cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)

find_package(OpenMP REQUIRED)

if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    message(STATUS "OpenMP found")
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# add_executable(jacobi_static src/jacobi_static.cpp)

set(N_VALUES 137 201 233 265 297)

foreach(N ${N_VALUES})
    add_executable(jacobi_s_N${N} src/jacobi_static.cpp)
    target_compile_definitions(jacobi_s_N${N} PRIVATE N_VALUE=${N})
    target_link_libraries(jacobi_s_N${N} OpenMP::OpenMP_C)

    add_custom_target(run_s_N${N}
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/jacobi_s_N${N}
            DEPENDS jacobi_s_N${N}
            COMMENT "Running Jacobi with N=${N}"
    )

    add_executable(jacobi_t_N${N} src/jacobi_threads.cpp)
    target_compile_definitions(jacobi_t_N${N} PRIVATE N_VALUE=${N})
    target_link_libraries(jacobi_t_N${N} OpenMP::OpenMP_C)

    add_custom_target(run_t_N${N}
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/jacobi_t_N${N}
            DEPENDS jacobi_t_N${N}
            COMMENT "Running Jacobi with N=${N}"
    )
endforeach()

# add_executable(jacobi_threads src/jacobi_threads.cpp)

target_link_libraries(jacobi_threads OpenMP::OpenMP_C)