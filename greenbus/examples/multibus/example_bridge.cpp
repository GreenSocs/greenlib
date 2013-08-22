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
 * This example uses the genericBridge to connect two GreenBusses together.
 */

#include <systemc.h>

#ifdef USE_DUST
#include "greenbus/utils/dust_model_and_transactions.h"
#endif

#include "greenbus/transport/genericRouter.h"
#include "greenbus/protocol/SimpleBus/simpleBusProtocol.h"
#include "greenbus/scheduler/fixedPriorityScheduler.h"
#include "greenbus/transport/PLB/PLB.h"
#include "greenbus/protocol/PLB/PLBBusProtocol.h"
#include "greenbus/scheduler/dynamicPriorityScheduler.h"
#include "greenbus/transport/genericBridge.h"

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
	
  // master for the first GreenBus
  sillysort m1("MasterMark",0x4010, "Call me or I will call you");

  // slaves for the second GreenBus
  simplememory sl1("SlaveWolle",3);
  sl1.setAddress(0x4000, 0x4100);

  // configure first GreenBus
  SimpleBusProtocol<GenericTransaction, GenericPhase> p1("SimpleBus", sc_time(10, SC_NS));
  fixedPriorityScheduler<GenericTransaction, GenericPhase> s1("SimpleBus-Scheduler");
  GenericRouter<GenericTransaction, GenericPhase, GenericRouterAccess> r1("SimpleBus-Router");
  r1.protocol_port(p1);
  p1.router_port(r1);
  p1.scheduler_port(s1);

  // configure second GreenBus
  PLBBusProtocol p2("PLB",10,SC_NS);
  dynamicPriorityScheduler s2("PLB-Scheduler",10,SC_NS); 
  GenericRouter<GenericTransaction, GenericPhase, GenericRouterAccess> r2("PLB-Router");  
  r2.protocol_port(p2);
  p2.router_port(r2);
  p2.scheduler_port(s2);

  // bus bridge
  GenericBridge<GenericTransaction, GenericPhase> bridge("SimpleBus_2_PLB");
  bridge.setAddress(0x4000, 0x5000);

  // set up bridge from SimpleBus to PLB
  bridge.init_port(r2.target_port);
  r1.init_port(bridge.target_port);  
  
  // connect master to SimpleBus
  m1.init_port(r1.target_port);

  // connect slaves to PLB
  r2.init_port(sl1.target_port);


  // run simulation
  sc_start(1, SC_MS);
  
  return 0;
}
