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

//-------------------------------------------------------------------------
//
// Purpose        : Outputter class for csv files for AC sensitivity info.
//                  This includes both the direct and adjoint information.
//
// Special Notes  :
//
// Creator        : Pete Sholander
//
// Creation Date  : 4/23/2019
//
//
//
//
//-------------------------------------------------------------------------

#include <Xyce_config.h>

#include <N_IO_OutputterSensitivityACCSV.h>
#include <N_IO_OutputMgr.h>
#include <N_IO_Op.h>
#include <N_UTL_DeleteList.h>

namespace Xyce {
namespace IO {
namespace Outputter {

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::SensitivityACCSV
// Purpose       : constructor
// Special Notes :
// Scope         :
// Creator       : Pete Sholander, SNL
// Creation Date : 4/23/2019
//-----------------------------------------------------------------------------
SensitivityACCSV::SensitivityACCSV(
    Parallel::Machine comm,
    OutputMgr &output_manager,
    const PrintParameters &print_parameters)
  : outputManager_(output_manager),
    printParameters_(print_parameters),
    outFilename_(),
    os_(0),
    index_(0),
    currentStep_(0),
    numberOfSteps_(0)
{
  if (printParameters_.defaultExtension_.empty())
  {
    printParameters_.defaultExtension_ = ".FD.SENS.csv";
  }

  // Add columns for FREQ and any variables on the .PRINT SEN line.  The
  // columns for the objective functions and the direct and adjoint sensivitity
  // values will be added in doOutputSensitivityAC().
  fixupColumns(comm, outputManager_.getOpBuilderManager(), printParameters_, opList_);

}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::~SensitivityACCSV
// Purpose       : destructor
// Special Notes :
// Scope         :
// Creator       : Pete Sholander
// Creation Date : 4/23/2019
//-----------------------------------------------------------------------------
SensitivityACCSV::~SensitivityACCSV()
{
  outputManager_.closeFile(os_);
  deleteList(opList_.begin(), opList_.end());
}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::SensitivityACHeader
// Purpose       :
// Special Notes :
// Scope         :
// Creator       : Pete Sholander, SNL
// Creation Date : 4/23/2019
//-----------------------------------------------------------------------------
void SensitivityACCSV::SensitivityACHeader()
{
  int column_index = 0;

  Table::ColumnList::const_iterator it = printParameters_.table_.columnList_.begin();
  Table::ColumnList::const_iterator end = printParameters_.table_.columnList_.end();
  for ( ; it != end; ++it, ++column_index)
  {
    if (it != printParameters_.table_.columnList_.begin())
    {
      *os_ << (printParameters_.delimiter_.empty() ? " " : printParameters_.delimiter_);
    }
    printHeader(*os_, (*it));
  }

  *os_ << std::endl;
}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::doOutputSensitivityAC
// Purpose       :
// Special Notes :
// Scope         :
// Creator       : Pete Sholander, SNL
// Creation Date : 4/23/2019
//-----------------------------------------------------------------------------
void SensitivityACCSV::doOutputSensitivityAC(
  Parallel::Machine                   comm,
  double                              frequency,
  const Linear::Vector &              real_solution_vector,
  const Linear::Vector &              imaginary_solution_vector,
  const std::vector<double> &         paramVals,
  const std::vector<std::string> &    paramNameVec,
  const std::vector<std::string> &    objFuncVars,
  const std::vector<double> &         objectiveVec,
  const std::vector<double> &         dOdpVec,
  const std::vector<double> &         dOdpAdjVec,
  const std::vector<double> &         scaled_dOdpVec,
  const std::vector<double> &         scaled_dOdpAdjVec)
{
  if (Parallel::rank(comm) == 0 && !os_)
  {
    outFilename_ = outputFilename(printParameters_.filename_,
                                  printParameters_.defaultExtension_,
                                  printParameters_.suffix_+outputManager_.getFilenameSuffix(),
                                  outputManager_.getNetlistFilename(),
                                  printParameters_.overrideRawFilename_,
                                  printParameters_.formatSupportsOverrideRaw_,
                                  printParameters_.dashoFilename_,
                                  printParameters_.fallback_);

    os_ = outputManager_.openFile(outFilename_);

    // Generate names for the  additional header columns needed for the objective
    // functions and sensitivity values
    std::vector<std::string> colNames;
    for (int i=0; i< objFuncVars.size(); ++i)
    {
      colNames.push_back("Re(" + objFuncVars[i] +")");
      colNames.push_back("Im(" + objFuncVars[i] +")");
      colNames.push_back("Mag(" + objFuncVars[i] +")");
      colNames.push_back("Ph(" + objFuncVars[i] +")");

      if (dOdpVec.size() > 0)
      {
        for (int j=0; j< paramNameVec.size(); ++j)
        {
          colNames.push_back("d_Re(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_dir");
          colNames.push_back("d_Im(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_dir");
          colNames.push_back("d_Mag(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_dir");
          colNames.push_back("d_Ph(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_dir");
        }
      }

      if (dOdpAdjVec.size() > 0)
      {
        for (int j=0; j< paramNameVec.size(); ++j)
        {
          colNames.push_back("d_Re(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_adj");
          colNames.push_back("d_Im(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_adj");
          colNames.push_back("d_Mag(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_adj");
          colNames.push_back("d_Ph(" + objFuncVars[i] + ")/d_" + paramNameVec[j] + "_adj");
        }
      }
    }

    // add those additional columns to the printParameters_.table_
    fixupColumnsFromStrVec(comm, printParameters_, colNames);

    // output the column names to the output file.
    SensitivityACHeader();
  }

  std::vector<complex> result_list;
  getValues(comm, opList_, Util::Op::OpData(index_, &real_solution_vector, &imaginary_solution_vector, 0, 0, 0), result_list);

  if (os_)
  {
    // Output the frequency values.  Also output any variables that appeared
    // explicitly on the .PRINT SENS line.  Xyce operators were made for those columns.
    for (int i = 0; i < result_list.size(); ++i)
    {
      if (os_)
        printValue(*os_, printParameters_.table_.columnList_[i], printParameters_.delimiter_, i, result_list[i].real());
    }

    // Now output the objective function values, and the sensitivity values, directly from the
    // appropriate vectors.

    // The next output column starts after the columns generated from the opList_
    int colIdx = result_list.size();

    // Counters used to keep track of our place in the objectiveVec, dOdpVec and dOdpAdjVec.  The
    // values in those vectors are laid out contiguously, as follows:
    //
    int ovIdx = 0;
    int dirIdx= 0;
    int adjIdx = 0;

    for (int i=0; i< objFuncVars.size(); ++i)
    {
      for (int j=0; j<4; ++j,++colIdx,++ovIdx)
        printValue(*os_, printParameters_.table_.columnList_[colIdx], printParameters_.delimiter_, colIdx, objectiveVec[ovIdx]);

      if (dOdpVec.size() > 0)
      {
        for (int j=0; j<4*paramNameVec.size(); ++j,++colIdx,++dirIdx)
          printValue(*os_, printParameters_.table_.columnList_[colIdx], printParameters_.delimiter_, colIdx, dOdpVec[dirIdx]);
      }

      if (dOdpAdjVec.size() > 0)
      {
        for (int j=0; j<4*paramNameVec.size(); ++j,++colIdx,++adjIdx)
          printValue(*os_, printParameters_.table_.columnList_[colIdx], printParameters_.delimiter_, colIdx, dOdpAdjVec[adjIdx]);
      }
    }

    // send end-of-line character
    *os_ << std::endl;
  }

  ++index_;
}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::doFinishOutput
// Purpose       : Close the stream if there is no .STEP loop.  This function
//                 is also called after each step, if there is a .STEP loop,
//                 but currently does nothing in that case.
// Special Notes :
// Scope         :
// Creator       : Eric Keiter
// Creation Date :
//-----------------------------------------------------------------------------
void SensitivityACCSV::doFinishOutput()
{
  if (os_)
  {
    if (numberOfSteps_ == 0)
    {
      outputManager_.closeFile(os_);
      os_ = 0;
    }
  }
}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::doStartStep
// Purpose       : This function is executed at the start of each step.
// Special Notes :
// Scope         :
// Creator       : David Baur, Raytheon
// Creation Date :
//-----------------------------------------------------------------------------
void SensitivityACCSV::doStartStep( int current_step, int number_of_steps)
{
  index_ = 0;
  currentStep_ = current_step;
  numberOfSteps_ = number_of_steps;
}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::doResetIndex
// Purpose       : Reset the value for the Index column to zero
// Special Notes :
// Scope         :
// Creator       : David Baur, Raytheon
// Creation Date :
//-----------------------------------------------------------------------------
void SensitivityACCSV::doResetIndex()
{
  index_ = 0;
}

//-----------------------------------------------------------------------------
// Function      : SensitivityACCSV::doSteppingComplete
// Purpose       : Close the stream when a .STEP loop is used.
// Special Notes :
// Scope         :
// Creator       : Eric Keiter
// Creation Date :
//-----------------------------------------------------------------------------
void SensitivityACCSV::doSteppingComplete()
{
  // close the sensitivity file.
  if (os_)
  {
    outputManager_.closeFile(os_);
    os_ = 0;
  }
}

} // namespace Outputter
} // namespace IO
} // namespace Xyce
