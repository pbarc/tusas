//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Triad National Security, LLC.  This file is part of the
//  Tusas code (LA-CC-17-001) and is subject to the revised BSD license terms
//  in the LICENSE file found in the top-level directory of this distribution.
//
//////////////////////////////////////////////////////////////////////////////



#ifndef FUNCTION_DEF_HPP
#define FUNCTION_DEF_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include "basis.hpp"
	
#include "Teuchos_ParameterList.hpp"
#include <Kokkos_Core.hpp>



#if defined (KOKKOS_HAVE_CUDA) || defined (KOKKOS_ENABLE_CUDA)
#define TUSAS_DEVICE __device__
#define TUSAS_HAVE_CUDA
#else
#define TUSAS_DEVICE /**/ 
#endif



/** Definition for residual function. Each residual function is called at each Gauss point for each equation with this signature:
- NAME:     name of function to call
- const boost::ptr_vector<Basis> &basis:     an array of basis function objects indexed by equation
- const int &i:    the current test function (row in residual vector)
- const double &dt_: the timestep size as prescribed in input file						
- const double &t_theta_: the timestep parameter as prescribed in input file
- const double &time: the current simulation time
- const int &eqn_id: the index of the current equation


*/


#define RES_FUNC(NAME)  double NAME(const boost::ptr_vector<Basis> &basis,\
                                    const int &i,\
                                    const double &dt_,\
                                    const double &dtold_,\
			            const double &t_theta_,\
			            const double &t_theta2_,\
                                    const double &time,\
				    const int &eqn_id)

/** Definition for precondition function. Each precondition function is called at each Gauss point for each equation with this signature:
- NAME:     name of function to call
- const boost::ptr_vector<Basis> &basis:     an array of basis function objects indexed by equation
- const int &i:    the current basis function (row in preconditioning matrix)
- const int &j:    the current test function (column in preconditioning matrix)
- const double &dt_: the timestep size as prescribed in input file						
- const double &t_theta_: the timestep parameter as prescribed in input file
- const double &time: the current simulation time
- const int &eqn_id: the index of the current equation


*/

#define PRE_FUNC(NAME)  double NAME(const boost::ptr_vector<Basis> &basis,\
                                    const int &i,\
				    const int &j,\
				    const double &dt_,\
				    const double &t_theta_,\
				    const int &eqn_id)

/** Definition for initialization function. Each initialization function is called at each node for each equation at the beginning of the simualtaion with this signature:
- NAME:     name of function to call
- const double &x: the x-ccordinate of the node
- const double &y: the y-ccordinate of the node
- const double &z: the z-ccordinate of the node
- const int &eqn_id: the index of the current equation

*/

#define INI_FUNC(NAME)  double NAME(const double &x,\
			            const double &y,\
			            const double &z,\
				    const int &eqn_id) 


/** Definition for Dirichlet function. Each Dirichlet function is called at each node for each equation with this signature:
- NAME:     name of function to call
- const double &x: the x-ccordinate of the node
- const double &y: the y-ccordinate of the node
- const double &z: the z-ccordinate of the node
- const int &eqn_id: the index of the current equation
- const double &t: the current time

*/

#define DBC_FUNC(NAME)  double NAME(const double &x,\
			            const double &y,\
			            const double &z,\
			            const double &t) 

/** Definition for Neumann function. Each Neumann function is called at each Gauss point for the current equation with this signature:
- NAME:     name of function to call
- const Basis *basis:     basis function object for current equation
- const int &i:    the current basis function (row in residual vector)
- const double &dt_: the timestep size as prescribed in input file						
- const double &t_theta_: the timestep parameter as prescribed in input file
- const double &time: the current simulation time


*/

#define NBC_FUNC(NAME)  double NAME(const Basis *basis,\
				    const int &i,\
				    const double &dt_,\
				    const double &t_theta_,\
				    const double &time)

/** Definition for post-process function. Each post-process function is called at each node for each equation at the end of each timestep with this signature:
- NAME:     name of function to call
- const double *u: an array of solution values indexed by equation
- const double *gradu: an array of gradient values indexed by equation, coordinates (NULL unless error estimation is activated)
- const double *xyz: an array of coordinates indexed by equation, coordinates
- const double &time: the current simulation time

*/

#define PPR_FUNC(NAME)  double NAME(const double *u,\
				    const double *uold,\
				    const double *uoldold,\
				    const double *gradu,\
				    const double *xyz,\
				    const double &time,\
				    const double &dt,\
				    const double &dtold,\
				    const int &eqn_id)

/** Parameter function to propogate information from input file. Each parameter function is called at the beginning of each simulation.
- NAME:     name of function to call
- Teuchos::ParameterList *plist: paramterlist containing information defined in input file

*/

#define PARAM_FUNC(NAME) void NAME(Teuchos::ParameterList *plist) 


namespace heat
{

/** Residual function for heat equation test problem. */
RES_FUNC(residual_heat_test_)
{

  //for heat eqn:
  //u[x,y,t]=exp(-2 pi^2 t)sin(pi x)sin(pi y)

  //for neumann:
  //u[x,y,t]=exp( -1/4 pi^2 t)sin(pi/4 x)
  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  double dtestdz = basis[0].dphidz[i];
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;

  double ut = (u-uold)/dt_*test;
  double divgradu = (basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  double divgradu_old = (basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
 
  return ut + t_theta_*divgradu + (1.-t_theta_)*divgradu_old;
}

PRE_FUNC(prec_heat_test_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  double dtestdz = basis[0].dphidz[i];

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz;
  double u_t =test * basis[0].phi[j]/dt_;
  return u_t + t_theta_*divgrad;
}

INI_FUNC(init_heat_test_)
{

  double pi = 3.141592653589793;

  return sin(pi*x)*sin(pi*y);
}
}//namespace heat

namespace timeonly
{
const double pi = 3.141592653589793;
  //const double lambda = 10.;//pi*pi;
const double lambda = pi*pi;

const double ff(const double &u)
{
  return -lambda*u;
}

RES_FUNC(residual_test_)
{
  //test function
  const double test = basis[0].phi[i];
  //u, phi
  const double u[3] = {basis[0].uu,basis[0].uuold,basis[0].uuoldold};

  const double ut = (u[0]-u[1])/dt_*test;

  const double f[3] = {ff(u[0])*test,ff(u[1])*test,ff(u[2])*test};
  //std::cout<<u[1]<<"  "<<u[2]<<std::endl;
  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

INI_FUNC(init_test_)
{
  return 1.;
}
PPR_FUNC(postproc1_)
{
  const double uu = u[eqn_id];

//   const double x = xyz[0];
//   const double y = xyz[1];

  const double pi = 3.141592653589793;
  //d2udt2 = 4 E^(-2 \[Pi]^2 t) \[Pi]^4 Sin[\[Pi] x] Sin[\[Pi] y];

  const double uex = exp(-lambda*time);
  return uex-uu;
}
PPR_FUNC(postproc2_)
{
  //const double uu = u[eqn_id];

//   const double x = xyz[0];
//   const double y = xyz[1];

  //const double pi = 3.141592653589793;
  //d2udt2 = 4 E^(-2 \[Pi]^2 t) \[Pi]^4 Sin[\[Pi] x] Sin[\[Pi] y];

  const double uex = exp(-lambda*time);

  return uex;
}
PPR_FUNC(postproc3_)
{
  //const double uu = u[eqn_id];

//   const double x = xyz[0];
//   const double y = xyz[1];

  //const double pi = 3.141592653589793;
  //d2udt2 = 4 E^(-2 \[Pi]^2 t) \[Pi]^4 Sin[\[Pi] x] Sin[\[Pi] y];

  //const double uex = exp(-pi*pi*time);

  return u[eqn_id];
}
}//namespace timeonly

namespace autocatalytic4
{

  //https://documen.site/download/math-3795-lecture-18-numerical-solution-of-ordinary-differential-equations-goals_pdf#
  //https://media.gradebuddy.com/documents/2449908/0c88cf76-7605-4aec-b2ad-513ddbebefec.pdf

const double k1 = .0001;
const double k2 = 1.;
const double k3 = .0008;

RES_FUNC(residual_a_)
{
  const double test = basis[0].phi[i];
  //u, phi
  const double u = basis[0].uu;
  const double uold = basis[0].uuold;
  const double uoldold = basis[0].uuoldold;

  const double ut = (u-uold)/dt_*test;
  //std::cout<<ut<<" "<<dt_<<" "<<time<<std::endl;

  double f[3];
  f[0] = (-k1*u       - k2*u*basis[1].uu)*test;
  f[1] = (-k1*uold    - k2*u*basis[1].uuold)*test;
  f[2] = (-k1*uoldold - k2*u*basis[1].uuoldold)*test;
  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC(residual_b_)
{
  const double test = basis[1].phi[i];
  //u, phi
  const double u = basis[1].uu;
  const double uold = basis[1].uuold;
  //const double uoldold = basis[1].uuoldold;
  const double a = basis[0].uu;
  const double aold = basis[0].uuold;
  const double aoldold = basis[0].uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k1*a       - k2*a*u                       + 2.*k3*basis[2].uu)*test;
  f[1] = (k1*aold    - k2*aold*uold                 + 2.*k3*basis[2].uuold)*test;
  f[2] = (k1*aoldold - k2*aoldold*basis[1].uuoldold + 2.*k3*basis[2].uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC(residual_ab_)
{
  const double test = basis[1].phi[i];
  //u, phi
  const double u = basis[2].uu;
  const double uold = basis[2].uuold;
  //const double uoldold = basis[1].uuoldold;
  const double b = basis[1].uu;
  const double bold = basis[1].uuold;
  const double boldold = basis[1].uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k2*b*basis[0].uu             - k3*u)*test;
  f[1] = (k2*bold*basis[0].uuold       - k3*uold)*test;
  f[2] = (k2*boldold*basis[0].uuoldold - k3*basis[2].uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC(residual_c_)
{
  const double test = basis[1].phi[i];
  //u, phi
  const double u = basis[3].uu;
  const double uold = basis[3].uuold;
  //const double uoldold = basis[1].uuoldold;
  const double a = basis[0].uu;
  const double aold = basis[0].uuold;
  const double aoldold = basis[0].uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k1*a       + k3*basis[2].uu)*test;
  f[1] = (k1*aold    + k3*basis[2].uuold)*test;
  f[2] = (k1*aoldold + k3*basis[2].uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

INI_FUNC(init_a_)
{
  return 1.;
}

INI_FUNC(init_b_)
{
  return 0.;
}

INI_FUNC(init_ab_)
{
  return 0.;
}


INI_FUNC(init_c_)
{
  return 0.;
}

}//namespace autocatalytic4

namespace chem
{

  //https://documen.site/download/math-3795-lecture-18-numerical-solution-of-ordinary-differential-equations-goals_pdf#
  //https://media.gradebuddy.com/documents/2449908/0c88cf76-7605-4aec-b2ad-513ddbebefec.pdf

const double k1 = .01;
const double k2 = 1.;
const double k3 = .0008;

RES_FUNC(residual_a_)
{
  const double test = basis[0].phi[i];
  //u, phi
  const double u = basis[0].uu;
  const double uold = basis[0].uuold;
  const double uoldold = basis[0].uuoldold;

  const double ut = (u-uold)/dt_*test;

  double f[3];
  f[0] = -k1*u*basis[1].uu*test;
  f[1] = -k1*uold*basis[1].uuold*test;
  f[2] = -k1*uoldold*basis[1].uuold*test;
  //if(t_theta2_>.99)std::cout<<t_theta2_<<" "<<t_theta_<<" "<<dt_<<" "<<dtold_<<std::endl;
  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC(residual_b_)
{
  const double test = basis[1].phi[i];
  //u, phi
  const double u = basis[1].uu;
  const double uold = basis[1].uuold;
  //const double uoldold = basis[1].uuoldold;
  const double a = basis[0].uu;
  const double aold = basis[0].uuold;
  const double aoldold = basis[0].uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = -k1*a*u*test;
  f[1] = -k1*aold*uold*test;
  f[2] = -k1*aoldold*basis[1].uuoldold*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC(residual_ab_)
{
  const double test = basis[1].phi[i];
  //u, phi
  const double u = basis[2].uu;
  const double uold = basis[2].uuold;
  //const double uoldold = basis[1].uuoldold;
  const double b = basis[1].uu;
  const double bold = basis[1].uuold;
  const double boldold = basis[1].uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k2*b*basis[0].uu - k3*u)*test;
  f[1] = (k2*bold*basis[0].uuold - k3*uold)*test;
  f[2] = (k2*boldold*basis[0].uuoldold - k3*basis[2].uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}


RES_FUNC(residual_c_)
{
  const double test = basis[1].phi[i];
  //u, phi
  const double u = basis[2].uu;
  const double uold = basis[2].uuold;
  //const double uoldold = basis[1].uuoldold;
  const double a = basis[0].uu;
  const double aold = basis[0].uuold;
  const double aoldold = basis[0].uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = k1*a*basis[1].uu*test;
  f[1] = k1*aold*basis[1].uuold*test;
  f[2] = k1*aoldold*basis[1].uuoldold*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

INI_FUNC(init_a_)
{
  return 1.;
}

INI_FUNC(init_b_)
{
  return .5;
}

INI_FUNC(init_ab_)
{
  return 0.;
}


INI_FUNC(init_c_)
{
  return 0.;
}

}//namespace chem


namespace timeadapt
{
PPR_FUNC(d2udt2_)
{
  const double uu = u[eqn_id];
  const double uuold = uold[eqn_id];
  const double uuoldold = uoldold[eqn_id];

  const double duuold = (uuold-uuoldold)/dtold;
  const double duu = (uu-uuold)/dt;
  const double d2udt2 = (duu-duuold)/dt;
  //const double ae = .5*dt*dt*d2udt2;

//   double r = 0;
//   r = abs(d2udt2/1.);
  //if (uu*uu > 1.e-8) r = abs(d2udt2/uu);
  //r = std::max(abs(d2udt2/uu),1.e-10);
  //return sqrt(2.*tol*abs(uu)/abs(d2udt2));
  //return abs(d2udt2/uu);
  //std::cout<<r<<" "<<uu*uu<<" "<<d2udt2<<std::endl;
  return .5*dt*dt*d2udt2;
}
PPR_FUNC(predictor_fe_)
{
  const double uu = u[eqn_id];
  //const double uuold = uold[eqn_id];
  //const double uuoldold = uoldold[eqn_id];
  const double uupred = gradu[eqn_id];//hack for now
  //std::cout<<eqn_id<<" "<<uold[eqn_id]<<std::endl;
  //std::cout<<eqn_id<<" "<<uu<<"  "<<uupred<<"  "<<uu - uupred<<std::endl;
  return (uu - uupred);
}
PPR_FUNC(postproc1_)
{
  const double uu = u[eqn_id];

  const double x = xyz[0];
  const double y = xyz[1];

  const double pi = 3.141592653589793;
  //d2udt2 = 4 E^(-2 \[Pi]^2 t) \[Pi]^4 Sin[\[Pi] x] Sin[\[Pi] y];

  const double uex = exp(-2.*pi*pi*time)*sin(pi*x)*sin(pi*y);
  //const double d2udt2ex = 4.*pi*pi*pi*pi*exp(-2.*pi*pi*time)*sin(pi*x)*sin(pi*y);
  //return sqrt(2.*tol*abs(uu)/abs(d2udt2));
  //return d2udt2;
  
  //return abs(d2udt2ex/uex);
  //return abs(d2udt2ex/1.);
  return uu;
}
PPR_FUNC(postproc2_)
{
  //const double uu = u[eqn_id];

  const double x = xyz[0];
  const double y = xyz[1];

  const double pi = 3.141592653589793;
  //d2udt2 = 4 E^(-2 \[Pi]^2 t) \[Pi]^4 Sin[\[Pi] x] Sin[\[Pi] y];

  const double uex = exp(-2.*pi*pi*time)*sin(pi*x)*sin(pi*y);
  //const double d2udt2ex = 4.*pi*pi*pi*pi*exp(-2.*pi*pi*time)*sin(pi*x)*sin(pi*y);
  //return sqrt(2.*tol*abs(uu)/abs(d2udt2));
  //return d2udt2;
  
  //return abs(d2udt2ex/uex);
  //return abs(d2udt2ex/1.);
  const double uuoldold = gradu[eqn_id];//hack for now
  return uuoldold;
}
PPR_FUNC(normu_)
{
  const double uu = u[eqn_id];

  return uu;
}
PPR_FUNC(dynamic_)
{
  const double uu = u[eqn_id];
  const double uuold = uold[eqn_id];
//   std::cout<<uu<<" "<<uuold<<" "<<dt<<std::endl;
//   return uu*dt/(uu-uuold);
  return (uu-uuold)/dt/uu;
}
}//namespace timeadapt

double rand_phi_zero_(const double &phi, const double &random_number)
{
  return 0.;
}

//furtado
double hp1_furtado_(const double &phi,const double &c)
{
  double dH = 2.35e9;
  double Tm = 1728.;
  return -30.*dH/Tm*phi*phi*(1.-phi)*(1.-phi);
}
double hpp1_furtado_(const double &phi,const double &c)
{
  double dH = 2.35e9;
  double Tm = 1728.;
  return -30.*dH/Tm* 2.* (1. -phi) *phi* (1. - 2. *phi)   ;
}
double gp1_furtado_(const double &phi)
{
  return 2.*phi*(1.-phi)*(1.-2.*phi);
}
double gpp1_furtado_(const double &phi)
{
  return 2.*( 1. - 6.* phi + 6.* phi*phi);
}
double w_furtado_(const double &delta)
{
  return .61e8;
}
double m_furtado_(const double &theta,const double &M,const double &eps)
{
  //return 1./13.47;
  return 1.;
}
double hp2_furtado_(const double &phi)
{
  double dH = 2.35e9;
  double rho = 0.37;
  double Cp = 5.42e6;
  //return dH/rho/Cp*30.*phi*phi*(1.-phi)*(1.-phi);
  //return 1.*phi*phi*(1.-phi)*(1.-phi);
  return 1.;
}
double rand_phi_furtado_(const double &phi, const double &random_number)
{
  //double a = .025;
  double a = 75.;
  double r = 16.*a*phi*phi
    *(1.-phi)*(1.-phi);
//   double r = 256.*a*phi*phi*phi*phi
//     *(1.-phi)*(1.-phi)*(1.-phi)*(1.-phi);
  if( phi > 1. || phi < 0. ) r = 0;
  return ((double)rand()/(RAND_MAX)*2.-1.)*r;
//   return random_number*16.*r;
}

//karma
//karma has phi in [-1 +1]
double hp2_karma_(const double &phi)
{
  //return 15./8./2.*(1.-2*phi*phi+phi*phi*phi*phi);//VF
  return .5;//IVF
}
double w_karma_(const double &delta)
{
  return 1.;
}
double gp1_karma_(const double &phi)
{
  return phi*(1.-phi*phi);
}
double hp1_karma_(const double &phi,const double &c)
{
  double lambda = 1.;
  return -lambda* (1. -phi*phi) * (1. -phi*phi)  ;
}
double hpp1_karma_(const double &phi,const double &c)//precon term
{
  double dH = 2.35e9;
  double Tm = 1728.;
  //return -30.*dH/Tm* 2.* (1. -phi) *phi* (1. - 2. *phi)   ;
  return 0.   ;
}
double gpp1_karma_(const double &phi)//precon term
{
  //return 2.*( 1. - 6.* phi + 6.* phi*phi);
  return 0.;
}
double m_karma_(const double &theta,const double &M,const double &eps)
{
  double eps4 = eps;
  double a_sbar = 1. - 3.* eps4;
  double eps_prime = 4.*eps4/a_sbar;
  double t0 = 1.;
  double g = a_sbar*(1. + eps_prime * (pow(sin(theta),M)+pow(cos(theta),M)));
  return t0*g*g;
}
double gs2_karma_( const double &theta, const double &M, const double &eps, const double &psi)
{ 
  //double g = 1. + eps_ * (M_*cos(theta));
  double W_0 = 1.;
  double eps4 = eps;
  double a_sbar = 1. - 3.* eps4;
  double eps_prime = 4.*eps4/a_sbar;
  double g = W_0*a_sbar*(1. + eps_prime * (pow(sin(theta),M)+pow(cos(theta),M)));
  return g*g;
}
double dgs2_2dtheta_karma_(const double &theta, const double &M, const double &eps, const double &psi)
{
  double W_0 = 1.;
  double eps4 = eps;
  double a_sbar = 1. - 3.* eps4;
  double eps_prime = 4.*eps4/a_sbar;
  double g = W_0*a_sbar*eps*(-4.*pow(cos(theta),3)*sin(theta) + 4.*cos(theta)*pow(sin(theta),3))*
    (1. + eps*(pow(cos(theta),4) + pow(sin(theta),4)));
  return g;
}

/*
@inproceedings{inproceedings,
author = {Cummins, Sharen and J Quirk, James and Kothe, Douglas},
year = {2002},
month = {11},
pages = {},
title = {An Exploration of the Phase Field Technique for Microstructure Solidification Modeling}
}
*/
namespace cummins
{

  double delta_ = -9999.;

  double m_cummins_(const double &theta,const double &M,const double &eps)
  {
    
    //double g = 1. + eps * (cos(M*(theta)));
    double g = (4.*eps*(cos(theta)*cos(theta)*cos(theta)*cos(theta) 
			+ sin(theta)*sin(theta)*sin(theta)*sin(theta) ) -3.*eps +1.   );
    return g*g;
  }
  double hp2_cummins_(const double &phi)
  {
    return 1.;
  }

RES_FUNC(residual_heat_)
{
  //derivatives of the test function
  double dtestdx = basis[eqn_id].dphidxi[i]*basis[eqn_id].dxidx
    +basis[eqn_id].dphideta[i]*basis[eqn_id].detadx
    +basis[eqn_id].dphidzta[i]*basis[eqn_id].dztadx;
  double dtestdy = basis[eqn_id].dphidxi[i]*basis[eqn_id].dxidy
    +basis[eqn_id].dphideta[i]*basis[eqn_id].detady
    +basis[eqn_id].dphidzta[i]*basis[eqn_id].dztady;
  double dtestdz = basis[eqn_id].dphidxi[i]*basis[eqn_id].dxidz
    +basis[eqn_id].dphideta[i]*basis[eqn_id].detadz
    +basis[eqn_id].dphidzta[i]*basis[eqn_id].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;
  double phi = basis[1].uu;
  double phiold = basis[1].uuold;

  double D_ = 4.;

  double ut = (u-uold)/dt_*test;
  double divgradu = D_*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  // 	      double divgradu_old = D_*(basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
  //	      double divgradu = diffusivity_(*ubasis)*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  double divgradu_old = D_*(basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
  double hp2 = hp2_cummins_(phi);
  double phitu = -hp2*(phi-phiold)/dt_*test; 
  hp2 = hp2_cummins_(phiold);	
  double phitu_old = -hp2*(phiold-basis[1].uuoldold)/dt_*test; 
  return (ut + t_theta_*divgradu + (1.-t_theta_)*divgradu_old + t_theta_*phitu 
						       + (1.-t_theta_)*phitu_old);
}
double theta(const double &x,const double &y)
{
  double small = 1e-9;
  double pi = 3.141592653589793;
  double t = 0.;
  double sy = 1.;
  if(y < 0.) sy = -1.;
  double n = sy*sqrt(y*y);
  //double n = y;
  //std::cout<<y<<"   "<<n<<std::endl;
//   if(abs(x) < small && y > 0. ) t = pi/2.;
//   else if(abs(x) < small && y < 0. ) t = 3.*pi/2.;
//   else t= atan(n/x);
  if(std::abs(x) < small && y > 0. ) t = pi/2.;
  if(std::abs(x) < small && y < 0. ) t = 3.*pi/2.;
  if(x > small && y >= 0.) t= atan(n/x);
  if(x > small && y <0.) t= atan(n/x) + 2.*pi;
  if(x < -small) t= atan(n/x)+ pi;

  return t;
}
double gs_cummins_(const double &theta, const double &M, const double &eps, const double &psi)
{
  double eps_0_ = 1.;
  double g = eps_0_*(4.*eps*(cos(theta)*cos(theta)*cos(theta)*cos(theta) 
			     + sin(theta)*sin(theta)*sin(theta)*sin(theta) 
			     *(1.-2.*sin(psi)*sin(psi)*cos(psi)*cos(psi))
			     ) -3.*eps +1.   );
  return g;

}
double gs2_cummins_( const double &theta, const double &M, const double &eps, const double &psi)
{ 
  double eps_0_ = 1.;
  //double g = eps_0_*(1. + eps * (cos(M*theta)));
  //   double g = eps_0_*(4.*eps*(cos(theta)*cos(theta)*cos(theta)*cos(theta) 
  // 			     + sin(theta)*sin(theta)*sin(theta)*sin(theta) ) -3.*eps +1.   );
  double g =  gs_cummins_(theta,M,eps,psi);
  return g*g;
}
double dgs2_2dtheta_cummins_(const double &theta, const double &M, const double &eps, const double &psi)
{
  double eps_0_ = 1.;
  //return -1.*eps_0_*(eps*M*(1. + eps*cos(M*(theta)))*sin(M*(theta)));

  double g = gs_cummins_(theta,M,eps,psi);

  //double dg = 4.* eps* (-4.*cos(theta)*cos(theta)*cos(theta)*sin(theta) + 4.* cos(theta)*sin(theta)*sin(theta)*sin(theta));
  double dg = 4.* eps* (-4.*cos(theta)*cos(theta)*cos(theta)*sin(theta) + 
			4.* cos(theta)*sin(theta)*sin(theta)*sin(theta)*(1.-2.*cos(psi)*cos(psi)*sin(psi)*sin(psi)));

  return g*dg;

//   return eps_0_*4.*eps*(-4.*cos(theta)*cos(theta)*cos(theta)*sin(theta) + 4.*cos(theta)*sin(theta)*sin(theta)*sin(theta))
//     *(1. - 3.*eps + 4.*eps*(cos(theta)*cos(theta)*cos(theta)*cos(theta) 
// 			    + sin(theta)*sin(theta)*sin(theta)*sin(theta)));
}
double dgs2_2dpsi_cummins_(const double &theta, const double &M, const double &eps, const double &psi)
{
  //4 eps (-4 Cos[psi]^3 Sin[psi] + 4 Cos[psi] Sin[psi]^3) Sin[theta]^4
  double g = gs_cummins_(theta,M,eps,psi);
  double dg = 4.* eps* (-4.*cos(psi)*cos(psi)*cos(psi)*sin(psi) + 4.*cos(psi)*sin(psi)*sin(psi)*sin(psi))*sin(theta)*sin(theta)*sin(theta)*sin(theta);

  return g*dg;
}
double w_cummins_(const double &delta)
{
  return 1./delta/delta;
}
double gp1_cummins_(const double &phi)
{
  return phi*(1.-phi)*(1.-2.*phi);
}
double hp1_cummins_(const double &phi,const double &c)
{
  return -c*phi*phi*(1.-phi)*(1.-phi);
}
double hpp1_cummins_(const double &phi,const double &c)
{
  return -c* 2.* (1. -phi) *phi* (1. - 2. *phi);
}
double gpp1_cummins_(const double &phi)
{
  return 1. - 6.* phi + 6.* phi*phi;
}

RES_FUNC(residual_phase_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;
  double phi = basis[1].uu;
  double phiold = basis[1].uuold;

  double eps_ = .05;
  double M_= 4.;

  double dphidx = basis[1].dudx;
  double dphidy = basis[1].dudy;
  double dphidz = basis[1].dudz;

  double theta_0_ =0.;
  double theta_ = theta(dphidx,dphidy)-theta_0_;

  double psi_ = 0.;
  double m = m_cummins_(theta_, M_, eps_);

  double phit = m*(phi-phiold)/dt_*test;

  double gs2 = gs2_cummins_(theta_, M_, eps_,0.);

  double divgradphi = gs2*(dphidx*dtestdx + dphidy*dtestdy + dphidz*dtestdz);//(grad u,grad phi)

  double dgdtheta = dgs2_2dtheta_cummins_(theta_, M_, eps_, 0.);	
  double dgdpsi = 0.;
  double curlgrad = dgdtheta*(-dphidy*dtestdx + dphidx*dtestdy);//cn could be a wrong sign here!!!
  //+dgdpsi*(-dphidz*dtestdx + dphidx*dtestdz);
  double w = w_cummins_(delta_);//cn_delta
  double gp1 = gp1_cummins_(phi);
  double phidel2 = gp1*w*test;

  double T_m_ = 1.55;
  double T_inf_ = 1.;
  double alpha_ = 191.82;

  double hp1 = hp1_cummins_(phi,5.*alpha_/delta_);
  double phidel = hp1*(T_m_ - u)*test;
  double rhs = divgradphi + curlgrad + phidel2 + phidel;

  dphidx = basis[1].duolddx;
  dphidy = basis[1].duolddy;
  dphidz = basis[1].duolddz;
  theta_ = theta(dphidx,dphidy)-theta_0_;
  //psi_ = psi(dphidx,dphidy,dphidz);
  psi_ =0.;
  gs2 = gs2_cummins_(theta_, M_, eps_,0.);
  divgradphi = gs2*dphidx*dtestdx + gs2*dphidy*dtestdy + gs2*dphidz*dtestdz;//(grad u,grad phi)
  dgdtheta = dgs2_2dtheta_cummins_(theta_, M_, eps_, 0.);

  curlgrad = dgdtheta*(-dphidy*dtestdx + dphidx*dtestdy);
  //+dgdpsi*(-dphidz*dtestdx + dphidx*dtestdz);

  gp1 = gp1_cummins_(phiold);
  
  phidel2 = gp1*w*basis[1].phi[i];
  
  hp1 = hp1_cummins_(phiold,5.*alpha_/delta_);

  phidel = hp1*(T_m_ - uold)*test;
	      
  double rhs_old = divgradphi + curlgrad + phidel2 + phidel;

  return phit + t_theta_*rhs + (1.-t_theta_)*rhs_old;

}

PRE_FUNC(prec_heat_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double D_ = 4.;
  double divgrad = D_*dbasisdx * dtestdx + D_*dbasisdy * dtestdy + D_*dbasisdz * dtestdz;
  double u_t =test * basis[0].phi[j]/dt_;
  return u_t + t_theta_*divgrad;
}

PRE_FUNC(prec_phase_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double test = basis[0].phi[i];
  
  double dphidx = basis[1].dudx;
  double dphidy = basis[1].dudy;
  double dphidz = basis[1].dudz;

  double eps_ = .05;
  double M_= 4.;

  double theta_0_ =0.;
  double theta_ = theta(dphidx,dphidy)-theta_0_;

  double m = m_cummins_(theta_, M_, eps_);

  double phit = m*(basis[1].phi[j])/dt_*test;

  double gs2 = gs2_cummins_(theta_, M_, eps_,0.);

  double divgrad = gs2*dbasisdx * dtestdx + gs2*dbasisdy * dtestdy + gs2*dbasisdz * dtestdz;

  double dg2 = dgs2_2dtheta_cummins_(theta_, M_, eps_,0.);
  double curlgrad = -dg2*(dtestdy*dphidx -dtestdx*dphidy);

  return phit + t_theta_*divgrad + t_theta_*curlgrad;
}
double R_cummins(const double &theta)
{
  double R_0_ = .3;
  double eps_ = .05;
  double M_ = 4.;
  return R_0_*(1. + eps_ * cos(M_*(theta)));
}
//double init_heat_(const double &x,
//		 const double &y,
//		 const double &z)
INI_FUNC(init_heat_)
{
  double theta_0_ = 0.;
  double t = theta(x,y) - theta_0_;
  double r = R_cummins(t);

  double T_m_ = 1.55;
  double T_inf_ = 1.;

  double val = 0.;  

  if(x*x+y*y+z*z < r*r){
    val=T_m_;
  }
  else {
    val=T_inf_;
  }
  return val;
}

INI_FUNC(init_heat_const_)
{

  double T_inf_ = 1.;

  return T_inf_;
}

INI_FUNC(init_phase_)
{
  double theta_0_ = 0.;
  double t = theta(x,y) - theta_0_;
  double r = R_cummins(t);

  double phi_sol_ = 1.;
  double phi_liq_ = 0.;

  double val = 0.;  

  if(x*x+y*y+z*z < r*r){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }
  return val;
}
PARAM_FUNC(param_)
{
  delta_ = plist->get<double>("delta");
}
}//cummins

INI_FUNC(init_zero_)
{

  return 0.;
}

NBC_FUNC(nbc_zero_)
{
  
  double phi = basis->phi[i];
  
  return 0.*phi;
}

KOKKOS_INLINE_FUNCTION
DBC_FUNC(dbc_zero_)
{  
  return 0.;
}

NBC_FUNC(nbc_one_)
{
  
  double phi = basis->phi[i];
  
  return 1.*phi;
}

DBC_FUNC(dbc_one_)
{ 
  return 1.;
}

DBC_FUNC(dbc_ten_)
{
  
  
  return 10.*dbc_one_(x,
	       y,
	       z,
	       t);
}

NBC_FUNC(nbc_mone_)
{

  double phi = basis->phi[i];

  return -1.*phi;
}

DBC_FUNC(dbc_mone_)
{
  return -1.;
}

INI_FUNC(init_neumann_test_)
{

  double pi = 3.141592653589793;

  return sin(pi*x);
}








namespace farzadi
{

  //it appears the mm mesh is in um
  // 1m = 1e3 mm
  // 1m = 1e6 um

  double pi = 3.141592653589793;
  double theta_0_ = 0.;

  double phi_sol_ = 1.;
  double phi_liq_ = -1.;
  double k_ =0.14;
  double eps_ = .01;
  double M_= 4.;
  double lambda = 10.;
  double c_inf = 3.;
  double D= 6.267;
  //double D_ = 3.e-9;//m^2/s
  //double D_ = .003;//mm^2/s
  double D_ = 3.e3;//um^2/s
  //double D_ = D;
  //double m = -2.6;
  double tl = 925.2;//k
  double ts = 877.3;//k
  double G = .290900;//k/um
  double R = 3000.;//um/s
  double V = R;//um/s
  double t0 = ts;
  //double t0 = 900.;//k
  double dt0 = tl-ts;
  //double d0 = 5.e-9;//m
  double d0 = 5.e-3;//um

  //for the farzadiQuad1000x360mmr.e mesh...
  double pp = 360.;
  double ll = 40.;
  double aa = 14.;

  //double init_conc_farzadi_(const double &x,
  //		 const double &y,
  //		 const double &z)
PARAM_FUNC(param_)
{
  pp = plist->get<double>("pp");
  ll = plist->get<double>("ll");
  aa = plist->get<double>("aa");
}
INI_FUNC(init_conc_farzadi_)
{

  double val = -1.;

  return val;
}
double ff(const double y)
{
  return 2.+sin(y*aa*pi/pp);
}
  //double init_phase_farzadi_(const double &x,
  //		 const double &y,
  //		 const double &z)
INI_FUNC(init_phase_farzadi_)
{
  double val = phi_liq_;
  double r = ll*(1.+ff(y)*ff(y/2.)*ff(y/4.));

  //r=.9;

  if(x < r){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }
  return val;
  //return 1.;
}
INI_FUNC(init_phase_rand_farzadi_)
{
  double val = phi_liq_;
  int ri = rand()%(100);//random int between 0 and 100
  double rd = (double)ri/ll;
  double r = .5+rd*std::abs(sin(y*aa*pi/pp));

  //r=.9;

  if(x < r){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }
  return val;
  //return 1.;
}
double tscale_(const double &x, const double &time)
{
  //x and time come in as nondimensional quantities here...
  double xx = d0*x;
  double tt = d0*d0/D_*time;
  double t = t0 + G*(xx-V*tt);
  return (t-ts)/dt0;
}

RES_FUNC(residual_phase_farzadi_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;
  double phi = basis[1].uu;
  double phiold = basis[1].uuold;

  double dphidx = basis[1].dudx;
  double dphidy = basis[1].dudy;

  //double theta_ = theta(basis[1].duolddx,basis[1].duolddy);
  double theta_ = cummins::theta(basis[1].dudx,basis[1].dudy);

  double m = (1+(1-k_)*u)*cummins::m_cummins_(theta_, M_, eps_);//cn we probably need u and uold here for CN...
  //double m = m_cummins_(theta_, M_, eps_);//cn we probably need u and uold here for CN...
  //double theta_old = theta(dphidx,dphidy);
  //double mold = (1+(1-k_)*uold)*m_cummins_(theta_old, M_, eps_);

  //double phit = (t_theta_*m+(1.-t_theta_)*mold)*(phi-phiold)/dt_*test;
  //double phit = m*(phi-phiold)/dt_*test;

  double gs2 = cummins::gs2_cummins_(theta_, M_, eps_,0.);
  double divgradphi = gs2*(dphidx*dtestdx + dphidy*dtestdy);//(grad u,grad phi)

  double phit = (1.+(1.-k_)*u)*gs2*(phi-phiold)/dt_*test;

  double dgdtheta = cummins::dgs2_2dtheta_cummins_(theta_, M_, eps_, 0.);	
  double dgdpsi = 0.;
  double curlgrad = dgdtheta*(-dphidy*dtestdx + dphidx*dtestdy);

  double gp1 = -(phi - phi*phi*phi);
  double phidel2 = gp1*test;

  double x = basis[0].xx;
  //double t = t0 + G*(x-R*time);
  //double t_scale = (t-ts)/dt0;
  double t_scale = tscale_(x,time);

  double hp1 = lambda*(1. - phi*phi)*(1. - phi*phi)*(u+t_scale);
  double phidel = hp1*test;
  //phidel = 0.;
  double rhs = divgradphi + curlgrad + phidel2 + phidel;

//   dphidx = basis[1].duolddx;
//   dphidy = basis[1].duolddy;
//   theta_ = theta(dphidx,dphidy);

//   gs2 = gs2_cummins_(theta_, M_, eps_,0.);
//   divgradphi = gs2*dphidx*dtestdx + gs2*dphidy*dtestdy;//(grad u,grad phi)
//   dgdtheta = dgs2_2dtheta_cummins_(theta_, M_, eps_, 0.);

//   curlgrad = dgdtheta*(-dphidy*dtestdx + dphidx*dtestdy);

//   gp1 = -(phiold-phiold*phiold*phiold);
  
//   phidel2 = gp1*test;
  
//   hp1 = lambda*(1.-phiold*phiold)*(1.-phiold*phiold)*(uold+t_scale);

//   phidel = hp1*test;
	      
//   double rhs_old = divgradphi + curlgrad + phidel2 + phidel;

  return phit + t_theta_*rhs;// + (1.-t_theta_)*rhs_old*0.;

}

RES_FUNC(residual_conc_farzadi_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;
  double phi = basis[1].uu;
  double phiold = basis[1].uuold;
  double dphidx = basis[1].dudx;
  double dphidy = basis[1].dudy;

  double ut = (1.+k_)/2.*(u-uold)/dt_*test;
  //ut = (u-uold)/dt_*test;
  double divgradu = D*(1.-phi)/2.*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy);//(grad u,grad phi)
  //divgradu = (basis[0].dudx*dtestdx + basis[0].dudy*dtestdy);
  double divgradu_old = D*(1.-phiold)/2*(basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy);//(grad u,grad phi)

  //j is antitrapping current
  // j grad test here... j1*dtestdx + j2*dtestdy 
  // what if dphidx*dphidx + dphidy*dphidy = 0?

  double norm = sqrt(dphidx*dphidx + dphidy*dphidy);
  double small = 1.e-12;
  
  double j_coef = 0.;
  if (small < norm) {
    j_coef = (1.+(1.-k_)*u)/sqrt(8.)/norm*(phi-phiold)/dt_;
  } 
  //j_coef = 0.;
  double j1 = j_coef*dphidx;
  double j2 = j_coef*dphidy;
  double divj = j1*dtestdx + j2*dtestdy;

  double dphiolddx = basis[1].duolddx;
  double dphiolddy = basis[1].duolddy; 
  norm = sqrt(dphidx*dphidx + dphidy*dphidy);
  j_coef = 0.;
  if (small < norm) {
    j_coef = (1.+(1.-k_)*uold)/sqrt(8.)/norm*(phiold-basis[1].uuoldold)/dt_; 
  }
  j1 = j_coef*dphidx;
  j2 = j_coef*dphidy;
  j_coef = 0.;
  double divj_old = j1 *dtestdx + j2 *dtestdy;

  double h = phi*(1.+(1.-k_)*u);
  double hold = phiold*(1. + (1.-k_)*uold);

  //double phitu = -.5*(h-hold)/dt_*test; 
  double phitu = -.5*(phi-phiold)/dt_*(1.+(1.-k_)*u)*test; 
  //phitu = 1.*test; 
//   h = hold;
//   hold = basis[1].uuoldold*(1. + (1.-k_)*basis[0].uuoldold);
//   double phitu_old = -.5*(h-hold)/dt_*test;
 
  //return ut*0.  + t_theta_*(divgradu + divj*0.) + (1.-t_theta_)*(divgradu_old + divj_old)*0. + t_theta_*phitu*0. + (1.-t_theta_)*phitu_old*0.;

  return ut + t_theta_*divgradu  + t_theta_*divj + t_theta_*phitu;
}

PRE_FUNC(prec_phase_farzadi_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double test = basis[1].phi[i];
  
  double dphidx = basis[1].dudx;
  double dphidy = basis[1].dudy;
  double dphidz = basis[1].dudz;

  double u = basis[0].uu;
  double phi = basis[1].uu;


  double theta_ = cummins::theta(dphidx,dphidy)-theta_0_;

  double gs2 = cummins::gs2_cummins_(theta_, M_, eps_,0.);

  double m = (1.+(1.-k_)*0.*u)*gs2;
  double phit = m*(basis[1].phi[j])/dt_*test;
  //phit = (basis[1].phi[j])*test;

  double divgrad = gs2*dbasisdx * dtestdx + gs2*dbasisdy * dtestdy;// + gs2*dbasisdz * dtestdz;

  //double dg2 = cummins::dgs2_2dtheta_cummins_(theta_, M_, eps_,0.);
  //double curlgrad = dg2*(-dtestdx*dphidy + dtestdy*dphidx );
  //double t1 = (-1.+3.*phi*phi)*basis[1].phi[j]*test;
  //double t2 = -4.*lambda*(u + 0.)*(-phi+phi*phi*phi)*basis[1].phi[j]*test;

  //return phit + t_theta_*(divgrad + 0.*curlgrad + 0.*t1 + 0.*t2);
  return phit + t_theta_*(divgrad);
}

PRE_FUNC(prec_conc_farzadi_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = D*(1.-basis[1].uu)/2.*(dbasisdx * dtestdx + dbasisdy * dtestdy);


  //double dphidx = basis[1].dudx;
  //double dphidy = basis[1].dudy;
  //double norm = sqrt(dphidx*dphidx + dphidy*dphidy);
  //double small = 1.e-12;
  //double phi = basis[1].uu;
  //double phiold = basis[1].uuold;
  //double j_coef = 0.;
  //if (small < norm) {
  // j_coef = (1.+(1.-k_)* basis[0].phi[j])/sqrt(8.)/norm*(phi-phiold)/dt_;
  //} 
  ////j_coef = 0.;
  //double j1 = j_coef*dphidx;
  //double j2 = j_coef*dphidy;
  //double divj = j1*dtestdx + j2*dtestdy;



  double u_t =(1.+k_)/2.*test * basis[0].phi[j]/dt_;
  //double phitu = -.5*(1.-k_)*basis[0].phi[j]*test*(phi-phiold)/dt_; 
  //return u_t + t_theta_*(divgrad + 0.*divj + 0.*phitu);
  return u_t + t_theta_*(divgrad);
}

PPR_FUNC(postproc_c_)
{

  double uu = u[0];
  double phi = u[1];

  return -c_inf*(1.+k_-phi+k_*phi)*(-1.-uu+k_*uu)/2./k_;
}

PPR_FUNC(postproc_t_)
{
  // x is in nondimensional space, tscale_ takes in nondimensional and converts to um
  double x = xyz[0];

  return tscale_(x,time);
}
}//namespace farzadi

namespace robin_steadystate
{
RES_FUNC(residual_robin_test_)
{
  //1-D robin bc test problem, steady state
  // -d^2 u/dx^2 + a^2 u = 0
  // u(x=0) = 0; grad u = b(1-u) | x=1
  // u[x;b,a]=b cosh ( ax)/[b cosh(a)+a sinh(a)]



  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  //double uold = basis[0].uuold;

  double a =10.;

  double au = a*a*u*test;
  double divgradu = (basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  //double divgradu_old = (basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
 
 
  return divgradu + au;
}

PRE_FUNC(prec_robin_test_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz;
  double a =10.;
  double u_t =test * a*a*basis[0].phi[j];
  return u_t + t_theta_*divgrad;
}

NBC_FUNC(nbc_robin_test_)
{

  double test = basis->phi[i];
  double u = basis[0].uu;

  double b =1.;

  return b*(1.-u)*test;
}
}//namespace robin_steadystate
namespace robin
{
  //  http://ramanujan.math.trinity.edu/rdaileda/teach/s12/m3357/lectures/lecture_2_28_short.pdf
  // 1-D robin bc test problem, time dependent
  // Solve D[u, t] - c^2 D[u, x, x] == 0
  // u(0,t) == 0
  // D[u, x] /. x -> L == -kappa u(t,L)
  // => du/dx + kappa u = g = 0
  // u(x,t) = a E^(-mu^2 t) Sin[mu x]
  // mu solution to: Tan[mu L] + mu/kappa == 0 && Pi/2 < mu < 3 Pi/2
  const double mu = 2.028757838110434;
  const double a = 10.;
  const double c = 1.;
  const double L = 1.;
  const double kappa = 1.;

RES_FUNC(residual_robin_test_)
{
  //1-D robin bc test problem, 

  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;

  //double a =10.;

  double ut = (u-uold)/dt_*test;
  double divgradu = c*c*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  //double divgradu_old = (basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
 
 
  return ut + divgradu;
}

PRE_FUNC(prec_robin_test_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = c*c*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  //double a =10.;
  double u_t = (basis[0].phi[j])/dt_*test;
  return u_t + t_theta_*divgrad;
}

NBC_FUNC(nbc_robin_test_)
{

  double test = basis->phi[i];
  double u = basis[0].uu;

  //du/dn + kappa u = g = 0 on L
  //(du,dv) - <du/dn,v> = (f,v)
  //(du,dv) - <g - kappa u,v> = (f,v)
  //(du,dv) - < - kappa u,v> = (f,v)
  //          ^^^^^^^^^^^^^^ return this

  return (-kappa*u)*test;
}
INI_FUNC(init_robin_test_)
{
  return a*sin(mu*x);
}
}//namespace robin
namespace liniso
{

RES_FUNC(residual_liniso_x_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double E = 1e6;
  double nu = .3;

  double strain[6], stress[6];//x,y,z,yx,zy,zx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  //double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  strain[2] = basis[2].dudz;
  strain[3] = basis[0].dudy + basis[1].dudx;
  strain[4] = basis[1].dudz + basis[2].dudy;
  strain[5] = basis[0].dudz + basis[2].dudx;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;

  stress[0] = c1*strain[0] + c2*strain[1] + c2*strain[2];
  //stress[1] = c2*strain[0] + c1*strain[1] + c2*strain[2];
  //stress[2] = c2*strain[0] + c2*strain[1] + c1*strain[2];
  stress[3] = c3*strain[3]; 
  //stress[4] = c3*strain[4]; 
  stress[5] = c3*strain[5]; 

  double divgradu = stress[0]*dtestdx + stress[3]*dtestdy + stress[5]*dtestdz;//(grad u,grad phi)
 
 
  return divgradu;
}

RES_FUNC(residual_liniso_y_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double E = 1e6;
  double nu = .3;

  double strain[6], stress[6];//x,y,z,yx,zy,zx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  //double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  strain[2] = basis[2].dudz;
  strain[3] = basis[0].dudy + basis[1].dudx;
  strain[4] = basis[1].dudz + basis[2].dudy;
  strain[5] = basis[0].dudz + basis[2].dudx;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;

  //stress[0] = c1*strain[0] + c2*strain[1] + c2*strain[2];
  stress[1] = c2*strain[0] + c1*strain[1] + c2*strain[2];
  //stress[2] = c2*strain[0] + c2*strain[1] + c1*strain[2];
  stress[3] = c3*strain[3]; 
  stress[4] = c3*strain[4]; 
  //stress[5] = c3*strain[5]; 

  double divgradu = stress[1]*dtestdy + stress[3]*dtestdx + stress[4]*dtestdz;//(grad u,grad phi)
 
 
  return divgradu;
}

RES_FUNC(residual_liniso_z_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double E = 1e6;
  double nu = .3;

  double strain[6], stress[6];//x,y,z,yx,zy,zx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  //double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  strain[2] = basis[2].dudz;
  strain[3] = basis[0].dudy + basis[1].dudx;
  strain[4] = basis[1].dudz + basis[2].dudy;
  strain[5] = basis[0].dudz + basis[2].dudx;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;

  //stress[0] = c1*strain[0] + c2*strain[1] + c2*strain[2];
  //stress[1] = c2*strain[0] + c1*strain[1] + c2*strain[2];
  stress[2] = c2*strain[0] + c2*strain[1] + c1*strain[2];
  //stress[3] = c3*strain[3]; 
  stress[4] = c3*strain[4]; 
  stress[5] = c3*strain[5]; 

  double divgradu = stress[2]*dtestdz + stress[4]*dtestdy + stress[5]*dtestdx;//(grad u,grad phi)
 
 
  return divgradu;
}

PRE_FUNC(prec_liniso_x_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double E = 1e6;
  double nu = .3;

  double strain[6], stress[6];//x,y,z,yx,zy,zx

  //strain[0] = basis[0].dudx;
  //strain[1] = basis[1].dudy;
  //strain[2] = basis[2].dudz;
  //strain[3] = basis[0].dudy + basis[1].dudx;
  //strain[4] = basis[1].dudz + basis[2].dudy;
  //strain[5] = basis[0].dudz + basis[2].dudx;

  //strain[0] = dbasisdx;
  //strain[1] = dbasisdy;
  //strain[2] = dbasisdz;
  //strain[3] = dbasisdy + dbasisdx;
  //strain[4] = dbasisdz + dbasisdy;
  //strain[5] = dbasisdz + dbasisdx;

  strain[0] = dbasisdx;
  strain[1] = 0;
  strain[2] = 0;
  strain[3] = 0 + dbasisdy;
  strain[4] = 0;
  strain[5] = dbasisdz +  0;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;

  stress[0] = c1*strain[0] + c2*strain[1] + c2*strain[2];
  //stress[1] = c2*strain[0] + c1*strain[1] + c2*strain[2];
  //stress[2] = c2*strain[0] + c2*strain[1] + c1*strain[2];
  stress[3] = c3*strain[3]; 
  //stress[4] = c3*strain[4]; 
  stress[5] = c3*strain[5]; 

  double divgradu = stress[0]*dtestdx + stress[3]*dtestdy + stress[5]*dtestdz;//(grad u,grad phi)
 
 
  return divgradu;
}

PRE_FUNC(prec_liniso_y_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double E = 1e6;
  double nu = .3;

  double strain[6], stress[6];//x,y,z,yx,zy,zx

  //strain[0] = basis[0].dudx;
  //strain[1] = basis[1].dudy;
  //strain[2] = basis[2].dudz;
  //strain[3] = basis[0].dudy + basis[1].dudx;
  //strain[4] = basis[1].dudz + basis[2].dudy;
  //strain[5] = basis[0].dudz + basis[2].dudx;

  //strain[0] = dbasisdx;
  //strain[1] = dbasisdy;
  //strain[2] = dbasisdz;
  //strain[3] = dbasisdy + dbasisdx;
  //strain[4] = dbasisdz + dbasisdy;
  //strain[5] = dbasisdz + dbasisdx;
  strain[0] = 0;
  strain[1] = dbasisdy;
  strain[2] = 0;
  strain[3] = 0 + dbasisdx;
  strain[4] = dbasisdz + 0;
  strain[5] = 0 + 0;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;

  //stress[0] = c1*strain[0] + c2*strain[1] + c2*strain[2];
  stress[1] = c2*strain[0] + c1*strain[1] + c2*strain[2];
  //stress[2] = c2*strain[0] + c2*strain[1] + c1*strain[2];
  stress[3] = c3*strain[3]; 
  stress[4] = c3*strain[4]; 
  //stress[5] = c3*strain[5]; 

  double divgradu = stress[1]*dtestdy + stress[3]*dtestdx + stress[4]*dtestdz;//(grad u,grad phi)

  return divgradu;
}

PRE_FUNC(prec_liniso_z_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double E = 1e6;
  double nu = .3;

  double strain[6], stress[6];//x,y,z,yx,zy,zx

  //strain[0] = basis[0].dudx;
  //strain[1] = basis[1].dudy;
  //strain[2] = basis[2].dudz;
  //strain[3] = basis[0].dudy + basis[1].dudx;
  //strain[4] = basis[1].dudz + basis[2].dudy;
  //strain[5] = basis[0].dudz + basis[2].dudx;

  //strain[0] = dbasisdx;
  //strain[1] = dbasisdy;
  //strain[2] = dbasisdz;
  //strain[3] = dbasisdy + dbasisdx;
  //strain[4] = dbasisdz + dbasisdy;
  //strain[5] = dbasisdz + dbasisdx;
  strain[0] = 0;
  strain[1] = 0;
  strain[2] = dbasisdz;
  strain[3] = 0 + 0;
  strain[4] = 0 + dbasisdy;
  strain[5] = 0 + dbasisdx;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;

  //stress[0] = c1*strain[0] + c2*strain[1] + c2*strain[2];
  //stress[1] = c2*strain[0] + c1*strain[1] + c2*strain[2];
  stress[2] = c2*strain[0] + c2*strain[1] + c1*strain[2];
  //stress[3] = c3*strain[3]; 
  stress[4] = c3*strain[4]; 
  stress[5] = c3*strain[5]; 

  double divgradu = stress[2]*dtestdz + stress[4]*dtestdy + stress[5]*dtestdx;//(grad u,grad phi)


  return divgradu;
}

RES_FUNC(residual_linisobodyforce_y_test_)
{
  //this is taken from a test that had E=2e11; nu=.3 and body force -1e10 in the y direction;
  //we reuse the test case that has E=1e6 and scale the body force accordingly by 5e-6  

  //test function
  double test = basis[0].phi[i]; 

  double bf = -1.e10*5.e-6;

  double divgradu = residual_liniso_y_test_(basis,i,dt_,dt_,t_theta_,t_theta_,time,eqn_id) + bf*test;
 
  return divgradu;
}

RES_FUNC(residual_linisoheat_x_test_)
{
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double gradu = basis[3].dudx;
  double c = 1.e-6;
  double alpha = 1.e-4;;
  double E = 1.;

  double divgradu = c*residual_liniso_x_test_(basis,i,dt_,dt_,t_theta_,t_theta_,time,eqn_id) - alpha*E*gradu*dtestdx;
 
  return divgradu;
}

RES_FUNC(residual_linisoheat_y_test_)
{
  //test function
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double gradu = basis[3].dudy;

  double c = 1.e-6;
  double alpha = 1.e-4;
  double E = 1.;


  double divgradu = c*residual_liniso_y_test_(basis,i,dt_,dt_,t_theta_,t_theta_,time,eqn_id) - alpha*E*gradu*dtestdy;
 
  return divgradu;
}

RES_FUNC(residual_linisoheat_z_test_)
{
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  double gradu = basis[3].dudz;
  double c = 1.e-6;
  double alpha = 1.e-4;
  double E = 1.;

  double divgradu = c*residual_liniso_z_test_(basis,i,dt_,dt_,t_theta_,t_theta_,time,eqn_id) - alpha*E*gradu*dtestdz;
 
  return divgradu;
}

RES_FUNC(residual_divgrad_test_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  //double test = basis[3].phi[i];
  //u, phi
  //double u = basis[0].uu;
  //double uold = basis[0].uuold;

  double divgradu = (basis[3].dudx*dtestdx + basis[3].dudy*dtestdy + basis[3].dudz*dtestdz);//(grad u,grad phi)
  //double divgradu_old = (basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
 
 
  return divgradu;
}
}//namespace liniso

namespace uehara
{

  double L = 3.e3;//J/m^3
  double m = 2.5e5;
  double a = 10.;//m^4
  double rho = 1.e3;//kg/m^3
  double c = 5.e2;//J/kg/K
  double k = 150.;//W/m/K
  //double k = 1.5;//W/m/K
  double r0 = 29.55;

  double giga = 1.e+9;
  double E = 200.*giga;//GPa
  //double E = 200.*1.e9;//Pa
  double nu = .3;

  //plane stress
  double c0 = E/(1.-nu*nu);
  double c1 = c0;
  double c2 = c0*nu;
  double c3 = c0*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
//   double c0 = E/(1.+nu)/(1.-2.*nu);
//   double c1 = c0*(1.-nu);
//   double c2 = c0*nu;
//   double c3 = c0*(1.-2.*nu)/2.;

  double alpha = 5.e-6;//1/K
  //double alpha = 0.;//1/K
  double beta = 1.5e-3;
  //double beta = 0.;
  
  //double init_heat_(const double &x,
  //	   const double &y,
  //	   const double &z)
INI_FUNC(init_heat_)
{
  double val = 400.;  

  return val;
}

INI_FUNC(init_phase_)
{

  //this is not exactly symmetric on the fine mesh
  double phi_sol_ = 1.;
  double phi_liq_ = 0.;

  double val = phi_sol_ ;  

  double dx = 1.e-2;
  double r = r0*dx;
  double r1 = (r0-1)*dx;

  //if(y > r){
  if( (x > r1 && y > r) ||(x > r && y > r1) ){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }

  return val;
}

INI_FUNC(init_phase_c_)
{

  //this is not exactly symmetric on the fine mesh
  double phi_sol_ = 1.;
  double phi_liq_ = 0.;

  double val = phi_sol_ ;  

  double dx = 1.e-2;
  double r = r0*dx;

  double rr = sqrt(x*x+y*y);

  //if(y > r){
  if( rr > r0*sqrt(2.)*dx ){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }

  return val;
}

INI_FUNC(init_heat_seed_)
{

  //this is not exactly symmetric on the fine mesh
  double phi_sol_ = 300.;
  double phi_liq_ = 400.;

  double val = phi_sol_ ;  

  double dx = 1.e-2;
  double r = r0*dx;
  double r1 = (r0-1)*dx;

  //if(y > r){
  if( (x > r1 && y > r) ||(x > r && y > r1) ){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }

  return val;
}

INI_FUNC(init_heat_seed_c_)
{

  //this is not exactly symmetric on the fine mesh
  double phi_sol_ = 300.;
  double phi_liq_ = 400.;

  double val = phi_sol_ ;  

  double dx = 1.e-2;
  double r = r0*dx;
  double r1 = (r0-1)*dx;

  double rr = sqrt(x*x+y*y);

  //if(y > r){
  if( rr > r0*sqrt(2.)*dx ){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }

  return val;
}

DBC_FUNC(dbc_)
{  
  return 300.;
}
double conv_bc_(const Basis *basis,
		 const int &i, 
		 const double &dt_, 
		 const double &t_theta_,
		 const double &time)
{

  double test = basis->phi[i];
  double u = basis->uu;
  double uw = 300.;//K
  double h =1.e4;//W/m^2/K
  
  return h*(uw-u)*test/rho/c;
}

NBC_FUNC(nbc_stress_)
{

  double test = basis->phi[i];
  
  return -alpha*300.*test;
}

RES_FUNC(residual_phase_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double phi = basis[0].uu;
  double phiold = basis[0].uuold;
  double u = basis[1].uu;
  //double uold = basis[1].uuold;

  double dphidx = basis[0].dudx;
  double dphidy = basis[0].dudy;
  double dphidz = basis[0].dudz;

  double b = 5.e-5;//m^3/J
  //b = 5.e-7;//m^3/J
  double f = 0.;
  double um = 400.;//K

  double phit = m*(phi-phiold)/dt_*test;
  double divgradphi = a*(dphidx*dtestdx + dphidy*dtestdy + dphidz*dtestdz);//(grad u,grad test)
  //double M = b*phi*(1. - phi)*(L*(um - u)/um + f);
  double M = .66*150000000.*b*phi*(1. - phi)*(L*(um - u)/um + f);
  //double g = -phi*(1. - phi)*(phi - .5 + M)*test;
  double g = -(phi*(1. - phi)*(phi - .5)+phi*(1. - phi)*M)*test;
  double rhs = divgradphi + g;

  return (phit + rhs)/m;

}

RES_FUNC(residual_stress_x_dt_)
{
  double strain[3];//x,y,yx

  strain[0] = (basis[2].dudx-basis[2].duolddx)/dt_;
  strain[1] = (basis[3].dudy-basis[3].duolddy)/dt_;

  double stress = c1*strain[0] + c2*strain[1];

  return stress;
}

RES_FUNC(residual_stress_y_dt_)
{
  double strain[3];//x,y,z,yx,zy,zx

  strain[0] = (basis[2].dudx-basis[2].duolddx)/dt_;
  strain[1] = (basis[3].dudy-basis[3].duolddy)/dt_;

  double stress = c2*strain[0] + c1*strain[1];

  return stress;
}

RES_FUNC(residual_heat_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[1].phi[i];
  //u, phi
  double phi = basis[0].uu;
  double phiold = basis[0].uuold;
  double phioldold = basis[0].uuoldold;
  double u = basis[1].uu;
  double uold = basis[1].uuold;

  double dudx = basis[1].dudx;
  double dudy = basis[1].dudy;
  //double dudz = basis[1].dudz;

  double ut = rho*c*(u-uold)/dt_*test;
  double divgradu = k*(dudx*dtestdx + dudy*dtestdy);
  //double phitu = -30.*L*phi*phi*(1.-phi)*(1.-phi)*(phi-phioldold)/2./dt_*test; 
  double h = phi*phi*(1.-phi)*(1.-phi);
  double phitu = -30.*L*h*(phi-phiold)/dt_*test; 
  
  //thermal term
  double stress = test*alpha*u*(residual_stress_x_dt_(basis, 
						      i, dt_, dt_, t_theta_,t_theta_,
						      time, eqn_id)
				+residual_stress_y_dt_(basis, 
						       i, dt_, dt_, t_theta_,t_theta_,
						       time, eqn_id));
  

  double rhs = divgradu + phitu + stress;

  return (ut + rhs)/rho/c;

}

RES_FUNC(residual_liniso_x_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,yx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  //double ut = (basis[1].uu - basis[1].uuoldold)/dt_/2;//thermal strain
  double ut = (basis[1].uu - basis[1].uuold)/dt_;//thermal strain

  double phi = basis[0].uu;
  double h = phi*phi*(1.-phi)*(1.-phi);
  //double hp = 2.*(1.-phi)*(1.-phi)*phi-2.*(1.-phi)*phi*phi;//2 (1 - x)^2 x - 2 (1 - x) x^2
  // h' p_t p_x +h p_t_x
  double strain_phi = 30.*beta*h*(phi-basis[0].uuold)/dt_;
//   double strain_phi = 0.*2.*30.*beta*(c1+c2)*(hp*(phi-basis[0].uuold)/dt_*basis[0].dudx
// 					   +h*(basis[0].dudx-basis[0].duolddx)/dt_
// 					   )*test;
  
  double ff =   alpha*ut + strain_phi;

  strain[0] = (basis[2].dudx-basis[2].duolddx)/dt_- ff;
  strain[1] = (basis[3].dudy-basis[3].duolddy)/dt_- ff;
  strain[2] = (basis[2].dudy-basis[2].duolddy + basis[3].dudx-basis[3].duolddx)/dt_;// - alpha*ut - strain_phi;

  stress[0] = c1*strain[0] + c2*strain[1];
  // stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  double divgradu = (stress[0]*dtestdx + stress[2]*dtestdy)/E;//(grad u,grad phi)
 
  //std::cout<<"residual_liniso_x_test_"<<std::endl;
 
  return divgradu;
}

RES_FUNC(residual_liniso_y_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,z,yx,zy,zx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  //strain[0] = basis[2].dudx;
  //strain[1] = basis[3].dudy;
  //strain[2] = basis[2].dudy + basis[3].dudx;

  //double ut = (basis[1].uu - basis[1].uuoldold)/dt_/2.;//thermal strain
  double ut = (basis[1].uu - basis[1].uuold)/dt_;//thermal strain

  double phi = basis[0].uu;
  double h = phi*phi*(1.-phi)*(1.-phi);
  //double hp = 2.*(1.-phi)*(1.-phi)*phi-2.*(1.-phi)*phi*phi;//2 (1 - x)^2 x - 2 (1 - x) x^2
  double strain_phi = 30.*beta*h*(phi-basis[0].uuold)/dt_;
//   double strain_phi = 0.*2.*30.*beta*(c1+c2)*(hp*(phi-basis[0].uuold)/dt_*basis[0].dudy
// 					+h*(basis[0].dudy-basis[0].duolddy)/dt_
// 					)*test;

  double ff =   alpha*ut + strain_phi;

  strain[0] = (basis[2].dudx-basis[2].duolddx)/dt_- ff;
  strain[1] = (basis[3].dudy-basis[3].duolddy)/dt_- ff;
  strain[2] = (basis[2].dudy-basis[2].duolddy + basis[3].dudx-basis[3].duolddx)/dt_;// - alpha*ut - strain_phi;

  //stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  double divgradu = (stress[1]*dtestdy + stress[2]*dtestdx)/E;//(grad u,grad phi)
  
  //std::cout<<"residual_liniso_y_test_"<<std::endl;

  return divgradu;
}

RES_FUNC(residual_stress_x_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double phi = basis[0].uu;
  double h = phi*phi*(1.-phi)*(1.-phi);
  double phit = (phi - basis[0].uuold)/dt_;

  double ut = (basis[1].uu - basis[1].uuold)/dt_;

  double strain[2]; 
  double stress = (basis[4].uu - basis[4].uuold)/dt_;//x,y,yx

  //test function
  double test = basis[0].phi[i];

  strain[0] = (basis[2].dudx-basis[2].duolddx)/dt_- alpha*ut - 30.*h*phit;
  strain[1] = (basis[3].dudy-basis[3].duolddy)/dt_- alpha*ut - 30.*h*phit;

  double sx = c1*strain[0] + c2*strain[1];

  //std::cout<<strain[0]<<" "<<strain[1]<<" "<<sx<<" "<<stress - sx<<" "<<(stress - sx)*test<<" "<<(stress - sx)*test/E<<std::endl;

  return (stress - sx)*test*dt_/E;
}

RES_FUNC(residual_stress_y_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double phi = basis[0].uu;
  double h = phi*phi*(1.-phi)*(1.-phi);
  double phit = (phi - basis[0].uuold)/dt_;

  double ut = (basis[1].uu - basis[1].uuold)/dt_;

  double strain[2]; 
  double stress = (basis[5].uu - basis[5].uuold)/dt_;//x,y,yx

  //test function
  double test = basis[0].phi[i];

  strain[0] = (basis[2].dudx-basis[2].duolddx)/dt_- alpha*ut - 30.*h*phit;
  strain[1] = (basis[3].dudy-basis[3].duolddy)/dt_- alpha*ut - 30.*h*phit;

  double sy = c2*strain[0] + c1*strain[1];

  return (stress - sy)*test*dt_/E;//(grad u,grad phi)
}

RES_FUNC(residual_stress_xy_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  //double strain[2]; 
  double stress = (basis[6].uu - basis[6].uuold)/dt_;

  double test = basis[0].phi[i];

  //strain[0] = basis[0].dudx;
  //strain[1] = basis[1].dudy;
  double strain = (basis[2].dudy-basis[2].duolddy + basis[3].dudx-basis[3].duolddx)/dt_;

  //stress[0] = c1*strain[0] + c2*strain[1];
  //stress[1] = c2*strain[0] + c1*strain[1];
  double sxy = c3*strain;

  return (stress - sxy)*test*dt_/E;//(grad u,grad phi)
}

PRE_FUNC(prec_phase_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double test = basis[0].phi[i];
  
  double phit = m*(basis[0].phi[j])/dt_*test;
  double divgrad = a*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);

  return (phit + t_theta_*divgrad)/m;
}

PRE_FUNC(prec_heat_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];

  double stress = test*alpha*basis[1].phi[j]*(residual_stress_x_dt_(basis, 
						      i, dt_, dt_, t_theta_,t_theta_,
								    0.,0)
				+residual_stress_y_dt_(basis, 
						       i, dt_, dt_, t_theta_,t_theta_,
						       0.,0));
  double divgrad = k*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  double u_t =rho*c*basis[1].phi[j]/dt_*test;
 
  return (u_t + t_theta_*divgrad + stress)/rho/c;
}

PRE_FUNC(prec_liniso_x_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;

  double strain[3], stress[3];//x,y,z,yx,zy,zx

  strain[0] = dbasisdx;
  strain[1] = dbasisdy;
  strain[2] = (dbasisdy + dbasisdx);

  stress[0] = c1*strain[0] + c2*strain[1];
  //stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  double divgradu = (stress[0]*dtestdx + stress[2]*dtestdy)/E/dt_;//(grad u,grad phi)
  //double divgradu = (stress[0]*dtestdx + stress[2]*dtestdy)/E;//(grad u,grad phi)
  
  return divgradu;
}

PRE_FUNC(prec_liniso_y_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;

  double strain[3], stress[3];//x,y,z,yx,zy,zx

  strain[0] = dbasisdx;
  strain[1] = dbasisdy;
  strain[2] = (dbasisdy + dbasisdx);

  //stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];


  double divgradu = (stress[1]*dtestdy + stress[2]*dtestdx)/E/dt_;//(grad u,grad phi)
  //double divgradu = (stress[1]*dtestdy + stress[2]*dtestdx)/E;//(grad u,grad phi)

  //std::cout<<divgradu<<std::endl;
  return divgradu;
}

PRE_FUNC(prec_stress_test_)
{
  double test = basis[0].phi[i];

  return test * basis[0].phi[j]/dt_*dt_/E;
}

PPR_FUNC(postproc_stress_x_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double strain[2];//x,y,z,yx,zy,zx
  double phi = u[0];
  if(phi < 0.) phi = 0.;
  if(phi > 1.) phi = 1.;
  double h = phi*phi*(1.-phi)*(1.-phi);
//   strain[0] = gradu[0] - alpha*u[1] - 30.*beta*h*phi;//var 0 dx
//   strain[1] = gradu[3] - alpha*u[1] - 30.*beta*h*phi;//var 1 dy
  strain[0] = gradu[0] - alpha*u[1];// - 30.*beta*h*phi;//var 0 dx
  strain[1] = gradu[4] - alpha*u[1];// - 30.*beta*h*phi;//var 1 dy

  return c1*strain[0] + c2*strain[1];
}

PPR_FUNC(postproc_stress_xd_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double strain[2];//x,y,z,yx,zy,zx
//   double phi = u[0];
//   if(phi < 0.) phi = 0.;
//   if(phi > 1.) phi = 1.;
//   double h = phi*phi*(1.-phi)*(1.-phi);
//   strain[0] = gradu[0] - alpha*u[1] - 30.*beta*h*phi;//var 0 dx
//   strain[1] = gradu[3] - alpha*u[1] - 30.*beta*h*phi;//var 1 dy
  strain[0] = gradu[0];//var 0 dx
  strain[1] = gradu[4];//var 1 dy

  return c1*strain[0] + c2*strain[1];
}

PPR_FUNC(postproc_stress_y_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double strain[2];//x,y,z,yx,zy,zx
  double phi = u[0];
  if(phi < 0.) phi = 0.;
  if(phi > 1.) phi = 1.;
  double h = phi*phi*(1.-phi)*(1.-phi);
//   strain[0] = gradu[0] - alpha*u[1] - 30.*beta*h*phi;//var 0 dx
//   strain[1] = gradu[3] - alpha*u[1] - 30.*beta*h*phi;//var 1 dy
  strain[0] = gradu[0] - alpha*u[1] - 30.*beta*h*phi;//var 0 dx
  strain[1] = gradu[4] - alpha*u[1] - 30.*beta*h*phi;//var 1 dy

  return c2*strain[0] + c1*strain[1];
}

PPR_FUNC(postproc_stress_xy_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double phi = u[0];
  if(phi < 0.) phi = 0.;
  if(phi > 1.) phi = 1.;
  double h = phi*phi*(1.-phi)*(1.-phi);

  double strain = gradu[1] + gradu[3] - alpha*u[1] - 30.*beta*h*phi;

  return c3*strain;
}

PPR_FUNC(postproc_stress_eq_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double strain[3], stress[3];//x,y,z,yx,zy,zx
  double phi = u[0];
  if(phi < 0.) phi = 0.;
  if(phi > 1.) phi = 1.;
  double h = phi*phi*(1.-phi)*(1.-phi);

  strain[0] = gradu[0] - alpha*u[1] - 30.*beta*h*phi;//var 0 dx
  strain[1] = gradu[4] - alpha*u[1] - 30.*beta*h*phi;//var 1 dy
  strain[2] = gradu[1] + gradu[3];// - alpha*u[1] - 30.*beta*h*phi;

  stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

//   return sqrt(((stress[0]-stress[1])*(stress[0]-stress[1])
// 	       + stress[0]*stress[0]
// 	       + stress[1]*stress[1]
// 	       + 6. *stress[2]*stress[2]
// 	       )/2.);
  return sqrt((stress[0]-stress[1])*(stress[0]-stress[1])
	       + 3.*stress[2]*stress[2]
	       );
}

PPR_FUNC(postproc_stress_eqd_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double strain[3], stress[3];//x,y,z,yx,zy,zx
  double phi = u[0];
  if(phi < 0.) phi = 0.;
  if(phi > 1.) phi = 1.;
  double h = phi*phi*(1.-phi)*(1.-phi);

  strain[0] = gradu[0];// - alpha*u[1] - 30.*beta*h*phi;//var 0 dx
  strain[1] = gradu[4];// - alpha*u[1] - 30.*beta*h*phi;//var 1 dy
  strain[2] = gradu[1] + gradu[3];// + gradu[2];// - alpha*u[1] - 30.*beta*h*phi;

  stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

//   return sqrt(((stress[0]-stress[1])*(stress[0]-stress[1])
// 	       + stress[0]*stress[0]
// 	       + stress[1]*stress[1]
// 	       + 6. *stress[2]*stress[2]
// 	       )/2.);

  return sqrt((stress[0]-stress[1])*(stress[0]-stress[1])
	       + 3.*stress[2]*stress[2]
	       );
//   return (stress[0]+stress[1])/2.
//     +sqrt((stress[0]-stress[1])*(stress[0]-stress[1])/4.+stress[3]*stress[3]);
}

PPR_FUNC(postproc_phi_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double phi = u[0];
  if(phi < 0.) phi = 0.;
  if(phi > 1.) phi = 1.;
  return phi;
}

PPR_FUNC(postproc_strain_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double phi = u[0];
  double uu = u[1];
//   if(phi < 0.) phi = 0.;
//   if(phi > 1.) phi = 1.;
  double h = phi*phi*(1.-phi)*(1.-phi);
  return alpha*uu;// + 30.*beta*h*phi;
}
}//namespace uehara


namespace uehara2
{

INI_FUNC(init_phase_c_)
{
  double phi_sol_ = 1.;
  double phi_liq_ = 0.;

  double val = phi_sol_ ;  

  double r0 = uehara::r0;
  double dx = 1.e-2;
  double x0 = 15.*dx;
  double r = .9*r0*dx/2.;

  double rr = sqrt((x-x0)*(x-x0)+(y-x0)*(y-x0));

  val=phi_liq_;

  if( rr > r0*sqrt(2.)*dx/2. ){
    val=phi_sol_;
  }
  else {
    val=phi_liq_;
  }

  return val;
}

INI_FUNC(init_heat_)
{
  double val = 300.;  

  return val;
}

RES_FUNC(residual_heat_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[1].phi[i];
  //u, phi
  double phi = basis[0].uu;
  double phiold = basis[0].uuold;
  double phioldold = basis[0].uuoldold;
  double u = basis[1].uu;
  double uold = basis[1].uuold;

  double dudx = basis[1].dudx;
  double dudy = basis[1].dudy;
  //double dudz = basis[1].dudz;

  double ut = uehara::rho*uehara::c*(u-uold)/dt_*test;
  double divgradu = uehara::k*(dudx*dtestdx + dudy*dtestdy);
  double h = phi*phi*(1.-phi)*(1.-phi);
  //double phitu = -30.*1e12*uehara::L*h*(phi-phioldold)/2./dt_*test; 
  double phitu = -30.*uehara::L*h*(phi-phiold)/dt_*test; 
  
  //thermal term
  double stress = test*uehara::alpha*u*(uehara::residual_stress_x_dt_(basis, 
						      i, dt_, dt_, t_theta_, t_theta_,
						      time, eqn_id)
				+uehara::residual_stress_y_dt_(basis, 
						       i, dt_,  dt_,t_theta_,t_theta_,
						       time, eqn_id));
  

  double rhs = divgradu + phitu + stress;

  return (ut + rhs)/uehara::rho/uehara::c;

}
}//namespace uehara2








namespace coupledstress
{
  double E = 1e6;
  double nu = .3;

  //plane stress
//   double c = E/(1.-nu*nu);
//   double c1 = c;
//   double c2 = c*nu;
//   double c3 = c*(1.-nu)/2.;//always confused about the 2 here
  //plane strain
  double c = E/(1.+nu)/(1.-2.*nu);
  double c1 = c*(1.-nu);
  double c2 = c*nu;
  double c3 = c*(1.-2.*nu)/2.;


RES_FUNC(residual_liniso_x_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,yx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  //test function
  //double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  strain[2] = basis[0].dudy + basis[1].dudx;

  stress[0] = c1*strain[0] + c2*strain[1];
  // stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  double divgradu = stress[0]*dtestdx + stress[2]*dtestdy;//(grad u,grad phi)
 
  //std::cout<<"residual_liniso_x_test_"<<std::endl;
 
  return divgradu;
}

RES_FUNC(residual_liniso_y_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,z,yx,zy,zx


  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  //double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  strain[2] = basis[0].dudy + basis[1].dudx;

  stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  double divgradu = stress[1]*dtestdy + stress[2]*dtestdx;//(grad u,grad phi)
  
  //std::cout<<"residual_liniso_y_test_"<<std::endl;

  return divgradu;
}

RES_FUNC(residual_stress_x_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,yx

  //test function
  double test = basis[0].phi[i];
  //u, phi
  double sx = basis[2].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  //strain[2] = basis[0].dudy + basis[1].dudx;

  stress[0] = c1*strain[0] + c2*strain[1];
  // stress[1] = c2*strain[0] + c1*strain[1];
  //stress[2] = c3*strain[2];

  return (sx - stress[0])*test;
}

RES_FUNC(residual_stress_y_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,z,yx,zy,zx

  double test = basis[0].phi[i];
  //u, phi
  double sy = basis[3].uu;

  strain[0] = basis[0].dudx;
  strain[1] = basis[1].dudy;
  //strain[2] = basis[0].dudy + basis[1].dudx;

  //stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  //stress[2] = c3*strain[2];

  return (sy - stress[1])*test;//(grad u,grad phi)
}

RES_FUNC(residual_stress_xy_test_)
{
  //3-D isotropic x-displacement based solid mech, steady state
  //strong form: sigma = stress  eps = strain
  // d^T sigma = d^T B D eps == 0

  double strain[3], stress[3];//x,y,z,yx,zy,zx

  double test = basis[0].phi[i];
  //u, phi
  double sxy = basis[4].uu;

  //strain[0] = basis[0].dudx;
  //strain[1] = basis[1].dudy;
  strain[2] = basis[0].dudy + basis[1].dudx;

  //stress[0] = c1*strain[0] + c2*strain[1];
  //stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  return (sxy - stress[2])*test;//(grad u,grad phi)
}

PRE_FUNC(prec_liniso_x_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;

  double strain[3], stress[3];//x,y,z,yx,zy,zx

  strain[0] = dbasisdx;
  strain[1] = dbasisdy;
  strain[2] = dbasisdy + dbasisdx;

  stress[0] = c1*strain[0] + c2*strain[1];
  //stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];

  double divgradu = stress[0]*dtestdx + stress[2]*dtestdy;//(grad u,grad phi)
 
  return divgradu;
}

PRE_FUNC(prec_liniso_y_test_)
{

  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;

  double strain[3], stress[3];//x,y,z,yx,zy,zx

  strain[0] = dbasisdx;
  strain[1] = dbasisdy;
  strain[2] = dbasisdy + dbasisdx;

  //stress[0] = c1*strain[0] + c2*strain[1];
  stress[1] = c2*strain[0] + c1*strain[1];
  stress[2] = c3*strain[2];


  double divgradu = stress[1]*dtestdy + stress[2]*dtestdx;//(grad u,grad phi)

  return divgradu;
}

PRE_FUNC(prec_stress_test_)
{
  double test = basis[0].phi[i];

  return test * basis[0].phi[j];
}

PPR_FUNC(postproc_stress_x_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...



  double strain[2];//x,y,z,yx,zy,zx
  strain[0] = gradu[0];//var 0 dx
  strain[1] = gradu[4];//var 1 dy

  return c1*strain[0] + c2*strain[1];
}

PPR_FUNC(postproc_stress_y_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...

  double strain[2];//x,y,z,yx,zy,zx
  strain[0] = gradu[0];//var 0 dx
  strain[1] = gradu[4];//var 1 dy

  return c2*strain[0] + c1*strain[1];
}

PPR_FUNC(postproc_stress_xy_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double strain = gradu[1] + gradu[3];

  return c3*strain;
}

}//namespace coupledstress

namespace laplace
{

RES_FUNC(residual_heat_test_)
{

  //u[x,y,t]=exp(-2 pi^2 t)sin(pi x)sin(pi y)
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;
  //double uold = basis[0].uuold;

  //double ut = (u-uold)/dt_*test;
  double divgradu = (basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  //double divgradu_old = (basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
 
 
  return divgradu - 8.*test;
}
}//namespace laplace

namespace cahnhilliard
{
  //https://www.sciencedirect.com/science/article/pii/S0021999112007243
  double M = 1.;
  double Eps = 1.;
  double alpha = 1.;//alpha >= 1
  double pi = 3.141592653589793;
  double fcoef_ = 0.;

double F(const double &x,const double &t)
{
// Sin(a*Pi*x) 
//  - M*(Power(a,2)*Power(Pi,2)*(1 + t)*Sin(a*Pi*x) - Power(a,4)*Ep*Power(Pi,4)*(1 + t)*Sin(a*Pi*x) + 
//       6*Power(a,2)*Power(Pi,2)*Power(1 + t,3)*Power(Cos(a*Pi*x),2)*Sin(a*Pi*x) - 
//       3*Power(a,2)*Power(Pi,2)*Power(1 + t,3)*Power(Sin(a*Pi*x),3))

  double a = alpha;
  return sin(a*pi*x) 
    - M*(std::pow(a,2)*std::pow(pi,2)*(1 + t)*sin(a*pi*x) - std::pow(a,4)*Eps*std::pow(pi,4)*(1 + t)*sin(a*pi*x) + 
	 6*std::pow(a,2)*std::pow(pi,2)*std::pow(1 + t,3)*std::pow(cos(a*pi*x),2)*sin(a*pi*x) - 
	 3*std::pow(a,2)*std::pow(pi,2)*std::pow(1 + t,3)*std::pow(sin(a*pi*x),3));
}
double fp(const double &u)
{
  return u*u*u - u;
}

INI_FUNC(init_c_)
{
  return sin(alpha*pi*x);
}

INI_FUNC(init_mu_)
{
  //-Sin[a \[Pi] x] + a^2 \[Pi]^2 Sin[a \[Pi] x] + Sin[a \[Pi] x]^3
  return -sin(alpha*pi*x) + alpha*alpha*pi*pi*sin(alpha*pi*x) + sin(alpha*pi*x)*sin(alpha*pi*x)*sin(alpha*pi*x);
}

RES_FUNC(residual_c_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  double c = basis[0].uu;
  double cold = basis[0].uuold;
  double mu = basis[1].uu;
  double x = basis[0].xx;

  double ct = (c - cold)/dt_*test;
  double divgradmu = M*t_theta_*(basis[1].dudx*dtestdx + basis[1].dudy*dtestdy + basis[1].dudz*dtestdz)
    + M*(1.-t_theta_)*(basis[1].duolddx*dtestdx + basis[1].duolddy*dtestdy + basis[1].duolddz*dtestdz);
  double f = t_theta_*fcoef_*F(x,time)*test + (1.-t_theta_)*fcoef_*F(x,time-dt_)*test;

  return ct + divgradmu - f;
}

RES_FUNC(residual_mu_)
{
  //derivatives of the test function
  double dtestdx = basis[1].dphidxi[i]*basis[1].dxidx
    +basis[1].dphideta[i]*basis[1].detadx
    +basis[1].dphidzta[i]*basis[1].dztadx;
  double dtestdy = basis[1].dphidxi[i]*basis[1].dxidy
    +basis[1].dphideta[i]*basis[1].detady
    +basis[1].dphidzta[i]*basis[1].dztady;
  double dtestdz = basis[1].dphidxi[i]*basis[1].dxidz
    +basis[1].dphideta[i]*basis[1].detadz
    +basis[1].dphidzta[i]*basis[1].dztadz;
  //test function
  double test = basis[1].phi[i];
  double c = basis[0].uu;
  double mu = basis[1].uu;

  double mut = mu*test;
  double f = fp(c)*test;
  double divgradc = Eps*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);

  return mut - f - divgradc;
}
PARAM_FUNC(param_)
{
  fcoef_ = plist->get<double>("fcoef");
}



}//namespace cahnhilliard

namespace quaternion
{

  //see
  //[1] LLNL-JRNL-409478 A Numerical Algorithm for the Solution of a Phase-Field Model of Polycrystalline Materials
  //M. R. Dorr, J.-L. Fattebert, M. E. Wickett, J. F. Belak, P. E. A. Turchi (2008)
  //[2] LLNL-JRNL-636233 Phase-field modeling of coring during solidification of Au-Ni alloy using quaternions and CALPHAD input
  //J. L. Fattebert, M. E. Wickett, P. E. A. Turchi May 7, 2013

  const double M = 10.;
  const double H = 1.;
  const double T = 1.;
  const double D = 2.*H*T;
  const double eps = .02;

  const int N = 4;

  void PI(const double *q, double *v){
    double norm2 = 0.;
    double dot = 0.;
    for(int k = 0; k < N; k++){
      norm2 = norm2 + q[k]*q[k];
      dot = dot +q[k]*v[k];
    }
    for(int k = 0; k < N; k++){
      v[k] = q[k]*dot/norm2;
    }
  }
  void PIT(const double *q, double *v){
    double norm2 = 0.;
    double dot = 0.;
    for(int k = 0; k < N; k++){
      norm2 = norm2 + q[k]*q[k];
      dot = dot +q[k]*v[k];
    }
    for(int k = 0; k < N; k++){
      v[k] = v[k]-q[k]*dot/norm2;
    }
  }

  //double pi = 3.141592653589793;


PARAM_FUNC(param_)
{
}

  double dist(const double &x, const double &y, const double &z,
	   const double &x0, const double &y0, const double &z0,
	   const double &r)
  {
    const double c = (x-x0)*(x-x0) + (y-y0)*(y-y0) + (z-z0)*(z-z0);
    return r-sqrt(c);
  }

INI_FUNC(init_)
{
  //return .001*r(x,eqn_id)*r(x,N-eqn_id)*(y,eqn_id)*r(y,N-eqn_id);
//   srand((int)(1000*x));
//   return ((rand() % 100)/50.-1.)*.001;
  const double r = .1;
  const double x0 = .5;
  const double y0 = .5;
  const double w = .1;//.025;
  //g1 is the background grain
  //g0 is black??
  const double g0[4] = {-0.707106781186547,
			-0.000000000000000,
			0.707106781186548,
			-0.000000000000000};
  const double g1[4] = {0.460849109679818,
		       0.025097870693789,
		       0.596761014095944,
		       0.656402686655941};   
  const double g2[4] = {0.514408948531741,
			0.574286304520035,
			0.563215252388157,
			0.297266300795322};
  const double g3[4] = {0.389320954032683,
			0.445278323410822,
			0.064316533986652,
			0.803753564786790};

  const double scale = (g3[eqn_id]-g1[eqn_id])/2.;
  const double shift = (g3[eqn_id]+g1[eqn_id])/2.;
  double d = dist(x,y,z,x0,y0,0,r);
  double val = shift + scale*tanh(d/sqrt(2.)/w);
  return val;
}

RES_FUNC(residual_)
{
  //derivatives of the test function
  const double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  const double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  const double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  const double test = basis[0].phi[i];

  const double u = basis[eqn_id].uu;
  const double uold = basis[eqn_id].uuold;

  double norm2 = 0.;
  double normgrad = 0.;
  double sumk = 0.;
  for(int k = 0; k < N; k++){
    norm2 = norm2 + basis[k].uu*basis[k].uu;
    normgrad = normgrad + basis[k].dudx*basis[k].dudx + basis[k].dudy*basis[k].dudy + basis[k].dudz*basis[k].dudz;
    sumk = sumk + basis[k].uu*(basis[k].dudx*dtestdx + basis[k].dudy*dtestdy + basis[k].dudz*dtestdz);
  }
  normgrad = sqrt(normgrad);
  const double c = ((normgrad > .0001) ? eps+D/normgrad : eps);

  sumk = -sumk*basis[eqn_id].uu*c/norm2;

  const double divgradu = c*(basis[eqn_id].dudx*dtestdx + basis[eqn_id].dudy*dtestdy + basis[eqn_id].dudz*dtestdz);

  return (u-uold)/dt_*test + M*(divgradu + sumk); 

}
PRE_FUNC(prec_)
{
  return 1;
}
PPR_FUNC(postproc_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double s =0.;
  for(int j = 0; j < N; j++){
    s = s + u[j]*u[j];
  }

  return s;
}
}//namespace quaternion

namespace grain
{

  //see
  //[1] Suwa et al, Mater. T. JIM., 44,11, (2003);
  //[2] Krill et al, Acta Mater., 50,12, (2002); 



  double L = 1.;
  double alpha = 1.;
  double beta = 1.;
  double gamma = 1.;
  double kappa = 2.;

  int N = 6;

  double pi = 3.141592653589793;

  double r(const double &x,const int &n){
    return sin(64./512.*x*n*pi);
  }

PARAM_FUNC(param_)
{
  N = plist->get<int>("numgrain");
}

INI_FUNC(init_)
{
  //return .001*r(x,eqn_id)*r(x,N-eqn_id)*(y,eqn_id)*r(y,N-eqn_id);
  return ((rand() % 100)/50.-1.)*.001;
}
RES_FUNC(residual_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  double test = basis[0].phi[i];

  double u = basis[eqn_id].uu;
  double uold = basis[eqn_id].uuold;

  double divgradu = kappa*(basis[eqn_id].dudx*dtestdx + basis[eqn_id].dudy*dtestdy + basis[eqn_id].dudz*dtestdz);

  double s = 0.;
  for(int k = 0; k < N; k++){
    s = s + basis[k].uu*basis[k].uu;
  }
  s = s - u*u;

  return (u-uold)/dt_*test + L* ((-alpha*u + beta*u*u*u +2.*gamma*u*s)*test +  divgradu); 

}
PRE_FUNC(prec_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double u = basis[eqn_id].uu;
  
  double test = basis[0].phi[i];
  double divgrad = L*kappa*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  double u_t =test * basis[0].phi[j]/dt_;
  double alphau = -test*L*alpha*basis[0].phi[j];
  double betau = 3.*u*u*basis[0].phi[j]*test*L*beta;

  double s = 0.;
  for(int k = 0; k < N; k++){
    s = s + basis[k].uu*basis[k].uu;
  }
  s = s - u*u;

  double gammau = 2.*gamma*L*basis[0].phi[j]*s*test;

  return u_t + divgrad + betau + gammau;// + alphau ;
}
PPR_FUNC(postproc_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double s =0.;
  for(int j = 0; j < N; j++){
    s = s + u[j]*u[j];
  }

  return s;
}
}//namespace grain

namespace periodic
{

RES_FUNC(residual_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  double test = basis[0].phi[i];

  double u = basis[0].uu;
  double uold = basis[0].uuold;
  double x = basis[0].xx;

  double f = sin(11.*x);

  double divgradu = basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz;

  return (u-uold)/dt_*test + divgradu - f*test; 

}


}//namespace periodic


namespace kundin
{
  //double vf = 1e5;// J/mol-at / J/m^3
  //double lf = 1.;//m/m    length conversion
  double lf = 1000.;//mm/m    length conversion
  double lf2 = lf*lf;
  double lf3 = lf2*lf;
  double W  = 0.00000007*lf;//m
  double dx = 0.000000032*lf; //m
  double lx = dx*320.; //m

  int N = 6;
  double a_1 = sqrt(2.)/3.;
  double a_2 = 1.175;
  double sigma = 0.12/lf2;//J/m^2
  double eps_4 = .05;

  //is this the correct conversion? the paper is confusing
  double x_fact = (1.e5)/lf3;// (J/mol-at/mf)   should convert J/mol-at to J/m^3
  double XA_L [6] = { 1.2*x_fact, 1.3*x_fact, .9*x_fact, 3.*x_fact, 3.*x_fact, 3.*x_fact };//J/mol-at/mf^2
  double deltaA_SLT0 [6] = {.01645, .034356, -.0098313, -.0412, -.00545,  .00005};//mf
  double CAeq_ST0 [6] =    {.20645, .219356,  .0201687,  .0098,  .00355,  .00505};//mf
  double CAeq_LT0 [6] =    {.19,    .185,     .03,       .051,   .009,    .005};//mf
  double kA_L [6] =        {3.48,   1.48,     1.48,      2.48,   1.51,    11.21};
  double mA_Sp [6] =       {9118.,  4365.8,  -15257.4,  -3640.8,-27522.9, 3700000.};//K/mf
  double mA_Lp [6] =       {2620.,  2936.,   -10309.,   -1468.7,-18131.3, 330000.};//K/mf
  double T0 = 1635.15;//K
  //double T0 = 1243.;//K
  //double T0 = 1362.;//C
  double d_fact = (1.e-10)*lf2;
  double DA_L [6] = {8.9843*d_fact, 9.0398*d_fact, 10.759*d_fact, 10.526*d_fact, 10.992*d_fact, 11.092*d_fact};//m^2/s
  double q_fact =  (1.e5)*lf;
  double QA_L [6] = {5.615*q_fact,20.3*q_fact, .967*q_fact, 68.9*q_fact, .977*q_fact, .000123*q_fact};//(J/mol-at)/(J/m^3) s/m^2
  double tau = 8.12e-8;//s


  double Tdot [4] = {28745.6, 14372.8, 7186.39,3593.2};//K/s
  //double lambda = 18.3;//J/(s m K) keep as meters
  double a = 4.7e-6 * lf2;//m^2/s
  //douple pi = 3.14159;


double df(const double p)
{
  return 2.* (1. - p)*(1. - p)*p - 2.* (1. - p)*p*p;
}
double dg(const double p)
{
  return p*p*p* (-15. + 12. *p) + 3 *p*p* (10. - 15.* p + 6 *p*p);
}
double CAeq_L(const int i,const double T){
  return CAeq_LT0[i] + (T-T0)/mA_Lp[i];
}
double CAeq_S(const int i,const double T){
  return CAeq_ST0[i] + (T-T0)/mA_Sp[i];
}
double deltaA_SL(const int i,const double T){
  double k_S = 1./kA_L[i];
  //return deltaA_SLT0[i] + (T0-T)*(1.-k_S)/mA_Lp[i];
  return CAeq_S(i,T) - CAeq_L(i,T);
}
  double temp(const double time, const double z){
  double G_z = Tdot[0]/2./a*z;//K/m(mm)
  //return T0 - Tdot[0]*time + G_z*z;
  //return T0 +5.*(z- .1*time);
  //return T0 - 1.e2*time+5.*z;
  return  T0 - 150.;
}
double deltaG_ch(const double * CA, const double p, const double T)
{

  double s = 0.;
  for (int i = 0; i < N; i++){
    double CAeq = CAeq_S(i,T)*p + CAeq_L(i,T)*(1. - p);
    double d = (p+(1.-p)*kA_L[i]);
    s += XA_L[i]*deltaA_SL(i,T)*(CA[i]-CAeq)/d;
  }
  return s;
}
double fn (const double p){
  return 16.*p*p*(1.-p)*(1.-p);
}
double XA_bar(const int i, const double p){
  //this should be evaluated at p = .5
  double XA_S = kA_L[i]*XA_L[i];
  //double val = (1.-p)/XA_L[i] + p /XA_S;
  double val = (1.-.5)/XA_L[i] + .5 /XA_S;
  return 1./val;
}
double QA_L_(const int i,const double p,const double T){
  return XA_bar(i,p)*deltaA_SL(i,T)*deltaA_SL(i,T)/DA_L[i];
}
double tau_(const double p,const double T){
  double t =0.;  
  for (int i = 0; i < N; i++){
    t += QA_L_(i,p,T);
  }
  t = t*a_1*a_2*W*W*W/sigma;
  return t;
}
  //double a_small =  5.e-3;//   => px < 1e-9
double a_small =  5.e-9;//   => px < 1e-9
inline const double a_s_(const double px, const double py, const double pz, const double ep){
    // in 2d, equivalent to: a_s = 1 + ep cos 4 theta
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 1. + ep;
    //return 1.-3.*ep + 4.*ep*(px2*px2 + py2*py2 + pz2*pz2)/norm4;
    return 1.;
  }
inline const double da_s_dpx(const double px, const double py, const double pz, const double ep){
    // (16 ep x (-y^4 - z^4 + x^2 y^2 + x^2 z^2))/(x^2 + y^2 + z^2)^3
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    double norm6 = norm4*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 0.;
    //return 16.*ep*px*(- py2*py2 - pz2*pz2 + px2*py2 + px2*pz2)/norm6;
    return 0.;
  }
inline const double da_s_dpy(const double px, const double py, const double pz, const double ep){
    // -((16 ep y (x^4 - x^2 y^2 - y^2 z^2 + z^4))/(x^2 + y^2 + z^2)^3)
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    double norm6 = norm4*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 0.;
    //return - 16.*ep*py*(px2*px2 + pz2*pz2 - px2*py2 - py2*pz2)/norm6;
    return 0.;
  }
inline const double da_s_dpz(const double px, const double py, const double pz, const double ep){
    //-((16 ep z (x^4 + y^4 - x^2 z^2 - y^2 z^2))/(x^2 + y^2 + z^2)^3)
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    double norm6 = norm4*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 0.;
    //return - 16.*ep*pz*(px2*px2 + py2*py2 - px2*pz2 - py2*pz2)/norm6;
    return 0.;
  }
  
  
RES_FUNC(phiresidual_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  double test = basis[0].phi[i];

  int phi_id = eqn_id;
  double phi[2] = {basis[phi_id].uu, basis[phi_id].uuold};
  double phi_x[2] = {basis[phi_id].dudx, basis[phi_id].duolddx};
  double phi_y[2] = {basis[phi_id].dudy, basis[phi_id].duolddy};
  double phi_z[2] = {basis[phi_id].dudz, basis[phi_id].duolddz};

  double y = basis[phi_id].yy;

  double a_s[2] = {a_s_(phi_x[0], phi_y[0], phi_z[0], eps_4), 
		   a_s_(phi_x[1], phi_y[1], phi_z[1], eps_4)};
  //a_s[0]=1;a_s[1]=1;
  double a_spx[2] = {da_s_dpx(phi_x[0], phi_y[0], phi_z[0], eps_4), 
		     da_s_dpx(phi_x[1], phi_y[1], phi_z[1], eps_4)};
  double a_spy[2] = {da_s_dpy(phi_x[0], phi_y[0], phi_z[0], eps_4), 
		     da_s_dpy(phi_x[1], phi_y[1], phi_z[1], eps_4)};
  double a_spz[2] = {da_s_dpz(phi_x[0], phi_y[0], phi_z[0], eps_4), 
		     da_s_dpz(phi_x[1], phi_y[1], phi_z[1], eps_4)};

  tau = tau_(phi[0],temp(time,y));
  tau=2.e-5;
  //tau=tau/1000.;
  double phit = (tau*phi[0]-tau*phi[1])/dt_*test;

  double divgrad = t_theta_*W*W*a_s[0]*a_s[0]*(phi_x[0]*dtestdx + phi_y[0]*dtestdy + phi_z[0]*dtestdz)
             +(1.-t_theta_)*W*W*a_s[1]*a_s[1]*(phi_x[1]*dtestdx + phi_y[1]*dtestdy + phi_z[1]*dtestdz);
 
  double normphi2[2] = {phi_x[0]*phi_x[0] + phi_y[0]*phi_y[0] + phi_z[0]*phi_z[0],
			phi_x[1]*phi_x[1] + phi_y[1]*phi_y[1] + phi_z[1]*phi_z[1]};

  double curlgrad = t_theta_*W*W*normphi2[0]*a_s[0]*(a_spx[0]*dtestdx + a_spy[0]*dtestdy + a_spz[0]*dtestdz)
              +(1.-t_theta_)*W*W*normphi2[1]*a_s[1]*(a_spx[1]*dtestdx + a_spy[1]*dtestdy + a_spz[1]*dtestdz);

  double dfdp = (t_theta_*df(phi[0])+(1.-t_theta_)*df(phi[1]))*test;

  double CA[6];
  double deltaG_ch_[2];
  for (int i = 0; i < N; i++) CA[i] =  basis[i].uu;
  double T = temp(time,y);
  deltaG_ch_[0] = deltaG_ch(CA,phi[0],T);

  for (int i = 0; i < N; i++) CA[i] =  basis[i].uuold;
  T = temp(time - dt_,y);
  deltaG_ch_[1] = deltaG_ch(CA,phi[1],T);

  double dgdp = -a_1*W/sigma*(t_theta_*dg(phi[0])*deltaG_ch_[0]
			+(1.-t_theta_)*dg(phi[1])*deltaG_ch_[1])*test;
//   if(phi[1] >0 && phi[1]<1)
//     std::cout<<dgdp<<" "<<dfdp<<" "<<phi[1]<<std::endl;

  return (phit + divgrad + curlgrad + dfdp + 10.*dgdp)/tau;// /tau/10.;

}
PRE_FUNC(phiprec_)
{
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];

  int phi_id = eqn_id;
  double phi =  basis[phi_id].uu;
  double phi_x = basis[phi_id].dudx;
  double phi_y = basis[phi_id].dudy;
  double phi_z = basis[phi_id].dudz;

  //tau = tau_(phi,temp(0.));
  tau = 2.e-5;
  double phit = tau*basis[phi_id].phi[j]/dt_*test;
  double a_s = a_s_(phi_x, phi_y, phi_z, eps_4);
  double divgrad = t_theta_*W*W*a_s*a_s*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);

  return (phit + t_theta_*divgrad)/tau;// /tau/10.;
}
RES_FUNC(cresidual_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  double test = basis[0].phi[i];

  double y = basis[eqn_id].yy;

  double CA[2] = {basis[eqn_id].uu, 
		  basis[eqn_id].uuold};
  double CA_x[2] = {basis[eqn_id].dudx,
		    basis[eqn_id].duolddx};
  double CA_y[2] = {basis[eqn_id].dudy,
		    basis[eqn_id].duolddy};
  double CA_z[2] = {basis[eqn_id].dudz,
		    basis[eqn_id].duolddz};
  double phi[2] = {basis[6].uu,
		   basis[6].uuold};
  double phi_x[2] = {basis[6].dudx,
		     basis[6].duolddx};
  double phi_y[2] = {basis[6].dudy,
		     basis[6].duolddy};
  double phi_z[2] = {basis[6].dudz,
		     basis[6].duolddz}; 

  double T[2] = {temp(time,y),
		 temp(time-dt_,y)};
  double D_L = DA_L[eqn_id];
  //double D_S = 0.;
  double D_S = D_L;
  double k_L =kA_L[eqn_id];

  double CAt = (CA[0] - CA[1])/dt_*test;

//   double CAeq[2] = {CAeq_S(eqn_id,T)*phi[0] + CAeq_L(eqn_id,T)*(1.- phi[0]),
// 		    CAeq_S(eqn_id,T)*phi[1] + CAeq_L(eqn_id,T)*(1.- phi[1])};
  
  double CAeq_x[2] = {CAeq_S(eqn_id,T[0])*phi_x[0] + CAeq_L(eqn_id,T[0])*(- phi_x[0]),
		      CAeq_S(eqn_id,T[1])*phi_x[1] + CAeq_L(eqn_id,T[1])*(- phi_x[1])};
  double CAeq_y[2] = {CAeq_S(eqn_id,T[0])*phi_y[0] + CAeq_L(eqn_id,T[0])*(- phi_y[0]),
		      CAeq_S(eqn_id,T[1])*phi_y[1] + CAeq_L(eqn_id,T[1])*(- phi_y[1])};
  double CAeq_z[2] = {CAeq_S(eqn_id,T[0])*phi_z[0] + CAeq_L(eqn_id,T[0])*(- phi_z[0]),
		      CAeq_S(eqn_id,T[1])*phi_z[1] + CAeq_L(eqn_id,T[1])*(- phi_z[1])};

  double CA_xd[2] = {CA_x[0] - CAeq_x[0],
		     CA_x[1] - CAeq_x[1]};
  double CA_yd[2] = {CA_y[0] - CAeq_y[0],
		     CA_y[1] - CAeq_y[1]};
  double CA_zd[2] = {CA_z[0] - CAeq_z[0],
		     CA_z[1] - CAeq_z[1]};

  //the denominator *should* be inside the gradient operator
  //however, most implementations factor it out like this
  double coef[2] = {(D_S*phi[0] + D_L*(1.-phi[0])*k_L)/(phi[0] + (1. - phi[0])*k_L),
		    (D_S*phi[1] + D_L*(1.-phi[1])*k_L)/(phi[1] + (1. - phi[1])*k_L)};
//   double coef[2] = {D_L,
// 		    D_L};
  double divgrad = t_theta_*coef[0]*(CA_xd[0]*dtestdx + CA_yd[0]*dtestdy + CA_zd[0]*dtestdz)
    +(1.-t_theta_)*coef[1]*(CA_xd[1]*dtestdx + CA_yd[1]*dtestdy + CA_zd[1]*dtestdz);

  double delta_SL[2] = {deltaA_SL(eqn_id,T[0]),
			deltaA_SL(eqn_id,T[1])};
  double normphi[2] = {sqrt(phi_x[0]*phi_x[0] + phi_y[0]*phi_y[0] + phi_z[0]*phi_z[0]),
		       sqrt(phi_x[1]*phi_x[1] + phi_y[1]*phi_y[1] + phi_z[0]*phi_z[1])};
  double small = 1.e-12;
  double coef2[2] = {0.,0.};
  if (normphi[0] > small) coef2[0] = W/sqrt(2.)*delta_SL[0]*(phi[0] - phi[1])/dt_/normphi[0];
  if (normphi[1] > small) coef2[1] = W/sqrt(2.)*delta_SL[1]*(phi[1] - basis[6].uuoldold)/dt_/normphi[1];
  double trap = t_theta_*coef2[0]*(phi_x[0]*dtestdx + phi_y[0]*dtestdy + phi_z[0]*dtestdz)
       + (1. - t_theta_)*coef2[1]*(phi_x[1]*dtestdx + phi_y[1]*dtestdy + phi_z[1]*dtestdz);

  //std::cout<<CAt<<" "<<divgrad<<"     :   "
  //std::cout<<CA<<" "<<CAold<<" "<<CA-CAold<<" "<<eqn_id<<" "<<basis[eqn_id].uu<<std::endl;
  return (CAt + divgrad + 0.*trap);
  //return CA[0]-CAeq_ST0[eqn_id];
}
PRE_FUNC(cprec_)
{
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[eqn_id].dphidxi[j]*basis[eqn_id].dxidx
    +basis[eqn_id].dphideta[j]*basis[eqn_id].detadx
    +basis[eqn_id].dphidzta[j]*basis[eqn_id].dztadx;
  double dbasisdy = basis[eqn_id].dphidxi[j]*basis[eqn_id].dxidy
    +basis[eqn_id].dphideta[j]*basis[eqn_id].detady
    +basis[eqn_id].dphidzta[j]*basis[eqn_id].dztady;
  double dbasisdz = basis[eqn_id].dphidxi[j]*basis[eqn_id].dxidz
    +basis[eqn_id].dphideta[j]*basis[eqn_id].detadz
    +basis[eqn_id].dphidzta[j]*basis[eqn_id].dztadz;
  double test = basis[0].phi[i];

  double CAt = basis[eqn_id].phi[j]/dt_*test;
  double CA = basis[eqn_id].phi[j];
  double CA_x = dbasisdx;
  double CA_y = dbasisdy;
  double CA_z = dbasisdz;
  double phi = basis[6].uu;
  double phi_x = basis[6].dudx;
  double phi_y = basis[6].dudy;
  double phi_z = basis[6].dudz;

  //double T = temp(0.);
  double D_L = DA_L[eqn_id];
  double k_L = kA_L[eqn_id];
  double den = (phi + (1. - phi)*k_L);
  double den_x = phi_x - k_L*phi_x;
  double den_y = phi_y - k_L*phi_y;
  double den_z = phi_z - k_L*phi_z;
  double num = CA;
  double num_x = CA_x;
  double num_y = CA_y;
  double num_z = CA_z;

  double CA_xd = (num_x*den - num*den_x)/den/den;
  double CA_yd = (num_y*den - num*den_y)/den/den;
  double CA_zd = (num_z*den - num*den_z)/den/den;
  double D_S = 0.;
  //double D_S = D_L;
  double coef = (D_S*phi + D_L*(1.-phi)*k_L)/den;
  double divgrad = coef*(CA_x*dtestdx + CA_y*dtestdy + CA_z*dtestdz);

  return (CAt + t_theta_*divgrad);
}
double init(const double x)
{
  double pi = 3.141592653589793;
  double val = lx/7.*sin(7.*pi*x/lx);
  return 10.*(std::abs(val)-.0005);
}
INI_FUNC(cinit_)
{
  double val = CAeq_LT0[eqn_id];
  if(y < init(x)) val=CAeq_ST0[eqn_id];
  //std::cout<<eqn_id<<" "<<val<<std::endl;
  return val;
}
DBC_FUNC(dbc0_)
{  
  return CAeq_LT0[0];
}
DBC_FUNC(dbc1_)
{  
  return CAeq_LT0[1];
}
DBC_FUNC(dbc2_)
{  
  return CAeq_LT0[2];
}
DBC_FUNC(dbc3_)
{  
  return CAeq_LT0[3];
}
DBC_FUNC(dbc4_)
{  
  return CAeq_LT0[4];
}
DBC_FUNC(dbc5_)
{  
  return CAeq_LT0[5];
}
INI_FUNC(phiinit_)
{

  double phi_sol_ = 1.;
  double phi_liq_ = 0.;

  double val = phi_liq_;  

  if(y < init(x)) val=phi_sol_;
  return val;
}
PPR_FUNC(postproc_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double y = xyz[1];
  return temp(time,y);
}

}//namespace kundin


namespace truchas
{
  // time is ms space is mm
  double rho_ = 7.57e-3;   // g/mm^3   density
  double cp_ = 750.65;    // (g-mm^2/ms^2)/g-K   specific heat

  //we can interp this as in truchas
  double k_ = .0213; //(g-mm^2/ms^3)/mm-K    thermal diffusivity
  double l_ = 2.1754e5; //g-mm^2/ms^2/g       latent heat
  double w_ =  2.4; 
  //double eps_ = .001;//       anisotropy strength
  double eps_ = 000547723;
  double m_ = .002149;// K ms/mm
  //double t_m_ = 1678.;
  double t_m_ = 1450.;

  double zmin_ = -.036;
  double dz_ = .0002;

  double get_k_liq_(const double temp){
    double c1 = 8.92e-3;
    double c2 = 1.474e-5;
    double r =273;
    return c1+c2*(temp-r);
    //return k_;
  }
  double get_k_sol_(const double temp){
    double c1 = 12.3e-3;
    double c2 = 1.472e-5;
    double r =273;
    return c1+c2*(temp-r);
    //return k_;
  }
RES_FUNC(residual_heat_)
{

  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double u = basis[0].uu;
  double uold = basis[0].uuold;

  double ut = rho_*cp_*(u-uold)/dt_*test;
  double divgradu = get_k_liq_(basis[0].uu)*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy + basis[0].dudz*dtestdz);//(grad u,grad phi)
  //double divgradu_old = k_*(basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy + basis[0].duolddz*dtestdz);//(grad u,grad phi)
 
 
  return (ut + t_theta_*divgradu);// /rho_/cp_;// + (1.-t_theta_)*divgradu_old;
}

PRE_FUNC(prec_heat_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = get_k_liq_(basis[0].uu)*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  double u_t =rho_*cp_*test * basis[0].phi[j]/dt_;
  return (u_t + t_theta_*divgrad);// /rho_/cp_;;
}
RES_FUNC(residual_phase_)
{

  //derivatives of the test function
  double dtestdx = basis[1].dphidxi[i]*basis[1].dxidx
    +basis[1].dphideta[i]*basis[1].detadx
    +basis[1].dphidzta[i]*basis[1].dztadx;
  double dtestdy = basis[1].dphidxi[i]*basis[1].dxidy
    +basis[1].dphideta[i]*basis[1].detady
    +basis[1].dphidzta[i]*basis[1].dztady;
  double dtestdz = basis[1].dphidxi[i]*basis[1].dxidz
    +basis[1].dphideta[i]*basis[1].detadz
    +basis[1].dphidzta[i]*basis[1].dztadz;
  //test function
  double test = basis[1].phi[i];
  //u, phi
  double u = basis[1].uu;
  double uold = basis[1].uuold;

  double ut = (u-uold)/dt_*test;
  double divgradu = m_*eps_*eps_*(basis[1].dudx*dtestdx + basis[1].dudy*dtestdy + basis[1].dudz*dtestdz);//(grad u,grad phi)
  double g = 2.*m_*w_*u*(1.-u)*(1.-2.*u);
  double p = 30.*m_*l_*(t_m_-basis[0].uu)/t_m_*u*u*(1.-u)*(1.-u);
 
  return (ut + t_theta_*divgradu+ t_theta_*g + t_theta_*p)/eps_/eps_/m_;
}
PRE_FUNC(prec_phase_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[1].dphidxi[i]*basis[1].dxidx
    +basis[1].dphideta[i]*basis[1].detadx
    +basis[1].dphidzta[i]*basis[1].dztadx;
  double dtestdy = basis[1].dphidxi[i]*basis[1].dxidy
    +basis[1].dphideta[i]*basis[1].detady
    +basis[1].dphidzta[i]*basis[1].dztady;
  double dtestdz = basis[1].dphidxi[i]*basis[1].dxidz
    +basis[1].dphideta[i]*basis[1].detadz
    +basis[1].dphidzta[i]*basis[1].dztadz;

  double dbasisdx = basis[1].dphidxi[j]*basis[1].dxidx
    +basis[1].dphideta[j]*basis[1].detadx
    +basis[1].dphidzta[j]*basis[1].dztadx;
  double dbasisdy = basis[1].dphidxi[j]*basis[1].dxidy
    +basis[1].dphideta[j]*basis[1].detady
    +basis[1].dphidzta[j]*basis[1].dztady;
  double dbasisdz = basis[1].dphidxi[j]*basis[1].dxidz
    +basis[1].dphideta[j]*basis[1].detadz
    +basis[1].dphidzta[j]*basis[1].dztadz;
  double test = basis[1].phi[i];
  double divgrad = m_*eps_*eps_*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  double u_t = test * basis[1].phi[j]/dt_;
  return (u_t + t_theta_*divgrad)/eps_/eps_/m_;
}
INI_FUNC(init_phase_)
{

  double phi_sol_ = 1.;
  double phi_liq_ = 0.;

  double val = phi_liq_;  

  double c = 1.5;//1.1;

  if ( z < zmin_ + c*dz_) {
    val = ((rand() % 100)/50.-1.)*1.5;
    if (val < 0.) val =0.;
    if (val > 1.) val =1.;
  }

  return val;
}

}//namespace truchas

namespace rotate
{
//t0 is rotation in x-y plane (wrt to z axis) only
//there is no derivative coded at this time
//note that to rotate around the x or y axis, is more complicated;
//requiring a rotation matrix in cartesian coordinates to be applied
const double a_sr_(const double px, const double py, const double pz, const double ep, const double t0){
    // in 2d, equivalent to: a_s = 1 + ep cos 4 (theta - t0)
    double nn = sqrt(px*px + py*py + pz*pz );
    double small = 5.e-3;//   => px < 1e-9
    //double small = a_small;
    if( nn < small*small ) return 1. + ep;

    double xx = px/nn;
    double yy = py/nn;
    double zz = pz/nn;
    double aa = .75 + (xx*xx*xx*xx + yy*yy*yy*yy -.75)*cos(4.*t0) + (xx*xx*xx*yy-yy*yy*yy*xx)*sin(4.*t0);

    return 1.-3.*ep + 4.*ep*(aa + zz*zz*zz*zz);
  }
const double da_sr_dpx(const double px, const double py, const double pz, const double ep, const double t0){
  return 0.;
}

}//namespace rotate
  //double a_small =  5.e-3;//   => px < 1e-9

namespace takaki {

  double a_small =  5.e-9;//   => px < 1e-9
  const double umperm_ = 1.e6; //1e6 um = 1 m

  const double ep4_=.02;
  const double w0_ = .9375; //um
  const double T0_ = 931.2; //K
  const double G_ = .2; //K/um
  const double Vp_ = 50.; //um/s
  const double tau0_ =1.;
  const double c0_ = .00245; //atm frac
  const double m_ = -620; //K/atm frac
  const double k_ = .14; //atm frac
  double lT_ = -m_*(1.- k_)*c0_/(k_*G_); //should be um
  double gamma_ = .24e-6/umperm_;// K/m -> K/um
  //double d0_ = k_*gamma_/(-m_*(1.-k_)*c0_);
  double d0_ = 1.3e-2;//um
  const double a1_ = 0.88388;
  double lambdastar_ = a1_*w0_/d0_;
  //const double abar_ = 1.;
  const double Dl_ = (3.e-9)*umperm_*umperm_; //m^2/s -> um^2/s
  //const double Ds_ = (2.e-12)*umperm_*umperm_; //m^2/s -> um^2/s
  const double Ds_ = Dl_;
  double a_ = (1.-k_*Ds_/Dl_)/2./std::sqrt(2.);


double dfdp(const double p){
  return -p + p*p*p;
}
double dgdp(const double p){
  return (1.-p*p)*(1.-p*p);
}
double q(const double p){
  return (k_*Ds_+Dl_+(k_*Ds_-Dl_)*p)/2./Dl_;;
}

  //see anisotropy_new.nb
  //rotation about z axis or x-y plane
const double rpx_z(const double px, const double py, const double pz, const double tz){
  double c = cos(tz);
  double s = sin(tz);
  return c*px + s*py;
}
const double drpx_zdpx(const double px, const double py, const double pz, const double tz){
  return cos(tz);
}
const double rpy_z(const double px, const double py, const double pz, const double tz){
  double c = cos(tz);
  double s = sin(tz);
  return -s*px + c*py;
}
const double drpy_zdpy(const double px, const double py, const double pz, const double tz){
  return cos(tz);
}
const double rpz_z(const double px, const double py, const double pz, const double tz){
  return pz;
}
const double drpz_zdpz(const double px, const double py, const double pz, const double tz){
  return 1.;
}
  //full 3d rotation about z axis, tz, y axis, ty, and x axis, tx
const double rpx_f(const double px, const double py, const double pz, const double tz, const double ty, const double tx){
  double cy = cos(ty);
  double cz = cos(tz);
  double sy = sin(ty);
  double sz = sin(tz);
  return cy*cz*px + cy*sz*py + sy*pz;
}
const double drpx_fdpx(const double px, const double py, const double pz, const double tz, const double ty, const double tx){
  return rpx_f(1.,0.,0.,tz,ty,tx);
}
const double rpy_f(const double px, const double py, const double pz, const double tz, const double ty, const double tx){
  double cx = cos(tx);
  double cy = cos(ty);
  double cz = cos(tz);
  double sx = sin(tx);
  double sy = sin(ty);
  double sz = sin(tz);
  return (-cz*sx*sy - cx*sz)*px + (cx*cz-sx*sy*sz)*py + cy*sx*pz;
}
const double drpy_fdpy(const double px, const double py, const double pz, const double tz, const double ty, const double tx){
  return rpy_f(0.,1.,0.,tz,ty,tx);
}
const double rpz_f(const double px, const double py, const double pz, const double tz, const double ty, const double tx){
  double cx = cos(tx);
  double cy = cos(ty);
  double cz = cos(tz);
  double sx = sin(tx);
  double sy = sin(ty);
  double sz = sin(tz);
  return (-cx*cz*sy + sx*sz)*px + (-cz*sx - cx*sy*sz)*py  + cx*cy*pz;
}
const double drpz_fdpz(const double px, const double py, const double pz, const double tz, const double ty, const double tx){
  return rpz_f(0.,0.,1.,tz,ty,tx);
}




const double a_s_(const double px, const double py, const double pz, const double ep){
    // in 2d, equivalent to: a_s = 1 + ep cos 4 theta
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 1. + ep;
    //return 1.-3.*ep + 4.*ep*(px2*px2 + py2*py2 + pz2*pz2)/norm4;
    return 1.;
  }
const double da_s_dpx(const double px, const double py, const double pz, const double ep){
    // (16 ep x (-y^4 - z^4 + x^2 y^2 + x^2 z^2))/(x^2 + y^2 + z^2)^3
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    double norm6 = norm4*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 0.;
    //return 16.*ep*px*(- py2*py2 - pz2*pz2 + px2*py2 + px2*pz2)/norm6;
    return 0.;
  }
const double da_s_dpy(const double px, const double py, const double pz, const double ep){
    // -((16 ep y (x^4 - x^2 y^2 - y^2 z^2 + z^4))/(x^2 + y^2 + z^2)^3)
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    double norm6 = norm4*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 0.;
    //return - 16.*ep*py*(px2*px2 + pz2*pz2 - px2*py2 - py2*pz2)/norm6;
    return 0.;
  }
const double da_s_dpz(const double px, const double py, const double pz, const double ep){
    //-((16 ep z (x^4 + y^4 - x^2 z^2 - y^2 z^2))/(x^2 + y^2 + z^2)^3)
    double px2 = px*px;
    double py2 = py*py;
    double pz2 = pz*pz;
    double norm4 = (px2 + py2 + pz2 )*(px2 + py2 + pz2 );
    double norm6 = norm4*(px2 + py2 + pz2 );
    //double small = 5.e-3;//   => px < 1e-9
    if( norm4 < a_small ) return 0.;
    //return - 16.*ep*pz*(px2*px2 + py2*py2 - px2*pz2 - py2*pz2)/norm6;
    return 0.;
  }

const double w_(const double px, const double py, const double pz, const double ep){
  double a2 = a_s_(px, py, pz, ep);
  return w0_*a2*a2;
}
const double dw_dpx(const double px, const double py, const double pz, const double ep){
  return 2.*w0_*a_s_(px, py, pz, ep)*da_s_dpx(px, py, pz, ep);
}
const double dw_dpy(const double px, const double py, const double pz, const double ep){
  return 2.*w0_*a_s_(px, py, pz, ep)*da_s_dpy(px, py, pz, ep);
}
const double dw_dpz(const double px, const double py, const double pz, const double ep){
  return 2.*w0_*a_s_(px, py, pz, ep)*da_s_dpz(px, py, pz, ep);
}

double temp(const double time, const double y){
  return  T0_ + G_*(y-Vp_*time);
}

double uprime(const double time, const double y){
  return  (y-Vp_*time)/lT_;
}

const double tau_(const double px, const double py, const double pz, const double ep){
  double a2 = a_s_(px, py, pz, ep);
  return tau0_*a2*a2;
}

INI_FUNC(init_conc_)
{
  //return c0_;//cn should be u0
  return 0.;
}

INI_FUNC(init_phase_)
{
  double dx = .75;
  double lfo = 230.*dx;
  double luo = 300.*dx; 
  double val = -1.;
  double r2 = x*x + y*y;

  double rr2 =.75*.75*dx*dx;

  if(r2 < rr2 ){
    val=1.;
  }

  r2 = (x - lfo)*(x - lfo) + y*y;

  if(r2 < rr2 ){
    val=1.;
  }
  r2 = (x - (lfo+luo))*(x - (lfo+luo)) + y*y;

  if(r2 < rr2 ){
    val=1.;
  }

  return val;
}

RES_FUNC(residual_conc_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;
  double u = basis[0].uuold;
  //double uold = basis[0].uuold;
  double uold = basis[0].uuoldold;
  //double phi = basis[1].uu;
  double phi = basis[1].uuold;
  //double phiold = basis[1].uuold;
  double phiold = basis[1].uuoldold;
  //double dphidx = basis[1].dudx;
  double dphidx = basis[1].duolddx;
  //double dphidy = basis[1].dudy;
  double dphidy = basis[1].duolddy;

  double dtc = (1.+k_-(1.-k_)*phi)/2.;
  //double ut = dtc*(u-uold)/dt_*test;
  double ut = dtc*(basis[0].uu-basis[0].uuold)/dt_*test;
  //ut = (u-uold)/dt_*test;
  double divgradu = Dl_*q(phi)*(basis[0].dudx*dtestdx + basis[0].dudy*dtestdy);//(grad u,grad phi)
  //double divgradu_old = D*(1.-phiold)/2*(basis[0].duolddx*dtestdx + basis[0].duolddy*dtestdy);//(grad u,grad phi)

  //j is antitrapping current
  // j grad test here... j1*dtestdx + j2*dtestdy 
  // what if dphidx*dphidx + dphidy*dphidy = 0?

  double norm = sqrt(dphidx*dphidx + dphidy*dphidy);
  double small = 1.e-12;
  
  double j_coef = 0.;
  if (small < norm) {
    j_coef = a_*w0_*(1.+(1.-k_)*u)/norm*(phi-phiold)/dt_;
  } 
  //j_coef = 0.;
  double j1 = j_coef*dphidx;
  double j2 = j_coef*dphidy;
  double divj = j1*dtestdx + j2*dtestdy;
  double phitu = -.5*(phi-phiold)/dt_*(1.+(1.-k_)*u)*test; 
  //phitu = 1.*test; 
//   h = hold;
//   hold = basis[1].uuoldold*(1. + (1.-k_)*basis[0].uuoldold);
//   double phitu_old = -.5*(h-hold)/dt_*test;
 
  //return ut*0.  + t_theta_*(divgradu + divj*0.) + (1.-t_theta_)*(divgradu_old + divj_old)*0. + t_theta_*phitu*0. + (1.-t_theta_)*phitu_old*0.;

  //std::cout<<"u: "<<ut<<" "<< t_theta_*divgradu  <<" "<< 0.*t_theta_*divj <<" "<< t_theta_*phitu<<"  :  "<<dtc<<std::endl;
  return (ut + t_theta_*divgradu  + 0.*t_theta_*divj + t_theta_*phitu)/dtc*dt_;
}
RES_FUNC(residual_phase_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;
  //test function
  double test = basis[0].phi[i];
  //u, phi
  //double u = basis[0].uu;
  double u = basis[0].uuold;
  //double uold = basis[0].uuold;
  double uold = basis[0].uuoldold;
  //double phi = basis[1].uu;
  double phi = basis[1].uuold;
  //double phiold = basis[1].uuold;
  double phiold = basis[1].uuoldold;

  //double dphidx = basis[1].dudx;
  double dphidx = basis[1].duolddx;
  //double dphidy = basis[1].dudy;
  double dphidy = basis[1].duolddy;

  double y = basis[0].yy;
  double up = uprime(y,time);
  double dtc = tau_(dphidx,dphidy,0.,ep4_)*(1.-(1.-k_)*up);
  //double phit = dtc*(phi-phiold)/dt_*test;
  double phit = dtc*(basis[1].uu-basis[1].uuold)/dt_*test;

  double w = w_(dphidx,dphidy,0.,ep4_);
  double divgrad = w*w*(dphidx*dtestdx + dphidy*dtestdy);//(grad u,grad phi)

  double norm2 = dphidx*dphidx + dphidy*dphidy;
  double curlgrad = w*norm2*(dw_dpx(dphidx,dphidy,0.,ep4_)*dtestdx + dw_dpy(dphidx,dphidy,0.,ep4_)*dtestdy);

  double df = dfdp(phi)*test;
  double dg = lambdastar_*dgdp(phi)*(u + up)*test; 

  //std::cout<<"phi: "<<phit <<" "<< divgrad <<" "<< curlgrad <<" "<< df <<" "<<dg<<" "<<d0_<<"  :  "<<dtc<<std::endl;
  return (phit + t_theta_*(divgrad + curlgrad + df +dg))/dtc*dt_;
}


}//namespace takaki

namespace allencahn {


double kappa = 0.0004;
const double pi = 3.141592653589793;
const double A1 = 0.0075;
const double B1 = 8.0*pi;
const double A2 = 0.03;
const double B2 = 22.0*pi;
const double C2 = 0.0625*pi;



PARAM_FUNC(param_)
{
  //kappa = plist->get<double>("kappa");
}

double alpha(double t, double x) {

   double alpha_result;
   alpha_result = A1*t*sin(B1*x) + A2*sin(B2*x + C2*t) + 0.25;
   return alpha_result;

}

double eta(double a, double y) {

   double eta_result;
   eta_result = -1.0L/2.0L*tanh((1.0L/2.0L)*sqrt(2)*(-a + y)/sqrt(kappa)) + 1.0L/2.0L;
   return eta_result;

}

double eta0(double a, double y) {

   double eta0_result;
   eta0_result = -1.0L/2.0L*tanh((1.0L/2.0L)*sqrt(2)*(-a + y)/sqrt(kappa)) + 1.0L/2.0L;
   return eta0_result;

}

double dadt(double t, double x) {

   double dadt_result;
   dadt_result = A1*sin(B1*x) + A2*C2*cos(B2*x + C2*t);
   return dadt_result;

}

double dadx(double t, double x) {

   double dadx_result;
   dadx_result = A1*B1*t*cos(B1*x) + A2*B2*cos(B2*x + C2*t);
   return dadx_result;

}

double d2adx2(double t, double x) {

   double d2adx2_result;
   d2adx2_result = -A1*pow(B1, 2)*t*sin(B1*x) - A2*pow(B2, 2)*sin(B2*x + C2*t);
   return d2adx2_result;

}

double source(double a, double t, double x, double y) {

   double source_result;
   source_result = (1.0L/4.0L)*(-pow(tanh((1.0L/2.0L)*sqrt(2)*(-a + y)/sqrt(kappa)), 2) + 1)*(-2*sqrt(kappa)*pow(A1*B1*t*cos(B1*x) + A2*B2*cos(B2*x + C2*t), 2)*tanh((1.0L/2.0L)*sqrt(2)*(-a + y)/sqrt(kappa)) - sqrt(2)*kappa*(-A1*pow(B1, 2)*t*sin(B1*x) - A2*pow(B2, 2)*sin(B2*x + C2*t)) + sqrt(2)*(A1*sin(B1*x) + A2*C2*cos(B2*x + C2*t)))/sqrt(kappa);
   return source_result;

}
RES_FUNC(residual_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  //test function
  double test = basis[0].phi[i];

  double u[2] = {basis[0].uu, basis[0].uuold};
  double u_x[2] = {basis[0].dudx, basis[0].duolddx};
  double u_y[2] = {basis[0].dudy, basis[0].duolddy};

  double ut = (u[0]-u[1])/dt_*test;

  double divgrad = t_theta_*kappa*(u_x[0]*dtestdx + u_y[0]*dtestdy)
             +(1.-t_theta_)*kappa*(u_x[1]*dtestdx + u_y[1]*dtestdy);
  double fp = (-t_theta_*4.*u[0]*(u[0]-1.)*(u[0]-.5)
	  -(1.-t_theta_)*4.*u[1]*(u[1]-1.)*(u[1]-.5))*test;
  
  double x = basis[0].xx;
  double y = basis[0].yy;
  double t[2] = {time, time - dt_};
  double a[2] = {alpha(t[0],x),alpha(t[1],x)};
  double s = (-t_theta_*source(a[0],t[0],x,y)
         -(1.-t_theta_)*source(a[1],t[1],x,y))*test;

  return ut + divgrad + fp + s;
}
PRE_FUNC(prec_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  
  double u_t = test * basis[0].phi[j]/dt_;

  double divgrad = t_theta_*kappa*(dbasisdx * dtestdx + dbasisdy * dtestdy);

  return (u_t + divgrad);
}
INI_FUNC(init_)
{
  double a = alpha(0.,x);
  double val = eta0(a,y);
  return val;
}
PPR_FUNC(postproc_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double x = xyz[0];
  double y = xyz[1];
  double a = alpha(time,x);
  double val = eta(a,y);

  return val;
}
PPR_FUNC(postproc_error)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  // x is in nondimensional space, tscale_ takes in nondimensional and converts to um
  double x = xyz[0];
  double y = xyz[1];
  double a = alpha(time,x);
  double val = (u[0]-eta(a,y));
  //double val = (u[0]-eta(a,y))*(u[0]-eta(a,y));

  return val;
}

}//namespace allencahn
namespace pfhub2 {

  int N_ = 1;
  int eqn_off_ =1;
  const double c0_ = .5;
  const double eps_ = .05;
  const double eps_eta_ = .1;
  const double psi_ = 1.5;
  const double rho_ = std::sqrt(2.);
  const double c_alpha_ = .3;
  const double c_beta_ = .7;
  const double alpha_ = 5.;
  const double k_c_ = 0.;//3.
  const double k_eta_ = 3.;
  const double M_ = 5.;
  const double L_ = 5.;
  const double w_ = 1.;
  double c_a[2] = {0., 0.};
  double c_b[2] = {0., 0.};

  PARAM_FUNC(param_)
  {
    N_ = plist->get<int>("N");
    eqn_off_ = plist->get<int>("OFFSET");
  }

  double f_alpha(const double c){
    return rho_*rho_*(c - c_alpha_)*(c - c_alpha_);
  }

  double df_alphadc(const double c){
    return 2.*rho_*rho_*(c - c_alpha_);
  }

  double f_beta(const double c){
    return rho_*rho_*(c_beta_ - c)*(c_beta_ - c);
  }

  double df_betadc(const double c){
    return -2.*rho_*rho_*(c_beta_ - c);
  }

  double d2fdc2(){
    return 2.*rho_*rho_;
  }

  double h(const double *eta){
    double val = 0.;
    for (int i = 0; i < N_; i++){
      val += eta[i]*eta[i]*eta[i]*(6.*eta[i]*eta[i] - 15.*eta[i] + 10.);
    }
    return val;
  }

  //double dhdeta(const double *eta, const int eqn_id){
  double dhdeta(const double eta){

    //return 30.*eta[eqn_id]*eta[eqn_id] - 60.*eta[eqn_id]*eta[eqn_id]*eta[eqn_id] + 30.*eta[eqn_id]*eta[eqn_id]*eta[eqn_id]*eta[eqn_id];
    return 30.*eta*eta - 60.*eta*eta*eta + 30.*eta*eta*eta*eta;
  }

//   double d2hdeta2(const double *eta, const int eqn_id){

//     return 60.*eta[eqn_id] - 180.*eta[eqn_id]*eta[eqn_id] + 120.*eta[eqn_id]*eta[eqn_id]*eta[eqn_id];
//   }

//   double g(const double *eta){

//     double aval =0.;
//     for (int i = 0; i < N_; i++){
//       aval += eta[i]*eta[i];
//     }
    
//     double val = 0.;
//     for (int i = 0; i < N_; i++){
//       val += eta[i]*eta[i]*(1.-eta[i])*(1.-eta[i]) + alpha_*eta[i]*eta[i]*aval 
// 	- alpha_*eta[i]*eta[i]*eta[i]*eta[i];
//     }
//     return val;
//   }

  double dgdeta(const double *eta, const int eqn_id){

    double aval =0.;
    for (int i = 0; i < N_; i++){
      aval += eta[i]*eta[i];
    }
    aval = aval - eta[eqn_id]* eta[eqn_id];
    return 2.*eta[eqn_id]*(1. - eta[eqn_id])*(1. - eta[eqn_id])  
      - 2.* eta[eqn_id]* eta[eqn_id]* (1. - eta[eqn_id])
      + 4.*alpha_*eta[eqn_id] *aval;
  }

//   double d2gdeta2(const double *eta, const int eqn_id){

//     double aval =0.;
//     for (int i = 0; i < N_; i++){
//       aval += eta[i]*eta[i];
//     }
//     return 2. - 12.*eta[eqn_id] + 12.*eta[eqn_id]*eta[eqn_id] + alpha_*4.*aval - 4.*alpha_*eta[eqn_id]*eta[eqn_id];
//   }



  //void solve_kks(double &c_a, double &c_b, const double c, double phi)//const double phi
  void solve_kks(const double c, double *phi)//const double phi
  {
    double delta_c_a = 0.;
    double delta_c_b = 0.;
    const int max_iter = 20;
    const double tol = 1.e-8;
    const double hh = h(phi);
    //c_a[0] = (1.-hh)*c;
    c_a[0] = c - hh*(c_beta_ - c_alpha_);

    //c_b[0]=hh*c;
    c_b[0]= c - (1.-hh)*(c_beta_ - c_alpha_);

    //std::cout<<"-1"<<" "<<delta_c_b<<" "<<delta_c_a<<" "<<c_b[0]<<" "<<c_a[0]<<" "<<hh*c_b[0] + (1.- hh)*c_a[0]<<" "<<c<<std::endl;
    for(int i = 0; i < max_iter; i++){
      double det = hh*d2fdc2() + (1.-hh)*d2fdc2();//-fa''*h+(1-h)*fb''
      double f1 = hh*c_b[0] + (1.- hh)*c_a[0] - c;
      double f2 = df_betadc(c_b[0]) - df_alphadc(c_a[0]);
      delta_c_b = (-d2fdc2()*f1 - (1-hh)*f2)/det;//-fa''f*1-(1-h)*f2
      delta_c_a = (-d2fdc2()*f1 + hh*f2)/det;//-fb''*f1+h*f2
      c_b[0] = delta_c_b + c_b[0];
      c_a[0] = delta_c_a + c_a[0];
      //std::cout<<i<<" "<<delta_c_b<<" "<<delta_c_a<<" "<<c_b[0]<<" "<<c_a[0]<<" "<<hh*c_b[0] + (1.- hh)*c_a[0]<<" "<<c<<std::endl;
      if(delta_c_a*delta_c_a+delta_c_b*delta_c_b < tol*tol) return;
    }
    std::cout<<"###################################  solve_kks falied to converge with delta_c_a*delta_c_a+delta_c_b*delta_c_b = "
	     <<delta_c_a*delta_c_a+delta_c_b*delta_c_b<<"  ###################################"<<std::endl;
    //if(delta_c_a*delta_c_a+delta_c_b*delta_c_b > 0) exit(0);
    exit(0);
    return;
  }

  void solve_kks_exact(const double c, double *phi)//const double phi
  {
    const double hh = h(phi);
    //c_a[0] = (1.-hh)*c;
    c_a[0] = (-c +(c_beta_ + c_alpha_)*hh)/(-1.+2.*hh);

    //c_b[0]=hh*c;
    c_b[0]= (c +(c_beta_ + c_alpha_)*(hh-1.))/(-1.+2.*hh);

    return;
  }

  void solve_kks(const double c, double *phi, double &ca, double &cb)//const double phi
  {
    double delta_c_a = 0.;
    double delta_c_b = 0.;
    const int max_iter = 20;
    const double tol = 1.e-8;
    const double hh = h(phi);
    //c_a[0] = (1.-hh)*c;
    ca = 0.;//c - hh*(c_beta_ - c_alpha_);

    //c_b[0]=hh*c;
    cb = 0.;//c - (1.-hh)*(c_beta_ - c_alpha_);

    //solve:
    // f1 = hh*cb + (1.- hh)*ca - c = 0
    // f2 = df_betadc(cb) - df_alphadc(ca) = 0

    // F'*delta_c = - F
    // delta_c = F'^-1 *(-F)
    // delta_c = cnew - cold
    // c = [cb ca]^T
    // F'= [ h            1-h ]
    //     [d2fdc2() -d2fdc2()]
    // F'^-1 = 1/det [-d2fdc2() -(1-h)]
    //               [-d2fdc2()    h  ]
    // det = -h d2fdc2() -(1-h)d2fdc2() = -d2fdc2()

    for(int i = 0; i < max_iter; i++){
      //const double det = hh*d2fdc2() + (1.-hh)*d2fdc2();
      const double det = -d2fdc2();
      const double f1 = hh*cb + (1.- hh)*ca - c;
      const double f2 = df_betadc(cb) - df_alphadc(ca);
      delta_c_b = (-d2fdc2()*(-f1) - (1-hh)*(-f2))/det;
      delta_c_a = (-d2fdc2()*(-f1) + hh*(-f2))/det;
      cb = delta_c_b + cb;
      ca = delta_c_a + ca;
      //std::cout<<i<<" "<<delta_c_b<<" "<<delta_c_a<<" "<<c_b[0]<<" "<<c_a[0]<<" "<<hh*c_b[0] + (1.- hh)*c_a[0]<<" "<<c<<std::endl;
      if(delta_c_a*delta_c_a+delta_c_b*delta_c_b < tol*tol) return;
    }
//     std::cout<<"###################################  solve_kks falied to converge with delta_c_a*delta_c_a+delta_c_b*delta_c_b = "
// 	     <<delta_c_a*delta_c_a+delta_c_b*delta_c_b<<"  ###################################"<<std::endl;
    exit(0);
    return;
  }

RES_FUNC(residual_c_)
{

  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  //double dtestdz = basis[0].dphidz[i];
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double c[2] = {basis[0].uu, basis[0].uuold};
  double dcdx[2] = {basis[0].dudx, basis[0].duolddx};
  double dcdy[2] = {basis[0].dudy, basis[0].duolddy};

  double dhdx[2] = {0., 0.};
  double dhdy[2] = {0., 0.};

  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    dhdx[0] += dhdeta(basis[kk_off].uu)*basis[kk_off].dudx;
    dhdx[1] += dhdeta(basis[kk_off].uuold)*basis[kk_off].duolddx;
    dhdy[0] += dhdeta(basis[kk_off].uu)*basis[kk_off].dudy;
    dhdy[1] += dhdeta(basis[kk_off].uuold)*basis[kk_off].duolddy;
  }

  double ct = (c[0]-c[1])/dt_*test;

  double DfDc[2] = {df_betadc(c[0])-df_alphadc(c[0]),
		    df_betadc(c[1])-df_alphadc(c[1])};

  double D2fDc2 = d2fdc2();

  double dfdx[2] = {DfDc[0]*dhdx[0] + D2fDc2*dcdx[0],
		    DfDc[1]*dhdx[1] + D2fDc2*dcdx[1]};
  double dfdy[2] = {DfDc[0]*dhdy[0] + D2fDc2*dcdy[0],
		    DfDc[1]*dhdy[1] + D2fDc2*dcdy[1]};

  double divgradc[2] = {M_*(dfdx[0]*dtestdx + dfdy[0]*dtestdy),
			M_*(dfdx[1]*dtestdx + dfdy[1]*dtestdy)};

  return ct + t_theta_*divgradc[0] + (1.-t_theta_)*divgradc[1];
}

PRE_FUNC(prec_c_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  double dtestdz = basis[0].dphidz[i];

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz;
  double u_t =test * basis[0].phi[j]/dt_;
  double D2fDc2 = d2fdc2();
  return u_t + t_theta_*M_*D2fDc2*divgrad;
}


RES_FUNC(residual_eta_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  //double dtestdz = basis[0].dphidz[i];
  //test function
  double test = basis[eqn_id].phi[i];
  //u, phi
  double c[2] = {basis[0].uu, basis[0].uuold};

  double eta[2] = {basis[eqn_id].uu, basis[eqn_id].uuold};
  double detadx[2] = {basis[eqn_id].dudx, basis[eqn_id].duolddx};
  double detady[2] = {basis[eqn_id].dudy, basis[eqn_id].duolddy};

  double eta_array[N_];
  double eta_array_old[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off].uu;
    eta_array_old[kk] = basis[kk_off].uuold;
  }

  double etat = (eta[0]-eta[1])/dt_*test;

  double F[2] = {f_beta(c[0])-f_alpha(c[0]),
		 f_beta(c[1])-f_alpha(c[1])};

  int k = eqn_id - eqn_off_;
  double dfdeta[2] = {L_*(F[0]*dhdeta(eta[0]) + w_*dgdeta(eta_array,k))*test,
		      L_*(F[1]*dhdeta(eta[1]) + w_*dgdeta(eta_array_old,k))*test};

  double divgradeta[2] = {L_*k_eta_*(detadx[0]*dtestdx + detady[0]*dtestdy), 
			  L_*k_eta_*(detadx[1]*dtestdx + detady[1]*dtestdy)};//(grad u,grad phi)
 
  return etat + t_theta_*divgradeta[0] + t_theta_*dfdeta[0] + (1.-t_theta_)*divgradeta[1] + (1.-t_theta_)*dfdeta[1];
}

PRE_FUNC(prec_eta_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  double dtestdz = basis[0].dphidz[i];

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;
  double test = basis[0].phi[i];
  double divgrad = dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz;
  double u_t =test * basis[0].phi[j]/dt_;

  const double eta = basis[eqn_id].uu;
  const double g1 = (2. - 12.*eta + 12.*eta*eta)*basis[eqn_id].phi[j]*test;
  return u_t + t_theta_*L_*k_eta_*divgrad + t_theta_*L_*g1;
}

RES_FUNC(residual_c_kks_)
{
  //derivatives of the test function
  double dtestdx = basis[0].dphidx[i];
  double dtestdy = basis[0].dphidy[i];
  //double dtestdz = basis[0].dphidz[i];
  //test function
  double test = basis[0].phi[i];
  //u, phi
  double c[2] = {basis[0].uu, basis[0].uuold};
  double dcdx[2] = {basis[0].dudx, basis[0].duolddx};
  double dcdy[2] = {basis[0].dudy, basis[0].duolddy};

  double dhdx[2] = {0., 0.};
  double dhdy[2] = {0., 0.};

  double eta_array[N_];
  double eta_array_old[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    dhdx[0] += dhdeta(basis[kk_off].uu)*basis[kk_off].dudx;
    dhdx[1] += dhdeta(basis[kk_off].uuold)*basis[kk_off].duolddx;
    dhdy[0] += dhdeta(basis[kk_off].uu)*basis[kk_off].dudy;
    dhdy[1] += dhdeta(basis[kk_off].uuold)*basis[kk_off].duolddy;

    eta_array[kk] = basis[kk_off].uu;
    eta_array_old[kk] = basis[kk_off].uuold;
  }

  double ct = (c[0]-c[1])/dt_*test;
  double ca,cb;
  solve_kks(c[0],eta_array,ca,cb);

  //kks paper
  double dfdx[2] = {dcdx[0] + dhdeta(eta_array[0])*(ca-cb)*basis[1].dudx,0.};
  double dfdy[2] = {dcdy[0] + dhdeta(eta_array[0])*(ca-cb)*basis[1].dudy,0.};

  double divgradc[2] = {M_*(dfdx[0]*dtestdx + dfdy[0]*dtestdy),
			M_*(dfdx[1]*dtestdx + dfdy[1]*dtestdy)};

  return ct + t_theta_*divgradc[0] + 0.*(1.-t_theta_)*divgradc[1];
}

RES_FUNC(residual_eta_kks_)
{

  //derivatives of the test function
  double dtestdx = basis[eqn_id].dphidx[i];
  double dtestdy = basis[eqn_id].dphidy[i];
  //double dtestdz = basis[0].dphidz[i];
  //test function
  double test = basis[eqn_id].phi[i];
  //u, phi
  double c[2] = {basis[0].uu, basis[0].uuold};

  double eta[2] = {basis[eqn_id].uu, basis[eqn_id].uuold};
  double detadx[2] = {basis[eqn_id].dudx, basis[eqn_id].duolddx};
  double detady[2] = {basis[eqn_id].dudy, basis[eqn_id].duolddy};

  double eta_array[N_];
  double eta_array_old[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off].uu;
    eta_array_old[kk] = basis[kk_off].uuold;
  }

  double ca,cb;
  solve_kks(c[0],eta_array,ca,cb);

  double etat = (eta[0]-eta[1])/dt_*test;


  double F[2] = {f_beta(cb)-f_alpha(ca) - (cb - ca)*df_alphadc(ca),
		 f_beta(c_b[1])-f_alpha(c_a[1]) - (c_b[1] - c_a[1])*df_betadc(c_b[1])};

  const int k = eqn_id - eqn_off_;
  double dfdeta[2] = {L_*(F[0]*dhdeta(eta_array[0])     + w_*dgdeta(eta_array,k)    )*test,
		      L_*(F[1]*dhdeta(eta_array_old[0]) + w_*dgdeta(eta_array_old,k))*test};

  double divgradeta[2] = {L_*k_eta_*(detadx[0]*dtestdx + detady[0]*dtestdy), 
			  L_*k_eta_*(detadx[1]*dtestdx + detady[1]*dtestdy)};//(grad u,grad phi)
 
  return etat + t_theta_*divgradeta[0] + t_theta_*dfdeta[0] + 0.*(1.-t_theta_)*divgradeta[1] + 0.*(1.-t_theta_)*dfdeta[1];
}

INI_FUNC(init_c_)
{
  return c0_ + eps_*(cos(0.105*x)*cos(0.11*y)
		     + cos(0.13*x)*cos(0.087*y)*cos(0.13*x)*cos(0.087*y)
		     + cos(0.025*x-0.15*y)*cos(0.07*x-0.02*y)
		     );
}

INI_FUNC(init_eta_)
{
  const double i = (double)(eqn_id - eqn_off_ + 1);
  return eps_eta_*std::pow(cos((0.01*i)*x-4.)*cos((0.007+0.01*i)*y)
			   + cos((0.11+0.01*i)*x)*cos((0.11+0.01*i)*y)		   
			   + psi_*std::pow(cos((0.046+0.001*i)*x+(0.0405+0.001*i)*y)
					   *cos((0.031+0.001*i)*x-(0.004+0.001*i)*y),2
					   ),2
			   );
}

RES_FUNC(residual_c_alpha_g_)
{
  //globally coupled global kks equation
  double test = basis[eqn_id].phi[i];
  double c = basis[0].uu;
  double ca = basis[1].uu;
  double cb = basis[2].uu;
  double eta_array[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off].uu;
  }

  return (h(eta_array)*cb + (1.- h(eta_array))*ca - c)*test;
}

RES_FUNC(residual_c_beta_g_)
{
  //globally coupled global kks equation
  double test = basis[eqn_id].phi[i];
  double ca = basis[1].uu;
  double cb = basis[2].uu;
  return (df_betadc(cb) - df_alphadc(ca))*test;
}

PRE_FUNC(prec_c_alpha_)
{
  double test = basis[0].phi[i];
  double eta_array[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off].uu;
  }
  return (1.-h(eta_array)) * basis[eqn_id].phi[j]*test;
  //return basis[eqn_id].phi[j]*test;
}

RES_FUNC(residual_c_alpha_l_)
{
  //globally coupled local kks equation
  double test = basis[eqn_id].phi[i];
  double cc = basis[0].uu;
  double ca = basis[1].uu;
  //double cb = basis[2].uu;
  double eta_array[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off].uu;
  }


  solve_kks(cc,eta_array);
  //solve_kks_exact(cc,eta_array);
  const double val = ca-c_a[0];

//   const double hh = h(eta_array);
//   const double val = (-1.+2.*hh)*ca-(-cc +(c_beta_ + c_alpha_)*hh);
  return val*test;
}

RES_FUNC(residual_c_beta_l_)
{
  //globally coupled local kks equation
  double test = basis[eqn_id].phi[i];
  double cc = basis[0].uu;
  //double ca = basis[1].uu;
  double cb = basis[2].uu;
  double eta_array[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off].uu;
  }

  solve_kks(cc,eta_array);
  //solve_kks_exact(cc,eta_array);
  const double val = cb-c_b[0];

//   const double hh = h(eta_array);
//   const double val = (-1.+2.*hh)*cb-(cc +(c_beta_ + c_alpha_)*(hh-1.));
//   std::cout<<val<<" "<<val*test<<std::endl;
  return val*test;
}

PRE_FUNC(prec_c_beta_)
{
  double test = basis[eqn_id].phi[i];
  double c_b = basis[eqn_id].phi[j];
  //return df_betadc(c_b)*c_b*test;
  return c_b*test;
 }

INI_FUNC(init_c_alpha_)
{
   double c = init_c_(x,y,z,0);

  double eta_array[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = init_eta_(x,y,z,kk_off);
  }

  return c-h(eta_array)*(c_beta_ - c_alpha_);
}

INI_FUNC(init_c_beta_)
{
  double c = init_c_(x,y,z,0);

  double eta_array[N_];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = init_eta_(x,y,z,kk_off);
  }

  return c+(1-h(eta_array))*(c_beta_ - c_alpha_);
}

PPR_FUNC(postproc_c_b_)
{
  //cn will need eta_array here...
  double cc = u[0];
  double phi = u[eqn_off_];

  solve_kks(cc,&phi);
  //solve_kks_exact(cc,&phi);

  return c_b[0];
}
PPR_FUNC(postproc_c_a_)
{

  //cn will need eta_array here...
  double cc = u[0];
  double phi = u[eqn_off_];

  solve_kks(cc,&phi);
  //solve_kks_exact(cc,&phi);

  return c_a[0];
}

PPR_FUNC(postproc_h_)
{

  //cn will need eta_array here...
  //double cc = u[0];
  double phi = u[eqn_off_];

  return h(&phi);
}

PPR_FUNC(postproc_c_)
{

  //cn will need eta_array here...
  double cc = u[0];
  double phi = u[eqn_off_];

  solve_kks(cc,&phi);
  //solve_kks_exact(cc,&phi);

  return (1.-h(&phi))*c_a[0]+h(&phi)*c_b[0];
}

}//namespace pfhub2

namespace cerium{
  //https://iopscience.iop.org/article/10.1088/2053-1591/ac1c32
  //units are in J, mm, s
  //note that 10^6 kJ/m^3 in figure 2 is J/mm^3
  //free energy density f_m = f_p(c) = = 4 (c - .5)^2 - 1
  //f_m'(c) = 8 (c - 1/2)
  //        = 2 am (c - bm)
  //f_h(c) = 30 (c - 2/3)^2 - 8
  //f_h'(c) = 60 (c - 2/3)
  //        = 2 ah (c - bh)
  //assume ordering is p, h, m; c; displacement
  const double am = 4.;// J/mm^3
  const double bm = .5;// mole fraction H
  const double cm = 1.;
  const double ah = 30.;// J/mm^3
  const double bh = 2./3.;// mole fraction H
  const double ch = 8.;
  const double m = 4.6e-3;// mm^3/J/s
//   const double M[3][3] = {
//     {0.,m,m},
//     {m,0.,m},
//     {m,m,0.}
//   };
  const double es = 3.9e-5;// J/mm
//   const double e2[3][3] = {
//     {0.,es,es},
//     {es,0.,es},
//     {es,es,0}
//   };
  const double h = 1.85e2;// J/mm^3
//   const double H[3][3] = {
//     {0.,h,h},
//     {h,0.,h},
//     {h,h,0.}
//   };
  const double D[3] = {1.e-5,3.5e-5,3.5e-5};// mm^2/s
  const double e0 = .2;
  const double E = 3.4e3;// J/mm^3
  const double nu = .24;
  double dG[3][3] = {
    {0.,0.,0.},
    {0.,0.,0.},
    {0.,0.,0.}
  };

  //solve eqn 5
  void solve_kks(const double cc, //c
		 const double pp, //phi_p
		 const double ph, //phi_h 
		 const double pm, //phi_m
		 double &cp, 
		 double &ch, 
		 double &cm)
  {
    const double d = am*ph + ah*pm + ah*pp;
    cp = -(-ah*cc + ah*bh*ph - am*bm*ph)/d;
    ch = -(-am*cc - ah*bh*pm + am*bm*pm - ah*bh*pp + am*bm*pp)/d;
    cm = -(-ah*cc + ah*bh*ph - am*bm*ph)/d;
  }

  const double fm(const int c)
  {
    return am*(c-bm)*(c-bm)-cm;
  }

  const double fp(const int c)
  {
    return fm(c);
  }

  const double fh(const int c)
  {
    return ah*(c-bh)*(c-bh)-ch;
  }

  const double dfm(const int c)
  {
    return 2.*am*(c-bm);
  }

  const double dfp(const int c)
  {
    return dfm(c);
  }

  const double dfh(const int c)
  {
    return 2.*ah*(c-bh);
  }
  //g10 = ghp = -gph
  const double g10(const double ch,const double cp)
  {
    return fh(ch) - fp(cp) - .5*(dfh(ch) + dfp(cp))*(ch - cp);
  }
  //g20 = gmp = - gpm
  const double g20(const double cm, const double cp)
  {
    return fm(cm) - fp(cp) - .5*(dfm(cm) + dfp(cp))*(cm - cp);
  }
  //g21 = gmh = -ghm
  const double g21(const double cm, const double ch)
  {
    return fm(cm) - fh(ch) - .5*(dfm(cm) + dfh(ch))*(cm - ch);
  }

  //note that there is a difference in the lapace terms by a factor of 2 
  //depending on summation; see cerium.nb
RES_FUNC(residual_p_)
{
  const double ut = (basis[eqn_id].uu-basis[eqn_id].uuold)/dt_*basis[eqn_id].phi[i];
  const double p[3] = {basis[0].uu, basis[1].uu, basis[2].uu};
  const double lp[3] = {-(basis[0].dudx*basis[eqn_id].dphidx[i]
			    + basis[0].dudy*basis[eqn_id].dphidy[i]
			    + basis[0].dudz*basis[eqn_id].dphidz[i]),
			-(basis[1].dudx*basis[eqn_id].dphidx[i]
			    + basis[1].dudy*basis[eqn_id].dphidy[i]
			    + basis[1].dudz*basis[eqn_id].dphidz[i]),
			-(basis[2].dudx*basis[eqn_id].dphidx[i]
			    + basis[2].dudy*basis[eqn_id].dphidy[i]
			    + basis[2].dudz*basis[eqn_id].dphidz[i])};
  double c[3] = {0.,0.,0.};

  solve_kks(basis[3].uu, //c
	    p[0], //phi_p
	    p[1], //phi_h 
	    p[2], //phi_m
	    c[0], 
	    c[1], 
	    c[2]);
  dG[1][0] = g10(c[1],c[0]);
  dG[2][0] = g20(c[2],c[0]);

  const double rhs = (2.*es*m*lp[0])/3. - (es*m*lp[1])/3. - (es*m*lp[2])/3. 
    - 6.*m*dG[1][0]*p[0]*p[1]*basis[eqn_id].phi[i]
    + h*m*pow(p[0],2)*p[1]*basis[eqn_id].phi[i] 
    - h*m*p[0]*pow(p[1],2)*basis[eqn_id].phi[i]
    - 6.*m*dG[2][0]*p[0]*p[2]*basis[eqn_id].phi[i] 
    + h*m*pow(p[0],2)*p[2]*basis[eqn_id].phi[i] 
    - h*m*p[0]*pow(p[2],2)*basis[eqn_id].phi[i];

  return ut-rhs;
}

RES_FUNC(residual_h_)
{
  const double ut = (basis[eqn_id].uu-basis[eqn_id].uuold)/dt_*basis[eqn_id].phi[i];
  const double p[3] = {basis[0].uu, basis[1].uu, basis[2].uu};
  const double lp[3] = {-(basis[0].dudx*basis[eqn_id].dphidx[i]
			    + basis[0].dudy*basis[eqn_id].dphidy[i]
			    + basis[0].dudz*basis[eqn_id].dphidz[i]),
			-(basis[1].dudx*basis[eqn_id].dphidx[i]
			    + basis[1].dudy*basis[eqn_id].dphidy[i]
			    + basis[1].dudz*basis[eqn_id].dphidz[i]),
			-(basis[2].dudx*basis[eqn_id].dphidx[i]
			    + basis[2].dudy*basis[eqn_id].dphidy[i]
			    + basis[2].dudz*basis[eqn_id].dphidz[i])};
  double c[3] = {0.,0.,0.};

  solve_kks(basis[3].uu, //c
	    p[0], //phi_p
	    p[1], //phi_h 
	    p[2], //phi_m
	    c[0], 
	    c[1], 
	    c[2]);
  dG[0][1] = -g10(c[1],c[0]);
  dG[2][1] = g21(c[2],c[1]);

  const double rhs = -(es*m*lp[0])/3. + (2.*es*m*lp[1])/3. - (es*m*lp[2])/3. 
    - 6.*m*dG[0][1]*p[0]*p[1]*basis[eqn_id].phi[i]
    - h*m*pow(p[0],2)*p[1]*basis[eqn_id].phi[i]
    + h*m*p[0]*pow(p[1],2)*basis[eqn_id].phi[i]
    - 6.*m*dG[2][1]*p[1]*p[2]*basis[eqn_id].phi[i]
    + h*m*pow(p[1],2)*p[2]*basis[eqn_id].phi[i]
    - h*m*p[1]*pow(p[2],2)*basis[eqn_id].phi[i];

  return ut-rhs;
}

RES_FUNC(residual_m_)
{
  const double ut = (basis[eqn_id].uu-basis[eqn_id].uuold)/dt_*basis[eqn_id].phi[i];
  const double p[3] = {basis[0].uu, basis[1].uu, basis[2].uu};
  const double lp[3] = {-(basis[0].dudx*basis[eqn_id].dphidx[i]
			    + basis[0].dudy*basis[eqn_id].dphidy[i]
			    + basis[0].dudz*basis[eqn_id].dphidz[i]),
			-(basis[1].dudx*basis[eqn_id].dphidx[i]
			    + basis[1].dudy*basis[eqn_id].dphidy[i]
			    + basis[1].dudz*basis[eqn_id].dphidz[i]),
			-(basis[2].dudx*basis[eqn_id].dphidx[i]
			    + basis[2].dudy*basis[eqn_id].dphidy[i]
			    + basis[2].dudz*basis[eqn_id].dphidz[i])};
  double c[3] = {0.,0.,0.};

  solve_kks(basis[3].uu, //c
	    p[0], //phi_p
	    p[1], //phi_h 
	    p[2], //phi_m
	    c[0], 
	    c[1], 
	    c[2]);
  dG[0][2] = g20(c[2],c[0]);
  dG[1][2] = -g21(c[2],c[1]);

  const double rhs = -(es*m*lp[0])/3. - (es*m*lp[1])/3. + (2.*es*m*lp[2])/3. 
    - 6.*m*dG[0][2]*p[0]*p[2]*basis[eqn_id].phi[i]
    - h*m*pow(p[0],2)*p[2]*basis[eqn_id].phi[i]
    - 6.*m*dG[1][2]*p[1]*p[2]*basis[eqn_id].phi[i]
    - h*m*pow(p[1],2)*p[2]*basis[eqn_id].phi[i]
    + h*m*p[0]*pow(p[2],2)*basis[eqn_id].phi[i]
    + h*m*p[1]*pow(p[2],2)*basis[eqn_id].phi[i];

  return ut-rhs;
}

RES_FUNC(residual_c_)
{
  const double ut = (basis[eqn_id].uu-basis[eqn_id].uuold)/dt_*basis[eqn_id].phi[i];
  const double p[3] = {basis[0].uu, basis[1].uu, basis[2].uu};
  double c[3] = {0.,0.,0.};

  solve_kks(basis[3].uu, //c
	    p[0], //phi_p
	    p[1], //phi_h 
	    p[2], //phi_m
	    c[0], 
	    c[1], 
	    c[2]);
  //the trouble is how to get gradient of c[0],c[1],c[2]
  //the kks paper has a solution

  const double rhs =0.;

  return ut-rhs;
}
}//namespace cerium

// #define RES_FUNC_TPETRA(NAME)  double NAME(const GPUBasis * const * basis, 
#define RES_FUNC_TPETRA(NAME)  double NAME(GPUBasis * basis[],	\
                                    const int &i,\
                                    const double &dt_,\
                                    const double &dtold_,\
			            const double &t_theta_,\
			            const double &t_theta2_,\
                                    const double &time,\
				    const int &eqn_id,	\
                                    const double &vol,	\
                                    const double &rand)

#define PRE_FUNC_TPETRA(NAME)  double NAME(const GPUBasis *basis, \
                                    const int &i,\
				    const int &j,\
				    const double &dt_,\
				    const double &t_theta_,\
				    const int &eqn_id)

#define NBC_FUNC_TPETRA(NAME)  double NAME(const GPUBasis *basis,\
				    const int &i,\
				    const double &dt_,\
                                    const double &dtold_,\
				    const double &t_theta_,\
			            const double &t_theta2_,\
				    const double &time)

namespace tpetra{//we can just put the KOKKOS... around the other dbc_zero_ later...

namespace heat{
TUSAS_DEVICE
double k_d = 1.;
TUSAS_DEVICE
double rho_d = 1.;
TUSAS_DEVICE
double cp_d = 1.;
TUSAS_DEVICE
double tau0_d = 1.;
TUSAS_DEVICE
double W0_d = 1.;
TUSAS_DEVICE
double deltau_d = 1.;
TUSAS_DEVICE
double uref_d = 0.;

double k_h = 1.;
double rho_h = 1.;
double cp_h = 1.;

double tau0_h = 1.;
double W0_h = 1.;

double deltau_h = 1.;
double uref_h = 0.;

  //KOKKOS_INLINE_FUNCTION 
DBC_FUNC(dbc_zero_) 
{
  return 0.;
}

  //KOKKOS_INLINE_FUNCTION 
INI_FUNC(init_heat_test_)
{

  const double pi = 3.141592653589793;

  return sin(pi*x)*sin(pi*y);
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_heat_test_)
{
  //right now, it is probably best to handle nondimensionalization of temperature via:
  // theta = (T-T_s)/(T_l-T_s) external to this module by multiplication of (T_l-T_s)=delta T

  const double ut = rho_d*cp_d/tau0_d*deltau_d*(basis[eqn_id]->uu-basis[eqn_id]->uuold)/dt_*basis[eqn_id]->phi[i];
  const double f[3] = {k_d/W0_d/W0_d*deltau_d*(basis[eqn_id]->dudx*basis[eqn_id]->dphidx[i]
			    + basis[eqn_id]->dudy*basis[eqn_id]->dphidy[i]
			    + basis[eqn_id]->dudz*basis[eqn_id]->dphidz[i]),
		       k_d/W0_d/W0_d*deltau_d*(basis[eqn_id]->duolddx*basis[eqn_id]->dphidx[i]
			    + basis[eqn_id]->duolddy*basis[eqn_id]->dphidy[i]
			    + basis[eqn_id]->duolddz*basis[eqn_id]->dphidz[i]),
		       k_d/W0_d/W0_d*deltau_d*(basis[eqn_id]->duoldolddx*basis[eqn_id]->dphidx[i]
			    + basis[eqn_id]->duoldolddy*basis[eqn_id]->dphidy[i]
			    + basis[eqn_id]->duoldolddz*basis[eqn_id]->dphidz[i])};
  //std::cout<<std::scientific<<f[0]<<std::endl<<std::defaultfloat;
  return ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_heat_test_dp_)) = residual_heat_test_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_heat_test_)
{
  return rho_d*cp_d/tau0_d*deltau_d*basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i]
    + t_theta_*k_d/W0_d/W0_d*deltau_d*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
}

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_heat_test_dp_)) = prec_heat_test_;

PARAM_FUNC(param_)
{
  double kk = plist->get<double>("k_",1.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(k_d,&kk,sizeof(double));
#else
  k_d = kk;
#endif
  k_h = kk;

  double rho = plist->get<double>("rho_",1.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(rho_d,&rho,sizeof(double));
#else
  rho_d = rho;
#endif
  rho_h = rho;

  double cp = plist->get<double>("cp_",1.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(cp_d,&cp,sizeof(double));
#else
  cp_d = cp;
#endif
  cp_h = cp;

  double tau0 = plist->get<double>("tau0_",1.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(tau0_d,&tau0,sizeof(double));
#else
  tau0_d = tau0;
#endif
  tau0_h = tau0;

  double W0 = plist->get<double>("W0_",1.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(W0_d,&W0,sizeof(double));
#else
  W0_d = W0;
#endif
  W0_h = W0;

  double deltau = plist->get<double>("deltau_",1.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(deltau_d,&deltau,sizeof(double));
#else
  deltau_d = deltau;
#endif
  deltau_h = deltau;

  double uref = plist->get<double>("uref_",0.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(uref_d,&uref,sizeof(double));
#else
  uref_d = uref;
#endif
  uref_h = uref;
}

PPR_FUNC(postproc_)
{
  //exact solution is: u[x,y,t]=exp(-2 pi^2 k t)sin(pi x)sin(pi y)
  const double uu = u[0];
  const double x = xyz[0];
  const double y = xyz[1];

  const double pi = 3.141592653589793;

  const double s= exp(-2.*k_h*pi*pi*time)*sin(pi*x)*sin(pi*y);

  return s-uu;
}
}//namespace heat

// the above solution is also a solution to the nonlinear problem:
// u_t - div ( u grad u) + 2 pi^2  (1-u) + u_x^2 + u_y^2
// we replace u_x^2 + u_y^2 with a forcing term f2(x,y,t)


KOKKOS_INLINE_FUNCTION 
double f1(const double &u)
{
  const double pi = 3.141592653589793;
  return 2.*pi*pi*u*(1.-u);
}

KOKKOS_INLINE_FUNCTION 
double f2(const double &x, const double &y, const double &t)
{
  const double pi = 3.141592653589793;
  const double pix = pi*x;
  const double piy = pi*y;
  const double pi2 = pi*pi;
  return exp(-4.*pi2*t)*pi2*(cos(piy)*cos(piy)*sin(pix)*sin(pix) + cos(pix)*cos(pix)*sin(piy)*sin(piy));
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_nlheatimr_test_)
{
  const double u_m = t_theta_*basis[eqn_id]->uu + (1. - t_theta_)*basis[eqn_id]->uuold;
  const double dudx_m = t_theta_*basis[eqn_id]->dudx + (1. - t_theta_)*basis[eqn_id]->duolddx;
  const double dudy_m = t_theta_*basis[eqn_id]->dudy + (1. - t_theta_)*basis[eqn_id]->duolddy;
  const double dudz_m = t_theta_*basis[eqn_id]->dudz + (1. - t_theta_)*basis[eqn_id]->duolddz;
  const double t_m = time + t_theta_*dt_;
  const double x = basis[0]->xx;
  const double y = basis[0]->yy;

  const double divgrad = u_m*(dudx_m*basis[eqn_id]->dphidx[i] 
			      + dudy_m*basis[eqn_id]->dphidy[i] 
			      + dudz_m*basis[eqn_id]->dphidz[i]);

  return (basis[eqn_id]->uu-basis[eqn_id]->uuold)/dt_*basis[eqn_id]->phi[i]
    + divgrad
    + f1(u_m)*basis[eqn_id]->phi[i]
    + f2(x,y,t_m)*basis[eqn_id]->phi[i];
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_nlheatimr_test_dp_)) = residual_nlheatimr_test_;


KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_nlheatcn_test_)
{
  const double u[2] = {basis[eqn_id]->uu, basis[eqn_id]->uuold};
  const double dudx[2] = {basis[eqn_id]->dudx, basis[eqn_id]->duolddx};
  const double dudy[2] = {basis[eqn_id]->dudy, basis[eqn_id]->duolddy};
  const double dudz[2] = {basis[eqn_id]->dudz, basis[eqn_id]->duolddz};
  //const double dudz_m = t_theta_*basis[eqn_id].dudz + (1. - t_theta_)*basis[eqn_id].duolddz;
  const double t[2] = {time, time+dt_};
  const double x = basis[0]->xx;
  const double y = basis[0]->yy;

  const double divgrad = t_theta_*
    u[0]*(dudx[0]*basis[eqn_id]->dphidx[i] 
	  + dudy[0]*basis[eqn_id]->dphidy[i] 
	  + dudz[0]*basis[eqn_id]->dphidz[i])
    + (1. - t_theta_)*
    u[1]*(dudx[1]*basis[eqn_id]->dphidx[i] 
	  + dudy[1]*basis[eqn_id]->dphidy[i] 
	  + dudz[1]*basis[eqn_id]->dphidz[i]);

  return (basis[eqn_id]->uu-basis[eqn_id]->uuold)/dt_*basis[eqn_id]->phi[i]
    + divgrad
    + (t_theta_*f1(u[0])
       + (1. - t_theta_)*f1(u[1]))*basis[eqn_id]->phi[i]
    + (t_theta_*f2(x,y,t[0])
       + (1. - t_theta_)*f2(x,y,t[1]))*basis[eqn_id]->phi[i];
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_nlheatcn_test_dp_)) = residual_nlheatcn_test_;


KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_nlheatcn_test_)
{
  return basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i]
    + t_theta_*basis[eqn_id].uu
    *(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
}

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_nlheatcn_test_dp_)) = prec_nlheatcn_test_;


//}//namespace heat

namespace localprojection
{
RES_FUNC_TPETRA(residual_u1_)
{
  const double test = basis[eqn_id]->phi[i];

  const double u2[3] = {basis[1]->uu, basis[1]->uuold, basis[1]->uuoldold};
  const double f[3] = {u2[0]*test,
		       u2[1]*test,
		       u2[2]*test};
  const double ut = (basis[eqn_id]->uu-basis[eqn_id]->uuold)/dt_*test;

  return ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC_TPETRA(residual_u2_)
{
  const double test = basis[eqn_id]->phi[i];

  const double u1[3] = {basis[0]->uu, basis[0]->uuold, basis[0]->uuoldold};

  const double f[3] = {-u1[0]*test,
		       -u1[1]*test,
		       -u1[2]*test};

  const double ut = (basis[eqn_id]->uu-basis[eqn_id]->uuold)/dt_*test;
  return ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

INI_FUNC(init_u1_)
{
  return 1.;
}

INI_FUNC(init_u2_)
{
  return 0.;
}

PPR_FUNC(postproc_u1_)
{
  return cos(time);
}

PPR_FUNC(postproc_u2_)
{
  return sin(time);
}

PPR_FUNC(postproc_norm_)
{
  return sqrt(u[0]*u[0]+u[1]*u[1])-1.;
}

PPR_FUNC(postproc_u1err_)
{
  return cos(time)-u[0];
}

PPR_FUNC(postproc_u2err_)
{
  return sin(time)-u[1];
}


}//namespace localprojection


namespace farzadi3d
{

  TUSAS_DEVICE
  double absphi = 0.9997;	//1.
  //double absphi = 0.999999;	//1.
  
  TUSAS_DEVICE
  double k = 0.14;

  TUSAS_DEVICE				//0.5
  double eps = 0.0;

  TUSAS_DEVICE
  double lambda = 10.;

  TUSAS_DEVICE
  double D_liquid = 3.e-9;			//1.e-11				//m^2/s
  
  TUSAS_DEVICE
  double m = -2.6;
  TUSAS_DEVICE					//-2.6 100.
  double c_inf = 3.;				//1.
  
  TUSAS_DEVICE
  double G = 3.e5;
  TUSAS_DEVICE											//k/m
  double R = 0.003;
//   TUSAS_DEVICE											//m/s
//   double V = 0.003;
	
  TUSAS_DEVICE											//m/s
  double d0 = 5.e-9;				//4.e-9					//m
  
  
  // parameters to scale dimensional quantities
  TUSAS_DEVICE
  double delta_T0 = 47.9143;

  TUSAS_DEVICE
  double w0 = 5.65675e-8;

  TUSAS_DEVICE
  double tau0 = 6.68455e-6;
  
//   TUSAS_DEVICE
//   double Vp0 = .354508;

  TUSAS_DEVICE
  double l_T0 = 2823.43;

  TUSAS_DEVICE
  double D_liquid_ = 6.267;
  
  TUSAS_DEVICE
  double dT = 0.0;
  
//   TUSAS_DEVICE
  double base_height = 15.;

//   TUSAS_DEVICE
  double amplitude = 0.2;
  
  //circle or sphere parameters
  double r = 0.5;
  double x0 = 20.0; 
  double y0 = 20.0;
  double z0 = 20.0;
  
  int C = 0;
  
  double t_activate_farzadi = 0.0;
  
PARAM_FUNC(param_)
{
  double k_p = plist->get<double>("k", 0.14);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(k,&k_p,sizeof(double));
#else
  k = k_p;
#endif
  double eps_p = plist->get<double>("eps", 0.0);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(eps,&eps_p,sizeof(double));
#else
  eps = eps_p;
#endif
  double lambda_p = plist->get<double>("lambda", 10.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(lambda,&lambda_p,sizeof(double));
#else
  lambda = lambda_p;
#endif
  double d0_p = plist->get<double>("d0", 5.e-9);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(d0,&d0_p,sizeof(double));
#else
  d0 = d0_p;
#endif
  double D_liquid_p = plist->get<double>("D_liquid", 3.e-9);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(D_liquid,&D_liquid_p,sizeof(double));
#else
  D_liquid = D_liquid_p;
#endif
  double m_p = plist->get<double>("m", -2.6);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(m,&m_p,sizeof(double));
#else
  m = m_p;
#endif
  double c_inf_p = plist->get<double>("c_inf", 3.);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(c_inf,&c_inf_p,sizeof(double));
#else
  c_inf = c_inf_p;
#endif
  double G_p = plist->get<double>("G", 3.e5);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(G,&G_p,sizeof(double));
#else
  G = G_p;
#endif
  double R_p = plist->get<double>("R", 0.003);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(R,&R_p,sizeof(double));
#else
  R = R_p;
#endif

// added dT here
double dT_p = plist->get<double>("dT", 0.0);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(dT,&dT_p,sizeof(double));
#else
  dT = dT_p;
#endif

  double base_height_p = plist->get<double>("base_height", 15.);
// #ifdef TUSAS_HAVE_CUDA
//   cudaMemcpyToSymbol(base_height,&base_height_p,sizeof(double));
// #else
  base_height = base_height_p;
// #endif
  double amplitude_p = plist->get<double>("amplitude", 0.2);
// #ifdef TUSAS_HAVE_CUDA
//   cudaMemcpyToSymbol(amplitude,&amplitude_p,sizeof(double));
// #else
  amplitude = amplitude_p;
// #endif

int C_p = plist->get<int>("C", 0);
C = C_p;

// circle or sphere parameters

double r_p = plist->get<double>("r", 0.5);
r = r_p;
double x0_p = plist->get<double>("x0", 20.0);
x0 = x0_p;
double y0_p = plist->get<double>("y0", 20.0);
y0 = y0_p;
double z0_p = plist->get<double>("z0", 20.0);
z0 = z0_p;

//double absphi_p = plist->get<double>("absphi", absphi);

  //the calculated values need local vars to work....

  //calculated values
  double w0_p = lambda_p*d0_p/0.8839;
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(w0,&w0_p,sizeof(double));
#else
  w0 = w0_p;
#endif
  double tau0_p = (lambda_p*0.6267*w0_p*w0_p)/D_liquid_p;
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(tau0,&tau0_p,sizeof(double));
#else
  tau0 = tau0_p;
#endif

//   double V_p = R_p;
// #ifdef TUSAS_HAVE_CUDA
//   cudaMemcpyToSymbol(V,&V_p,sizeof(double));
// #else
//   V = V_p;
// #endif

//   double Vp0_p = V_p*tau0_p/w0_p;
// #ifdef TUSAS_HAVE_CUDA
//   cudaMemcpyToSymbol(Vp0,&Vp0_p,sizeof(double));
// #else
//   Vp0 = Vp0_p;
// #endif

  double delta_T0_p = abs(m_p)*c_inf_p*(1.-k_p)/k_p;
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(delta_T0,&delta_T0_p,sizeof(double));
#else
  delta_T0 = delta_T0_p;
#endif
  double l_T0_p = delta_T0_p/G_p;
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(l_T0,&l_T0_p,sizeof(double));
#else
  l_T0 = l_T0_p;
#endif
  double D_liquid__p = D_liquid_p*tau0_p/(w0_p*w0_p);
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(D_liquid_,&D_liquid__p,sizeof(double));
#else
  D_liquid_ = D_liquid__p;
#endif

t_activate_farzadi = plist->get<double>("t_activate_farzadi", 0.0);


  //std::cout<<l_T0<<"   "<<G<<"  "<<Vp0<<"  "<<tau0<<"   "<<w0<<std::endl;
}
  
  //see tpetra::pfhub3 for a possibly better implementation of a,ap
KOKKOS_INLINE_FUNCTION 
double a(const double &p,const double &px,const double &py,const double &pz, const double ep)
{
  double val = 1. + ep;
  val = (p*p < farzadi3d::absphi)&&(p*p > 1.-farzadi3d::absphi) ? (1.-3.*ep)*(1.+4.*ep/(1.-3.*ep)*
				    (px*px*px*px+py*py*py*py+pz*pz*pz*pz)/(px*px+py*py+pz*pz)/(px*px+py*py+pz*pz))
    : 1. + ep;
//   if(val!=val)  std::cout<<farzadi3d::absphi<<" "<<1.-farzadi3d::absphi<<" "<<p*p<<" "<<px*px+py*py+pz*pz<<" "<<val<<" "<<
// 	   (1.-3.*ep)*(1.+4.*ep/(1.-3.*ep)*
// 				    (px*px*px*px+py*py*py*py+pz*pz*pz*pz)/(px*px+py*py+pz*pz)/(px*px+py*py+pz*pz))<<std::endl;
  return val;
}

KOKKOS_INLINE_FUNCTION 
double ap(const double &p,const double &px,const double &py,const double &pz,const double &pd, const double ep)
{
  return (p*p < farzadi3d::absphi)&&(p*p > 1.-farzadi3d::absphi) ? 4.*ep*
				    (4.*pd*pd*pd*(px*px+py*py+pz*pz)-4.*pd*(px*px*px*px+py*py*py*py+pz*pz*pz*pz))
				    /(px*px+py*py+pz*pz)/(px*px+py*py+pz*pz)/(px*px+py*py+pz*pz)
    : 0.;
}

//the current ordering in set_test_case is conc (u), phase (phi)

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_conc_farzadi_)
{
  //right now, if explicit, we will have some problems with time derivates below
  const double dtestdx = basis[eqn_id]->dphidx[i];
  const double dtestdy = basis[eqn_id]->dphidy[i];
  const double dtestdz = basis[eqn_id]->dphidz[i];
  const double test = basis[eqn_id]->phi[i];
  const double u[3] = {basis[eqn_id]->uu,basis[eqn_id]->uuold,basis[eqn_id]->uuoldold};

  const int phi_id = eqn_id+1;
  const double phi[3] = {basis[phi_id]->uu,basis[phi_id]->uuold,basis[phi_id]->uuoldold};
  const double dphidx[3] = {basis[phi_id]->dudx,basis[phi_id]->duolddx,basis[phi_id]->duoldolddx};
  const double dphidy[3] = {basis[phi_id]->dudy,basis[phi_id]->duolddy,basis[phi_id]->duoldolddy};
  const double dphidz[3] = {basis[phi_id]->dudz,basis[phi_id]->duolddz,basis[phi_id]->duoldolddz};

  const double ut = (1.+k)/2.*(u[0]-u[1])/dt_*test;
  const double divgradu[3] = {D_liquid_*(1.-phi[0])/2.*(basis[eqn_id]->dudx*dtestdx + basis[eqn_id]->dudy*dtestdy + basis[eqn_id]->dudz*dtestdz),
			      D_liquid_*(1.-phi[1])/2.*(basis[eqn_id]->duolddx*dtestdx + basis[eqn_id]->duolddy*dtestdy + basis[eqn_id]->duolddz*dtestdz),
			      D_liquid_*(1.-phi[2])/2.*(basis[eqn_id]->duoldolddx*dtestdx + basis[eqn_id]->duoldolddy*dtestdy + basis[eqn_id]->duoldolddz*dtestdz)};//(grad u,grad phi)

  const double normd[3] = {(phi[0]*phi[0] < absphi)&&(phi[0]*phi[0] > 0.) ? 1./sqrt(dphidx[0]*dphidx[0] + dphidy[0]*dphidy[0] + dphidz[0]*dphidz[0]) : 0.,
			   (phi[1]*phi[1] < absphi)&&(phi[1]*phi[1] > 0.) ? 1./sqrt(dphidx[1]*dphidx[1] + dphidy[1]*dphidy[1] + dphidz[1]*dphidz[1]) : 0.,
			   (phi[2]*phi[2] < absphi)&&(phi[2]*phi[2] > 0.) ? 1./sqrt(dphidx[2]*dphidx[2] + dphidy[2]*dphidy[2] + dphidz[2]*dphidz[2]) : 0.}; //cn lim grad phi/|grad phi| may -> 1 here?

  //we need to double check these terms with temporal derivatives....
  const double phit = (phi[0]-phi[1])/dt_;
  const double j_coef[3] = {(1.+(1.-k)*u[0])/sqrt(8.)*normd[0]*phit,
			    (1.+(1.-k)*u[1])/sqrt(8.)*normd[1]*phit,
			    (1.+(1.-k)*u[2])/sqrt(8.)*normd[2]*phit};
  const double divj[3] = {j_coef[0]*(dphidx[0]*dtestdx + dphidy[0]*dtestdy + dphidz[0]*dtestdz),
			  j_coef[1]*(dphidx[1]*dtestdx + dphidy[1]*dtestdy + dphidz[1]*dtestdz),
			  j_coef[2]*(dphidx[2]*dtestdx + dphidy[2]*dtestdy + dphidz[2]*dtestdz)};

  double phitu[3] = {-.5*phit*(1.+(1.-k)*u[0])*test,
		     -.5*phit*(1.+(1.-k)*u[1])*test,
		     -.5*phit*(1.+(1.-k)*u[2])*test}; 
  
  //double val = ut + t_theta_*divgradu  + t_theta_*divj + t_theta_*phitu;
  //printf("%lf\n",val);

  const double f[3] = {divgradu[0] + divj[0] + phitu[0],
		       divgradu[1] + divj[1] + phitu[1],
		       divgradu[2] + divj[2] + phitu[2]};

  return (ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]));

}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_conc_farzadi_dp_)) = residual_conc_farzadi_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_phase_farzadi_)
{
  //derivatives of the test function
  const double dtestdx = basis[eqn_id]->dphidx[i];
  const double dtestdy = basis[eqn_id]->dphidy[i];
  const double dtestdz = basis[eqn_id]->dphidz[i];
  //test function
  const double test = basis[eqn_id]->phi[i];
  //u, phi
  const int u_id = eqn_id-1;
  const double u[3] = {basis[u_id]->uu,basis[u_id]->uuold,basis[u_id]->uuoldold};
  const double phi[3] = {basis[eqn_id]->uu,basis[eqn_id]->uuold,basis[eqn_id]->uuoldold};

  const double dphidx[3] = {basis[eqn_id]->dudx,basis[eqn_id]->duolddx,basis[eqn_id]->duoldolddx};
  const double dphidy[3] = {basis[eqn_id]->dudy,basis[eqn_id]->duolddy,basis[eqn_id]->duoldolddy};
  const double dphidz[3] = {basis[eqn_id]->dudz,basis[eqn_id]->duolddz,basis[eqn_id]->duoldolddz};

  const double as[3] = {a(phi[0],dphidx[0],dphidy[0],dphidz[0],eps),
			a(phi[1],dphidx[1],dphidy[1],dphidz[1],eps),
			a(phi[2],dphidx[2],dphidy[2],dphidz[2],eps)};

  const double divgradphi[3] = {as[0]*as[0]*(dphidx[0]*dtestdx + dphidy[0]*dtestdy + dphidz[0]*dtestdz),
				as[1]*as[1]*(dphidx[1]*dtestdx + dphidy[1]*dtestdy + dphidz[1]*dtestdz),
				as[2]*as[2]*(dphidx[2]*dtestdx + dphidy[2]*dtestdy + dphidz[2]*dtestdz)};//(grad u,grad phi)

  const double mob[3] = {(1.+(1.-k)*u[0])*as[0]*as[0],(1.+(1.-k)*u[1])*as[1]*as[1],(1.+(1.-k)*u[2])*as[2]*as[2]};
  const double phit = (phi[0]-phi[1])/dt_*test;

  //double curlgrad = -dgdtheta*dphidy*dtestdx + dgdtheta*dphidx*dtestdy;
  const double curlgrad[3] = {as[0]*(dphidx[0]*dphidx[0] + dphidy[0]*dphidy[0] + dphidz[0]*dphidz[0])
			      *(ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidx[0],eps)*dtestdx 
				+ ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidy[0],eps)*dtestdy 
				+ ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidz[0],eps)*dtestdz),
			      as[1]*(dphidx[1]*dphidx[1] + dphidy[1]*dphidy[1] + dphidz[1]*dphidz[1])
			      *(ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidx[1],eps)*dtestdx 
				+ ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidy[1],eps)*dtestdy 
				+ ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidz[1],eps)*dtestdz),
			      as[2]*(dphidx[2]*dphidx[2] + dphidy[2]*dphidy[2] + dphidz[2]*dphidz[2])
			      *(ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidx[2],eps)*dtestdx 
				+ ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidy[2],eps)*dtestdy 
				+ ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidz[2],eps)*dtestdz)};
  
  const double gp1[3] = {-(phi[0] - phi[0]*phi[0]*phi[0])*test,
			 -(phi[1] - phi[1]*phi[1]*phi[1])*test,
			 -(phi[2] - phi[2]*phi[2]*phi[2])*test};

  //note in paper eq 39 has g3 different
  //here (as implemented) our g3 = lambda*(1. - phi[0]*phi[0])*(1. - phi[0]*phi[0])
  //matches farzadi eq 10

  const double hp1u[3] = {lambda*(1. - phi[0]*phi[0])*(1. - phi[0]*phi[0])*(u[0])*test,
			 lambda*(1. - phi[1]*phi[1])*(1. - phi[1]*phi[1])*(u[1])*test,
			 lambda*(1. - phi[2]*phi[2])*(1. - phi[2]*phi[2])*(u[2])*test};
  
  const double f[3] = {(divgradphi[0] + curlgrad[0] + gp1[0] + hp1u[0])/mob[0],
		       (divgradphi[1] + curlgrad[1] + gp1[1] + hp1u[1])/mob[1],
		       (divgradphi[2] + curlgrad[2] + gp1[2] + hp1u[2])/mob[2]};

  const double val = phit 
    + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);

  return mob[0]*val;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_phase_farzadi_dp_)) = residual_phase_farzadi_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_phase_farzadi_uncoupled_)
{
  //test function
  const double test = basis[eqn_id]->phi[i];
  //u, phi
  const int u_id = eqn_id-1;
  const double u[3] = {basis[u_id]->uu,basis[u_id]->uuold,basis[u_id]->uuoldold};
  const double phi[3] = {basis[eqn_id]->uu,basis[eqn_id]->uuold,basis[eqn_id]->uuoldold};

  const double dphidx[3] = {basis[eqn_id]->dudx,basis[eqn_id]->duolddx,basis[eqn_id]->duoldolddx};
  const double dphidy[3] = {basis[eqn_id]->dudy,basis[eqn_id]->duolddy,basis[eqn_id]->duoldolddy};
  const double dphidz[3] = {basis[eqn_id]->dudz,basis[eqn_id]->duolddz,basis[eqn_id]->duoldolddz};

  const double as[3] = {a(phi[0],dphidx[0],dphidy[0],dphidz[0],eps),
			a(phi[1],dphidx[1],dphidy[1],dphidz[1],eps),
			a(phi[2],dphidx[2],dphidy[2],dphidz[2],eps)};

  const double mob[3] = {(1.+(1.-k)*u[0])*as[0]*as[0],(1.+(1.-k)*u[1])*as[1]*as[1],(1.+(1.-k)*u[2])*as[2]*as[2]};

  const double x = basis[eqn_id]->xx;
  
  // frozen temperature approximation: linear pulling of the temperature field
  const double xx = x*w0;

  //cn this should probablly be: (time+dt_)*tau
  const double tt[3] = {(time+dt_)*tau0,time*tau0,(time-dtold_)*tau0};

  const double g4[3] = {((dT < 0.001) ? G*(xx-R*tt[0])/delta_T0 : dT),
			     ((dT < 0.001) ? G*(xx-R*tt[1])/delta_T0 : dT),
			     ((dT < 0.001) ? G*(xx-R*tt[2])/delta_T0 : dT)};
  
  const double hp1g4[3] = {lambda*(1. - phi[0]*phi[0])*(1. - phi[0]*phi[0])*(g4[0])*test,
			 lambda*(1. - phi[1]*phi[1])*(1. - phi[1]*phi[1])*(g4[1])*test,
			 lambda*(1. - phi[2]*phi[2])*(1. - phi[2]*phi[2])*(g4[2])*test};

  const double val = tpetra::farzadi3d::residual_phase_farzadi_dp_(basis,
								   i,
								   dt_,
								   dtold_,
								   t_theta_,
								   t_theta2_,
								   time,
								   eqn_id,
								   vol,
								   rand);

  const double rv = val/mob[0]
    + (1.-t_theta2_)*t_theta_*hp1g4[0]/mob[0]
    + (1.-t_theta2_)*(1.-t_theta_)*hp1g4[1]/mob[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*hp1g4[1]/mob[1]-dt_/dtold_*hp1g4[2]/mob[2]);

  return mob[0]*rv;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_phase_farzadi_uncoupled_dp_)) = residual_phase_farzadi_uncoupled_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_phase_farzadi_coupled_)
{
  //test function
  const double test = basis[eqn_id]->phi[i];
  //u, phi
  const int u_id = eqn_id-1;
  const int theta_id = eqn_id+1;
  const double u[3] = {basis[u_id]->uu,basis[u_id]->uuold,basis[u_id]->uuoldold};
  const double phi[3] = {basis[eqn_id]->uu,basis[eqn_id]->uuold,basis[eqn_id]->uuoldold};

  const double dphidx[3] = {basis[eqn_id]->dudx,basis[eqn_id]->duolddx,basis[eqn_id]->duoldolddx};
  const double dphidy[3] = {basis[eqn_id]->dudy,basis[eqn_id]->duolddy,basis[eqn_id]->duoldolddy};
  const double dphidz[3] = {basis[eqn_id]->dudz,basis[eqn_id]->duolddz,basis[eqn_id]->duoldolddz};

  const double as[3] = {a(phi[0],dphidx[0],dphidy[0],dphidz[0],eps),
			a(phi[1],dphidx[1],dphidy[1],dphidz[1],eps),
			a(phi[2],dphidx[2],dphidy[2],dphidz[2],eps)};

  const double mob[3] = {(1.+(1.-k)*u[0])*as[0]*as[0],(1.+(1.-k)*u[1])*as[1]*as[1],(1.+(1.-k)*u[2])*as[2]*as[2]};

  const double theta[3] = {basis[theta_id]->uu,basis[theta_id]->uuold,basis[theta_id]->uuoldold};
  
  const double g4[3] = {theta[0],theta[1],theta[2]};
  
  const double hp1g4[3] = {lambda*(1. - phi[0]*phi[0])*(1. - phi[0]*phi[0])*(g4[0])*test,
			 lambda*(1. - phi[1]*phi[1])*(1. - phi[1]*phi[1])*(g4[1])*test,
			 lambda*(1. - phi[2]*phi[2])*(1. - phi[2]*phi[2])*(g4[2])*test};

  const double val = tpetra::farzadi3d::residual_phase_farzadi_dp_(basis,
								   i,
								   dt_,
								   dtold_,
								   t_theta_,
								   t_theta2_,
								   time,
								   eqn_id,
								   vol,
								   rand);

  const double rv = val/mob[0]
    + (1.-t_theta2_)*t_theta_*hp1g4[0]/mob[0]
    + (1.-t_theta2_)*(1.-t_theta_)*hp1g4[1]/mob[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*hp1g4[1]/mob[1]-dt_/dtold_*hp1g4[2]/mob[2]);
	
  return mob[0]*rv;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_phase_farzadi_coupled_dp_)) = residual_phase_farzadi_coupled_;

RES_FUNC_TPETRA(residual_conc_farzadi_activated_)
{
	const double val = tpetra::farzadi3d::residual_conc_farzadi_dp_(basis,
  						 i,
  						 dt_,
  						 dtold_,
  						 t_theta_,
  						 t_theta2_,
  						 time,
  						 eqn_id,
  						 vol,
  						 rand);
	
	const double u[2] = {basis[eqn_id]->uu,basis[eqn_id]->uuold};
	
	// Coefficient to turn Farzadi evolution off until a specified time
	const double delta = 1.0e12; 			   
	const double sigmoid_var = delta * (time-t_activate_farzadi/tau0);
	const double sigmoid = 0.5 * (1.0 + sigmoid_var / (std::sqrt(1.0 + sigmoid_var*sigmoid_var))); 			   
	//std::cout<<val * sigmoid + (u[1]-u[0]) * (1.0 - sigmoid)*basis[eqn_id]->phi[i]<<std::endl;
	return val * sigmoid + (u[1]-u[0]) * (1.0 - sigmoid)*basis[eqn_id]->phi[i];
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_conc_farzadi_activated_dp_)) = residual_conc_farzadi_activated_;

RES_FUNC_TPETRA(residual_phase_farzadi_coupled_activated_)
{
	const double val = tpetra::farzadi3d::residual_phase_farzadi_coupled_dp_(basis,
  						 i,
  						 dt_,
  						 dtold_,
  						 t_theta_,
  						 t_theta2_,
  						 time,
  						 eqn_id,
  						 vol,
  						 rand);
	
	const double phi[2] = {basis[eqn_id]->uu,basis[eqn_id]->uuold};
	
	// Coefficient to turn Farzadi evolution off until a specified time
	const double delta = 1.0e12; 			   
	const double sigmoid_var = delta * (time-t_activate_farzadi/tau0);
	const double sigmoid = 0.5 * (1.0 + sigmoid_var / (std::sqrt(1.0 + sigmoid_var*sigmoid_var))); 			   

	return val * sigmoid + (phi[1]-phi[0]) * (1.0 - sigmoid)*basis[eqn_id]->phi[i];
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_phase_farzadi_coupled_activated_dp_)) = residual_phase_farzadi_coupled_activated_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_conc_farzadi_)
{
  const double dtestdx = basis[eqn_id].dphidx[i];
  const double dtestdy = basis[eqn_id].dphidy[i];
  const double dtestdz = basis[eqn_id].dphidz[i];
  const double dbasisdx = basis[eqn_id].dphidx[j];
  const double dbasisdy = basis[eqn_id].dphidy[j];
  const double dbasisdz = basis[eqn_id].dphidz[j];

  const double test = basis[0].phi[i];
  const double divgrad = D_liquid_*(1.-basis[1].uu)/2.*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);

  const double u_t =(1.+k)/2.*test * basis[0].phi[j]/dt_;

  return u_t + t_theta_*(divgrad);

}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_phase_farzadi_)
{
  const double dtestdx = basis[eqn_id].dphidx[i];
  const double dtestdy = basis[eqn_id].dphidy[i];
  const double dtestdz = basis[eqn_id].dphidz[i];
  const double dbasisdx = basis[eqn_id].dphidx[j];
  const double dbasisdy = basis[eqn_id].dphidy[j];
  const double dbasisdz = basis[eqn_id].dphidz[j];

  const double test = basis[1].phi[i];
  
  const double dphidx = basis[1].dudx;
  const double dphidy = basis[1].dudy;
  const double dphidz = basis[1].dudz;

  const double u = basis[0].uu;
  const double phi = basis[1].uu;

  const double as = a(phi,dphidx,dphidy,dphidz,eps);

  const double m = (1.+(1.-k)*u)*as*as;
  const double phit = (basis[1].phi[j])/dt_*test;

  const double divgrad = as*as*(dbasisdx*dtestdx + dbasisdy*dtestdy + dbasisdz*dtestdz);

  return (phit + t_theta_*(divgrad)/m)*m;
}

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_phase_farzadi_dp_)) = prec_phase_farzadi_;

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_conc_farzadi_dp_)) = prec_conc_farzadi_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_conc_farzadi_exp_)
{
  //this is the explicit case with explicit phit
  const double dtestdx = basis[0]->dphidx[i];
  const double dtestdy = basis[0]->dphidy[i];
  const double dtestdz = basis[0]->dphidz[i];
  const double test = basis[0]->phi[i];
  const double u[2] = {basis[0]->uu,basis[0]->uuold};
  const double phi[2] = {basis[1]->uu,basis[1]->uuold};
  const double dphidx[2] = {basis[1]->dudx,basis[1]->duolddx};
  const double dphidy[2] = {basis[1]->dudy,basis[1]->duolddy};
  const double dphidz[2] = {basis[1]->dudz,basis[1]->duolddz};

  const double ut = (1.+k)/2.*(u[0]-u[1])/dt_*test;
  const double divgradu[2] = {D_liquid_*(1.-phi[0])/2.*(basis[0]->dudx*dtestdx + basis[0]->dudy*dtestdy + basis[0]->dudz*dtestdz),
			      D_liquid_*(1.-phi[1])/2.*(basis[0]->duolddx*dtestdx + basis[0]->duolddy*dtestdy + basis[0]->duolddz*dtestdz)};//(grad u,grad phi)

  const double normd[2] = {(phi[0]*phi[0] < absphi)&&(phi[0]*phi[0] > 0.) ? 1./sqrt(dphidx[0]*dphidx[0] + dphidy[0]*dphidy[0] + dphidz[0]*dphidz[0]) : 0.,
			   (phi[1]*phi[1] < absphi)&&(phi[1]*phi[1] > 0.) ? 1./sqrt(dphidx[1]*dphidx[1] + dphidy[1]*dphidy[1] + dphidz[1]*dphidz[1]) : 0.}; //cn lim grad phi/|grad phi| may -> 1 here?

  const double phit = (phi[0]-phi[1])/dt_;
  const double j_coef[2] = {(1.+(1.-k)*u[0])/sqrt(8.)*normd[0]*phit,
			    (1.+(1.-k)*u[1])/sqrt(8.)*normd[1]*phit};
  const double divj[2] = {j_coef[0]*(dphidx[0]*dtestdx + dphidy[0]*dtestdy + dphidz[0]*dtestdz),
			  j_coef[1]*(dphidx[1]*dtestdx + dphidy[1]*dtestdy + dphidz[1]*dtestdz)};

  double phitu[2] = {-.5*phit*(1.+(1.-k)*u[0])*test,
		     -.5*phit*(1.+(1.-k)*u[1])*test}; 
  
  //double val = ut + t_theta_*divgradu  + t_theta_*divj + t_theta_*phitu;
  //printf("%lf\n",val);

  return ut + t_theta_*(divgradu[0] + divj[0] + phitu[0]) + (1.-t_theta_)*(divgradu[1] + divj[1] + phitu[1]);
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_conc_farzadi_exp_dp_)) = residual_conc_farzadi_exp_;

INI_FUNC(init_phase_farzadi_)
{

  double h = base_height + amplitude*((double)rand()/(RAND_MAX));
  
  double c = (x-x0)*(x-x0) + (y-y0)*(y-y0) + (z-z0)*(z-z0);
  
  return (C == 0) ? (tanh((h-x)/sqrt(2.))) : ((c < r*r) ? 1. : -1.);	

}

INI_FUNC(init_phase_farzadi_test_)
{
  const double pp = 36.;
  const double ll = .2;
  const double aa = 9.;
  const double pi = 3.141592653589793;
  double r = ll*(1.+(2.+sin(y*aa*pi/pp))
		 *(2.+sin(y*aa*pi/pp/2.))
		 *(2.+sin(y*aa*pi/pp/4.)));
  double val = -1.;
  if(x < r) val = 1.;
  return val;
}

INI_FUNC(init_conc_farzadi_)
{
  return -1.;
}

PPR_FUNC(postproc_c_)
{
  // return the physical concentration
  const double uu = u[0];
  const double phi = u[1];

  return -c_inf*(1.+k-phi+k*phi)*(-1.-uu+k*uu)/2./k;
}

PPR_FUNC(postproc_t_)
{
  // return the physical temperature in K here
  double x = xyz[0];

  double xx = x*w0;
  double tt = time*tau0;
  //return ((dT < 0.001) ? 877.3 + (xx-R*tt)/l_T0*delta_T0 : 877.3);
  return ((dT < 0.001) ? 877.3 + G*(xx-R*tt) : 877.3);
}
}//namespace farzadi3d

namespace noise
{
KOKKOS_INLINE_FUNCTION 
double noise_(const double rand)
{
  return 20.*rand;
}
}//namespace noise

namespace pfhub3
{
  const double R_ = 8.;// 8.;

  TUSAS_DEVICE
  double smalld_ = 0.;
  TUSAS_DEVICE
  const double delta_ = -.3;//-.3;
  TUSAS_DEVICE
  const double D_ = 10.;
  TUSAS_DEVICE
  const double eps_ = .05;
  TUSAS_DEVICE
  const double tau0_ = 1.;
  TUSAS_DEVICE
  const double W_ = 1.;
  TUSAS_DEVICE
  const double lambda_ = D_*tau0_/.6267/W_/W_;

PARAM_FUNC(param_)
{
  //we will need to propgate this to device
  double smalld_p = plist->get<double>("smalld", smalld_);
  smalld_ = smalld_p;
}

KOKKOS_INLINE_FUNCTION 
double a(const double &p,const double &px,const double &py,const double &pz, const double ep)
{
  double val = 1. + ep;
  const double d = (px*px+py*py+pz*pz)*(px*px+py*py+pz*pz);
  val = (d > smalld_) ? (1.-3.*ep)*(1.+4.*ep/(1.-3.*ep)*(px*px*px*px+py*py*py*py+pz*pz*pz*pz)/d)
    : 1. + ep;
  //older version produced nicer dendrite
//   const double d = (px*px+py*py+pz*pz)*(px*px+py*py+pz*pz);
//   val = (d > smalld_) ? (1.-3.*ep)*(1.+4.*ep/(1.-3.*ep)*(px*px*px*px+py*py*py*py+pz*pz*pz*pz)/d)
//     : 1. + ep;

  return val;
}

KOKKOS_INLINE_FUNCTION 
double ap(const double &p,const double &px,const double &py,const double &pz,const double &pd, const double ep)
{
  //older version produced nicer dendrite  
//   const double d = (px*px+py*py+pz*pz)*(px*px+py*py+pz*pz);
//   return (d > smalld_) ? 4.*ep*
// 				    (4.*pd*pd*pd*(px*px+py*py+pz*pz)-4.*pd*(px*px*px*px+py*py*py*py+pz*pz*pz*pz))
// 				    /(px*px+py*py+pz*pz)/d
//     : 0.;
  const double d = (px*px+py*py+pz*pz)*(px*px+py*py+pz*pz);
  return (d > smalld_) ? 4.*ep*
    (4.*pd*pd*pd*(px*px+py*py+pz*pz)-4.*pd*(px*px*px*px+py*py*py*py+pz*pz*pz*pz))
    /((px*px+py*py+pz*pz)*d)
    : 0.;
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_heat_pfhub3_)
{
  const double ut = (basis[eqn_id]->uu-basis[eqn_id]->uuold)/dt_*basis[eqn_id]->phi[i];
  double divgradu[3] = {D_*(basis[eqn_id]->dudx*basis[eqn_id]->dphidx[i]
			  + basis[eqn_id]->dudy*basis[eqn_id]->dphidy[i]
			  + basis[eqn_id]->dudz*basis[eqn_id]->dphidz[i]),
			D_*(basis[eqn_id]->duolddx*basis[eqn_id]->dphidx[i]
			  + basis[eqn_id]->duolddy*basis[eqn_id]->dphidy[i]
			  + basis[eqn_id]->duolddz*basis[eqn_id]->dphidz[i]),
			D_*(basis[eqn_id]->duoldolddx*basis[eqn_id]->dphidx[i]
			  + basis[eqn_id]->duoldolddy*basis[eqn_id]->dphidy[i]
			  + basis[eqn_id]->duoldolddz*basis[eqn_id]->dphidz[i])};

  const double phit[2] = {.5*(basis[1]->uu-basis[1]->uuold)/dt_*basis[0]->phi[i],
			  .5*(basis[1]->uuold-basis[1]->uuoldold)/dt_*basis[0]->phi[i]};

  double f[3];
  f[0] = -divgradu[0] + phit[0];
  f[1] = -divgradu[1] + phit[1];
  f[2] = -divgradu[2] + phit[1];

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}
TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_heat_pfhub3_dp_)) = residual_heat_pfhub3_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_phase_pfhub3_)
{
  const double test = basis[eqn_id]->phi[i];
  const double dtestdx = basis[eqn_id]->dphidx[i];
  const double dtestdy = basis[eqn_id]->dphidy[i];
  const double dtestdz = basis[eqn_id]->dphidz[i];

  const double phi[3] = {basis[eqn_id]->uu,basis[eqn_id]->uuold,basis[eqn_id]->uuoldold};
  const double dphidx[3] = {basis[eqn_id]->dudx,basis[eqn_id]->duolddx,basis[eqn_id]->duoldolddx};
  const double dphidy[3] = {basis[eqn_id]->dudy,basis[eqn_id]->duolddy,basis[eqn_id]->duoldolddy};
  const double dphidz[3] = {basis[eqn_id]->dudz,basis[eqn_id]->duolddz,basis[eqn_id]->duoldolddz};

  const double as[3] = {a(phi[0],
			  dphidx[0],
			  dphidy[0],
			  dphidz[0],
			  eps_),
			a(phi[1],
			  dphidx[1],
			  dphidy[1],
			  dphidz[1],
			  eps_),
			a(phi[2],
			  dphidx[2],
			  dphidy[2],
			  dphidz[2],
			  eps_)};

  const double tau[3] = {tau0_*as[0]*as[0],tau0_*as[1]*as[1],tau0_*as[2]*as[2]};

  const double phit = (phi[0]-phi[1])/dt_*test;

  const double w[3] = {W_*as[0],W_*as[1],W_*as[2]};

//   const double divgradphi[3] = {w[0]*w[0]*(dphidx[0]*dtestdx
// 					     + dphidy[0]*dtestdy
// 					     + dphidz[0]*dtestdz),
// 				w[1]*w[1]*(dphidx[1]*dtestdx
// 					     + dphidy[1]*dtestdy
// 					     + dphidz[1]*dtestdz),
// 				w[2]*w[2]*(dphidx[2]*dtestdx
// 					     + dphidy[2]*dtestdy
// 					     + dphidz[2]*dtestdz)};
  const double divgradphi[3] = {W_*W_*(dphidx[0]*dtestdx
					     + dphidy[0]*dtestdy
					     + dphidz[0]*dtestdz),
				W_*W_*(dphidx[1]*dtestdx
					     + dphidy[1]*dtestdy
					     + dphidz[1]*dtestdz),
				W_*W_*(dphidx[2]*dtestdx
					     + dphidy[2]*dtestdy
					     + dphidz[2]*dtestdz)};

  const double wp[3] = {W_*(ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidx[0],eps_)*dtestdx 
			    + ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidy[0],eps_)*dtestdy 
			    + ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidz[0],eps_)*dtestdz),
			W_*(ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidx[1],eps_)*dtestdx 
			    + ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidy[1],eps_)*dtestdy 
			    + ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidz[1],eps_)*dtestdz),
			W_*(ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidx[2],eps_)*dtestdx 
			    + ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidy[2],eps_)*dtestdy 
			    + ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidz[2],eps_)*dtestdz)};

  const double curlgrad[3] = {w[0]*(dphidx[0]*dphidx[0] + dphidy[0]*dphidy[0] + dphidz[0]*dphidz[0])*wp[0],
			      w[1]*(dphidx[1]*dphidx[1] + dphidy[1]*dphidy[1] + dphidz[1]*dphidz[1])*wp[1],
			      w[2]*(dphidx[2]*dphidx[2] + dphidy[2]*dphidy[2] + dphidz[2]*dphidz[2])*wp[2]};

  const double g[3] = {((phi[0]-lambda_*basis[0]->uu*(1.-phi[0]*phi[0]))*(1.-phi[0]*phi[0]))*test,
		       ((phi[1]-lambda_*basis[0]->uuold*(1.-phi[1]*phi[1]))*(1.-phi[1]*phi[1]))*test,
		       ((phi[2]-lambda_*basis[0]->uuoldold*(1.-phi[2]*phi[2]))*(1.-phi[2]*phi[2]))*test};

  double f[3];
  f[0] = -(divgradphi[0]/tau0_+curlgrad[0]/tau[0]-g[0]/tau[0]);
  f[1] = -(divgradphi[1]/tau0_+curlgrad[1]/tau[1]-g[1]/tau[1]);
  f[2] = -(divgradphi[2]/tau0_+curlgrad[2]/tau[2]-g[2]/tau[2]);

  return phit - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC_TPETRA(residual_phase_pfhub3_noise_)
{
  double val = residual_phase_pfhub3_(basis,
				      i,
				      dt_,
				      dtold_,
				      t_theta_,
				      t_theta2_,
				      time,
				      eqn_id,
				      vol,
				      rand);
  const double phi[1] ={ basis[eqn_id]->uu};
  const double g = (1.-phi[0]*phi[0])*(1.-phi[0]*phi[0]);
  double noise[3] = {g*tpetra::noise::noise_(rand)*basis[eqn_id]->phi[i],0.*basis[eqn_id]->phi[i],0.*basis[eqn_id]->phi[i]};

  double rv = (val + (1.-t_theta2_)*t_theta_*noise[0]
	  + (1.-t_theta2_)*(1.-t_theta_)*noise[1]
	       +.5*t_theta2_*((2.+dt_/dtold_)*noise[1]-dt_/dtold_*noise[2]));

  return rv;
}

RES_FUNC(residual_heat_pfhub3_n_)
{
  const double ut = (basis[eqn_id].uu-basis[eqn_id].uuold)/dt_*basis[eqn_id].phi[i];
  const double divgradu[3] = {D_*(basis[eqn_id].dudx*basis[eqn_id].dphidx[i]
				  + basis[eqn_id].dudy*basis[eqn_id].dphidy[i]
				  + basis[eqn_id].dudz*basis[eqn_id].dphidz[i]),
			      D_*(basis[eqn_id].duolddx*basis[eqn_id].dphidx[i]
				  + basis[eqn_id].duolddy*basis[eqn_id].dphidy[i]
				  + basis[eqn_id].duolddz*basis[eqn_id].dphidz[i]),
			      D_*(basis[eqn_id].duoldolddx*basis[eqn_id].dphidx[i]
				  + basis[eqn_id].duoldolddy*basis[eqn_id].dphidy[i]
				  + basis[eqn_id].duoldolddz*basis[eqn_id].dphidz[i])};

  const double phit[2] = {.5*(basis[1].uu-basis[1].uuold)/dt_*basis[0].phi[i],
			  .5*(basis[1].uuold-basis[1].uuoldold)/dt_*basis[0].phi[i]};

  double f[3];
  f[0] = -divgradu[0] + phit[0];
  f[1] = -divgradu[1] + phit[1];
  f[2] = -divgradu[2] + phit[1];
//   std::cout<<ut
//     + t_theta_*divgradu[0] + (1. - t_theta_)*divgradu[1]
//     - t_theta_*phit<<std::endl

//   return ut - t_theta_*f[0]
//     - (1.-t_theta_)*f[1];
  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC(residual_phase_pfhub3_n_)
{
  const double test = basis[eqn_id].phi[i];
  const double dtestdx = basis[eqn_id].dphidx[i];
  const double dtestdy = basis[eqn_id].dphidy[i];
  const double dtestdz = basis[eqn_id].dphidz[i];

  const double phi[3] = {basis[eqn_id].uu,basis[eqn_id].uuold,basis[eqn_id].uuoldold};
  const double dphidx[3] = {basis[eqn_id].dudx,basis[eqn_id].duolddx,basis[eqn_id].duoldolddx};
  const double dphidy[3] = {basis[eqn_id].dudy,basis[eqn_id].duolddy,basis[eqn_id].duoldolddy};
  const double dphidz[3] = {basis[eqn_id].dudz,basis[eqn_id].duolddz,basis[eqn_id].duoldolddz};

  const double as[3] = {a(phi[0],
			  dphidx[0],
			  dphidy[0],
			  dphidz[0],
			  eps_),
			a(phi[1],
			  dphidx[1],
			  dphidy[1],
			  dphidz[1],
			  eps_),
			a(phi[2],
			  dphidx[2],
			  dphidy[2],
			  dphidz[2],
			  eps_)};

  const double tau[3] = {tau0_*as[0]*as[0],tau0_*as[1]*as[1],tau0_*as[2]*as[2]};
//   if(tau[0]!= tau[0]) std::cout<<tau[0]<<" "<<as[0]<<" "
// 			       <<dphidx[0]<<" "<<dphidy[0]<<" "<<dphidz[0]
// 			       <<" "<<phi[0]<<" "<<phi[0]*phi[0]<<std::endl;

  const double phit = (phi[0]-phi[1])/dt_*test;

  const double w[3] = {W_*as[0],W_*as[1],W_*as[2]};

//   const double divgradphi[3] = {w[0]*w[0]*(dphidx[0]*dtestdx
// 					     + dphidy[0]*dtestdy
// 					     + dphidz[0]*dtestdz),
// 				w[1]*w[1]*(dphidx[1]*dtestdx
// 					     + dphidy[1]*dtestdy
// 					     + dphidz[1]*dtestdz),
// 				w[2]*w[2]*(dphidx[2]*dtestdx
// 					     + dphidy[2]*dtestdy
// 					     + dphidz[2]*dtestdz)};

  const double divgradphi[3] = {W_*W_*(dphidx[0]*dtestdx
					     + dphidy[0]*dtestdy
					     + dphidz[0]*dtestdz),
				W_*W_*(dphidx[1]*dtestdx
					     + dphidy[1]*dtestdy
					     + dphidz[1]*dtestdz),
				W_*W_*(dphidx[2]*dtestdx
					     + dphidy[2]*dtestdy
					     + dphidz[2]*dtestdz)};

  const double wp[3] = {W_*(ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidx[0],eps_)*dtestdx 
			    + ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidy[0],eps_)*dtestdy 
			    + ap(phi[0],dphidx[0],dphidy[0],dphidz[0],dphidz[0],eps_)*dtestdz),
			W_*(ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidx[1],eps_)*dtestdx 
			    + ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidy[1],eps_)*dtestdy 
			    + ap(phi[1],dphidx[1],dphidy[1],dphidz[1],dphidz[1],eps_)*dtestdz),
			W_*(ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidx[2],eps_)*dtestdx 
			    + ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidy[2],eps_)*dtestdy 
			    + ap(phi[2],dphidx[2],dphidy[2],dphidz[2],dphidz[2],eps_)*dtestdz)};

  const double curlgrad[3] = {w[0]*(dphidx[0]*dphidx[0] + dphidy[0]*dphidy[0] + dphidz[0]*dphidz[0])*wp[0],
			      w[1]*(dphidx[1]*dphidx[1] + dphidy[1]*dphidy[1] + dphidz[1]*dphidz[1])*wp[1],
			      w[2]*(dphidx[2]*dphidx[2] + dphidy[2]*dphidy[2] + dphidz[2]*dphidz[2])*wp[2]};

  const double g[3] = {((phi[0]-lambda_*basis[0].uu*(1.-phi[0]*phi[0]))*(1.-phi[0]*phi[0]))*test,
		       ((phi[1]-lambda_*basis[0].uuold*(1.-phi[1]*phi[1]))*(1.-phi[1]*phi[1]))*test,
		       ((phi[2]-lambda_*basis[0].uuoldold*(1.-phi[2]*phi[2]))*(1.-phi[2]*phi[2]))*test};

//   if(tau[0]!= tau[0]) std::cout<<tau[0]<<" "<<as[0]<<" "
// 			       <<dphidx[0]<<" "<<dphidy[0]<<" "<<dphidz[0]
// 			       <<" "<<phi[0]<<" "<<phi[0]*phi[0]<<" "<<g[0]<<" "<<divgradphi[0]
// 			       <<" "<<curlgrad[0]<<std::endl;

  double f[3];
  f[0] = -(divgradphi[0]/tau0_+curlgrad[0]/tau[0]-g[0]/tau[0]);
  f[1] = -(divgradphi[1]/tau0_+curlgrad[1]/tau[1]-g[1]/tau[1]);
  f[2] = -(divgradphi[2]/tau0_+curlgrad[2]/tau[2]-g[2]/tau[2]);
//   return phit - t_theta_*f[0]
//     - (1.-t_theta_)*f[1];
  return phit - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_phase_pfhub3_dp_)) = residual_phase_pfhub3_;

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_phase_pfhub3_noise_dp_)) = residual_phase_pfhub3_noise_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_heat_pfhub3_)
{
  const double ut = basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i];
  const double divgradu = D_*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
			  + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
			      + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
  return ut + t_theta_*divgradu;
}

PRE_FUNC(prec_heat_pfhub3_n_)
{
  const double ut = basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i];
  const double divgradu = D_*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
			  + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
			      + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
  return ut + t_theta_*divgradu;
}


TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_heat_pfhub3_dp_)) = prec_heat_pfhub3_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_phase_pfhub3_)
{
  const double test = basis[eqn_id].phi[i];
  const double dtestdx = basis[eqn_id].dphidx[i];
  const double dtestdy = basis[eqn_id].dphidy[i];
  const double dtestdz = basis[eqn_id].dphidz[i];

  const double phi = basis[eqn_id].uu;
  const double phit = basis[eqn_id].phi[j]/dt_*test;
//   const double as = a(phi,
// 		      basis[eqn_id].dudx,
// 		      basis[eqn_id].dudy,
// 		      basis[eqn_id].dudz,
// 		      eps_);
  const double tau = tau0_;//*as*as;

  const double divgradphi = W_*W_*(basis[eqn_id].dphidx[j]*dtestdx
				   + basis[eqn_id].dphidy[j]*dtestdy
				   + basis[eqn_id].dphidz[j]*dtestdz);

  return phit
    + t_theta_*divgradphi/tau;
}

PRE_FUNC(prec_phase_pfhub3_n_)
{
  const double test = basis[eqn_id].phi[i];
  const double dtestdx = basis[eqn_id].dphidx[i];
  const double dtestdy = basis[eqn_id].dphidy[i];
  const double dtestdz = basis[eqn_id].dphidz[i];

  const double phi = basis[eqn_id].uu;
  const double phit = basis[eqn_id].phi[j]/dt_*test;
//   const double as = tpetra::farzadi3d::a(phi,
// 					     basis[eqn_id].dudx,
// 					     basis[eqn_id].dudy,
// 					     basis[eqn_id].dudz,
// 					     eps_);
//   const double tau = tau0_*as*as;
  const double tau = tau0_;
//   const double divgradphi = W_*as*W_*as*(basis[eqn_id].dphidx[j]*dtestdx
// 					 + basis[eqn_id].dphidy[j]*dtestdy
// 					 + basis[eqn_id].dphidz[j]*dtestdz);
  const double divgradphi = W_*W_*(basis[eqn_id].dphidx[j]*dtestdx
					 + basis[eqn_id].dphidy[j]*dtestdy
					 + basis[eqn_id].dphidz[j]*dtestdz);
  return phit
    + t_theta_*divgradphi/tau;
}

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_phase_pfhub3_dp_)) = prec_phase_pfhub3_;

INI_FUNC(init_heat_pfhub3_)
{
  return delta_;
}

INI_FUNC(init_phase_pfhub3_)
{
  double val = -1.;
  const double r = sqrt(x*x+y*y+z*z);
  //if(x*x+y*y+z*z < R_*R_) val = 1.;
  //see https://aip.scitation.org/doi/pdf/10.1063/1.5142353
  //we should have a general function for this
  //val = tanh((R_-r)/(sqrt(8.)*W_));
  val = tanh((R_-r)/(sqrt(2.)*W_));


  //should probably be:
  //val = -tanh( (x*x+y*y+z*z - R_*R_)/(sqrt(2.)*W_) );
  return val;
}

}//namespace pfhub3

namespace pfhub2
{
  TUSAS_DEVICE
  const int N_MAX = 1;
  TUSAS_DEVICE
  int N_ = 1;
  TUSAS_DEVICE
  int eqn_off_ = 2;
  TUSAS_DEVICE
  int ci_ = 0;
  TUSAS_DEVICE
  int mui_ = 1;
  TUSAS_DEVICE
  const double c0_ = .5;
  TUSAS_DEVICE
  const double eps_ = .05;
  TUSAS_DEVICE
  const double eps_eta_ = .1;
  TUSAS_DEVICE
  const double psi_ = 1.5;
  TUSAS_DEVICE
  const double rho_ = 1.414213562373095;//std::sqrt(2.);
  TUSAS_DEVICE
  const double c_alpha_ = .3;
  TUSAS_DEVICE
  const double c_beta_ = .7;
  TUSAS_DEVICE
  const double alpha_ = 5.;
  TUSAS_DEVICE
  //const double k_c_ = 3.;
  const double k_c_ = 0.0;
  TUSAS_DEVICE
  const double k_eta_ = 3.;
  TUSAS_DEVICE
  const double M_ = 5.;
  TUSAS_DEVICE
  const double L_ = 5.;
  TUSAS_DEVICE
  const double w_ = 1.;
//   double c_a[2] = {0., 0.};
//   double c_b[2] = {0., 0.};

  PARAM_FUNC(param_)
  {
    int N_p = plist->get<int>("N");
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(N_,&N_p,sizeof(int));
#else
    N_ = N_p;
#endif
    int eqn_off_p = plist->get<int>("OFFSET");
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(eqn_off_,&eqn_off_p,sizeof(int));
#else
    eqn_off_ = eqn_off_p;
#endif
  }
 
  PARAM_FUNC(param_trans_)
  {
    int N_p = plist->get<int>("N");
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(N_,&N_p,sizeof(int));
#else
    N_ = N_p;
#endif
    int eqn_off_p = plist->get<int>("OFFSET");
#ifdef TUSAS_HAVE_CUDA
  cudaMemcpyToSymbol(eqn_off_,&eqn_off_p,sizeof(int));
#else
    eqn_off_ = eqn_off_p;
#endif
    ci_ = 1;
    mui_ = 0;
  }
 
KOKKOS_INLINE_FUNCTION 
  double dhdeta(const double eta)
  {
    //return 30.*eta[eqn_id]*eta[eqn_id] - 60.*eta[eqn_id]*eta[eqn_id]*eta[eqn_id] + 30.*eta[eqn_id]*eta[eqn_id]*eta[eqn_id]*eta[eqn_id];
    return 30.*eta*eta - 60.*eta*eta*eta + 30.*eta*eta*eta*eta;
  }
 
KOKKOS_INLINE_FUNCTION 
  double h(const double *eta)
  {
    double val = 0.;
    for (int i = 0; i < N_; i++){
      val += eta[i]*eta[i]*eta[i]*(6.*eta[i]*eta[i] - 15.*eta[i] + 10.);
    }
    return val;
  }
 
KOKKOS_INLINE_FUNCTION 
  double d2fdc2()
  {
    return 2.*rho_*rho_;
  }
 
KOKKOS_INLINE_FUNCTION 
  double df_alphadc(const double c)
  {
    return 2.*rho_*rho_*(c - c_alpha_);
  }
 
KOKKOS_INLINE_FUNCTION 
  double df_betadc(const double c)
  {
    return -2.*rho_*rho_*(c_beta_ - c);
  }
 
KOKKOS_INLINE_FUNCTION 
  void solve_kks(const double c, double *phi, double &ca, double &cb)//const double phi
  {
    double delta_c_a = 0.;
    double delta_c_b = 0.;
    const int max_iter = 20;
    const double tol = 1.e-8;
    const double hh = h(phi);
    //c_a[0] = (1.-hh)*c;
    ca = c - hh*(c_beta_ - c_alpha_);

    //c_b[0]=hh*c;
    cb = c - (1.-hh)*(c_beta_ - c_alpha_);

    //std::cout<<"-1"<<" "<<delta_c_b<<" "<<delta_c_a<<" "<<c_b[0]<<" "<<c_a[0]<<" "<<hh*c_b[0] + (1.- hh)*c_a[0]<<" "<<c<<std::endl;
    for(int i = 0; i < max_iter; i++){
      const double det = hh*d2fdc2() + (1.-hh)*d2fdc2();
      const double f1 = hh*cb + (1.- hh)*ca - c;
      const double f2 = df_betadc(cb) - df_alphadc(ca);
      delta_c_b = (-d2fdc2()*f1 - (1-hh)*f2)/det;
      delta_c_a = (-d2fdc2()*f1 + hh*f2)/det;
      cb = delta_c_b + cb;
      ca = delta_c_a + ca;
      //std::cout<<i<<" "<<delta_c_b<<" "<<delta_c_a<<" "<<c_b[0]<<" "<<c_a[0]<<" "<<hh*c_b[0] + (1.- hh)*c_a[0]<<" "<<c<<std::endl;
      if(delta_c_a*delta_c_a+delta_c_b*delta_c_b < tol*tol) return;
    }
//     std::cout<<"###################################  solve_kks falied to converge with delta_c_a*delta_c_a+delta_c_b*delta_c_b = "
// 	     <<delta_c_a*delta_c_a+delta_c_b*delta_c_b<<"  ###################################"<<std::endl;
    exit(0);
    return;
  }
 
KOKKOS_INLINE_FUNCTION 
  double f_alpha(const double c)
  {
    return rho_*rho_*(c - c_alpha_)*(c - c_alpha_);
  }
 
KOKKOS_INLINE_FUNCTION 
  double f_beta(const double c)
  {
    return rho_*rho_*(c_beta_ - c)*(c_beta_ - c);
  }
 
KOKKOS_INLINE_FUNCTION 
  double dgdeta(const double *eta, const int eqn_id){

    double aval =0.;
    for (int i = 0; i < N_; i++){
      aval += eta[i]*eta[i];
    }
    aval = aval - eta[eqn_id]* eta[eqn_id];
    return 2.*eta[eqn_id]*(1. - eta[eqn_id])*(1. - eta[eqn_id])  
      - 2.* eta[eqn_id]* eta[eqn_id]* (1. - eta[eqn_id])
      + 4.*alpha_*eta[eqn_id] *aval;
  }

KOKKOS_INLINE_FUNCTION 
double dfdc(const double c, const double *eta)
{
  const double hh = h(eta);
  return df_alphadc(c)*(1.-hh)+df_betadc(c)*hh;
}

KOKKOS_INLINE_FUNCTION 
double dfdeta(const double c, const double eta)
{
  //this does not include the w g' term

  //dh(eta1,eta2)/deta1 is a function of eta1 only
  const double dh_deta = dhdeta(eta);
  return f_alpha(c)*(-dh_deta)+f_beta(c)*dh_deta;
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_c_)
{
  // c_t + M grad mu grad test
  const double ut = (basis[ci_]->uu-basis[ci_]->uuold)/dt_*basis[eqn_id]->phi[i];
  //M_ divgrad mu

  const double f[3] = {M_*(basis[mui_]->dudx*basis[eqn_id]->dphidx[i]
			   + basis[mui_]->dudy*basis[eqn_id]->dphidy[i]
			   + basis[mui_]->dudz*basis[eqn_id]->dphidz[i]),
		       M_*(basis[mui_]->duolddx*basis[eqn_id]->dphidx[i]
			   + basis[mui_]->duolddy*basis[eqn_id]->dphidy[i]
			   + basis[mui_]->duolddz*basis[eqn_id]->dphidz[i]),
		       M_*(basis[mui_]->duoldolddx*basis[eqn_id]->dphidx[i]
			   + basis[mui_]->duoldolddy*basis[eqn_id]->dphidy[i]
			   + basis[mui_]->duoldolddz*basis[eqn_id]->dphidz[i])};

  return ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_c_dp_)) = residual_c_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_c_kks_)
{
  //derivatives of the test function
  const double dtestdx = basis[0]->dphidx[i];
  const double dtestdy = basis[0]->dphidy[i];
  //double dtestdz = basis[0]->dphidz[i];
  //test function
  const double test = basis[0]->phi[i];
  //u, phi
  const double c[2] = {basis[0]->uu, basis[0]->uuold};
  const double dcdx[2] = {basis[0]->dudx, basis[0]->duolddx};
  const double dcdy[2] = {basis[0]->dudy, basis[0]->duolddy};

  double dhdx[2] = {0., 0.};
  double dhdy[2] = {0., 0.};

  double c_a[2] = {0., 0.};
  double c_b[2] = {0., 0.};

  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    dhdx[0] += dhdeta(basis[kk_off]->uu)*basis[kk_off]->dudx;
    dhdx[1] += dhdeta(basis[kk_off]->uuold)*basis[kk_off]->duolddx;
    dhdy[0] += dhdeta(basis[kk_off]->uu)*basis[kk_off]->dudy;
    dhdy[1] += dhdeta(basis[kk_off]->uuold)*basis[kk_off]->duolddy;
  }

  const double ct = (c[0]-c[1])/dt_*test;

  double eta_array[N_MAX];
  double eta_array_old[N_MAX];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off]->uu;
    eta_array_old[kk] = basis[kk_off]->uuold;
  }

  solve_kks(c[0],eta_array,c_a[0],c_b[0]);

  const double DfDc[2] = {-c_b[0] + c_a[0],
 		    -c_b[1] + c_a[1]};

  const double D2fDc2 = 1.;
  //double D2fDc2 = 1.*d2fdc2();

  const double dfdx[2] = {DfDc[0]*dhdx[0] + D2fDc2*dcdx[0],
		    DfDc[1]*dhdx[1] + D2fDc2*dcdx[1]};
  const double dfdy[2] = {DfDc[0]*dhdy[0] + D2fDc2*dcdy[0],
		    DfDc[1]*dhdy[1] + D2fDc2*dcdy[1]};

  const double divgradc[2] = {M_*(dfdx[0]*dtestdx + dfdy[0]*dtestdy),
			M_*(dfdx[1]*dtestdx + dfdy[1]*dtestdy)};

  return ct + t_theta_*divgradc[0] + (1.-t_theta_)*divgradc[1];
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_c_kks_dp_)) = residual_c_kks_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_eta_)
{
  //test function
  const double test = basis[eqn_id]->phi[i];
  //u, phi
  const double c[3] = {basis[ci_]->uu, basis[ci_]->uuold, basis[ci_]->uuoldold};

  const double eta[3] = {basis[eqn_id]->uu, basis[eqn_id]->uuold, basis[eqn_id]->uuoldold};

  const double divgradeta[3] = {L_*k_eta_*(basis[eqn_id]->dudx*basis[eqn_id]->dphidx[i]
				    + basis[eqn_id]->dudy*basis[eqn_id]->dphidy[i]
				    + basis[eqn_id]->dudz*basis[eqn_id]->dphidz[i]),
				L_*k_eta_*(basis[eqn_id]->duolddx*basis[eqn_id]->dphidx[i]
				    + basis[eqn_id]->duolddy*basis[eqn_id]->dphidy[i]
				    + basis[eqn_id]->duolddz*basis[eqn_id]->dphidz[i]),
				L_*k_eta_*(basis[eqn_id]->duoldolddx*basis[eqn_id]->dphidx[i]
				    + basis[eqn_id]->duoldolddy*basis[eqn_id]->dphidy[i]
				    + basis[eqn_id]->duoldolddz*basis[eqn_id]->dphidz[i])};

  double eta_array[N_MAX];
  double eta_array_old[N_MAX];
  double eta_array_oldold[N_MAX];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off]->uu;
    eta_array_old[kk] = basis[kk_off]->uuold;
    eta_array_oldold[kk] = basis[kk_off]->uuoldold;
  }
  const int k = eqn_id - eqn_off_;

  const double df_deta[3] = {L_*(dfdeta(c[0],eta[0]) + w_*dgdeta(eta_array,k))*test,
			     L_*(dfdeta(c[1],eta[1]) + w_*dgdeta(eta_array_old,k))*test,
			     L_*(dfdeta(c[2],eta[2]) + w_*dgdeta(eta_array_oldold,k))*test};
  
  const double f[3] = {df_deta[0] + divgradeta[0],
		       df_deta[1] + divgradeta[1],
		       df_deta[2] + divgradeta[2]};

  const double ut = (eta[0]-eta[1])/dt_*test;

  return ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_eta_dp_)) = residual_eta_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_eta_kks_)
{

  //derivatives of the test function
  const double dtestdx = basis[eqn_id]->dphidx[i];
  const double dtestdy = basis[eqn_id]->dphidy[i];
  //double dtestdz = basis[0].dphidz[i];
  //test function
  const double test = basis[eqn_id]->phi[i];
  //u, phi
  const double c[2] = {basis[0]->uu, basis[0]->uuold};

  const double eta[2] = {basis[eqn_id]->uu, basis[eqn_id]->uuold};
  const double detadx[2] = {basis[eqn_id]->dudx, basis[eqn_id]->duolddx};
  const double detady[2] = {basis[eqn_id]->dudy, basis[eqn_id]->duolddy};

  double c_a[2] = {0., 0.};
  double c_b[2] = {0., 0.};

  double eta_array[N_MAX];
  double eta_array_old[N_MAX];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off]->uu;
    eta_array_old[kk] = basis[kk_off]->uuold;
  }

  solve_kks(c[0],eta_array,c_a[0],c_b[0]);

  const double etat = (eta[0]-eta[1])/dt_*test;


  const double F[2] = {f_beta(c_b[0]) - f_alpha(c_a[0]) 
		 - (c_b[0] - c_a[0])*df_betadc(c_b[0]),
		 f_beta(c_b[1]) - f_alpha(c_a[1]) 
		 - (c_b[1] - c_a[1])*df_betadc(c_b[1])};

  const int k = eqn_id - eqn_off_;
  const double dfdeta[2] = {L_*(F[0]*dhdeta(eta[0]) 
				    + w_*dgdeta(eta_array,k)    )*test,
		      L_*(F[1]*dhdeta(eta[1]) 
				    + w_*dgdeta(eta_array_old,k))*test};

  const double divgradeta[2] = {L_*k_eta_
			  *(detadx[0]*dtestdx + detady[0]*dtestdy), 
			  L_*k_eta_
			  *(detadx[1]*dtestdx + detady[1]*dtestdy)};//(grad u,grad phi)
 
  return etat + t_theta_*divgradeta[0] + t_theta_*dfdeta[0] + (1.-t_theta_)*divgradeta[1] + (1.-t_theta_)*dfdeta[1];
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_eta_kks_dp_)) = residual_eta_kks_;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_mu_)
{
  //-mu + df/dc +div c grad test
  const double c = basis[ci_]->uu;
  const double mu = basis[mui_]->uu;
  //const double eta = basis[2]->uu;
  const double test = basis[eqn_id]->phi[i];

  const double divgradc = k_c_*(basis[ci_]->dudx*basis[eqn_id]->dphidx[i]
				+ basis[ci_]->dudy*basis[eqn_id]->dphidy[i]
				+ basis[ci_]->dudz*basis[eqn_id]->dphidz[i]);

  double eta_array[N_MAX];
  for( int kk = 0; kk < N_; kk++){
    int kk_off = kk + eqn_off_;
    eta_array[kk] = basis[kk_off]->uu;
  };

  const double df_dc = dfdc(c,eta_array)*test;

  //return dt_*(-mu*test + df_dc + divgradc);
  return -mu*test + df_dc + divgradc;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_mu_dp_)) = residual_mu_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_c_)
{
  //const int ci = 1;
  //const int mui = 0;
  const double D = k_c_;
  const double test = basis[eqn_id].phi[i];
  //const double u_t =test * basis[eqn_id].phi[j]/dt_;
  const double divgrad = D*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
  const double d2 = d2fdc2()*basis[eqn_id].phi[j]*test;
  return divgrad + d2;
}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_mu_)
{
  const double D = M_;
  //const double test = basis[eqn_id].phi[i];
  //const double u_t =test * basis[eqn_id].phi[j]/dt_;
  const double divgrad = D*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
  return divgrad;
}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_eta_)
{
  const double ut = basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i];
  const double divgrad = L_*k_eta_*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
//   const double eta = basis[eqn_id].uu;
//   const double c = basis[0].uu;
//   const double g1 = L_*(2. - 12.*eta + 12.*eta*eta)*basis[eqn_id].phi[j]*basis[eqn_id].phi[i];
//   const double h1 = L_*(-f_alpha(c)+f_beta(c))*(60.*eta-180.*eta*eta+120.*eta*eta*eta)*basis[eqn_id].phi[j]*basis[eqn_id].phi[i];
  return ut + t_theta_*divgrad;// + t_theta_*(g1+h1);
}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_ut_)
{
  const double test = basis[eqn_id].phi[i];
  const double u_t =test * basis[eqn_id].phi[j]/dt_;
  return u_t;
}

PPR_FUNC(postproc_c_a_)
{

  //cn will need eta_array here...
  const double cc = u[0];
  double phi = u[2];
  double c_a = 0.;
  double c_b = 0.;

  solve_kks(cc,&phi,c_a,c_b);

  return c_a;
}

PPR_FUNC(postproc_c_b_)
{
  //cn will need eta_array here...
  const double cc = u[0];
  double phi = u[2];
  double c_a = 0.;
  double c_b = 0.;

  solve_kks(cc,&phi,c_a,c_b);

  return c_b;
}

INI_FUNC(init_mu_)
{
  //this will need the eta_array version
  const double c = ::pfhub2::init_c_(x,y,z,eqn_id);
  const double eta = ::pfhub2::init_eta_(x,y,z,2);
  return dfdc(c,&eta);
//   return 0.;
}
INI_FUNC(init_eta_)
{
  return ::pfhub2::init_eta_(x,y,z,eqn_id-1);
}

}//namespace pfhub2
namespace cahnhilliard
{
  //this has an mms described at:
  //https://www.sciencedirect.com/science/article/pii/S0021999112007243

  //residual_*_ is a traditional formulation for [c mu] with
  //R_c = c_t - divgrad mu
  //R_mu = -mu + df/dc - divgrad c

  //residual_*_trans_ utilizes a transformation as [mu c] with
  //R_mu = c_t - divgrad mu
  //R_c = -mu + df/dc - divgrad c
  //that puts the elliptic terms on the diagonal, with better solver convergence and 
  //potential for preconditioning
  //the transformation is inspired by:
  //https://web.archive.org/web/20220201192736id_/https://publikationen.bibliothek.kit.edu/1000141249/136305383
  //https://www.sciencedirect.com/science/article/pii/S037704271930319X

  //right now, the preconditioner can probably be improved by scaling c by dt

  double M = 1.;
  double Eps = 1.;
  double alpha = 1.;//alpha >= 1
  double pi = 3.141592653589793;
  double fcoef_ = 0.;

double F(const double &x,const double &t)
{
// Sin(a*Pi*x) 
//  - M*(Power(a,2)*Power(Pi,2)*(1 + t)*Sin(a*Pi*x) - Power(a,4)*Ep*Power(Pi,4)*(1 + t)*Sin(a*Pi*x) + 
//       6*Power(a,2)*Power(Pi,2)*Power(1 + t,3)*Power(Cos(a*Pi*x),2)*Sin(a*Pi*x) - 
//       3*Power(a,2)*Power(Pi,2)*Power(1 + t,3)*Power(Sin(a*Pi*x),3))

  double a = alpha;
  return sin(a*pi*x) 
    - M*(std::pow(a,2)*std::pow(pi,2)*(1 + t)*sin(a*pi*x) - std::pow(a,4)*Eps*std::pow(pi,4)*(1 + t)*sin(a*pi*x) + 
	 6*std::pow(a,2)*std::pow(pi,2)*std::pow(1 + t,3)*std::pow(cos(a*pi*x),2)*sin(a*pi*x) - 
	 3*std::pow(a,2)*std::pow(pi,2)*std::pow(1 + t,3)*std::pow(sin(a*pi*x),3));
}
double fp(const double &u)
{
  return u*u*u - u;
}

INI_FUNC(init_c_)
{
  return sin(alpha*pi*x);
}

INI_FUNC(init_mu_)
{
  //-Sin[a \[Pi] x] + a^2 \[Pi]^2 Sin[a \[Pi] x] + Sin[a \[Pi] x]^3
  return -sin(alpha*pi*x) + alpha*alpha*pi*pi*sin(alpha*pi*x) + sin(alpha*pi*x)*sin(alpha*pi*x)*sin(alpha*pi*x);
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_c_)
{
  //derivatives of the test function
  double dtestdx = basis[0]->dphidx[i];
  double dtestdy = basis[0]->dphidy[i];
  double dtestdz = basis[0]->dphidz[i];
  //test function
  double test = basis[0]->phi[i];
  double c = basis[0]->uu;
  double cold = basis[0]->uuold;
  double mu = basis[1]->uu;
  double x = basis[0]->xx;

  double ct = (c - cold)/dt_*test;
  double divgradmu = M*t_theta_*(basis[1]->dudx*dtestdx + basis[1]->dudy*dtestdy + basis[1]->dudz*dtestdz)
    + M*(1.-t_theta_)*(basis[1]->duolddx*dtestdx + basis[1]->duolddy*dtestdy + basis[1]->duolddz*dtestdz);
  double f = t_theta_*fcoef_*F(x,time)*test + (1.-t_theta_)*fcoef_*F(x,time-dt_)*test;

  return ct + divgradmu - f;
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_mu_)
{
  //derivatives of the test function
  double dtestdx = basis[0]->dphidx[i];
  double dtestdy = basis[0]->dphidy[i];
  double dtestdz = basis[0]->dphidz[i];
  //test function
  double test = basis[1]->phi[i];
  double c = basis[0]->uu;
  double mu = basis[1]->uu;

  double mut = mu*test;
  double f = fp(c)*test;
  double divgradc = Eps*(basis[0]->dudx*dtestdx + basis[0]->dudy*dtestdy + basis[0]->dudz*dtestdz);

  return -mut + f + divgradc;
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_mu_trans_)
{
  //derivatives of the test function
  double dtestdx = basis[0]->dphidx[i];
  double dtestdy = basis[0]->dphidy[i];
  double dtestdz = basis[0]->dphidz[i];
  //test function
  double test = basis[0]->phi[i];
  double c = basis[1]->uu;
  double cold = basis[1]->uuold;
  //double mu = basis[1]->uu;
  double x = basis[0]->xx;

  double ct = (c - cold)/dt_*test;
  double divgradmu = M*t_theta_*(basis[0]->dudx*dtestdx + basis[0]->dudy*dtestdy + basis[0]->dudz*dtestdz)
    + M*(1.-t_theta_)*(basis[0]->duolddx*dtestdx + basis[0]->duolddy*dtestdy + basis[0]->duolddz*dtestdz);
  double f = t_theta_*fcoef_*F(x,time)*test + (1.-t_theta_)*fcoef_*F(x,time-dt_)*test;

  return ct + divgradmu - f;
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_c_trans_)
{
  //derivatives of the test function
  double dtestdx = basis[0]->dphidx[i];
  double dtestdy = basis[0]->dphidy[i];
  double dtestdz = basis[0]->dphidz[i];
  //test function
  double test = basis[1]->phi[i];
  double c = basis[1]->uu;
  double mu = basis[0]->uu;

  double mut = mu*test;
  double f = fp(c)*test;
  double divgradc = Eps*(basis[1]->dudx*dtestdx + basis[1]->dudy*dtestdy + basis[1]->dudz*dtestdz);

  return -mut + f + divgradc;
}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_mu_trans_)
{
  const double divgradmu = M*t_theta_*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
  return divgradmu;
}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_c_trans_)
{
  const double divgradc = Eps*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);
  return divgradc;
}

PARAM_FUNC(param_)
{
  fcoef_ = plist->get<double>("fcoef");
}
}//namespace cahnhilliard


namespace robin
{
  //  http://ramanujan.math.trinity.edu/rdaileda/teach/s12/m3357/lectures/lecture_2_28_short.pdf
  // 1-D robin bc test problem, time dependent
  // Solve D[u, t] - c^2 D[u, x, x] == 0
  // u(0,t) == 0
  // D[u, x] /. x -> L == -kappa u(t,L)
  // => du/dx + kappa u = g = 0
  // u(x,t) = a E^(-mu^2 t) Sin[mu x]
  // mu solution to: Tan[mu L] + mu/kappa == 0 && Pi/2 < mu < 3 Pi/2
  const double mu = 2.028757838110434;
  const double a = 10.;
  const double c = 1.;
  const double L = 1.;
  const double kappa = 1.;

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_robin_test_)
{
  //1-D robin bc test problem, 

  //derivatives of the test function
  const double dtestdx = basis[0]->dphidx[i];
  const double dtestdy = basis[0]->dphidy[i];
  const double dtestdz = basis[0]->dphidz[i];
  //test function
  const double test = basis[0]->phi[i];
  //u, phi
  const double u = basis[0]->uu;
  const double uold = basis[0]->uuold;

  //double a =10.;

  const double ut = (u-uold)/dt_*test;
 
  const double f[3] = {c*c*(basis[0]->dudx*dtestdx + basis[0]->dudy*dtestdy + basis[0]->dudz*dtestdz),
		       c*c*(basis[0]->duolddx*dtestdx + basis[0]->duolddy*dtestdy + basis[0]->duolddz*dtestdz),
		       c*c*(basis[0]->duoldolddx*dtestdx + basis[0]->duoldolddy*dtestdy + basis[0]->duoldolddz*dtestdz)};
  return ut + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_robin_test_dp_)) = residual_robin_test_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_robin_test_)
{
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  const double dtestdx = basis[0].dphidx[i];
  const double dtestdy = basis[0].dphidy[i];
  const double dtestdz = basis[0].dphidz[i];

  const double dbasisdx = basis[0].dphidx[j];
  const double dbasisdy = basis[0].dphidy[j];
  const double dbasisdz = basis[0].dphidz[j];
  const double test = basis[0].phi[i];
  const double divgrad = c*c*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  //double a =10.;
  const double u_t = (basis[0].phi[j])/dt_*test;
  return u_t + t_theta_*divgrad;
}

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_robin_test_dp_)) = prec_robin_test_;

NBC_FUNC_TPETRA(nbc_robin_test_)
{

  const double test = basis[0].phi[i];

  //du/dn + kappa u = g = 0 on L
  //(du,dv) - <du/dn,v> = (f,v)
  //(du,dv) - <g - kappa u,v> = (f,v)
  //(du,dv) - < - kappa u,v> = (f,v)
  //          ^^^^^^^^^^^^^^ return this
  const double f[3] = {-kappa*basis[0].uu*test,
		       -kappa*basis[0].uuold*test,
		       -kappa*basis[0].uuoldold*test};
  return (1.-t_theta2_)*t_theta_*f[0]
    +(1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}
INI_FUNC(init_robin_test_)
{
  return a*sin(mu*x);
}
PPR_FUNC(postproc_robin_)
{
  const double uu = u[0];
  const double x = xyz[0];
  //const double y = xyz[1];
  //const double z = xyz[2];

  const double s= a*exp(-mu*mu*time)*sin(mu*x);//c?

  return s-uu;
}
}//namespace robin

namespace autocatalytic4
{

  //https://documen.site/download/math-3795-lecture-18-numerical-solution-of-ordinary-differential-equations-goals_pdf#
  //https://media.gradebuddy.com/documents/2449908/0c88cf76-7605-4aec-b2ad-513ddbebefec.pdf

const double k1 = .0001;
const double k2 = 1.;
const double k3 = .0008;

RES_FUNC_TPETRA(residual_a_)
{
  const double test = basis[0]->phi[i];
  //u, phi
  const double u = basis[0]->uu;
  const double uold = basis[0]->uuold;
  const double uoldold = basis[0]->uuoldold;

  const double ut = (u-uold)/dt_*test;
  //std::cout<<ut<<" "<<dt_<<" "<<time<<std::endl;
 
  double f[3];
  f[0] = (-k1*u       - k2*u*basis[1]->uu)*test;
  f[1] = (-k1*uold    - k2*u*basis[1]->uuold)*test;
  f[2] = (-k1*uoldold - k2*u*basis[1]->uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC_TPETRA(residual_b_)
{
  const double test = basis[1]->phi[i];
  //u, phi
  const double u = basis[1]->uu;
  const double uold = basis[1]->uuold;
  //const double uoldold = basis[1]->uuoldold;
  const double a = basis[0]->uu;
  const double aold = basis[0]->uuold;
  const double aoldold = basis[0]->uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k1*a       - k2*a*u                        + 2.*k3*basis[2]->uu)*test;
  f[1] = (k1*aold    - k2*aold*uold                  + 2.*k3*basis[2]->uuold)*test;
  f[2] = (k1*aoldold - k2*aoldold*basis[1]->uuoldold + 2.*k3*basis[2]->uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC_TPETRA(residual_ab_)
{
  const double test = basis[1]->phi[i];
  //u, phi
  const double u = basis[2]->uu;
  const double uold = basis[2]->uuold;
  //const double uoldold = basis[1]->uuoldold;
  const double b = basis[1]->uu;
  const double bold = basis[1]->uuold;
  const double boldold = basis[1]->uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k2*b*basis[0]->uu             - k3*u)*test;
  f[1] = (k2*bold*basis[0]->uuold       - k3*uold)*test;
  f[2] = (k2*boldold*basis[0]->uuoldold - k3*basis[2]->uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

RES_FUNC_TPETRA(residual_c_)
{
  const double test = basis[1]->phi[i];
  //u, phi
  const double u = basis[3]->uu;
  const double uold = basis[3]->uuold;
  //const double uoldold = basis[1]->uuoldold;
  const double a = basis[0]->uu;
  const double aold = basis[0]->uuold;
  const double aoldold = basis[0]->uuoldold;

  const double ut = (u-uold)/dt_*test;
  double f[3];
  f[0] = (k1*a       + k3*basis[2]->uu)*test;
  f[1] = (k1*aold    + k3*basis[2]->uuold)*test;
  f[2] = (k1*aoldold + k3*basis[2]->uuoldold)*test;

  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

}//namespace autocatalytic4

namespace timeonly
{
const double pi = 3.141592653589793;
  //const double lambda = 10.;//pi*pi;
const double lambda = pi*pi;

const double ff(const double &u)
{
  return -lambda*u;
}

RES_FUNC_TPETRA(residual_test_)
{
  //test function
  const double test = basis[0]->phi[i];
  //u, phi
  const double u[3] = {basis[0]->uu,basis[0]->uuold,basis[0]->uuoldold};

  const double ut = (u[0]-u[1])/dt_*test;

  const double f[3] = {ff(u[0])*test,ff(u[1])*test,ff(u[2])*test};
 
  return ut - (1.-t_theta2_)*t_theta_*f[0]
    - (1.-t_theta2_)*(1.-t_theta_)*f[1]
    -.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}
}//namespace timeonly

namespace radconvbc
{
  double h = 50.;
  double ep = .7;
  double sigma = 5.67037e-9;
  double ti = 323.;

  double tau0_h = 1.;
  double W0_h = 1.;
  
  double deltau_h = 1.;
  double uref_h = 0.;

  double scaling_constant = 1.0;

DBC_FUNC(dbc_) 
{
  return 1173.;
}

NBC_FUNC_TPETRA(nbc_)
{
  //https://reference.wolfram.com/language/PDEModels/tutorial/HeatTransfer/HeatTransfer.html#2048120463
  //h(t-ti)+\ep\sigma(t^4-ti^4) = -g(t)
  //du/dn = g
  //return g*test here

  //std::cout<<h<<" "<<ep<<" "<<sigma<<" "<<ti<<std::endl;
  const double test = basis[0].phi[i];
  const double u = deltau_h*basis[0].uu+uref_h; // T=deltau_h*theta+uref_h
  const double uold = deltau_h*basis[0].uuold+uref_h;
  const double uoldold = deltau_h*basis[0].uuoldold+uref_h;
#if 1
  const double f[3] = {(h*(ti-u)+ep*sigma*(ti*ti*ti*ti-u*u*u*u))*test,
		       (h*(ti-uold)+ep*sigma*(ti*ti*ti*ti-uold*uold*uold*uold))*test,
		       (h*(ti-uoldold)+ep*sigma*(ti*ti*ti*ti-uoldold*uoldold*uoldold*uoldold))*test};
#else
  const double c = h+4.*ep*sigma*ti*ti*ti;
  const double f[3] = {(c*(ti-u))*test,
		       (c*(ti-uold))*test,
		       (c*(ti-uoldold))*test};
#endif  
  const double coef = deltau_h / W0_h;
  //std::cout<<f[0]<<" "<<f[1]<<" "<<f[2]<<std::endl;
  const double rv = (1.-t_theta2_)*t_theta_*f[0]
    +(1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
  
  return f[0] * coef * scaling_constant;
}

INI_FUNC(init_heat_)
{
  return 1173.;
}

PARAM_FUNC(param_)
{
  h = plist->get<double>("h_",50.);
  ep = plist->get<double>("ep_",.7);
  sigma = plist->get<double>("sigma_",5.67037e-9);
  ti = plist->get<double>("ti_",323.);
  deltau_h = plist->get<double>("deltau_",1.);
  uref_h = plist->get<double>("uref_",0.); 

  W0_h = plist->get<double>("W0_",1.);

  scaling_constant = plist->get<double>("scaling_constant_",1.);
//   std::cout<<"tpetra::radconvbc::param_:"<<std::endl
// 	   <<"  h     = "<<h<<std::endl
// 	   <<"  ep    = "<<ep<<std::endl
// 	   <<"  sigma = "<<sigma<<std::endl
// 	   <<"  ti    = "<<ti<<std::endl<<std::endl;
}
}//namespace radconvbc

namespace goldak{
TUSAS_DEVICE
const double pi_d = 3.141592653589793;

double te = 1641.;
double tl = 1706.;
double Lf = 2.95e5;

TUSAS_DEVICE
double dfldu_mushy_d = tpetra::heat::rho_d*Lf/(tl-te);//fl=(t-te)/(tl-te);

TUSAS_DEVICE
double eta_d = 0.3;
TUSAS_DEVICE
double P_d = 50.;
TUSAS_DEVICE
double s_d = 2.;
TUSAS_DEVICE
double r_d = .00005;
TUSAS_DEVICE
double d_d = .00001;
TUSAS_DEVICE
double gamma_d = 0.886227;
TUSAS_DEVICE
double x0_d = 0.;
TUSAS_DEVICE
double y0_d = 0.;
TUSAS_DEVICE
double z0_d = 0.;
TUSAS_DEVICE
double t_hold_d = 0.005;
TUSAS_DEVICE
double t_decay_d = 0.01;
TUSAS_DEVICE
double tau0_d = 1.;
TUSAS_DEVICE
double W0_d = 1.;

TUSAS_DEVICE
double t0_d = 300.;

TUSAS_DEVICE
double scaling_constant_d = 1.;

KOKKOS_INLINE_FUNCTION 
void dfldt_uncoupled(GPUBasis * basis[], const int index, const double dt_, const double dtold_, double *a)
{
  //the latent heat term is zero outside of the mushy region (ie outside Te < T < Tl)

  //we need device versions of deltau_h,uref_h
  const double coef = 1./tau0_d;

  const double tt[3] = {tpetra::heat::deltau_h*basis[index]->uu+tpetra::heat::uref_h,
			tpetra::heat::deltau_h*basis[index]->uuold+tpetra::heat::uref_h,
			tpetra::heat::deltau_h*basis[index]->uuoldold+tpetra::heat::uref_h};
  const double dfldu_d[3] = {((tt[0] > te) && (tt[0] < tl)) ? coef*dfldu_mushy_d : 0.0,
			     ((tt[1] > te) && (tt[1] < tl)) ? coef*dfldu_mushy_d : 0.0,
			     ((tt[2] > te) && (tt[2] < tl)) ? coef*dfldu_mushy_d : 0.0};

  a[0] = ((1. + dt_/dtold_)*(dfldu_d[0]*basis[index]->uu-dfldu_d[1]*basis[index]->uuold)/dt_
                                 -dt_/dtold_*(dfldu_d[0]*basis[index]->uu-dfldu_d[2]*basis[index]->uuoldold)/(dt_+dtold_)
                                 );
  a[1] = (dtold_/dt_/(dt_+dtold_)*(dfldu_d[0]*basis[index]->uu)
                                 -(dtold_-dt_)/dt_/dtold_*(dfldu_d[1]*basis[index]->uuold)
                                 -dt_/dtold_/(dt_+dtold_)*(dfldu_d[2]*basis[index]->uuoldold)
                                 );
  a[2] = (-(1.+dtold_/dt_)*(dfldu_d[2]*basis[index]->uuoldold-dfldu_d[1]*basis[index]->uuold)/dtold_
                                 +dtold_/dt_*(dfldu_d[2]*basis[index]->uuoldold-dfldu_d[0]*basis[index]->uu)/(dtold_+dt_)
                                 );
  return;
}

KOKKOS_INLINE_FUNCTION 
void dfldt_coupled(GPUBasis * basis[], const int index, const double dt_, const double dtold_, double *a)
{
  const double coef = tpetra::heat::rho_d*tpetra::goldak::Lf/tau0_d;
  const double dfldu_d[3] = {-.5*coef,-.5*coef,-.5*coef};

  a[0] = ((1. + dt_/dtold_)*(dfldu_d[0]*basis[index]->uu-dfldu_d[1]*basis[index]->uuold)/dt_
                                 -dt_/dtold_*(dfldu_d[0]*basis[index]->uu-dfldu_d[2]*basis[index]->uuoldold)/(dt_+dtold_)
                                 );
  a[1] = (dtold_/dt_/(dt_+dtold_)*(dfldu_d[0]*basis[index]->uu)
                                 -(dtold_-dt_)/dt_/dtold_*(dfldu_d[1]*basis[index]->uuold)
                                 -dt_/dtold_/(dt_+dtold_)*(dfldu_d[2]*basis[index]->uuoldold)
                                 );
  a[2] = (-(1.+dtold_/dt_)*(dfldu_d[2]*basis[index]->uuoldold-dfldu_d[1]*basis[index]->uuold)/dtold_
                                 +dtold_/dt_*(dfldu_d[2]*basis[index]->uuoldold-dfldu_d[0]*basis[index]->uu)/(dtold_+dt_)
                                 );
  return;
}

KOKKOS_INLINE_FUNCTION 
const double P(const double t)
{
  // t is nondimensional
  // t_hold, t_decay, and tt are dimensional
  
  const double t_hold = t_hold_d;
  const double t_decay = t_decay_d;
  const double tt = t*tau0_d;
  return (tt < t_hold) ? P_d : 
    ((tt<t_hold+t_decay) ? P_d*((t_hold+t_decay)-tt)/(t_decay)
     :0.);
}

KOKKOS_INLINE_FUNCTION 
const double qdot(const double &x, const double &y, const double &z, const double &t)
{
  // x, y, z, and t are nondimensional values
  // r, d, and p and dimensional
  // Qdot as a whole has dimensions, but that's ok since it's written in terms of non-dimensional (x,y,z,t)

  const double p = P(t);
  const double r = r_d;
  const double d = d_d;
  
  //s_d = 2 below; we can simplify this expression 5.19615=3^1.5
  const double coef = eta_d*p*5.19615/r/r/d/gamma_d/pi_d;
  const double exparg = ((W0_d*x-x0_d)*(W0_d*x-x0_d)+(W0_d*y-y0_d)*(W0_d*y-y0_d))/r/r+(W0_d*z-z0_d)*(W0_d*z-z0_d)/d/d;
  const double f = exp( -3.* exparg );

  return coef*f;
}

KOKKOS_INLINE_FUNCTION
RES_FUNC_TPETRA(residual_test_)
{
  //u_t,v + grad u,grad v - qdot,v = 0

  double val = tpetra::heat::residual_heat_test_dp_(basis,
						    i,
						    dt_,
						    dtold_,
						    t_theta_,
						    t_theta2_,
						    time,
						    eqn_id,
						    vol,
						    rand);

  const double qd[3] = {-qdot(basis[eqn_id]->xx,basis[eqn_id]->yy,basis[eqn_id]->zz,time)*basis[eqn_id]->phi[i],
			-qdot(basis[eqn_id]->xx,basis[eqn_id]->yy,basis[eqn_id]->zz,time-dt_)*basis[eqn_id]->phi[i],
			-qdot(basis[eqn_id]->xx,basis[eqn_id]->yy,basis[eqn_id]->zz,time-dt_-dtold_)*basis[eqn_id]->phi[i]};

  const double rv = (val 
		     + (1.-t_theta2_)*t_theta_*qd[0]
		     + (1.-t_theta2_)*(1.-t_theta_)*qd[1]
		     +.5*t_theta2_*((2.+dt_/dtold_)*qd[1]-dt_/dtold_*qd[2]));

  return rv;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_test_dp_)) = residual_test_;

KOKKOS_INLINE_FUNCTION
RES_FUNC_TPETRA(residual_uncoupled_test_)
{
  //u_t,v + grad u,grad v + dfldt,v - qdot,v = 0

  double val = tpetra::goldak::residual_test_dp_(basis,
						 i,
						 dt_,
						 dtold_,
						 t_theta_,
						 t_theta2_,
						 time,
						 eqn_id,
						 vol,
						 rand);

  double dfldu_d[3];
  dfldt_uncoupled(basis,eqn_id,dt_,dtold_,dfldu_d);

  const double dfldt[3] = {dfldu_d[0]*basis[eqn_id]->phi[i],
			   dfldu_d[1]*basis[eqn_id]->phi[i],
			   dfldu_d[2]*basis[eqn_id]->phi[i]};
  
  const double rv = (val 
		     + (1.-t_theta2_)*t_theta_*dfldt[0]
		     + (1.-t_theta2_)*(1.-t_theta_)*dfldt[1]
		     +.5*t_theta2_*((2.+dt_/dtold_)*dfldt[1]-dt_/dtold_*dfldt[2]));

  return rv * scaling_constant_d;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_uncoupled_test_dp_)) = residual_uncoupled_test_;

KOKKOS_INLINE_FUNCTION
RES_FUNC_TPETRA(residual_coupled_test_)
{
  //u_t,v + grad u,grad v + dfldt,v - qdot,v = 0

  double val = tpetra::goldak::residual_test_dp_(basis,
						 i,
						 dt_,
						 dtold_,
						 t_theta_,
						 t_theta2_,
						 time,
						 eqn_id,
						 vol,
						 rand);

  int phi_index = 1;
  double dfldu_d[3];
  dfldt_coupled(basis,phi_index,dt_,dtold_,dfldu_d);

  const double dfldt[3] = {dfldu_d[0]*basis[eqn_id]->phi[i],
			   dfldu_d[1]*basis[eqn_id]->phi[i],
			   dfldu_d[2]*basis[eqn_id]->phi[i]};
  
  const double rv = (val 
		     + (1.-t_theta2_)*t_theta_*dfldt[0]
		     + (1.-t_theta2_)*(1.-t_theta_)*dfldt[1]
		     +.5*t_theta2_*((2.+dt_/dtold_)*dfldt[1]-dt_/dtold_*dfldt[2]));
  
  return rv * scaling_constant_d;
}

TUSAS_DEVICE
RES_FUNC_TPETRA((*residual_coupled_test_dp_)) = residual_coupled_test_;

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(prec_test_)
{
  
  const double val = tpetra::heat::prec_heat_test_dp_(basis,
						      i,
						      j,
						      dt_,
						      t_theta_,
						      eqn_id);

  return val * scaling_constant_d;
}

TUSAS_DEVICE
PRE_FUNC_TPETRA((*prec_test_dp_)) = prec_test_;

INI_FUNC(init_heat_)
{
  const double t_preheat = t0_d;
  const double val = (t_preheat-tpetra::heat::uref_h)/tpetra::heat::deltau_h;
  return val;
}

DBC_FUNC(dbc_) 
{
  // The assumption here is that the desired Dirichlet BC is the initial temperature,
  // that may not be true in the future.
  const double t_preheat = t0_d;
  const double val = (t_preheat-tpetra::heat::uref_h)/tpetra::heat::deltau_h;
  return val;
}

PPR_FUNC(postproc_qdot_)
{
  const double x = xyz[0];
  const double y = xyz[1];
  const double z = xyz[2];

  return qdot(x,y,z,time);
}

PPR_FUNC(postproc_u_)
{
  return u[0]*tpetra::heat::deltau_h + tpetra::heat::uref_h;
}

PARAM_FUNC(param_)
{
  //we need to set h, ep, sigma, ti in radconv params as follows:
  //h = 100 W/(m2*K)
  //ep = .3
  //sigma = 5.6704 x 10-5 g s^-3 K^-4
  //ti = 300 K

  //we need to set rho_*, k_* and cp_* in heat params 
  //and also *maybe* figure out a way to distinguish between k_lig and k_sol
  //when phasefield is coupled
  //rho_* = 8.9 g/cm^3
  //kliq = 90 W/(m*K)
  //ksol = 90 W/(m*K)
  //cpliq = 0.44 J/(g*K)
  //cpsol = 0.44 J/(g*K)

  //here we need the rest..
  //and pull fro xml
  //te = 1635.;// K
  te = plist->get<double>("te_",1641.);
  //tl = 1706.;// K
  tl = plist->get<double>("tl_",1706.);
  //Lf = 17.2;// kJ/mol
  Lf = plist->get<double>("Lf_",2.95e5);

  //eta_d = 0.3;//dimensionless
  eta_d = plist->get<double>("eta_",0.3);
  //P_d = 50.;// W
  P_d = plist->get<double>("P_",50.);
  //s_d = 2.;//dimensionless
  s_d = plist->get<double>("s_",2.);
  //r_d = .005;// 50 um
  r_d = plist->get<double>("r_",.00005);
  //d_d = .001;// 10 um
  d_d = plist->get<double>("d_",.00001);
  //gamma_d = is gamma function
  //gamma(3/s):
  //gamma(3/2) = sqrt(pi)/2
  gamma_d = plist->get<double>("gamma_",0.886227);
  x0_d = plist->get<double>("x0_",0.);
  y0_d = plist->get<double>("y0_",0.);
  z0_d = plist->get<double>("z0_",0.);
  t_hold_d = plist->get<double>("t_hold_",0.005);
  t_decay_d = plist->get<double>("t_decay_",0.01);
  tau0_d = plist->get<double>("tau0_",1.);
  W0_d = plist->get<double>("W0_",1.);
  
  t0_d = plist->get<double>("t0_",300.);

  dfldu_mushy_d = tpetra::heat::rho_d*Lf/(tl-te); //fl=(t-te)/(tl-te);
  
  scaling_constant_d = plist->get<double>("scaling_constant_",1.);

}
}//namespace goldak

namespace fullycoupled
{
  double hemisphere_IC_rad = 1.0;
  double hemispherical_IC_x0 = 0.0;
  double hemispherical_IC_y0 = 0.0;
  double hemispherical_IC_z0 = 0.0;
  bool hemispherical_IC = false;	
  
INI_FUNC(init_conc_farzadi_)
{
  return -1.;
}

INI_FUNC(init_phase_farzadi_)
{
  if (hemispherical_IC){
	  const double w0 = tpetra::farzadi3d::w0;
	  
	  const double dist = std::sqrt( (x-hemispherical_IC_x0/w0)*(x-hemispherical_IC_x0/w0) 
	  	+ (y-hemispherical_IC_y0/w0)*(y-hemispherical_IC_y0/w0) 
	  	+ (z-hemispherical_IC_z0/w0)*(z-hemispherical_IC_z0/w0));
	  const double r = hemisphere_IC_rad/w0 + tpetra::farzadi3d::amplitude*((double)rand()/(RAND_MAX));
	  return std::tanh( (dist-r)/std::sqrt(2.));
  }
  else {
	  double h = tpetra::farzadi3d::base_height + tpetra::farzadi3d::amplitude*((double)rand()/(RAND_MAX));
	  
	  return std::tanh((h-z)/std::sqrt(2.));
	  
	  double c = (x-tpetra::farzadi3d::x0)*(x-tpetra::farzadi3d::x0) + (y-tpetra::farzadi3d::y0)*(y-tpetra::farzadi3d::y0) + (z-tpetra::farzadi3d::z0)*(z-tpetra::farzadi3d::z0);
	  return ((tpetra::farzadi3d::C == 0) ? (tanh((h-z)/sqrt(2.))) : (c < tpetra::farzadi3d::r*tpetra::farzadi3d::r) ? 1. : -1.);	
  }
}

INI_FUNC(init_heat_)
{
  const double t_preheat = tpetra::goldak::t0_d;
  const double val = (t_preheat-tpetra::heat::uref_h)/tpetra::heat::deltau_h;
  return val;
}

DBC_FUNC(dbc_) 
{
  // The assumption here is that the desired Dirichlet BC is the initial temperature,
  // that may not be true in the future.
  const double t_preheat = tpetra::goldak::t0_d;
  const double val = (t_preheat-tpetra::heat::uref_h)/tpetra::heat::deltau_h;
  return val;
}

PPR_FUNC(postproc_t_)
{
  // return the physical temperature in K here
  const double theta = u[2];
  return theta * tpetra::heat::deltau_h + tpetra::heat::uref_h;
}

PARAM_FUNC(param_)
{
	hemispherical_IC = plist->get<bool>("hemispherical_IC", false);
	hemisphere_IC_rad = plist->get<double>("hemisphere_IC_rad", 1.0);
	hemispherical_IC_x0 = plist->get<double>("hemispherical_IC_x0", 0.0);
	hemispherical_IC_y0 = plist->get<double>("hemispherical_IC_y0", 0.0);
	hemispherical_IC_z0 = plist->get<double>("hemispherical_IC_z0", 0.0);
}
}//namespace fullycoupled

namespace quaternion
{
  //see
  //[1] LLNL-JRNL-409478 A Numerical Algorithm for the Solution of a Phase-Field Model of Polycrystalline Materials
  //M. R. Dorr, J.-L. Fattebert, M. E. Wickett, J. F. Belak, P. E. A. Turchi (2008)
  //[2] LLNL-JRNL-636233 Phase-field modeling of coring during solidification of Au-Ni alloy using quaternions and CALPHAD input
  //J. L. Fattebert, M. E. Wickett, P. E. A. Turchi May 7, 2013

  const double beta = 1.e-10;

  const double r0 = .064;
  const double halfdx = .001;

  const double Mq = 1.;//1/sec/pJ
  const double Mphi = 1.;//1/sec/pJ
  //const double Mmax = .64;//1/sec/pJ
  //const double Mmin= 1.e-6;
  const double Dmax = 1000.;
  //const double Dmin = 1.e-6;
  const double epq = .0477;//(pJ/um)^1/2
  //const double epq = .0;//(pJ/um)^1/2
  const double epphi = .083852;//(pJ/um)^1/2
  //                          const double L = 2.e9;//J/m^3   =======>
  const double L = 2000.;//pJ/um^3
  const double omega = 31.25;//pJ/um^3
  const double H = .884e-3;//pJ/K/um^2
  //const double T = 1000.;//K
  const double T = 1000.;//K
  const double Tm = 1025.;//K

  const int N = 4;
  const double pi = 3.141592653589793;

  const double dist(const double &x, const double &y, const double &z,
	   const double &x0, const double &y0, const double &z0,
	   const double &r)
  {
    const double c = (x-x0)*(x-x0) + (y-y0)*(y-y0) + (z-z0)*(z-z0);
    return r-sqrt(c);
  }
#if 0
  const double getphi(const double &x, const double &y, const double &z,
	   const double &x0, const double &y0, const double &z0,
		      const double &r, const double &t)
  {
    double rnew = r + t*.003125*1.e5;
    double d = dist(x,y,z,x0,y0,0,rnew);
    const double scale = (1-0)/2.;
    const double shift = (1+0)/2.;
    double val = shift + scale*tanh(d/sqrt(2.)/.01);
    return val;
  }
#endif
  const double p(const double &phi)
  {
    return phi*phi;
  }
  const double pp(const double &phi)
  {
    return 2.*phi;
  }
  const double h(const double &phi)
  {
    return phi*phi*phi*(10.-15.*phi+6.*phi*phi);
  }
  const double hp(const double &phi)
  {
    return 3.*phi*phi*(10.-15.*phi+6.*phi*phi)+phi*phi*phi*(-15.+12.*phi);
  }
  const double g(const double &phi)
  {
    return 16.*phi*phi*(1.-phi)*(1.-phi);
  }
  const double gp(const double &phi)
  {
    return 32.*phi*(1.-3.*phi+2.*phi*phi);
  }
    
KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_)
{
  //derivatives of the test function
  const double dtestdx = basis[0]->dphidx[i];
  const double dtestdy = basis[0]->dphidy[i];
  const double dtestdz = basis[0]->dphidz[i];
  //test function
  const double test = basis[0]->phi[i];
  //u, phi
  const double u = basis[eqn_id]->uu;
  const double uold = basis[eqn_id]->uuold;

  const double phi[3] = {basis[4]->uu,basis[4]->uuold,basis[4]->uuoldold};
  const double m[3] = {1.-h(phi[0]),1.-h(phi[1]),1.-h(phi[2])};
  double M[3] = {m[0]*(Mq+1e-6)+1e-6,m[1]*(Mq+1e-6)+1e-6,m[2]*(Mq+1e-6)+1e-6};
  //double M[3] = {Mq,Mq,Mq};

  //we need norm grad q here
  //grad q is nonzero only within interfaces
  double normgradq[3] = {0.,0.,0.};
  for(int k = 0; k < N; k++){
    normgradq[0] = normgradq[0] + basis[k]->dudx*basis[k]->dudx + basis[k]->dudy*basis[k]->dudy + basis[k]->dudz*basis[k]->dudz;
    normgradq[1] = normgradq[1] + basis[k]->duolddx*basis[k]->duolddx + basis[k]->duolddy*basis[k]->duolddy + basis[k]->duolddz*basis[k]->duolddz;
    normgradq[2] = normgradq[2] + basis[k]->duoldolddx*basis[k]->duoldolddx + basis[k]->duoldolddy*basis[k]->duoldolddy + basis[k]->duoldolddz*basis[k]->duoldolddz;
  }
  //std::cout<<normgradq[0]<<std::endl;
  const double betal = 0.*beta;
  double b2 = beta*beta;
  normgradq[0] = (normgradq[0] > b2) ? sqrt(normgradq[0]) : betal;
  normgradq[1] = (normgradq[1] > b2) ? sqrt(normgradq[1]) : betal;
  normgradq[2] = (normgradq[2] > b2) ? sqrt(normgradq[2]) : betal;

  //const double ep[3] = {epq*epq,epq*epq,epq*epq};
  const double ep[3] = {epq*epq*p(phi[0]),epq*epq*p(phi[1]),epq*epq*p(phi[2])};

  double Dq[3] = {2.*T*H*p(phi[0]),2.*T*H*p(phi[1]),2.*T*H*p(phi[2])};

  double pfu[3] = {0., 0., 0.};

  double normq[3] = {0., 0., 0.};

  for(int k = 0; k < N; k++){
    pfu[0] = pfu[0] + basis[k]->uu*(basis[k]->dudx*dtestdx + basis[k]->dudy*dtestdy + basis[k]->dudz*dtestdz);
    pfu[1] = pfu[1] + basis[k]->uuold*(basis[k]->duolddx*dtestdx + basis[k]->duolddy*dtestdy + basis[k]->duolddz*dtestdz);
    pfu[2] = pfu[2] + basis[k]->uuoldold*(basis[k]->duoldolddx*dtestdx + basis[k]->duoldolddy*dtestdy + basis[k]->duoldolddz*dtestdz);
    normq[0] = normq[0] + basis[k]->uu*basis[k]->uu;
    normq[1] = normq[1] + basis[k]->uuold*basis[k]->uuold;
    normq[2] = normq[2] + basis[k]->uuoldold*basis[k]->uuoldold;
  }

  b2 = beta*beta;
  normq[0] = (normq[0] > b2 ) ? normq[0] : b2;
  normq[1] = (normq[1] > b2 ) ? normq[1] : b2;
  normq[2] = (normq[2] > b2 ) ? normq[2] : b2;

  //DD is the diffusion term ep + Dq/normgradq
  const double DD[3] = {ep[0]*normgradq[0]+Dq[0],ep[1]*normgradq[1]+Dq[1],ep[2]*normgradq[2]+Dq[2]};

  //const double ut = normgradq[0]*(u-uold)/dt_*test;
  const double ut = normgradq[1]*(u-uold)/dt_*test;

  pfu[0] = -pfu[0]*u*M[0]*DD[0]/normq[0];
  pfu[1] = -pfu[1]*uold*M[1]*DD[1]/normq[1];
  pfu[2] = -pfu[2]*basis[eqn_id]->uuoldold*M[2]*DD[2]/normq[2];
  if(pfu[0] != pfu[0]) exit(0);
  if(pfu[1] != pfu[1]) exit(0);
  if(pfu[2] != pfu[2]) exit(0);

  const double divgradu[3] = {M[0]*DD[0]*(basis[eqn_id]->dudx*dtestdx + basis[eqn_id]->dudy*dtestdy + basis[eqn_id]->dudz*dtestdz),
			      M[1]*DD[1]*(basis[eqn_id]->duolddx*dtestdx + basis[eqn_id]->duolddy*dtestdy + basis[eqn_id]->duolddz*dtestdz),
			      M[2]*DD[2]*(basis[eqn_id]->duoldolddx*dtestdx + basis[eqn_id]->duoldolddy*dtestdy + basis[eqn_id]->duoldolddz*dtestdz)};
  const double a = 1.;
  const double f[3] = {divgradu[0]+a*pfu[0], divgradu[1]+a*pfu[1], divgradu[2]+a*pfu[2]};

  double val= (ut + (1.-t_theta2_)*t_theta_*f[0]
	       + (1.-t_theta2_)*(1.-t_theta_)*f[1]
	       +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]))/1.;//*normgradq[0];
//   if(val!=val){
//     std::cout<<divgradu[0]<<" "<<divgradu[1]<<" "<<divgradu[2]<<std::endl;
//   }
//   std::cout<<val<<" "<<i<<std::endl;
  //val = 0.;
  return val;
}
  
KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(precon_)
{
  const double phi = basis[4].uu;
  double Dq = 2.*T*H*p(phi);
  double m = 1.-h(phi);
  double M = m*Mq;
  //double M = Mq;
  const double ep = epq*epq*p(phi);

  double normgradq = 0.;
  for(int k = 0; k < N; k++){
    normgradq = normgradq + basis[k].dudx*basis[k].dudx + basis[k].dudy*basis[k].dudy + basis[k].dudz*basis[k].dudz;
  }
  const double betal = 0.*beta;
  const double b2 = betal*betal;
  normgradq = (normgradq > b2) ? sqrt(normgradq) : betal;
  //Dq = Dq/normgradq;
  const double DD = ep*normgradq+Dq;
  const double ut = normgradq*basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i];
  const double divgradu = M*DD*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
       + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
       + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);

  return (ut + t_theta_*divgradu)/10.;
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_phi_)
{
  //derivatives of the test function
  const double dtestdx = basis[0]->dphidx[i];
  const double dtestdy = basis[0]->dphidy[i];
  const double dtestdz = basis[0]->dphidz[i];
  //test function
  const double test = basis[0]->phi[i];
  //u, phi
  const double phi[3] = {basis[eqn_id]->uu, basis[eqn_id]->uuold, basis[eqn_id]->uuoldold};

  const double phit = (phi[0]-phi[1])/dt_*test;

  //M eps^2 grad phi rrad test
  const double divgradu[3] = {Mphi*epphi*epphi*(basis[eqn_id]->dudx*dtestdx + basis[eqn_id]->dudy*dtestdy + basis[eqn_id]->dudz*dtestdz),
			      Mphi*epphi*epphi*(basis[eqn_id]->duolddx*dtestdx + basis[eqn_id]->duolddy*dtestdy + basis[eqn_id]->duolddz*dtestdz),
			      Mphi*epphi*epphi*(basis[eqn_id]->duoldolddx*dtestdx + basis[eqn_id]->duoldolddy*dtestdy + basis[eqn_id]->duoldolddz*dtestdz)};

  const double ww[3] = {Mphi*gp(phi[0])*test,
			Mphi*gp(phi[1])*test,
			Mphi*gp(phi[2])*test};

  double normgradq[3] = {0.,0.,0.};
  for(int k = 0; k < N; k++){
    normgradq[0] = normgradq[0] + basis[k]->dudx*basis[k]->dudx + basis[k]->dudy*basis[k]->dudy + basis[k]->dudz*basis[k]->dudz;
    normgradq[1] = normgradq[1] + basis[k]->duolddx*basis[k]->duolddx + basis[k]->duolddy*basis[k]->duolddy + basis[k]->duolddz*basis[k]->duolddz;
    normgradq[2] = normgradq[2] + basis[k]->duoldolddx*basis[k]->duoldolddx + basis[k]->duoldolddy*basis[k]->duoldolddy + basis[k]->duoldolddz*basis[k]->duoldolddz;
  }
  const double betal = 0.*beta;
  const double b2 = betal*betal;
  normgradq[0] = (normgradq[0] > b2) ? sqrt(normgradq[0]) : betal;
  normgradq[1] = (normgradq[1] > b2) ? sqrt(normgradq[1]) : betal;
  normgradq[2] = (normgradq[2] > b2) ? sqrt(normgradq[2]) : betal;

  //coupling term
  const double pq[3] = {Mphi*2.*H*T*pp(phi[0])*normgradq[0]*test,
			Mphi*2.*H*T*pp(phi[1])*normgradq[1]*test,
			Mphi*2.*H*T*pp(phi[2])*normgradq[2]*test};

  const double hh[3] = {(Mphi*hp(phi[0])*L*(T-Tm)/Tm*test),
			(Mphi*hp(phi[1])*L*(T-Tm)/Tm*test),
			(Mphi*hp(phi[2])*L*(T-Tm)/Tm*test)};

  const double epqq[3] = {0.,0.,0.};
//   const double epqq[3] = {Mphi*epq*epq*phi[0]*normgradq[0]*normgradq[0]*test,
// 			  Mphi*epq*epq*phi[1]*normgradq[1]*normgradq[1]*test,
// 			  Mphi*epq*epq*phi[2]*normgradq[2]*normgradq[2]*test};

  const double f[3] = {divgradu[0] + ww[0] + pq[0] + hh[0] + epqq[0],
		       divgradu[1] + ww[1] + pq[1] + hh[1] + epqq[1],
		       divgradu[2] + ww[2] + pq[2] + hh[2] + epqq[2]};

  return phit + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

KOKKOS_INLINE_FUNCTION 
RES_FUNC_TPETRA(residual_phase_)
{
  //derivatives of the test function
  const double dtestdx = basis[0]->dphidx[i];
  const double dtestdy = basis[0]->dphidy[i];
  const double dtestdz = basis[0]->dphidz[i];
  //test function
  const double test = basis[0]->phi[i];
  //u, phi
  const double phi[3] = {basis[eqn_id]->uu, basis[eqn_id]->uuold, basis[eqn_id]->uuoldold};

  const double phit = (phi[0]-phi[1])/dt_*test;

  //M eps^2 grad phi rrad test
  const double divgradu[3] = {Mphi*epphi*epphi*(basis[eqn_id]->dudx*dtestdx + basis[eqn_id]->dudy*dtestdy + basis[eqn_id]->dudz*dtestdz),
			      Mphi*epphi*epphi*(basis[eqn_id]->duolddx*dtestdx + basis[eqn_id]->duolddy*dtestdy + basis[eqn_id]->duolddz*dtestdz),
			      Mphi*epphi*epphi*(basis[eqn_id]->duoldolddx*dtestdx + basis[eqn_id]->duoldolddy*dtestdy + basis[eqn_id]->duoldolddz*dtestdz)};

  const double ww[3] = {Mphi*gp(phi[0])*test,
			Mphi*gp(phi[1])*test,
			Mphi*gp(phi[2])*test};

  //double normgradq[3] = {0.,0.,0.};
  double normgradq[3] = {1.,1.,1.};

  //coupling term
  const double pq[3] = {Mphi*2.*H*T*pp(phi[0])*normgradq[0]*test,
			Mphi*2.*H*T*pp(phi[1])*normgradq[1]*test,
			Mphi*2.*H*T*pp(phi[2])*normgradq[2]*test};

  const double hh[3] = {(Mphi*hp(phi[0])*L*(T-Tm)/Tm*test),
			(Mphi*hp(phi[1])*L*(T-Tm)/Tm*test),
			(Mphi*hp(phi[2])*L*(T-Tm)/Tm*test)};

  const double epqq[3] = {0.,0.,0.};
//   const double epqq[3] = {Mphi*epq*epq*phi[0]*normgradq[0]*normgradq[0]*test,
// 			  Mphi*epq*epq*phi[1]*normgradq[1]*normgradq[1]*test,
// 			  Mphi*epq*epq*phi[2]*normgradq[2]*normgradq[2]*test};

  const double f[3] = {divgradu[0] + ww[0] + pq[0] + hh[0] + epqq[0],
		       divgradu[1] + ww[1] + pq[1] + hh[1] + epqq[1],
		       divgradu[2] + ww[2] + pq[2] + hh[2] + epqq[2]};

  return phit + (1.-t_theta2_)*t_theta_*f[0]
    + (1.-t_theta2_)*(1.-t_theta_)*f[1]
    +.5*t_theta2_*((2.+dt_/dtold_)*f[1]-dt_/dtold_*f[2]);
}

KOKKOS_INLINE_FUNCTION 
PRE_FUNC_TPETRA(precon_phi_)
{
  const double phit = basis[eqn_id].phi[j]/dt_*basis[eqn_id].phi[i];
  const double divgradphi = Mphi*epphi*epphi*(basis[eqn_id].dphidx[j]*basis[eqn_id].dphidx[i]
					    + basis[eqn_id].dphidy[j]*basis[eqn_id].dphidy[i]
					    + basis[eqn_id].dphidz[j]*basis[eqn_id].dphidz[i]);

  const double ww = Mphi*16.*omega*(1.-2.*basis[eqn_id].uu)*basis[eqn_id].phi[j]*basis[eqn_id].phi[i];
  return phit + t_theta_*(divgradphi + ww);
}

//q0 is -.5 lower right
INI_FUNC(initq0_)
{
  double val = .0;
  const double s = .001;
  const double den = sqrt(2)*s;

  val = .5*(1.+tanh((r0-x)/den)+tanh((y-r0)/den));
  if (val > .5) val = .5;
  return val;
}

INI_FUNC(initq0s_)
{
  double val = .5;

  const double s = r0 + halfdx;

  if (x > s && y < s) val = -.5;
  return val;
}


//q1 is -.5 in upper right
INI_FUNC(initq1_)
{
  double val = .0;//.5
  const double s = .001;
  const double den = sqrt(2)*s;
  //if (x > r0 && y > r0 ) val = -.5;
  val = .5*(1.+tanh((r0-x)/den)+tanh((r0-y)/den));
  if (val > .5) val = .5;
  return val;
}

INI_FUNC(initq1s_)
{
  double val = .5;

  const double s = r0 + halfdx;

  if (x > s && y > s) val = -.5;
  return val;

}

//q2 and q3 are .5 everywhere
INI_FUNC(initq2_)
{
  return 0.5;
}

INI_FUNC(initq3_)
{
  return 0.5;
}

INI_FUNC(initphi_)
{
  double val = 0.;
  const double x0 = 0.;
  const double x1 = .128;
  const double s = .001;
  const double den = sqrt(2)*s;
  //if( x*x + y*y <= r0*r0 ) val = 1.;

  val = .5*(tanh((r0-sqrt(x*x + y*y))/den) + 1.);

  //if( (x-x1)*(x-x1) + y*y <= r0*r0 ) val = 1.;
  val += .5*(tanh((r0-sqrt((x-x1)*(x-x1) + y*y))/den) + 1.);
  //if( (x-x1)*(x-x1) + (y-x1)*(y-x1) <= r0*r0 ) val = 1.;
  val += .5*(tanh((r0-sqrt((x-x1)*(x-x1) + (y-x1)*(y-x1)))/den) + 1.);
  //if( x*x + (y-x1)*(y-x1) <= r0*r0 ) val = 1.;
  val += .5*(tanh((r0-sqrt(x*x + (y-x1)*(y-x1)))/den) + 1.);

  return val;
}

INI_FUNC(init_)
{
  //this will be a function of eqn_id...

  const double r = .2;
  const double x0 = .5;
  const double y0 = .5;
  const double w = .01;//.025;
  //g1 is the background grain
  //g0 is black??
  const double g0[4] = {-0.707106781186547,
			-0.000000000000000,
			0.707106781186548,
			-0.000000000000000};
  const double g1[4] = {0.460849109679818,
		       0.025097870693789,
		       0.596761014095944,
		       0.656402686655941};   
  const double g2[4] = {0.659622270251558,
			0.314355942642574,
			0.581479198986258,
			-0.357716008950928};
  const double g3[4] = {0.610985843880283,
			-0.625504969359875,
			-0.474932634235629,
			-0.099392277476709};

  double scale = (g2[eqn_id]-g1[eqn_id])/2.;
  double shift = (g2[eqn_id]+g1[eqn_id])/2.;
  double d = dist(x,y,z,x0,y0,0,r);
  double val = shift + scale*tanh(d/sqrt(2.)/w);
  return val;
}

  //https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

//atan2 returns [-pi,pi]
//asin returns [-pi/2,pi/2]
//paraview expects [0,1] for an rgb value
//https://discourse.paraview.org/t/coloring-surface-by-predefined-rgb-values/6011/6
//so should we shift and scale each of these here?

//Also the page seems different today, 4-12-23

//And our convention to normalize via:
//  return (s+pi)/2./pi;
//so that each angle is between 0 and 1 can be used as RGB coloring

//Also, with the example, q=[.5 -.5 .5 .5] and roundoff, etc 
//produces arguments to atan2 with near +/- 0, producing oscillations
//we need to fix this
//ie small chages in q lead to large changes in euler angle

#if 0
EulerAngles ToEulerAngles(Quaternion q) {
    EulerAngles angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    angles.roll = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
    double cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
    angles.pitch = 2 * std::atan2(sinp, cosp) - M_PI / 2;

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    angles.yaw = std::atan2(siny_cosp, cosy_cosp);

    return angles;
}
#endif

PPR_FUNC(postproc_ea0_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...
  //                       w      x      y      z
  double sinr_cosp = 2. * (u[0] * u[1] + u[2] * u[3]);
  //                            x      x      y      y
  double cosr_cosp = 1. - 2. * (u[1] * u[1] + u[2] * u[2]);
  double s = std::atan2(sinr_cosp, cosr_cosp);
  return (s+pi)/2./pi;
  //return s;
}

PPR_FUNC(postproc_ea1_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...
  //                 w      y      z      x
  double sinp = 2 * (u[0] * u[2] - u[3] * u[1]);
  double s = 0.;
  if (std::abs(sinp) >= 1.){
  //if (std::abs(sinp) >= .95){
    s = std::copysign(pi / 2., sinp); // use 90 degrees if out of range
  }else{
    s = std::asin(sinp);
  }
  return (s+pi/2.)/pi;
  //return s;
}

PPR_FUNC(postproc_ea2_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...
  //                       w      z      x      y
  double siny_cosp = 2. * (u[0] * u[3] + u[1] * u[2]);
  //                            y      y      z      z
  double cosy_cosp = 1. - 2. * (u[2] * u[2] + u[3] * u[3]);
  double s = std::atan2(siny_cosp, cosy_cosp);
  return (s+pi)/2./pi;
  //return s;
}

//a possible alternative to euler angles is to just consider the quaternion as rgba color
//there may be visualization ttols that allow for this, although doesn't seem easy in 
//paraview
//rgba can be converted to rgb via the following, assuming some background color BGColor,
//and all values normalized in [0 1]:
//  Source => Target = (BGColor + Source) =
//  Target.R = ((1 - Source.A) * BGColor.R) + (Source.A * Source.R)
//  Target.G = ((1 - Source.A) * BGColor.G) + (Source.A * Source.G)
//  Target.B = ((1 - Source.A) * BGColor.B) + (Source.A * Source.B)
//
//note that in our example problem, q0 and q1 can have values in [-.5 .5]
//but paraview does not seem to care about negative values

PPR_FUNC(postproc_rgb_r_)
{
  return (1.-u[3])*0.+u[3]*u[0];
}
PPR_FUNC(postproc_rgb_g_)
{
  return (1.-u[3])*0.+u[3]*u[1];
}
PPR_FUNC(postproc_rgb_b_)
{
  return (1.-u[3])*0.+u[3]*u[2];
}

PPR_FUNC(postproc_normqold_)
{
  return uold[0]*uold[0]+uold[1]*uold[1]+uold[2]*uold[2]+uold[3]*uold[3];
}

PPR_FUNC(postproc_normq_)
{
  return u[0]*u[0]+u[1]*u[1]+u[2]*u[2]+u[3]*u[3];
}

PPR_FUNC(postproc_d_)
{
  double phi=u[4];
  return 2*H*T*p(phi);
}

PPR_FUNC(postproc_qdotqt_)
{
  double s = 0.;
  for(int k = 0; k < 4; k++) s = s + u[k]*(u[k]-uold[k])/dt;
  return s;
}

PPR_FUNC(postproc_normgradq_)
{
  double s = 0.;
  for(int k = 0; k < 12; k++) s = s + gradu[k]*gradu[k];
  return s;
}

PPR_FUNC(postproc_qdotqold_)
{
  double s = 0.;
  for(int k = 0; k < 4; k++) s = s + u[k]*uold[k];
  return s;
}

PPR_FUNC(postproc_normphi_)
{
  double s = u[4]*u[4];
  return s;
}

}//namespace quaternion
namespace grain
{

  //see
  //[1] Suwa et al, Mater. T. JIM., 44,11, (2003);
  //[2] Krill et al, Acta Mater., 50,12, (2002); 



  double L = 1.;
  double alpha = 1.;
  double beta = 1.;
  double gamma = 1.;
  double kappa = 2.;

  int N = 6;

  double pi = 3.141592653589793;

  double r(const double &x,const int &n){
    return sin(64./512.*x*n*pi);
  }

PARAM_FUNC(param_)
{
  N = plist->get<int>("numgrain");
}

KOKKOS_INLINE_FUNCTION
RES_FUNC_TPETRA(residual_)
{
  //derivatives of the test function
  double dtestdx = basis[0]->dphidx[i];
  double dtestdy = basis[0]->dphidy[i];
  double dtestdz = basis[0]->dphidz[i];
  double test = basis[0]->phi[i];

  double u = basis[eqn_id]->uu;
  double uold = basis[eqn_id]->uuold;

  double divgradu = kappa*(basis[eqn_id]->dudx*dtestdx + basis[eqn_id]->dudy*dtestdy + basis[eqn_id]->dudz*dtestdz);

  double s = 0.;
  for(int k = 0; k < N; k++){
    s = s + basis[k]->uu*basis[k]->uu;
  }
  s = s - u*u;

  return (u-uold)/dt_*test + L* ((-alpha*u + beta*u*u*u +2.*gamma*u*s)*test +  divgradu); 

}
PRE_FUNC(prec_)
{
#if 0
  //cn probably want to move each of these operations inside of getbasis
  //derivatives of the test function
  double dtestdx = basis[0].dphidxi[i]*basis[0].dxidx
    +basis[0].dphideta[i]*basis[0].detadx
    +basis[0].dphidzta[i]*basis[0].dztadx;
  double dtestdy = basis[0].dphidxi[i]*basis[0].dxidy
    +basis[0].dphideta[i]*basis[0].detady
    +basis[0].dphidzta[i]*basis[0].dztady;
  double dtestdz = basis[0].dphidxi[i]*basis[0].dxidz
    +basis[0].dphideta[i]*basis[0].detadz
    +basis[0].dphidzta[i]*basis[0].dztadz;

  double dbasisdx = basis[0].dphidxi[j]*basis[0].dxidx
    +basis[0].dphideta[j]*basis[0].detadx
    +basis[0].dphidzta[j]*basis[0].dztadx;
  double dbasisdy = basis[0].dphidxi[j]*basis[0].dxidy
    +basis[0].dphideta[j]*basis[0].detady
    +basis[0].dphidzta[j]*basis[0].dztady;
  double dbasisdz = basis[0].dphidxi[j]*basis[0].dxidz
    +basis[0].dphideta[j]*basis[0].detadz
    +basis[0].dphidzta[j]*basis[0].dztadz;

  double u = basis[eqn_id].uu;
  
  double test = basis[0].phi[i];
  double divgrad = L*kappa*(dbasisdx * dtestdx + dbasisdy * dtestdy + dbasisdz * dtestdz);
  double u_t =test * basis[0].phi[j]/dt_;
  double alphau = -test*L*alpha*basis[0].phi[j];
  double betau = 3.*u*u*basis[0].phi[j]*test*L*beta;

  double s = 0.;
  for(int k = 0; k < N; k++){
    s = s + basis[k].uu*basis[k].uu;
  }
  s = s - u*u;

  double gammau = 2.*gamma*L*basis[0].phi[j]*s*test;

  return u_t + divgrad + betau + gammau;// + alphau ;
#endif
  return 1.;
}
PPR_FUNC(postproc_)
{
  //u is u0,u1,...
  //gradu is dee0/dx,dee0/dy,dee0/dz,dee1/dx,dee1/dy,dee1/dz...


  double s =0.;
  for(int j = 0; j < N; j++){
    s = s + u[j]*u[j];
  }

  return s;
}
}//namespace grain

namespace random
{

RES_FUNC_TPETRA(residual_test_)
{
  //test function
  const double test = basis[0]->phi[i];
  //printf("%d %le \n",i,rand);
  return (basis[0]->uu - rand)*test;
}

}//namespace random

NBC_FUNC_TPETRA(nbc_one_)
{
  
  double phi = basis->phi[i];
  
  return 1.*phi;
}

}//namespace tpetra


#endif
