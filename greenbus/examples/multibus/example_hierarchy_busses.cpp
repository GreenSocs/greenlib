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

/**
 * Example for more than one GreenBusses in different hierarchy layers.
 */

#define VERBOSE                                                      


#include <systemc.h>

// DUST structure reflection + transaction recording
#ifdef USE_DUST
#include "greenbus/utils/dust_model_and_transactions.h"
#endif

// includes to build a SimpleBus GreenBus
#include "greenbus/transport/genericRouter.h"
#include "greenbus/protocol/SimpleBus/simpleBusProtocol.h"
#include "greenbus/scheduler/fixedPriorityScheduler.h"

// test master and test slave
#include "sillysort.h"
#include "simplememory.h"

// the basement
#include "basement.h"


                                                       

////////////////////////////////////////
// main
////////////////////////////////////////

int sc_main(int, char**)
{

#ifdef USE_DUST
  DUST_INITIALIZE;
#endif
	
  // masters for the ground floor GreenBus
  sillysort m1("Master_0_ground_floor",0x4010, "Call me or I will call you");
  sillysort m3("Master_1_ground_floor",0xffffff00, "My bus even can handle timeouts");
  sillysort m4("Master_2_ground_floor",0x1202, "TLM rocks");

  // slaves for the ground floor GreenBus
  simplememory sl1("Slave_0_ground_floor",1);
  sl1.setAddress(0x4000, 0x4050);
  simplememory sl2("Slave_1_ground_floor",19);
  sl2.setAddress(0xffffff00, 0xffffffff);
  simplememory sl3("Slave_2_ground_floor",4);
  sl3.setAddress(0x1200, 0x120f);


  // configure ground floor GreenBus
  SimpleBusProtocol<GenericTransaction, GenericPhase> p1("SimpleBus", sc_time(10, SC_NS));
  fixedPriorityScheduler<GenericTransaction, GenericPhase> s1("SimpleBus-Scheduler");
  GenericRouter<GenericTransaction, GenericPhase, GenericRouterAccess> r1("SimpleBus-Router");
  r1.protocol_port(p1);
  p1.router_port(r1);
  p1.scheduler_port(s1);

    // connect masters to ground floor bus 
  m3.init_port(r1.target_port);
  m4.init_port(r1.target_port);
  m1.init_port(r1.target_port);

  // connect slaves to ground floor bus
  r1.init_port(sl1.target_port);
  r1.init_port(sl3.target_port);
  r1.init_port(sl2.target_port);

  // create basement
  // (note: in this design, there are no connections between basement and ground floor!)
  basement bment("Basement");

  // run simulation
  sc_start(1, SC_MS);
  
  return 0;
}
