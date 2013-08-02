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
 * Example for more than one GreenBusses in a design.
 */

#include <systemc.h>

#ifdef USE_DUST
#include "greenbus/utils/dust_model_and_transactions.h"
#endif


#include "greenbus/transport/PLB/PLB.h"
#include "greenbus/transport/genericRouter.h"
#include "greenbus/protocol/SimpleBus/simpleBusProtocol.h"
#include "greenbus/scheduler/fixedPriorityScheduler.h"
#include "greenbus/scheduler/dynamicPriorityScheduler.h"
#include "greenbus/protocol/PLB/PLBBusProtocol.h"

#include "sillysort.h"
#include "simplememory.h"


                                                      
                                                       
////////////////////////////////////////
// main
////////////////////////////////////////

int sc_main(int, char**)
{

#ifdef USE_DUST
  DUST_INITIALIZE;
#endif
	
  // masters for the first GreenBus
  sillysort m1("MasterMark",0x4010, "Call me or I will call you");
  sillysort m3("MasterRobert",0xffffff00, "My bus even can handle timeouts");
  sillysort m4("MasterMarcus",0x1202, "TLM rocks");

  // masters for the second GreenBus
  sillysort m2("MasterWolle",0x00ff, "I want a faster bus");
  sillysort m5("MasterOliver",0x0400, "We dumped the core");

  // slaves for the first GreenBus
  simplememory sl1("Slave0",1);
  sl1.setAddress(0x4000, 0x4050);
  simplememory sl2("Slave1",19);
  sl2.setAddress(0xffffff00, 0xffffffff);
  simplememory sl3("Slave2",4);
  sl3.setAddress(0x1200, 0x120f);

  // slaves for the second GreenBus
  simplememory sl4("Slave3",3);
  sl4.setAddress(0x0000, 0x11ff);

  // configure first GreenBus
  SimpleBusProtocol p1("SimpleBus", sc_time(10, SC_NS));
  fixedPriorityScheduler s1("SimpleBus-Scheduler");
  GenericRouter r1("SimpleBus-Router");
  r1.protocol_port(p1);
  p1.router_port(r1);
  p1.scheduler_port(s1);

  // configure second GreenBus
  PLBBusProtocol p2("PLB",10,SC_NS);
  dynamicPriorityScheduler s2("PLB-Scheduler",10,SC_NS); 
  GenericRouter r2("PLB-Router");  
  r2.protocol_port(p2);
  p2.router_port(r2);
  p2.scheduler_port(s2);
  
  // connect masters to first bus 
  m3.init_port(r1.target_port);
  m4.init_port(r1.target_port);
  m1.init_port(r1.target_port);

  // connect masters to second bus
  m2.init_port(r2.target_port);
  m5.init_port(r2.target_port);

  // connect slaves to first bus
  r1.init_port(sl1.target_port);
  r1.init_port(sl3.target_port);
  r1.init_port(sl2.target_port);

  // connect slaves to second bus
  r2.init_port(sl4.target_port);

  // run simulation
  sc_start(1, SC_MS);
  
  return 0;
}
