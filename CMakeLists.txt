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

add_executable(jacobi_static src/jacobi_static.cpp)
add_executable(jacobi_threads src/jacobi_threads.cpp)

target_link_libraries(jacobi_threads OpenMP::OpenMP_C)