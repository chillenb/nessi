# A list of executables to build

set( regular_EXECUTABLES
     hubbard_chain_2b.x
     hubbard_chain_gw.x
     hubbard_chain_tpp.x
     test_equilibrium.x
     test_nonequilibrium.x
     integration.x
     test_jason.x
)

set( withhdf5_EXECUTABLES
     Holstein_bethe_Nambu_Migdal.x
     Holstein_bethe_Nambu_uMig.x
     Holstein_bethe_Migdal.x
     #Holstein_bethe_cdw_uMig.x
     #Holstein_bethe_2temp_cdw_uMig.x
     Holstein_bethe_uMig.x
     Holstein_impurity_singlebath_Migdal.x
     Holstein_impurity_singlebath_uMig.x
     fkm_nessi_2x2.x
     fkm_bethe_quench.x
)

set (mpi_EXECUTABLES
    gw.x
)

set( all_EXECUTABLES ${regular_EXECUTABLES} ${withhdf5_EXECUTABLES} ${mpi_EXECUTABLES} )

set( EXE_hubbard_chain_2b.x_SOURCES hubbard_chain_selfen_impl.cpp hubbard_chain_2b.cpp )
set( EXE_hubbard_chain_gw.x_SOURCES hubbard_chain_selfen_impl.cpp hubbard_chain_gw.cpp )
set( EXE_hubbard_chain_tpp.x_SOURCES hubbard_chain_selfen_impl.cpp hubbard_chain_tpp.cpp )

set( EXE_test_equilibrium.x_SOURCES test_equilibrium.cpp )
set( EXE_test_nonequilibrium.x_SOURCES test_nonequilibrium.cpp )
set( EXE_test_jason.x_SOURCES test_jason.cpp )
set( EXE_fkm_nessi_2x2.x_SOURCES fkm_nessi_2x2.cpp )
set( EXE_fkm_bethe_quench.x_SOURCES fkm_bethe_quench.cpp )
set( EXE_gw.x_SOURCES gw.cpp gw_latt_impl.cpp gw_kpoints_impl.cpp  gw_selfene_impl.cpp )
set( EXE_integration.x_SOURCES integration.cpp )
set( EXE_Holstein_bethe_Nambu_Migdal.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_bethe_Nambu_Migdal.cpp )
set( EXE_Holstein_bethe_Nambu_uMig.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_bethe_Nambu_uMig.cpp )

set( EXE_Holstein_bethe_Migdal.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_bethe_Migdal.cpp )
set( EXE_Holstein_bethe_uMig.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_bethe_uMig.cpp )
set( EXE_Holstein_impurity_singlebath_Migdal.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_impurity_singlebath_Migdal.cpp )
set( EXE_Holstein_impurity_singlebath_uMig.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_impurity_singlebath_uMig.cpp )
#set( EXE_Holstein_bethe_cdw_uMig.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_bethe_cdw_uMig.cpp)
#set( EXE_Holstein_bethe_2temp_cdw_uMig.x_SOURCES Holstein_impurity_impl.cpp Holstein_utils_impl.cpp Holstein_bethe_2temp_cdw_uMig.cpp)

set(demos ${regular_EXECUTABLES})

foreach( exe ${regular_EXECUTABLES} )
    # declare the target...
    add_executable( ${exe} ${EXE_${exe}_SOURCES} )
    target_link_libraries(${exe} PUBLIC cntr::cntr Eigen3::Eigen)

    if(CNTR_MKL)
        target_link_libraries(${exe} PUBLIC MKL::MKL)
        target_compile_definitions(${exe} PUBLIC -DEIGEN_USE_MKL_ALL)
    endif()
endforeach()

if(CNTR_HDF5)
    foreach( exe ${withhdf5_EXECUTABLES} )
        # declare the target...
        add_executable( ${exe} ${EXE_${exe}_SOURCES} )
        target_link_libraries(${exe} PUBLIC cntr::cntr cntr::cntr_hdf5 Eigen3::Eigen)
    
        if(CNTR_MKL)
            target_link_libraries(${exe} PUBLIC MKL::MKL)
            target_compile_definitions(${exe} PUBLIC -DEIGEN_USE_MKL_ALL)
        endif()
    endforeach()
    list(APPEND demos ${withhdf5_EXECUTABLES})
endif(CNTR_HDF5)

if(CNTR_MPI) 
    foreach( exe ${mpi_EXECUTABLES} )
        # declare the target...
        add_executable( ${exe} ${EXE_${exe}_SOURCES} )
        target_link_libraries(${exe} PUBLIC cntr::cntr Eigen3::Eigen)
    
        if(CNTR_MKL)
            target_link_libraries(${exe} PUBLIC MKL::MKL)
            target_compile_definitions(${exe} PUBLIC -DEIGEN_USE_MKL_ALL)
        endif()
        target_link_libraries(${exe} PUBLIC MPI::MPI_CXX)
    endforeach()
    list(APPEND demos ${mpi_EXECUTABLES})
endif(CNTR_MPI)

if(CNTR_OPENMP)
    foreach( exe ${demos} )
        target_link_libraries(${exe} PUBLIC OpenMP::OpenMP_CXX)
    endforeach()
endif()

