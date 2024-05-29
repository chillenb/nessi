# -- Catch tests compiled into target "runtest"


# Copy h5 files to binary dir
FILE(GLOB all_h5_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h5)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${all_h5_files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(CNTR_TEST_SRC
    runtest.cpp
    bubble.cpp    
    convolution.cpp
    downfold.cpp
    dyson.cpp
    dyson_new.cpp
    equilibrium.cpp
    function.cpp
    getset.cpp
    getset_nonherm.cpp
    green_single_pole_XX.cpp
    herm_matrix_algebra.cpp
    herm_matrix_member.cpp  
    herm_matrix_readwrite.cpp
    herm_matrix_setget_timestep.cpp
    herm_matrix_submatrix.cpp
    herm_member_timestep.cpp
    herm_member_timestep_view.cpp
    integration.cpp
    linalg.cpp
    matsubara.cpp    
    utilities.cpp
  )

set(CNTR_TEST_SRC_HDF5
  herm_matrix_hdf5.cpp
  herm_matrix_readwrite_hdf5.cpp
  fkm_bethe_quench.cpp
)

add_executable(runtest 
  ${CNTR_TEST_SRC}
  $<$<BOOL:${CNTR_USE_HDF5}>:${CNTR_TEST_SRC_HDF5}>
)

if (CNTR_MPI)
  add_executable(runtest_mpi
    runtest_mpi.cpp
  )
endif (CNTR_MPI)


target_link_libraries(runtest 
  PRIVATE
  cntr
)


if(CNTR_USE_HDF5)
  target_link_libraries(runtest PRIVATE cntr::hdf5)
endif(CNTR_USE_HDF5)


# -- "test_all" target for running all the test using Catch directly

add_custom_target(test COMMAND runtest -d yes DEPENDS runtest)

if (CNTR_MPI)
   add_custom_target(test_mpi COMMAND ${MPIEXEC} -np 2 runtest_mpi -d yes DEPENDS runtest_mpi)
endif (CNTR_MPI)

# ~~ Temp programs

#add_executable(fkm_bethe_quench.ex fkm_bethe_quench.cpp)
#target_link_libraries(fkm_bethe_quench.ex cntr ${COMMON_LIBS})
#install(TARGETS fkm_bethe_quench.ex DESTINATION bin)
