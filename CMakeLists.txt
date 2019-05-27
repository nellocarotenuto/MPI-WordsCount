# Set minimum required version for CMake
cmake_minimum_required(VERSION 3.12)

# Define the project name - It will be used for the executable as well
project(MPI-WordsCount C)

# Define C standard
set(CMAKE_C_STANDARD 99)

# Use Open MPI C wrapper compiler
set(CMAKE_C_COMPILER mpicc)

# Require external dependencies
find_package(OpenSSL 1.1.1 REQUIRED)

# Define libraries
add_library(wordsmap STATIC src/wordsmap.c)
target_include_directories(wordsmap PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(counter STATIC src/counter.c)
target_include_directories(counter PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(workloads STATIC src/workloads.c)
target_include_directories(workloads PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(mpiworkloads STATIC src/mpiworkloads.c)
target_include_directories(mpiworkloads PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Define the (only) executable of the project
add_executable(${PROJECT_NAME} src/main.c)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE wordsmap counter workloads mpiworkloads -lssl -lcrypto)