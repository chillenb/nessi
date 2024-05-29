#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
#include <cstring>
#include <chrono>


// contour library headers
#include "cntr/cntr.hpp"
#include "cntr/hdf5_interface.hpp"
#include "cntr/hdf5_interface_cntr.hpp"

// local headers to include
#include "formats.hpp"
#include "cntr/utils/read_inputfile.hpp"
#include "Holstein_impurity_decl.hpp"
#include "Holstein_utils_decl.hpp"

using namespace std;

#define CFUNC cntr::function<double>
#define GREEN cntr::herm_matrix<double>
#define GREEN_TSTP cntr::herm_matrix_timestep<double>
#define CINTEG integration::I<double>
#define CPLX complex<double>


//==============================================================================
//         main program
//==============================================================================
int main(int argc,char *argv[]){
	// use maximum order
 	int SolverOrder=MAX_SOLVE_ORDER;
	//..................................................
	//                input
	//..................................................
	int SaveGreen;
	int Nt,Ntau,MatsMaxIter,CorrectorSteps;
	int BootstrapMaxIter;
	double Hopping,El_Ph_g,Phfreq_w0,beta,h;
    double MuChem_MF,MuChem;
    double MatsMaxErr,BootstrapMaxErr;
	std::vector<double> dHopping,dEl_Ph_g;
	char* flin;

	//..................................................
	//                output
	//..................................................
	char* flout;
	int OutEvery;
	//..................................................
	//                internal
	//..................................................
	int tstp;
	double err;
	int Norb=1;
	GREEN G,Sigma,Hyb,Hyb_Sig; //For the impurity site
    GREEN D0,Pi; //For the phonon propagator
    GREEN gtemp; 
	CFUNC g_elph_t,J_hop_t,h0_imp_t,h0_imp_MF_t;//
    CFUNC Xph_t,n_tot_t,Pph_t;
	CFUNC Ekin_t,Enx_MF_t,Enx_corr_t,Eph_qu_t,Eph_cl_t;
	cdmatrix h0_imp,h0_imp_MF;
	//..................................................
	//                timer
	//..................................................
	chrono::time_point<std::chrono::system_clock> start, end, start_tot, end_tot;

	print_line_star(60);
	cout << "   Test program: Holstein model on Bethe lattice in Midal approximation" << endl;
	print_line_star(60);

	start_tot = chrono::system_clock::now();

	cout << endl;
	cout << " reading input file ..." << endl;
	cout << endl;
	try{
		//============================================================================
		//                          (II) READ INPUT
		//============================================================================
		{

			if(argc<2) throw("COMMAND LINE ARGUMENT MISSING");

			if (argc < 3) {
				// Tell the user how to run the program
				std::cerr << " Please provide a prefix for the output files. Exiting ..." << std::endl;
				/* "Usage messages" are a conventional way of telling the user
				 * how to run a program if they enter the command incorrectly.
				 */
				return 1;
			}

			flin=argv[1];

			// system parameters
			find_param(flin,"__Hopping=",Hopping);
			find_param(flin,"__El_Ph_g=",El_Ph_g);
			find_param(flin,"__Phfreq_w0=",Phfreq_w0);
            find_param(flin,"__MuChem_MF=",MuChem_MF);
			find_param(flin,"__beta=",beta);
			// ramp parameters

			// solver parameters
			find_param(flin,"__SaveGreen=",SaveGreen);
			find_param(flin,"__Nt=",Nt);
			find_param(flin,"__Ntau=",Ntau);
			find_param(flin,"__h=",h);
			find_param(flin,"__MatsMaxIter=",MatsMaxIter);
			find_param(flin,"__MatsMaxErr=",MatsMaxErr);
			find_param(flin,"__BootstrapMaxIter=",BootstrapMaxIter);
			find_param(flin,"__BootstrapMaxErr=",BootstrapMaxErr);
			find_param(flin,"__CorrectorSteps=",CorrectorSteps);

			// excitation parameters
			find_param_tvector(flin, "__dHopping=",dHopping,Nt);// The vector size is set to Nt+2,i.e. [-1,Nt]
			find_param_tvector(flin, "__dEl_Ph_g=",dEl_Ph_g,Nt);

			// output parameters
			find_param(flin,"__OutEvery=",OutEvery);
			// output file prefix
			flout=argv[2];

		}
		//============================================================================
		//               (IV) INITIALIZE GREEN'S FUNCTIONS
		//============================================================================
		{
			//allocation
			//system parameters
			g_elph_t = CFUNC(Nt,Norb);
			J_hop_t = CFUNC(Nt,Norb);
            
            h0_imp = cdmatrix::Zero(Norb,Norb);
            h0_imp_MF = cdmatrix::Zero(Norb,Norb);
			h0_imp_t = CFUNC(Nt,Norb);
            h0_imp_MF_t = CFUNC(Nt,Norb);
            
			//electron
			G = GREEN(Nt,Ntau,Norb,FERMION);
			Hyb = GREEN(Nt,Ntau,Norb,FERMION);
			Sigma = GREEN(Nt,Ntau,Norb,FERMION);
			Hyb_Sig = GREEN(Nt,Ntau,Norb,FERMION); //Hyb+Sigma
            
			//phonon
			Pi = GREEN(Nt,Ntau,1,BOSON);
			D0 = GREEN(Nt,Ntau,1,BOSON);

			//observables:energies
            Xph_t = CFUNC(Nt,1);
            Pph_t = CFUNC(Nt,1);
            n_tot_t = CFUNC(Nt,1);
            
			Ekin_t = CFUNC(Nt,1);
			Enx_MF_t = CFUNC(Nt,1);
			Enx_corr_t = CFUNC(Nt,1);
			Eph_qu_t = CFUNC(Nt,1);
            Eph_cl_t = CFUNC(Nt,1);


			//initialization
			//system parameters in equilibrium
            h0_imp_MF(0,0)=-MuChem_MF;
			h0_imp_MF_t.set_constant(h0_imp_MF);

			//system parameters with excitations
            cdmatrix Iu = cdmatrix::Identity(1,1);
			for(int it=-1 ; it<=Nt ; it++) J_hop_t.set_value(it,(Hopping+dHopping[it+1])*Iu);
			for(int it=-1 ; it<=Nt ; it++) g_elph_t.set_value(it,(El_Ph_g+dEl_Ph_g[it+1])*Iu);

			//electron[note]:this part can be improved for better guess.
            cntr::green_equilibrium_mat_bethe(G, beta);
	

			//Hybridization function; Hyb(t,t') = J(t)G0_11(t,t')J(t')
			tstp=-1;
			Hyb.set_timestep(tstp,G);
			Hyb.right_multiply(tstp,J_hop_t);
			Hyb.left_multiply(tstp,J_hop_t);

			//initialize D0
			cntr::green_single_pole_XX(D0,Phfreq_w0,beta,h);
			for(int it=-1 ; it<=Nt ; it++) D0.smul(it,2.0);
            
            
            

		}
		//============================================================================
		//            SELF-CONSISTET SOLUTION, Sigma=0 IN THE BEGINNING
		//============================================================================
		{ // begin Matsubara Dyson iteration
			print_line_minus(50);
			cout << "     Solution of equilibrium impurity problem " << endl;
			print_line_minus(50);

			start = std::chrono::system_clock::now();

			bool matsubara_converged=false;
			tstp=-1;

			gtemp = GREEN(SolverOrder,Ntau,Norb,FERMION);
			gtemp.set_timestep(tstp,G);

			bool FIXPOINT = false;

			for(int iter=0;iter<=MatsMaxIter;iter++){
                
                
                //////////////////
				//Solving impurity
				//////////////////
				//update self-energy
            
                Hols::Sigma_uMig(tstp, G, D0, g_elph_t, Sigma);
                 

				//solve Dyson for impurity G_imp=[(i\partial_t I -eps_MF)delta_c-Hyb-Sigma_imp]^{-1}
				Hyb_Sig.set_timestep(tstp,Hyb);
				Hyb_Sig.incr_timestep(tstp,Sigma,1.0);

				if(FIXPOINT) cntr::dyson_mat(G, 0.0, h0_imp_MF_t, Hyb_Sig, beta, SolverOrder, CNTR_MAT_FIXPOINT);
				else cntr::dyson_mat(G, 0.0, h0_imp_MF_t, Hyb_Sig, beta, SolverOrder, CNTR_MAT_FOURIER);

				//mixing
				G.smul(tstp,0.5);
				G.incr_timestep(tstp,gtemp,0.5);
                
                
                ////////////////////////////////////////////
				//Lattice self-consistency for Bethe lattice
				////////////////////////////////////////////
				//Update hybridization
				Hyb.set_timestep(tstp,G);
				Hyb.right_multiply(tstp,J_hop_t);
				Hyb.left_multiply(tstp,J_hop_t);

				// self-consistency check
				err = cntr::distance_norm2(tstp,G,gtemp);
				if(FIXPOINT) cout << "Fixpoint method: iteration : " << iter << "  |  Error = " << err << endl;
				else cout << "Fourier method: iteration : " << iter << "  |  Error = " << err << endl;
                
				// Output temporal values
				cdmatrix rho_M=cdmatrix::Zero(1,1);
				G.get_mat(Ntau,rho_M);
				cout << "rho_loc:"<<-rho_M(0,0)<<endl;

				if(err<MatsMaxErr && FIXPOINT){
					matsubara_converged=true;
					break;
				}
				else if(err<MatsMaxErr && !FIXPOINT){
					FIXPOINT=true;
				}
				gtemp.set_timestep(tstp,G);
			}

			if(!matsubara_converged){
				cout << endl;
				cout << " Matsubara iteration not converged! Exiting ... " << endl;
				// should end here ....
				return 0;
			}

			print_line_dot(50);
			end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end-start;
			cout << "Time [equilibrium calculation] = " << elapsed_seconds.count() << "s\n\n";
            

            //set equilibrium observables
            tstp=-1;
            cdmatrix rho_M=cdmatrix::Zero(1,1);
            
            G.density_matrix(tstp,rho_M);
            rho_M *= 2.0;//spin number=2
            n_tot_t.set_value(tstp,rho_M);
            Hols::get_phonon_displace(tstp, Xph_t, n_tot_t, g_elph_t, D0, Phfreq_w0, SolverOrder,h);
            
            cdmatrix Xph_0(1,1),g_elph_0(1,1);
            Xph_t.get_value(tstp,Xph_0);
            g_elph_t.get_value(tstp,g_elph_0);
            MuChem = MuChem_MF + real(Xph_0(0,0)*g_elph_0(0,0));
            h0_imp(0,0) = -MuChem;
            h0_imp_t.set_constant(h0_imp);
            
            cout<<"Xph:"<<Xph_0(0,0)<<" n_tot:"<<rho_M(0,0)<<" MuChem:"<<MuChem<<endl;
            
		} // end Matsubara Dyson iteration
        

		//============================================================================
		//            SET FIRST kt TIMESTEPS FROM MATSUBARA
		//============================================================================
		{
			cntr::set_tk_from_mat(G,SolverOrder);
            cntr::set_tk_from_mat(Hyb,SolverOrder);
		}
		//============================================================================
		//           BOOTSTRAPPING PHASE
		//============================================================================
		{ // begin bootstrapping

			print_line_minus(50);
			cout << "     Time propagation: bootstrapping phase " << endl;
			print_line_minus(50);

			start = std::chrono::system_clock::now();

			bool bootstrap_converged=false;

			for(tstp=0; tstp<=SolverOrder; tstp++)
				gtemp.set_timestep(tstp,G);

			for (int iter = 0; iter <= BootstrapMaxIter; iter++) {
                
                //////////////////
				//Solving impurity
				//////////////////

				// update self-energy
                
                Hols::Sigma_uMig(G, D0, g_elph_t, Sigma, SolverOrder);

                //updating phonon displacement
                for(tstp=0; tstp<=SolverOrder; tstp++){
                    cdmatrix rho_M=cdmatrix::Zero(1,1);
                    G.density_matrix(tstp,rho_M);
                    rho_M *= 2.0;//spin number=2
                    n_tot_t.set_value(tstp,rho_M);
                }
                
                Hols::get_phonon_displace(Xph_t, n_tot_t, g_elph_t, D0, Phfreq_w0, SolverOrder,h);
                
				//solve Dyson for impurity
				for(tstp=0; tstp<=SolverOrder; tstp++){
                    
                    cdmatrix Xph_tmp(1,1),g_elph_tmp(1,1),h0_imp_MF_tmp(1,1);
                    Xph_t.get_value(tstp,Xph_tmp);
                    g_elph_t.get_value(tstp,g_elph_tmp);
                    h0_imp_MF_tmp = h0_imp + Xph_tmp*g_elph_tmp;
                    h0_imp_MF_t.set_value(tstp,h0_imp_MF_tmp);
                    
					Hyb_Sig.set_timestep(tstp,Hyb);
					Hyb_Sig.incr_timestep(tstp,Sigma,1.0);
				}

				cntr::dyson_start(G, 0.0, h0_imp_MF_t, Hyb_Sig, beta, h, SolverOrder);

                ////////////////////////////////////////////
				//Lattice self-consistency for Bethe lattice
				////////////////////////////////////////////
                //Update hybridization
				for(tstp=0; tstp<=SolverOrder; tstp++){
					Hyb.set_timestep(tstp,G);
					Hyb.right_multiply(tstp,J_hop_t);
					Hyb.left_multiply(tstp,J_hop_t);
				}
                

				// self-consistency check
				err=0.0;
				for(tstp=0; tstp<=SolverOrder; tstp++) {
					err += cntr::distance_norm2(tstp,G,gtemp);
				}
				cout << "bootstrap iteration : " << iter << "  |  Error = " << err << endl;
				if(err<BootstrapMaxErr && iter>2){
					bootstrap_converged=true;
					break;
				}
				for(tstp=0; tstp<=SolverOrder; tstp++) {
					gtemp.set_timestep(tstp,G);
				}
			}

			print_line_dot(50);
			end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds= end -start;
			cout << "Time [bootstrapping] = " << elapsed_seconds.count() << "s\n";

			if(!bootstrap_converged){
				cout << endl;
				cout << " Bootstrap iteration not converged! Exiting ... " << endl;
				// should end here ....
				return 0;
			}

		} // end bootstrapping

		//============================================================================
		//             TIME PROPAGATION
		//============================================================================
		{ // begin propagation loop
			print_line_minus(50);
			cout << "               Time propagation" << endl;
			print_line_minus(50);

			start = std::chrono::system_clock::now();

			for(tstp = SolverOrder+1; tstp <= Nt; tstp++){

				if (tstp%50 ==0) cout<<"tstp:"<<tstp<<endl;

				// Predictor: extrapolation
				cntr::extrapolate_timestep(tstp-1,G,SolverOrder);
                cntr::extrapolate_timestep(tstp-1,Hyb,SolverOrder);

				// Corrector
				for (int iter=0; iter < CorrectorSteps; iter++){
                    
                    //////////////////
					//Solving impurity
					//////////////////
                    
                    cdmatrix rho_M=cdmatrix::Zero(1,1);
                    cdmatrix Xph_tmp(1,1),g_elph_tmp(1,1),h0_imp_MF_tmp(1,1);
                    
					//update self-energy
                    Hols::Sigma_uMig(tstp, G, D0, g_elph_t, Sigma);

                    //update phonon displacement 
                    G.density_matrix(tstp,rho_M);
                    rho_M *= 2.0;//spin number=2
                    n_tot_t.set_value(tstp,rho_M);
                    
                    Hols::get_phonon_displace(tstp, Xph_t, n_tot_t, g_elph_t, D0, Phfreq_w0, SolverOrder,h);
                    
					//solve Dyson for impurity
                    Xph_t.get_value(tstp,Xph_tmp);
                    g_elph_t.get_value(tstp,g_elph_tmp);
                    h0_imp_MF_tmp = h0_imp + Xph_tmp*g_elph_tmp;
                    h0_imp_MF_t.set_value(tstp,h0_imp_MF_tmp);
                    
					Hyb_Sig.set_timestep(tstp,Hyb);
					Hyb_Sig.incr_timestep(tstp,Sigma,1.0);
                    
					cntr::dyson_timestep(tstp, G, 0.0, h0_imp_MF_t, Hyb_Sig, beta, h, SolverOrder);
                    
                    ////////////////////////////////////////////
					//Lattice self-consistency for Bethe lattice
					////////////////////////////////////////////
                    
					//Update hybridization
					Hyb.set_timestep(tstp,G);
					Hyb.right_multiply(tstp,J_hop_t);
					Hyb.left_multiply(tstp,J_hop_t);

				}
			}

			print_line_dot(50);
			end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end-start;
			cout << "Time [KBEs] = " << elapsed_seconds.count() << "s\n";

		} // end propagation loop
        
        
		//============================================================================
		//             Postprocess2: Evaluateion of energies and P
		//============================================================================
		{
			print_line_minus(50);
			cout << "               Evaluation of Energies and P" << endl;
			print_line_minus(50);

			cdmatrix Mat_tmp(1,1);

            // Ekin for two spin
			for(tstp = -1; tstp <= Nt ; tstp++){
                
                Mat_tmp(0,0) = 4.0*cntr::correlation_energy(tstp, G, Hyb, beta, h, SolverOrder);
				Ekin_t.set_value(tstp,Mat_tmp);

			}
            
			// Enk_MF
            for(tstp = -1; tstp <= Nt ; tstp++){
                cdmatrix Xph_tmp(1,1),g_elph_tmp(1,1),n_tot_tmp(1,1);
                Xph_t.get_value(tstp,Xph_tmp);
                g_elph_t.get_value(tstp,g_elph_tmp);
                n_tot_t.get_value(tstp,n_tot_tmp);
                Mat_tmp(0,0) = g_elph_tmp(0,0)*Xph_tmp(0,0)*n_tot_tmp(0,0);
                Enx_MF_t.set_value(tstp,Mat_tmp);
                
            }
            
			// Enx_corr
			for(tstp = -1; tstp <= Nt ; tstp++){
				Mat_tmp(0,0) = 4.0*cntr::correlation_energy(tstp, G, Sigma, beta, h, SolverOrder);
				Enx_corr_t.set_value(tstp,Mat_tmp);
			}

            // Eph_cl and P
            Hols::get_phonon_momentum(Pph_t, n_tot_t, g_elph_t, D0, Phfreq_w0, SolverOrder, h);
            Hols::evaluate_phonon_energy_cl(Eph_cl_t, Xph_t, Pph_t, Phfreq_w0);
            
			// Eph_corr
            for(tstp = -1; tstp <= Nt ; tstp++) Pi.set_timestep_zero(tstp);
            Hols::evaluate_phonon_energy_qu(Eph_qu_t, D0, Pi, SolverOrder, beta, h, Phfreq_w0);
            
		}
		//============================================================================
		//             OUT PUT
		//============================================================================

		{   char file_occ[255];
			strcpy(file_occ,flout);
			strcat(file_occ,"_obs.dat");
			ofstream f_out;
			f_out.open (file_occ);

			//observables
			cdmatrix rho(Norb,Norb);
			cdmatrix XX(1,1);
			cdmatrix ekin(1,1);
			cdmatrix enx_mf(1,1);
			cdmatrix enx_corr(1,1);
			cdmatrix eph_qu(1,1);
            cdmatrix eph_cl(1,1);
			cdmatrix etot(1,1);
            cdmatrix Xph(1,1);
            cdmatrix Pph(1,1);

			int nobs=0;
			f_out<<"#0:time ";

			for(int a=0; a<Norb; a++){
				for(int b=0; b<Norb; b++){
					nobs++;
					f_out << nobs << ":rho(" << a << "," << b << ").re ";
					nobs++;
					f_out << nobs << ":rho(" << a << "," << b << ").im ";
				}
			}
            
            nobs++;
			f_out<<nobs<<":Xph ";
            nobs++;
			f_out<<nobs<<":Pph ";
			nobs++;
			f_out<<nobs<<":XX ";
			nobs++;
			f_out<<nobs<<":Ekin ";
			nobs++;
			f_out<<nobs<<":Exn_MF ";
			nobs++;
			f_out<<nobs<<":Exn_corr ";
            nobs++;
			f_out<<nobs<<":Eph_cl ";
			nobs++;
			f_out<<nobs<<":Eph_corr ";
			nobs++;
			f_out<<nobs<<":Etot ";

			f_out<<endl;
			for(tstp=0; tstp <= Nt; tstp++){

				G.density_matrix(tstp, rho);
				D0.get_les(tstp,tstp,XX);
				XX(0,0)*=CPLX(0.0,1.0);
                Xph_t.get_value(tstp,Xph);
                Pph_t.get_value(tstp,Pph);
				Ekin_t.get_value(tstp,ekin);
				Enx_MF_t.get_value(tstp,enx_mf);
				Enx_corr_t.get_value(tstp,enx_corr);
                Eph_cl_t.get_value(tstp,eph_cl);
				Eph_qu_t.get_value(tstp,eph_qu);
				etot = ekin + enx_mf + enx_corr + eph_cl+ eph_qu;

				f_out << tstp*h << "  " ;
                
				for(int a=0; a<Norb; a++){
					for(int b=0; b<Norb; b++){
						f_out << rho(a,b).real() << " " << rho(a,b).imag()<<" ";
					}
				}
		
                f_out<< Xph(0,0).real()<<" ";
                f_out<< Pph(0,0).real()<<" ";
				f_out<< XX(0,0).real()<<" ";
				f_out<< ekin(0,0).real()<<" ";
				f_out<< enx_mf(0,0).real()<<" ";
				f_out<< enx_corr(0,0).real()<<" ";
                f_out<< eph_cl(0,0).real()<<" ";
				f_out<< eph_qu(0,0).real()<<" ";
				f_out<< etot(0,0).real()<<" ";

				f_out << endl;
			}
			f_out.close();
		}
			////////////
			/// HDF5////
			////////////
		{
            
			char fnametmp[1000];
			std::sprintf(fnametmp,"%s_green.h5",flout);
			hid_t file_id = open_hdf5_file(std::string(fnametmp));
			hid_t group_id = create_group(file_id,"parm");

			//parameters
			store_int_attribute_to_hid(group_id,"Nt",Nt);
			store_int_attribute_to_hid(group_id,"Ntau",Ntau);
			store_double_attribute_to_hid(group_id,"dt",h);
			store_double_attribute_to_hid(group_id,"beta",beta);
			close_group(group_id);

			//electron
			//G at each time step t_n: G^R(t_n,t), G^<(t,t_n), G^tv(t_n,\tau) for t<t_n
			group_id = create_group(file_id, "Gloc_slice");
            G.write_to_hdf5_slices(group_id,OutEvery);
            close_group(group_id);
            // G(t_rel,t_av) for t_av = [0,OutEvery*dt,2*OutEvery*dt...].
            group_id = create_group(file_id, "Gloc_tavrel");
            G.write_to_hdf5_tavtrel(group_id,OutEvery);
            close_group(group_id);

			//phonon
			group_id = create_group(file_id, "Dloc_slice");
            D0.write_to_hdf5_slices(group_id,OutEvery);
            close_group(group_id);

            group_id = create_group(file_id, "Dloc_tavrel");
            D0.write_to_hdf5_tavtrel(group_id,OutEvery);
            close_group(group_id);

            close_hdf5_file(file_id);
             

		} // end output

		end_tot = std::chrono::system_clock::now();
		std::chrono::duration<double> runtime_seconds = end_tot-start_tot;

		cout << endl;
		cout << endl;
		cout << "Time [total] = " << runtime_seconds.count() << "s\n";

		print_line_star(60);


    } // try
    catch(char *message){
		cerr << "exception\n**** " << message << " ****" << endl;
		cerr << " No input file found. Exiting ... " << endl;
    }
    catch(...){
		cerr << " No input file found. Exiting ... " << endl;
    }
    return 0;
  }
  //==============================================================================
