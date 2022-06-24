//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Triad National Security, LLC.  This file is part of the
//  Tusas code (LA-CC-17-001) and is subject to the revised BSD license terms
//  in the LICENSE file found in the top-level directory of this distribution.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef RANDOM_DISTRIBUTION_H
#define RANDOM_DISTRIBUTION_H

#include "Mesh.h"

#include <Teuchos_RCP.hpp>

#include "Epetra_Comm.h"
#include "Epetra_Map.h"

class random_distribution
{
public:
  random_distribution(const Teuchos::RCP<const Epetra_Comm>& comm,  ///< MPI communicator 
		      Mesh *mesh,  ///< mesh object
		      const int ltpquadorder  ///< quadrature order
		      );

  ~random_distribution();

  void compute_random(const int nt);

  std::vector<std::vector<double> > get_gauss_val(){return gauss_val;}

  void compute_correlation(){};

  void print();

private:
  /// number of Gauss points in element
  int ngp;
  /// Element map object.
  Teuchos::RCP<const Epetra_Map>   elem_map_;
  /// random values at Gauss points
  std::vector<std::vector<double> > gauss_val;
  /// MPI comm object.
  const Teuchos::RCP<const Epetra_Comm>  comm_;

};

#endif
