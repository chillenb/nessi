#ifndef CNTR_MPITOOLS_EXTERN_TEMPLATES_H
#define CNTR_MPITOOLS_EXTERN_TEMPLATES_H

#include "cntr_mpitools_decl.hpp"

namespace cntr{
#ifdef CNTR_USE_MPI
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix_timestep<double> &Gred, 
		herm_matrix_timestep<double> &G);
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix_timestep_view<double> &Gred, 
		herm_matrix_timestep_view<double> &G);
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix<double> &Gred, 
		herm_matrix_timestep<double> &G);
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix_timestep<double> &Gred, 
		herm_matrix<double> &G);
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix<double> &Gred, 
		herm_matrix_timestep_view<double> &G);
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix_timestep_view<double> &Gred, 
		herm_matrix<double> &G);
	///@private
	extern template void Reduce_timestep<double>(int tstp, int root, herm_matrix<double> &Gred, 
		herm_matrix<double> &G);
#endif

} // namespace cntr


#endif // CNTR_MPITOOLS_EXTERN_TEMPLATES_H