cmake_minimum_required(VERSION 3.12)
project(MPI_WordsCount C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_COMPILER mpicc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

add_executable(MPI_WordsCount src/main.c)
