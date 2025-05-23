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
// Purpose       : Transient analysis functions.
// Special Notes :
// Creator       : Richard Schiek, SNL, Electrical and Microsystem Modeling
// Creation Date : 03/10/2009
//
//-----------------------------------------------------------------------------

#include <Xyce_config.h>

#include <N_IO_MeasureAverage.h>
#include <N_ERH_ErrorMgr.h>

namespace Xyce {
namespace IO {
namespace Measure {

//-----------------------------------------------------------------------------
// Function      : Average::Average()
// Purpose       :
// Special Notes :
// Scope         : public
// Creator       : Rich Schiek, Electrical and Microsystems Modeling
// Creation Date : 3/10/2009
//-----------------------------------------------------------------------------
Average::Average(const Manager &measureMgr, const Util::OptionBlock & measureBlock):
  Stats(measureMgr, measureBlock),
  averageValue_(0.0),
  totalAveragingWindow_(0.0)
{
  // indicate that this measure type is supported and should be processed in simulation
  typeSupported_ = true;

  // updateTran() is likely to segfault if the .MEASURE line was incomplete
  checkMeasureLine();
}


//-----------------------------------------------------------------------------
// Function      : Average::reset()
// Purpose       : Called when restarting a measure function.  Resets any state
// Special Notes :
// Scope         : public
// Creator       : Rich Schiek, Electrical and Microsystems Modeling
// Creation Date : 8/28/2014
//-----------------------------------------------------------------------------
void Average::reset() 
{
  resetStats();
  averageValue_ = 0.0;
  totalAveragingWindow_ = 0.0;
}

//-----------------------------------------------------------------------------
// Function      : Average::setMeasureVarsForNewWindow()
// Purpose       : Called when entering a new RFC window for TRAN measures
// Special Notes : 
// Scope         : public
// Creator       : Pete Sholander, SNL
// Creation Date : 04/28/2020
//-----------------------------------------------------------------------------
void Average::setMeasureVarsForNewWindow()
{
  averageValue_ = 0.0;
  totalAveragingWindow_ = 0.0;
  initialized_ = false;

  return;
}

//-----------------------------------------------------------------------------
// Function      : Average::updateMeasureVars()
// Purpose       : Updates the average value and the averaging window
// Special Notes : For TRAN measures, the independent variable is time.  For AC
//                 and NOISE measures, it is frequency.  For DC measures, it
//                 is the value of the first variable in the DC sweep vector.
// Scope         : public
// Creator       : Pete Sholander, SNL
// Creation Date : 04/28/2020
//-----------------------------------------------------------------------------
void Average::updateMeasureVars(double indepVarVal, double signalVal)
{
  // the MIN_THRESH and MAX_THRESH qualifiers are only used by the AVG measure
  if ( withinMinMaxThresh(signalVal) )
  {
    // The fabs() is needed to account for both increasing and decreasing DC sweeps.  The fabs() has
    // no effect on TRAN, AC or NOISE, since the time and frequency values are monotonically increasing
    // for those two cases.
    averageValue_ += 0.5 * fabs(indepVarVal - lastIndepVarValue_) * (signalVal + lastSignalValue_);
    totalAveragingWindow_ += fabs(indepVarVal - lastIndepVarValue_);
  }

  return;
}

//-----------------------------------------------------------------------------
// Function      : Average::getMeasureResult()
// Purpose       :
// Special Notes : If the averaging window is zero, then measure is reported
//                 as "FAILED".
// Scope         : public
// Creator       : Rich Schiek, Electrical and Microsystems Modeling
// Creation Date : 3/10/2009
//-----------------------------------------------------------------------------
double Average::getMeasureResult()
{
  if( initialized_ && (numPointsFound_ > 1) )
      calculationResult_ =  averageValue_ / totalAveragingWindow_;

  return calculationResult_;
}

//-----------------------------------------------------------------------------
// Function      : Average::printMeasureResult()
// Purpose       : used to print the measurement result to an output stream
//                 object, which is typically the mt0, ma0 or ms0 file
// Special Notes :
// Scope         : public
// Creator       : Pete Sholander, Electrical and Microsystems Modeling
// Creation Date : 5/13/2020
//-----------------------------------------------------------------------------
std::ostream& Average::printMeasureResult(std::ostream& os)
{
  basic_ios_all_saver<std::ostream::char_type> save(os);
  os << std::scientific << std::setprecision(precision_);

  if ( (!initialized_ || (numPointsFound_ < 2)) && measureMgr_.getMeasFail() )
  {
    // output FAILED to .mt file if .OPTIONS MEASURE MEASFAIL=1 and this is a failed measure.
    os << name_ << " = FAILED" << std::endl;
  }
  else
  {
     os << name_ << " = " << this->getMeasureResult() << std::endl;
  }

  return os;
}

//-----------------------------------------------------------------------------
// Function      : Average::printVerboseMeasureResult()
// Purpose       : used to print the measurement result to an output stream
//                 object, which is typically the mt0, ma0 or ms0 file
// Special Notes :
// Scope         : public
// Creator       : Pete Sholander, Electrical and Microsystems Modeling
// Creation Date : 5/13/2020
//-----------------------------------------------------------------------------
std::ostream& Average::printVerboseMeasureResult(std::ostream& os)
{
  basic_ios_all_saver<std::ostream::char_type> save(os);
  os << std::scientific << std::setprecision(precision_);

  if (initialized_ && (numPointsFound_ > 1))
  {
    os << name_ << " = " << this->getMeasureResult() << std::endl;
  }
  else
  {
    os << name_ << " = FAILED" << std::endl;
  }

  return os;
}

//-----------------------------------------------------------------------------
// Function      : Average::printMeasureWindow
// Purpose       : prints information related to measure window
// Special Notes :
// Scope         : public
// Creator       : Pete Sholander, SNL
// Creation Date : 03/25/2020
//-----------------------------------------------------------------------------
std::ostream& Average::printMeasureWindow(std::ostream& os, double endSimTime,
				          double startSweepVal, double endSweepVal) const
{
  // Pathological case of FROM=TO within an otherwise valid FROM-TO window.
  // This a failed measure, but the FROM-TO window should be printed correctly.
  if ( (fromGiven_ || toGiven_) && (from_==to_) && firstSweepValueFound_ &&
       ((mode_ == "AC") || (mode_ == "DC") || mode_ == "NOISE") )
  {
    basic_ios_all_saver<std::ostream::char_type> save(os);
    os << std::scientific << std::setprecision(precision_);
    std::string modeStr = setModeStringForMeasureWindowText();
    os << "Measure Start " << modeStr << "= " << from_
       << "\tMeasure End " << modeStr << "= " << to_ << std::endl;
  }
  else
  {
    Base::printMeasureWindow(os, endSimTime, startSweepVal, endSweepVal);
  }

  return os;
}

} // namespace Measure
} // namespace IO
} // namespace Xyce
