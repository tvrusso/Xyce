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
// Purpose        : Synapse classes
//
// Special Notes  :
//
// Creator        : Eric R. Keiter, SNL, Parallel Computational Sciences
//
// Creation Date  : 02/28/00
//
//
//
//
//-----------------------------------------------------------------------------

#ifndef Xyce_N_DEV_Synapse_h
#define Xyce_N_DEV_Synapse_h

// ----------   Xyce Includes   ----------
#include <N_DEV_Configuration.h>
#include <N_DEV_DeviceMaster.h>
#include <N_DEV_DeviceBlock.h>
#include <N_DEV_DeviceInstance.h>
#include <N_DEV_DeviceModel.h>

#include <Sacado_No_Kokkos.hpp>

namespace Xyce {
namespace Device {
namespace Synapse {

class Model;
class Instance;
class Master;

struct Traits : public DeviceTraits<Model, Instance>
{
  static const char *name() {return "Synapse";}
  static const char *deviceTypeName() {return "YSYNAPSE level 1";}
  static int numNodes() {return 2;}
  static bool isLinearDevice() {return false;}

  static Device *factory(const Configuration &configuration, const FactoryBlock &factory_block);
  static void loadModelParameters(ParametricData<Model> &model_parameters);
  static void loadInstanceParameters(ParametricData<Instance> &instance_parameters);
};

//-----------------------------------------------------------------------------
// Class         : Instance
// Purpose       :
//
//	This  is  the  instance class  for Synapses.  It
//	contains "unique" Synapse  information - ie stuff that
//	will be true of only one  Synapse in the circuit, such
//	as the nodes to which it is connected.  A Synapse is
//	connected to only two circuit nodes.
//
//	This class  does not directly contain information about
//	its node indices. It contains indices into the 3 parts
//	(A, dx, and  b) of the matrix  problem A*dx = b, and
//	also x.  A is the Jacobian  matrix, dx is the update to
//	the solution vector x, and b is the right hand side
//	function vector.  These indices are global, and
//	determined by topology during  the initialization stage
//	of execution.
//
// Special Notes :
// Creator       : Eric Keiter, SNL, Parallel Computational Sciences
// Creation Date : 3/16/00
//-----------------------------------------------------------------------------
class Instance : public DeviceInstance
{
  friend class ParametricData<Instance>;
  friend class Model;
  friend struct Traits;
  friend class Master;

public:
  static std::vector< std::vector<int> > jacStamp;

  Instance(
     const Configuration &       configuration,
     const InstanceBlock &     IB,
     Model &                   Riter,
     const FactoryBlock &      factory_block);

  ~Instance();

private:
  Instance(const Instance &);
  Instance &operator=(const Instance &);

public:
  void registerLIDs( const std::vector<int> & intLIDVecRef,
                     const std::vector<int> & extLIDVecRef );
  void registerStateLIDs( const std::vector<int> & staLIDVecRef );

  void loadNodeSymbols(Util::SymbolTable &symbol_table) const; // override

  bool processParams ();

  bool updateTemperature(const double & temp_tmp);

  bool updateIntermediateVars ();
  bool updatePrimaryState ();
  bool updateSecondaryState ();

  const std::vector< std::vector<int> > & jacobianStamp() const;
  void registerJacLIDs( const std::vector< std::vector<int> > & jacLIDVec );

  // load functions, residual:
  bool loadDAEQVector ();
  bool loadDAEFVector ();

  // load functions, Jacobian:
  bool loadDAEdQdx ();
  bool loadDAEdFdx ();

  void setupPointers();


  Model &getModel() 
  {
    return model_;
  }

private:

  Model &       model_;         //< Owning model

  // user-specified paramters:

  //Vector local index for Positive Node
  int li_Prev;
  //Vector local index for Negative Node
  int li_Post;
  int li_rVar;

  // Offset variables corresponding to the above declared indices.
  int APostEquPostNodeOffset;
  int APostEquRNodeOffset;
  int AREquPostNodeOffset;
  int AREquRNodeOffset;

  // Pointers for Jacobian
  double *f_PostEquPostNodePtr;
  double *f_PostEquRNodePtr;
  double *f_REquPostNodePtr;
  double *f_REquRNodePtr;

  // vars used for load calculations
  double ipost;  // post synapse current
  double didVpost;
  double didr;
  double rFval;
  double drFdVpre;
  double drFdr;
};


//-----------------------------------------------------------------------------
// Class         : Model
// Purpose       :
//
//
// Special Notes :
// Creator       : Eric Keiter, SNL, Parallel Computational Sciences
// Creation Date : 3/16/00
//-----------------------------------------------------------------------------
class Model : public DeviceModel
{
  typedef std::vector<Instance *> InstanceVector;

  friend class ParametricData<Model>;
  friend class Instance;
  friend struct Traits;
  friend class Master;

public:
  Model(
     const Configuration &       configuration,
     const ModelBlock &        MB,
     const FactoryBlock &      factory_block);
  ~Model();

private:
  Model();
  Model(const Model &);
  Model &operator=(const Model &);

public:
  virtual void forEachInstance(DeviceInstanceOp &op) const /* override */;

  virtual std::ostream &printOutInstances(std::ostream &os) const;

  bool processParams ();
  bool processInstanceParams ();


public:
  void addInstance(Instance *instance) 
  {
    instanceContainer.push_back(instance);
  }

  void setupBaseInstanceContainer()
  {
    std::vector<Instance*>::iterator iter = instanceContainer.begin();
    std::vector<Instance*>::iterator end   = instanceContainer.end();
    for ( ; iter!=end; ++iter)
    {
      Xyce::Device::DeviceModel::baseInstanceContainer.push_back( static_cast<Xyce::Device::DeviceInstance *>(*iter) );
    }
  }

private:
  std::vector<Instance*> instanceContainer;

private:

protected:
  // Synapse parameters
  double gMax;
  double eRev;
  double alpha;
  double beta;
  double vP;
  double kP;
  double tMax;
};

//-----------------------------------------------------------------------------
// Class         : Master
// Purpose       :
// Special Notes :
// Creator       : Eric Keiter, SNL, Parallel Computational Sciences
// Creation Date : 11/26/08
//-----------------------------------------------------------------------------
class Master : public DeviceMaster<Traits>
{
  friend class Synapse::Instance;
  friend class Synapse::Model;

public:
  Master(
     const Configuration &       configuration,
     const FactoryBlock &      factory_block,
     const SolverState & ss1,
     const DeviceOptions & do1)
    : DeviceMaster<Traits>(configuration, factory_block, ss1, do1)
  {}

  virtual bool updateState (double * solVec, double * staVec, double * stoVec);
  virtual bool updateSecondaryState (double * staDeriv, double * stoVec);
};

// These functions represents the equations that need to be solved
// for this device.  Since Xyce loads an F and Q contribution, the
// equations are broken up into their F and Q components.  Thus there
// is a kcl1EquF() and kcl1EquQ().  Automatic differentiation will
// be used to generate all the derivatives of these equations for the
// dF/dX and dQ/dX loads

template <typename ScalarT>
static ScalarT Tsyn( const ScalarT V, const ScalarT Tmax, const ScalarT Vthres, const ScalarT Kp)
{
  ScalarT result =  Tmax / (1.0 + std::exp( -(V - Vthres) / Kp ) );
  return result;
}

template <typename ScalarT>
static ScalarT PostCurrentEqu( const ScalarT Vpost, const ScalarT r, const ScalarT g, const ScalarT Erev)
{
  ScalarT result =  g * r * (Vpost - Erev);
  return result;
}

template <typename ScalarT>
static ScalarT rEquF( const ScalarT V, const ScalarT r, const ScalarT alpha, const ScalarT beta,
                      const ScalarT Tmax, const ScalarT Vthres, const ScalarT Kp)
{
  ScalarT result =  alpha * Tsyn< ScalarT >(V, Tmax, Vthres, Kp) * (1.0 - r) - beta * r;
  return result;
}

void registerDevice(const DeviceCountMap& deviceMap = DeviceCountMap(),
                    const std::set<int>& levelSet = std::set<int>());

} // namespace Synapse
} // namespace Device
} // namespace Xyce

#endif

