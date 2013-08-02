// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
//     GreenSocs Ltd
// 
// 
//   This program is free software.
// 
//   If you have no applicable agreement with GreenSocs Ltd, this software
//   is licensed to you, and you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
// 
//   If you have a applicable agreement with GreenSocs Ltd, the terms of that
//   agreement prevail.
// 
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
// 
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
//   02110-1301  USA 
// 
// ENDLICENSETEXT


#ifdef USE_STATIC_CASTS
#ifdef __GNUC__
# warning "generic.h should not be included when using static casts!"
#elif defined(_WIN32)
#pragma message("generic.h should not be included when using static casts!")
#endif
# define __GENERIC_H__
#endif


#ifndef __GENERIC_H__
#define __GENERIC_H__



#include "greenbus/core/tlm.h"
#include "greenbus/api/basicPorts.h"
#include <stdarg.h>
#include <sstream>

#ifdef USE_DUST 
# include "scv.h"
#endif

//#ifdef(VERBOSE)
//#define GS_DELTA_WARNING SC_REPORT_WARNING("GreenBus GenericAPI", "Using an untimed GreenBus notification. If you call this method multiple times while in the same delta cycle, only the last call will be processed. As a solution, use SC_ZERO_TIME delay.")
//#else
#define GS_DELTA_WARNING
//#endif

namespace tlm {

//---------------------------------------------------------------------------
// Convenience typedefs  //should that happen here??? RG
//---------------------------------------------------------------------------

#ifndef USE_STATIC_CASTS
# ifdef __GNUC__
#  warning "Dynamic casts are deprecated. Use static casts instead (make STATIC=yes)."
# elif defined(_WIN32)
#  pragma message("Dynamic casts are deprecated. Use static casts instead (make STATIC=yes).")
# endif
#endif

class GenericTransaction;
# ifdef USE_PCIE_TRANSACTION
  namespace PCIe {
    class PCIeTransaction; 
  }
  typedef tlm::PCIe::PCIeTransaction TRANSACTION;
#else
typedef GenericTransaction TRANSACTION;
#endif

class GenericPhase;
# ifdef USE_PCIE_TRANSACTION
  namespace PCIe {
    class PCIePhase; 
  }
  typedef tlm::PCIe::PCIePhase PHASE;
#else
typedef GenericPhase PHASE;
#endif


// I believe that the generic stuff should be the typedefs. No one is using e.g. ROUTERACCESS nowhere!
// the only used things are GS_ATOM and PHASE
# ifdef USE_PCIE_TRANSACTION
  namespace PCIe {
    class PCIeMasterAccess;
    class PCIeSlaveAccess;
    class PCIeRouterAccess;
  }
  typedef tlm::PCIe::PCIeRouterAccess ROUTERACCESS;
  typedef tlm::PCIe::PCIeSlaveAccess TARGETACCESS;
  typedef tlm::PCIe::PCIeMasterAccess MASTERACCESS;
#else
  class GenericRouterAccess;
  typedef GenericRouterAccess ROUTERACCESS;
  class GenericSlaveAccess;
  typedef GenericSlaveAccess TARGETACCESS;
  class GenericMasterAccess;
  typedef GenericMasterAccess MASTERACCESS;
#endif
  
typedef boost::shared_ptr<TRANSACTION> GenericTransactionHandle;
typedef boost::shared_ptr<ROUTERACCESS> GenericRouterAccessHandle;
typedef boost::shared_ptr<TARGETACCESS> GenericSlaveAccessHandle;
typedef boost::shared_ptr<MASTERACCESS> GenericMasterAccessHandle;
typedef unevenpair<GenericTransactionHandle, PHASE> GS_ATOM;

#include "generic.dynamic_casts.blocking.h"

typedef GS_ATOM TRANSACTION_CONTAINER; // depricated!!
 
} // end of namespace tlm

#endif
