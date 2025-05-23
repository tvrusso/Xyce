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
// Purpose        : This file contains some enumerated types which are needed 
//                  both inside and outside the expression interface.
//
// Special Notes  :
//
// Creator        : 
//
// Creation Date  : 
//
//-----------------------------------------------------------------------------

#ifndef N_UTL_ExpressionSymbolTable_H
#define N_UTL_ExpressionSymbolTable_H

/// I would have preferred to just use std::tuple, but that is a C++11 construct
#include <string>
namespace Xyce {
namespace Util {

struct ExpressionSymbolTableEntry
{
  ExpressionSymbolTableEntry(const std::string & theName="",
                             const int & theType=0,
                             const char &theLD=' ')
    : name(theName),
      type(theType),
      leadDesignator(theLD)
  {};
  
  std::string name;
  int type;
  char leadDesignator;
  
};

}
}
#endif // N_UTL_ExpressionSymbolTable_H
