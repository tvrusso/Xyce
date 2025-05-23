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
// Purpose        : Migrator tool using Zoltan utilities
//
// Special Notes  :
//
// Creator        : Robert Hoekstra, SNL, Parallel Computational Sciences
//
// Creation Date  : 06/07/00
//
//
//
//
//-----------------------------------------------------------------------------

#ifndef Xyce_N_PDS_GlobalAccessor_h
#define Xyce_N_PDS_GlobalAccessor_h

#include <list>
#include <map>
#include <vector>

#include <N_PDS_fwd.h>
#include <N_LAS_fwd.h>

class Epetra_Distributor;
class Epetra_Comm;

namespace Xyce {
namespace Parallel {

//-----------------------------------------------------------------------------
// Class         : GlobalAccessor
// Purpose       : Migrator utility to retrieve off processor elements
//                 of distributed objects
// Special Notes :
// Creator       : Robert Hoekstra, SNL, Parallel Compuational Sciences
// Creation Date : 06/07/00
//-----------------------------------------------------------------------------
class GlobalAccessor
{
public:
  // Constructors
  GlobalAccessor(const Communicator& comm);

  // Destructor
  ~GlobalAccessor();

private:
  // No public copy construction, assignment, or equality operators.

  // Copy constructor (private).
  GlobalAccessor(const GlobalAccessor & right);

  // Assignment operator (private).
  GlobalAccessor & operator = (const GlobalAccessor & right);

  // Equality operation.
  bool operator == (const GlobalAccessor & right) const;

  // Non-equality operation.
  bool operator != (const GlobalAccessor & right) const;

public:
  void registerExternGIDVector(const std::vector< std::pair< int,
                               int > > & extGIDVector);

  // Uses data from TopoLSUtil::ExternSVarList to generate migration plan using
  // Zoltan utilities.
  void generateMigrationPlan();

  // Migrates off processor elements of MultiVector using plan generated by
  // above method.
  void migrateMultiVector(Xyce::Linear::MultiVector * mVector);

  // Migrator for integer array (currently to be used for global index reset).
  void migrateIntArray(std::map< int, int > & sendMap, std::map< int,
                       int > & recvMap);

  // Migrator for integer array (currently to be used for global index reset).
  void migrateIntVecs(std::map< int, std::vector< int > > & sendMap, std::map< int,
                      std::vector< int > > & recvMap);

private:

  // PDS Comm
  const Communicator & pdsComm_;

  // Vector listing extern GIDs and their respective procs needed for nonlocal
  // dependencies.
  std::vector< std::pair< int, int > > externGIDVector_;

  // Number of objects to be received.
  int numReceiveObjs_;

  // Vector listing receive GIDs.
  std::vector< std::pair< int, int > > receiveGIDVector_;

  int * arrayReceiveGIDs_;
  int * arrayReceiveProcs_;

  // Pointer to the receiving buffer.
  char * recvBuf_;
  int recvBufSize_;

  // Number of objects to be sent.
  int numSendObjs_;

  // Vector listing sending GIDs.
  std::vector< std::pair< int, int > > sendGIDVector_;

  int * arraySendGIDs_;
  int * arraySendProcs_;

  // Pointer to the sending buffer.
  char * sendBuf_;
  int sendBufSize_;

  Epetra_Distributor * distributor_;
  const Epetra_Comm * petraComm_;
};

//-----------------------------------------------------------------------------
// Function      : GlobalAccessor::registerExternGIDVector
// Purpose       : Registers vector of external GIDs and procs.
// Special Notes :
// Scope         : public
// Creator       : Robert Hoekstra, SNL, Parallel Compuational Sciences
// Creation Date : 06/07/00
//-----------------------------------------------------------------------------
inline void GlobalAccessor::registerExternGIDVector(
  const std::vector< std::pair< int, int > > & extGIDVector)
{

  // First, clear the vector...
  externGIDVector_.clear();

  // Then, assign the GIDs and processors.
  externGIDVector_.assign(extGIDVector.begin(), extGIDVector.end());
}

} // namespace Parallel
} // namespace Xyce

#endif

