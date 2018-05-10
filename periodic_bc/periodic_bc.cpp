//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Los Alamos National Security, LLC.  This file is part of the
//  Tusas code (LA-CC-17-001) and is subject to the revised BSD license terms
//  in the LICENSE file found in the top-level directory of this distribution.
//
//////////////////////////////////////////////////////////////////////////////



#include "periodic_bc.h"

#ifdef HAVE_MPI
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif


periodic_bc::periodic_bc(const int ns_id1, 
			 const int ns_id2,
			 const int index,
			 const int numeqs, 
			 Mesh *mesh, 
			 const Teuchos::RCP<const Epetra_Comm>& comm):
  ns_id1_(ns_id1),
  ns_id2_(ns_id2),
  index_(index),
  numeqs_(numeqs),
  mesh_(mesh),
  comm_(comm)
{
  //cn the first step is add all of the ns1 equations to the ns2 equations
  //cn then replace all the ns1 eqns with u(ns1)-u(ns2)

	//cn see:
	// [1]  http://www.kurims.kyoto-u.ac.jp/~kyodo/kokyuroku/contents/pdf/0836-11.pdf
	// and similar to
	// [2] https://orbi.ulg.ac.be/bitstream/2268/100283/1/2012_COMMAT_PBC.pdf

        // we implement eq 14 in [1]

  if( 0 == comm_->MyPID() ){
    std::cout<<"Entering periodic_bc::periodic_bc for:"<<std::endl
	     <<"    ns_id1_ = "<<ns_id1_<<std::endl
	     <<"    ns_id2_ = "<<ns_id2_<<std::endl
	     <<"    index_  = "<<index_<<std::endl
	     <<"    numeqs_ = "<<numeqs_<<std::endl<<std::endl;
  }
  //this sorts all nodelists, we don't want to do this everytime so we need
  //to pass it the nodeset ids eventually
  mesh_->create_sorted_nodesetlists();
  ns1_map_ = get_replicated_map(ns_id1_);
  //ns1_map_->Print(std::cout);
  ns2_map_ = get_replicated_map(ns_id2_);
  //ns2_map_->Print(std::cout);

  //here we check for same size maps
  if( ns1_map_->NumGlobalElements () != ns2_map_->NumGlobalElements () ){
    if( 0 == comm_->MyPID() ){
      std::cout<<"Incompatible global node set sizes found for index = "<<index_<<"."<<std::endl;
    }
    exit(0);
  }
  if( ns1_map_->NumMyElements () != ns2_map_->NumMyElements () ){
    if( 0 == comm_->MyPID() ){
      std::cout<<"Incompatible local node set sizes found for index = "<<index_<<"."<<std::endl;
    }
    exit(0);
  }



  //cn we will eventually do an import of global f and global u into replicated vectors here,
  //cn hence we may need to change the map routine from global mesh id to global u and f id;
  //cn ie numeqs_*gid+index_ this could be done at the if(-99... line below.

  std::vector<int> node_num_map(mesh_->get_node_num_map());

  //we want this map to be a one equation version of the x_owned_map in tusas
  //do it this way and hope it is the same

  overlap_map_ = Teuchos::rcp(new Epetra_Map(-1,
					     node_num_map.size(),
					     &node_num_map[0],
					     0,
					     *comm_));
  if( 1 == comm_->NumProc() ){
    node_map_ = overlap_map_;
  }else{
    node_map_ = Teuchos::rcp(new Epetra_Map(Epetra_Util::Create_OneToOne_Map(*overlap_map_)));
  }

  // target, source
  importer1_ = Teuchos::rcp(new Epetra_Import(*ns1_map_, *node_map_));
  if( 0 == comm_->MyPID() )
    std::cout<<"importer1_"<<std::endl;

  //cn I think we are doing something wrong here....
  //cn the documentation says the source map should have unique gids, 
  //cn which over_lap_map does not.  In the residual fill we use an inporter like:
  //cn  importer_ = rcp(new Epetra_Import(*x_overlap_map_, *x_owned_map_));


  //importer2_ = Teuchos::rcp(new Epetra_Import(*ns2_map_, *overlap_map_));
  importer2_ = Teuchos::rcp(new Epetra_Import(*ns2_map_, *node_map_));
  if( 0 == comm_->MyPID() )
    std::cout<<"importer2_"<<std::endl;

  f_rep_ = Teuchos::rcp(new Epetra_Vector(*ns1_map_));
  u_rep_ = Teuchos::rcp(new Epetra_Vector(*ns2_map_));
  //exit(0);
  if( 0 == comm_->MyPID() ){
    std::cout<<"periodic_bc::periodic_bc completed for:"<<std::endl
	     <<"    ns_id1_ = "<<ns_id1_<<std::endl
	     <<"    ns_id2_ = "<<ns_id2_<<std::endl
	     <<"    index_  = "<<index_<<std::endl
	     <<"    numeqs_ = "<<numeqs_<<std::endl<<std::endl;
  }
}
void periodic_bc::import_data(const Epetra_FEVector &f_full,
			      const Teuchos::RCP<const Epetra_Vector> u_full ) const
{
  //cn f_fe is on the x_owned map and u is on the overlap_map 
  //Teuchos::RCP< Epetra_Vector> u1 = Teuchos::rcp(new Epetra_Vector(*overlap_map_)); 
  Teuchos::RCP< Epetra_Vector> u1 = Teuchos::rcp(new Epetra_Vector(*node_map_));  
  //#pragma omp parallel for 
  // for(int nn = 0; nn < overlap_map_->NumMyElements(); nn++ ){
  for(int nn = 0; nn < node_map_->NumMyElements(); nn++ ){
    (*u1)[nn]=(*u_full)[numeqs_*nn+index_];
  }
  int err1 = u_rep_->Import(*u1, *importer2_, Insert);
  if( 0 != err1){
    std::cout<<"periodic_bc::import_data import err1 = "<<err1<<std::endl;
    exit(0);
  }
  //u_rep_->Print(std::cout);
  
  Teuchos::RCP< Epetra_Vector> f1 = Teuchos::rcp(new Epetra_Vector(*node_map_)); 
  
  //#pragma omp parallel for 
  for(int nn = 0; nn < node_map_->NumMyElements(); nn++ ){
      (*f1)[nn]=f_full[0][numeqs_*nn+index_]; 
    }
  int err2 = f_rep_->Import(*f1, *importer1_, Insert);
  if( 0 != err2){
    std::cout<<"periodic_bc::import_data import err2 = "<<err2<<std::endl;
    exit(0);
  }
  //f_rep_->Print(std::cout);
  //f_rep_->Reduce();
  //u_rep_->Reduce();
  //f_rep_->Print(std::cout);
  //exit(0);
}

Teuchos::RCP<const Epetra_Map> periodic_bc::get_replicated_map(const int id){

  std::vector<int> node_num_map(mesh_->get_node_num_map());
  //cn the ids in sorted_node_set are local
  int ns_size = mesh_->get_sorted_node_set(id).size(); 
  int max_size = 0;
  comm_->MaxAll(&ns_size,
		&max_size,
		(int)1 );	
  //std::cout<<"max = "<<max_size<<" "<<comm_->MyPID()<<std::endl;
  std::vector<int> gids(max_size,-99);
  for ( int j = 0; j < ns_size; j++ ){
    int lid = mesh_->get_sorted_node_set_entry(id, j);
    int gid = node_num_map[lid];
    gids[j] = gid;
  }//j

  int count = comm_->NumProc()*max_size;
  std::vector<int> AllVals(count,-99);

  comm_->GatherAll(&gids[0],
		   &AllVals[0],
		   max_size );
  
  std::vector<int> g_gids;

  for ( int j = 0; j < count; j++ ){
    //std::cout<<j<<" "<<AllVals[j]<<std::endl;
    //if(-99 < AllVals[j]) g_gids.push_back(numeqs_*AllVals[j]+index_);
    if(-99 < AllVals[j]) g_gids.push_back(AllVals[j]);
  }


  //cn because of the way the mesh is decomposed, we now have a repeatative gid on processor boundaries
  //cn need to do something about this...
  //cn since we have already sorted them wrt to coordinates we can not do a sort again
  //cn it is possible to do a std::unique on the vector to fix
  std::vector<int>::iterator it;
  it = std::unique (g_gids.begin(), g_gids.end());
                                                       
  g_gids.resize( std::distance(g_gids.begin(),it) );

  Teuchos::RCP<const Epetra_Map> ns_map_ = Teuchos::rcp(new Epetra_Map(g_gids.size(),
					 g_gids.size(),
					 &g_gids[0],
					 0,
					 *comm_));
  //ns_map_->Print(std::cout);

  //All this and we now have a replicated map for the first nodeset on all procs...

  if( 0 == comm_->MyPID() )
    std::cout<<"periodic_bc::get_replicated_map completed for id: "<<id<<std::endl;

  return ns_map_;
}



periodic_bc::~periodic_bc()
{
  //delete mesh_;
}
