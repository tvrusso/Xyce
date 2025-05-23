//-------------------------------------------------------------------------
//   Copyright 2002-2025 National Technology & Engineering Solutions of
//   Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
//   NTESS, the U.S. Government retains certain rights in this software.
//
//   This file is part of the Xyce(TM) Parallel Electrical Simulator.
//
//   Xyce(TM) is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Xyce(TM) is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Xyce(TM).
//   If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Purpose        : Specification file for the abstract parallel communication
//                  class for Xyce.  This class will contain parallel data and
//                  functions.
//
// Special Notes  : It assumes that all parallel communication is being handled
//                  through MPI.
//
// Creator        : Scott A. Hutchinson, SNL, Parallel Computational Sciences
//
// Creation Date  : 03/08/00
//
//
//
//
//-----------------------------------------------------------------------------

#ifndef Xyce_N_PDS_EpetraMPIComm_h
#define Xyce_N_PDS_EpetraMPIComm_h

#ifdef Xyce_PARALLEL_MPI
#include <mpi.h>
#include <Epetra_MpiComm.h>
#else
#include <Epetra_SerialComm.h>
#endif

#include <list>
#include <N_PDS_Comm.h>
#include <N_ERH_ErrorMgr.h>

namespace Xyce {
namespace Parallel {

//-----------------------------------------------------------------------------
// Class         : EpetraMPIComm
// Purpose       : Parallel communication class for Xyce.  This class
//                 will contain parallel data and functions.
// Special Notes :
// Creator       : Robert Hoekstra, SNL, Parallel Compuational Sciences
// Creation Date : 06/26/01
//-----------------------------------------------------------------------------
class EpetraMPIComm : public Communicator
{

public:

  // Constructor
  EpetraMPIComm(int iargs, char * cargs[]);

#ifdef Xyce_PARALLEL_MPI
  EpetraMPIComm( MPI_Comm comm );
#endif

  //Destructor
  ~EpetraMPIComm();

  // Cloning
  Communicator* clone() const { return new EpetraMPIComm(* this); }

  Xyce::Parallel::Machine comm() const;
    
  // Get my processor ID.
  int procID() const;

  // Get the total number of processors in this communicator.
  int numProc() const;

  // Is this a serial run?
  bool isSerial() const { return isSerial_; }

  // Get the last processor flag (by default true in serial)
  // NOTE:  This is used for generating augmented linear systems.
  bool isLastProc() const { return (procID() == (numProc()-1)); }

  // Wrappers for select MPI Scan and AllReduce operations.
  bool scanSum(const double * vals, double * sums, const int & count) const;
  bool sumAll(const double * vals, double * sums, const int & count) const;
  bool maxAll(const double * vals, double * maxs, const int & count) const;
  bool minAll(const double * vals, double * mins, const int & count) const;

  bool scanSum(const int * vals, int * sums, const int & count) const;
  bool sumAll(const int * vals, int * sums, const int & count) const;
  bool maxAll(const int * vals, int * maxs, const int & count) const;
  bool minAll(const int * vals, int * mins, const int & count) const;

  // Wrapper for MPI broadcasts.
  bool bcast(int * val, const int & count, const int & root) const;
  bool bcast(char * val, const int & count, const int & root) const;
  bool bcast(double * val, const int & count, const int & root) const;

  // Wrappers for MPI Sends.
  bool send(const int * val, const int & count, const int & dest) const;
  bool send(const char * val, const int & count, const int & dest) const;
  bool send(const double * val, const int & count, const int & dest) const;
  bool send(const long * val, const int & count, const int & dest) const;

  // Wrappers for MPI Recvs.
  bool recv(int * val, const int & count, const int & src) const;
  bool recv(char * val, const int & count, const int & src) const;
  bool recv(double * val, const int & count, const int & src) const;
  bool recv(long * val, const int & count, const int & src) const;

  // Wrappers for MPI pack and unpack functionality.
  bool rSend( const int * val, const int & count, const int & dest ) const;
  bool rSend( const char * val, const int & count, const int & dest ) const;
  bool rSend( const double * val, const int & count, const int & dest ) const;
                                                                                              
  // MPI_Irecv wrappers
  bool iRecv( int * val, const int & count, const int & src ) const;
  bool iRecv( char * val, const int & count, const int & src ) const;
  bool iRecv( double * val, const int & count, const int & src ) const;
                                                                                              
  // MPI_Waitall wrappers
  bool waitAll();
                                                                                              
  bool pack(const int * val, const int count, char * buf, const int size,
            int & pos) const;
  bool pack(const char * val, const int count, char * buf, const int size,
            int & pos) const;
  bool pack(const double * val, const int count, char * buf, const int size,
            int & pos) const;
  bool pack(const long * val, const int count, char * buf, const int size,
            int & pos) const;

  bool unpack(const char * buf, const int size, int & pos, int * val,
              const int count) const;
  bool unpack(const char * buf, const int size, int & pos, char * val,
              const int count) const;
  bool unpack(const char * buf, const int size, int & pos, double * val,
              const int count) const;
  bool unpack(const char * buf, const int size, int & pos, long * val,
              const int count) const;

  Epetra_Comm * petraComm() { return petraComm_; }
  const Epetra_Comm * petraComm() const { return petraComm_; }

  // Communicator Barrier function.
  // A no-op for a serial communicator.  For MPI, it causes each processor in
  // the communicator to wait until all processors have arrived.
  void barrier() const;

private:
  
  // Copy constructor - reuse the same Petra_Comm object.
  EpetraMPIComm(const EpetraMPIComm& right);

  // Serial or parallel flag - "true" implies a serial run.
  bool isSerial_;

  // Pointer to library-support comm object.
  bool petraCommOwned_;
  Epetra_Comm * petraComm_;

#ifdef Xyce_PARALLEL_MPI
  bool mpiCommOwned_;
  MPI_Comm mpiComm_;

  mutable MPI_Status status_;
  mutable std::list<MPI_Request> request_;

  void createMPIComm( int iargs, char * cargs[] );
#endif

};

} // namespace Parallel
} // namespace Xyce

#endif
