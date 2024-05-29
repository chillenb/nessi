cmake_minimum_required(VERSION 3.23)
project("libcntr")

include(GNUInstallDirs)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# ~~ OpenMP and MPI flags ~~
option(CNTR_OPENMP "Build with OpenMP support" ON)

option(CNTR_MPI "Build with MPI support" OFF)
if(CNTR_MPI)
    message(STATUS "Building with MPI")
    find_package(MPI REQUIRED)
endif()

option(CNTR_USE_HDF5 "Build with HDF5 support" ON)

if (CNTR_USE_HDF5)
    message(STATUS "Building with HDF5")
    find_package(HDF5 REQUIRED)
endif()

# ~~ Add Eigen ~~
find_package(Eigen3 REQUIRED)
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})


option(CNTR_MKL "Build with MKL support" ON)
if (CNTR_MKL)
    message(STATUS "Building with MKL")
    set(MKL_THREADING "gnu_thread")
    find_package(MKL REQUIRED)
    include_directories(${MKL_INCLUDE_DIR})
    list(APPEND COMMON_LIBS ${MKL_LIBRARIES})
    add_definitions("-DEIGEN_USE_MKL_ALL")
endif ()


# ~~ Warning Flags ~~
# Re-enable whenever you feel strong enough.
# set(GCC_WARNING_FLAGS "-Wall -Wextra -pedantic")
# add_definitions(${GCC_WARNING_FLAGS})

# ~~ CMake Configuration ~~
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(cntr_SRCS
    fourier.cpp
    linalg_eigen.cpp
    integration.cpp
    integration_extern_templates.cpp
    cntr_convolution_extern_templates.cpp
    cntr_pseudo_convolution_extern_templates.cpp
    cntr_differentiation_extern_templates.cpp
    cntr_dyson_extern_templates.cpp
    cntr_dyson_omp_extern_templates.cpp
    cntr_equilibrium_extern_templates.cpp
    cntr_function_extern_templates.cpp
    cntr_herm_matrix_extern_templates.cpp
    cntr_herm_matrix_timestep_extern_templates.cpp
    cntr_herm_matrix_timestep_view_extern_templates.cpp
    cntr_herm_pseudo_extern_templates.cpp
    cntr_equilibrium_extern_templates.cpp
    cntr_utilities_extern_templates.cpp
    cntr_vie2_extern_templates.cpp
    cntr_getset_extern_templates.cpp
)

set(cntr_MPI_SRCS
    cntr_distributed_array_extern_templates.cpp
    cntr_distributed_timestep_array_extern_templates.cpp
    cntr_mpitools_extern_templates.cpp
)

list(TRANSFORM cntr_SRCS PREPEND "cntr/")
list(TRANSFORM cntr_MPI_SRCS PREPEND "cntr/")



set(cntr_HEADERS
  cntr_bubble_decl.hpp
  cntr_bubble_impl.hpp
  cntr_convolution_decl.hpp
  cntr_convolution_extern_templates.hpp
  cntr_convolution_impl.hpp
  cntr_decl.hpp
  cntr_differentiation_decl.hpp
  cntr_differentiation_extern_templates.hpp
  cntr_differentiation_impl.hpp
  cntr_distributed_array_decl.hpp
  cntr_distributed_array_extern_templates.hpp
  cntr_distributed_array_impl.hpp
  cntr_distributed_timestep_array_decl.hpp
  cntr_distributed_timestep_array_extern_templates.hpp
  cntr_distributed_timestep_array_impl.hpp
  cntr_dyson_decl.hpp
  cntr_dyson_extern_templates.hpp
  cntr_dyson_impl.hpp
  cntr_dyson_omp_decl.hpp
  cntr_dyson_omp_extern_templates.hpp
  cntr_dyson_omp_impl.hpp
  cntr_elements.hpp
  cntr_equilibrium_decl.hpp
  cntr_equilibrium_extern_templates.hpp
  cntr_equilibrium_impl.hpp
  cntr_extern_templates.hpp
  cntr_function_decl.hpp
  cntr_function_extern_templates.hpp
  cntr_function_impl.hpp
  cntr_getset_decl.hpp
  cntr_getset_extern_templates.hpp
  cntr_getset_herm_matrix_inc.hpp
  cntr_getset_herm_matrix_timestep_inc.hpp
  cntr_getset_herm_matrix_timestep_view_inc.hpp
  cntr_getset_impl.hpp
  cntr_global_settings.hpp
  cntr_herm_matrix_decl.hpp
  cntr_herm_matrix_extern_templates.hpp
  cntr_herm_matrix_impl.hpp
  cntr_herm_matrix_timestep_decl.hpp
  cntr_herm_matrix_timestep_extern_templates.hpp
  cntr_herm_matrix_timestep_impl.hpp
  cntr_herm_matrix_timestep_view_decl.hpp
  cntr_herm_matrix_timestep_view_extern_templates.hpp
  cntr_herm_matrix_timestep_view_impl.hpp
  cntr_herm_pseudo_decl.hpp
  cntr_herm_pseudo_extern_templates.hpp
  cntr_herm_pseudo_impl.hpp
  cntr.hpp
  cntr_impl.hpp
  cntr_matsubara_decl.hpp
  cntr_matsubara_impl.hpp
  cntr_mpitools_decl.hpp
  cntr_mpitools_extern_templates.hpp
  cntr_mpitools_impl.hpp
  cntr_pseudo_convolution_decl.hpp
  cntr_pseudo_convolution_extern_templates.hpp
  cntr_pseudo_convolution_impl.hpp
  cntr_pseudodyson_decl.hpp
  cntr_pseudodyson_impl.hpp
  cntr_pseudo_vie2_decl.hpp
  cntr_pseudo_vie2_impl.hpp
  cntr_response_convolution_decl.hpp
  cntr_response_convolution_impl.hpp
  cntr_utilities_decl.hpp
  cntr_utilities_extern_templates.hpp
  cntr_utilities_impl.hpp
  cntr_vie2_decl.hpp
  cntr_vie2_extern_templates.hpp
  cntr_vie2_impl.hpp
  eigen_map.hpp
  eigen_typedef.h
  fourier.hpp
  hdf5_interface_cntr.hpp
  hdf5_interface.hpp
  hdf5_interface_rpa.hpp
  integration_extern_templates.hpp
  integration.hpp
  linalg.hpp
  utils/read_inputfile.hpp
)

list(TRANSFORM cntr_HEADERS PREPEND "include/")

add_library(cntr SHARED
    ${cntr_SRCS}
)
target_compile_options(cntr PRIVATE "-w")

add_library(cntr::cntr ALIAS cntr)

set_target_properties(cntr PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(cntr PUBLIC 
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES ${cntr_HEADERS}
)

if (CNTR_MKL)
    target_link_libraries(cntr PUBLIC MKL::MKL)
endif ()

if (CNTR_USE_HDF5)
    add_library(cntr_hdf5 SHARED cntr/hdf5_interface.cpp)
    add_library(cntr::hdf5 ALIAS cntr_hdf5)
    target_link_libraries(cntr_hdf5 PUBLIC HDF5::HDF5)
    target_sources(cntr_hdf5 PRIVATE 
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES ${cntr_HEADERS}
    )
    install(TARGETS cntr_hdf5
            EXPORT cntr
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    target_link_libraries(cntr PUBLIC cntr_hdf5)
    target_compile_definitions(cntr PUBLIC CNTR_USE_HDF5)
    target_compile_definitions(cntr_hdf5 PUBLIC CNTR_USE_HDF5)
endif (CNTR_USE_HDF5)


if (CNTR_OPENMP)
    message(STATUS "Building with OpenMP")
    find_package(OpenMP REQUIRED)
    target_link_libraries(cntr PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(cntr PUBLIC CNTR_OPENMP)
endif ()

if(CNTR_MPI)
    target_link_libraries(cntr PUBLIC MPI::MPI_CXX)
    target_compile_definitions(cntr PUBLIC CNTR_MPI)
endif()


# ~~ Installation targets ~~
install(TARGETS cntr
        EXPORT cntr
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILE_SET public_headers
)

install(EXPORT cntr
        FILE cntr-config.cmake
        NAMESPACE cntr::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cntr
)

option(BUILD_TEST "Build test suite" ON)
if(BUILD_TEST)
    add_subdirectory(test)
endif(BUILD_TEST)

# ~~ Generation of documentation ~~
# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" OFF)

if(BUILD_DOC)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/Doxyfile.txt)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        #set(DOXYGEN_OUT ${PROJECT_SOURCE_DIR}/Doxyfile.txt)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message(STATUS "Doxygen build started")

         # set input and output files
        set(MAIN_IN ${PROJECT_SOURCE_DIR}/main.dox_inc)
        set(MAIN_OUT ${CMAKE_CURRENT_BINARY_DIR}/main.dox)
        configure_file(${MAIN_IN} ${MAIN_OUT} @ONLY)

        # copy images
        file(GLOB IMAGES ${CMAKE_CURRENT_SOURCE_DIR}/doc/*.png)
        file(COPY ${IMAGES}
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doc/html/)
        

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
      message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif(BUILD_DOC)

# ~~ Include Guard ~~
set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing, that CMake has configured at least once.")
