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

#ifndef __router_fabric_h__
#define __router_fabric_h__

#include "systemc.h"

/**
 * \file router_fabric.h For each bus functional model in the GreenBus framework, 
 * this file contains a convenience create method. These methods are used
 * by the configuration framework to created busses specified in the configuration file.
 * 
 * To add a new bus, three things are important:
 *  1. import all header files that are required by your bus
 *  2. add a line ADDBUS(yourBusName, yourBusProtocolClass, yourBusSchedulerClass) below.
 *  3. add a line busmap["yourBusName"] = create_yourBusName; in the createBusMap function below.
 * 
 */

// the generic router
#include "greenbus/transport/genericRouter.h"

//------------------------------------------------------------------------------
// Bus packages include files
//------------------------------------------------------------------------------

// Buses that dont use transaction extensions
// SimpleBus
#include "greenbus/protocol/SimpleBus/simpleBusProtocol.h"
#include "greenbus/scheduler/fixedPriorityScheduler.h"

// Buses that use transaction extensions
// PLB 
#if (defined EXTENDED_TRANSACTION) && (EXTENDED_TRANSACTION == PLB)
# include "greenbus/transport/PLB/PLB.h"
# include "greenbus/protocol/PLB/PLBBusProtocol.h"
# include "greenbus/scheduler/dynamicPriorityScheduler.h"
#endif


//------------------------------------------------------------------------------
// Bus definitions
//------------------------------------------------------------------------------

#define ADDBUS(name, protocol, scheduler)                               \
  inline GenericRouter* create_##name (sc_module_name &busname, sc_time &t) { \
    std::string pname(busname); pname += "_protocol";                   \
    std::string sname(busname); sname += "_scheduler";                  \
    protocol *p = new protocol(pname.c_str(), t);                       \
    scheduler *s = new scheduler(sname.c_str(), t);                     \
    GenericRouter *r = new GenericRouter(busname);                      \
    r->protocol_port(*p);                                               \
    p->router_port(*r);                                                 \
    p->scheduler_port(*s);                                              \
    return r;                                                           \
  }                                                                     \


// Buses that dont use transaction extensions
ADDBUS(SimpleBus, SimpleBusProtocol, fixedPriorityScheduler)

// Buses that use transaction extensions
#if (defined EXTENDED_TRANSACTION) && (EXTENDED_TRANSACTION == PLB)
ADDBUS(PLB, PLBBusProtocol, dynamicPriorityScheduler)
#endif

//------------------------------------------------------------------------------
// Create bus map
//------------------------------------------------------------------------------

typedef std::map<std::string, GenericRouter*(*)(sc_module_name&, sc_time&)> busmap;

inline busmap *create_busmap() {
  busmap *m = new busmap();
  
  // add bus create functions to busmap

  // Buses that dont use transaction extensions
  (*m)["SimpleBus"] = create_SimpleBus;
  
  // Buses that use transaction extensions
#if (defined EXTENDED_TRANSACTION) && (EXTENDED_TRANSACTION == PLB)
  (*m)["PLB"]       = create_PLB;
#endif
  
  return m;
}



#endif
