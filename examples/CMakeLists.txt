cmake_minimum_required(VERSION 3.23)
project("demo_programs")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/exe)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/exe)


option(CNTR_OPENMP "Build with OpenMP support" ON)
if (CNTR_OPENMP)
    message(STATUS "Building with OpenMP")
    find_package(OpenMP REQUIRED)
endif()

option(CNTR_MPI "Build with MPI support" OFF)
if(CNTR_MPI)
    message(STATUS "Building with MPI")
    find_package(MPI REQUIRED)
endif()

option(CNTR_HDF5 "Build with HDF5 support" ON)
if (CNTR_HDF5)
    message(STATUS "Building with HDF5")
    find_package(HDF5 REQUIRED)
endif()

option(CNTR_MKL "Build with MKL support" ON)
if (CNTR_MKL)
    message(STATUS "Building with MKL")
    set(MKL_THREADING "gnu_thread")
    find_package(MKL REQUIRED)
    add_definitions("-DEIGEN_USE_MKL_ALL")
endif ()

find_package(Eigen3 REQUIRED)

find_package(cntr REQUIRED)

add_subdirectory(programs)


# ~~ Include Guard ~~
set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing, that CMake has configured at least once.")
