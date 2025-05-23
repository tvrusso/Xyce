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
// Purpose        :
//
// Special Notes  :
//
// Creator        : Dave Baur
//
// Creation Date  :
//
//
//
//
//-------------------------------------------------------------------------

#include <Xyce_config.h>

#include <N_ERH_ErrorMgr.h>
#include <N_IO_OutputMgr.h>
#include <N_IO_OutputterFrequencyCSV.h>
#include <N_IO_OutputterFrequencyPrn.h>
#include <N_IO_OutputterTimeCSV.h>
#include <N_IO_OutputterTimePrn.h>
#include <N_IO_OutputterTimeProbe.h>
#include <N_IO_OutputterTimeRaw.h>
#include <N_IO_OutputterTimeRawASCII.h>
#include <N_IO_OutputterTimeTecplot.h>
#include <N_IO_OutputterExternal.h>
#include <N_IO_OutputterTransient.h>

namespace Xyce {
namespace IO {
namespace Outputter {

//-----------------------------------------------------------------------------
// Function      : enableTransientOutput
// Purpose       :
// Special Notes :
// Scope         :
// Creator       : Dave Baur
// Creation Date : June 23, 2014
//-----------------------------------------------------------------------------
void enableTransientOutput(Parallel::Machine comm, OutputMgr &output_manager, Analysis::Mode analysis_mode)
{
  std::pair<OutputParameterMap::const_iterator, bool> result = output_manager.findOutputParameter(OutputType::TRAN);
  if (result.second)
  {
    for (std::vector<PrintParameters>::const_iterator it = (*result.first).second.begin(), end = (*result.first).second.end(); it != end; ++it)
    {
      PrintParameters transient_print_parameters = (*it);

        // If we are creating snapshots, replace this list with all the solution variables.
        if ( output_manager.getCreateSnapshots() )
          output_manager.createAllPrintParameters( comm, transient_print_parameters );
 
        if (transient_print_parameters.format_ != Format::PROBE)
          transient_print_parameters.variableList_.push_front(Util::Param("TIME", 0.0));
        if (transient_print_parameters.printIndexColumn_)
          transient_print_parameters.variableList_.push_front(Util::Param("INDEX", 0.0));
        if (transient_print_parameters.printStepNumColumn_)
          transient_print_parameters.variableList_.push_front(Util::Param("STEPNUM", 0.0));

        output_manager.fixupPrintParameters(comm, transient_print_parameters);

        Outputter::Interface *outputter;
        if (transient_print_parameters.format_ == Format::STD)
        {
          outputter = new Outputter::TimePrn(comm, output_manager, transient_print_parameters);
        }
        else if (transient_print_parameters.format_ == Format::CSV)
        {
          outputter = new Outputter::TimeCSV(comm, output_manager, transient_print_parameters);
        }
        else if (transient_print_parameters.format_ == Format::TECPLOT)
        {
          outputter = new Outputter::TimeTecplot(comm, output_manager, transient_print_parameters);
        }
        else if (transient_print_parameters.format_ == Format::PROBE)
        {
          outputter = new Outputter::TimeProbe(comm, output_manager, transient_print_parameters);
        }
        else if (transient_print_parameters.format_ == Format::RAW)
        {
            outputter = new Outputter::TimeRaw(comm, output_manager, transient_print_parameters);
        }
        else if (transient_print_parameters.format_ == Format::RAW_ASCII)
        {
          outputter = new Outputter::TimeRawAscii(comm, output_manager, transient_print_parameters);
        }
        else if ( (transient_print_parameters.format_ == Format::TS1) ||
                  (transient_print_parameters.format_ == Format::TS2) )
        {
          Report::UserWarning0() << "Transient output cannot be written in Touchstone format, using standard format";
          transient_print_parameters.format_ = Format::STD;
          outputter = new Outputter::TimePrn(comm, output_manager, transient_print_parameters);
        }
        else
        {
          Report::UserWarning0() << "Transient output cannot be written in " << transient_print_parameters.format_ << " format, using standard format";

          outputter = new Outputter::TimePrn(comm, output_manager, transient_print_parameters);
        }

        output_manager.addOutputter(PrintType::TRAN, outputter);
    }
  }
  // Now do pretty much the same thing, but using the External output stuff
  std::pair<ExternalOutputWrapperMap::const_iterator, bool> result2 = output_manager.findExternalOutputWrapper(OutputType::TRAN);
  if  (result2.second)
  {
    for (std::vector<ExternalOutputWrapper *>::const_iterator it = (*result2.first).second.begin(), end = (*result2.first).second.end(); it != end; ++it)
    {
      ExternalOutputWrapper * theWrapperPtr = (*it);
      output_manager.fixupOutputVariables(comm, theWrapperPtr->getParamList());
      Outputter::Interface *outputter;
      outputter = new Outputter::OutputterExternal(comm,output_manager,
                                                   theWrapperPtr);
      output_manager.addOutputter(PrintType::TRAN, outputter);
    }
  }

}

} // namespace Outputter
} // namespace IO
} // namespace Xyce
