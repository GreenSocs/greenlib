// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
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

//
// Simple example with two generic devices which communicate to each other
// via GenericTransaction OR PCIeTransaction (see defines).
// When using PCIeTransactions they can be connected with a PCIeRouter
//
// This example is based on the example greenbus/examples/simple
//

#include <systemc.h>

// ////////////// User defines: //////////////////////////// //

// also see globals.h !!

// define USE_PCIE_ROUTER_CONNECT to connect the example modules through a PCIe Router
// does not work without defined USE_PCIE_TRANSACTION
// Test:
//      Whether Generic Devices can be connected to a PCIe Router
//#define USE_PCIE_ROUTER_CONNECT

// define USE_GENERIC_ROUTER_CONNECT to connect the generic modules 
// and the PCIe module to a generic router
// does not work without defined USE_PCIE_TRANACTION
// Test:
//      Whether PCIe Devices can be connected to a Generic Router
#define USE_GENERIC_ROUTER_CONNECT

// ///////////////////////////////////////////////////////// //

#include "globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeRouter.h"

#ifdef USE_GENERIC_ROUTER_CONNECT
# include "greenbus/transport/genericRouter.h"
# include "greenbus/protocol/SimpleBus/simpleBusProtocol.h"
# include "greenbus/scheduler/fixedPriorityScheduler.h"
# include "greenbus/transport/PCIe/PCIeGenericWrapper.h"
# include "PCIeSendDevice3.h"
# include "Mem.h"
#endif

#define MEMSIZE 10000
#include "genericSimplememory.h"
#include "genericSillysort.h"

#include "PCIeRootComplex2.h"

///  main
int sc_main(int, char**)
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT);

#ifndef USE_GENERIC_ROUTER_CONNECT
# ifndef USE_PCIE_ROUTER_CONNECT

  // works (directly connected without router):
  sillysort m("master");
  simplememory s("slave");
  m.init_port(s.target_port);

# else
  
  // works connnected with PCIeRouter
  
  cout << endl << "------------------------------------------" << endl << endl;
	
  sillysort gen_m("Generic_Master_Sillysort");
  simplememory gen_s("Generic_Slave_Memory");

  PCIeRootComplex2 pcieRootComplex("PCIe_Root_Complex");
  PCIeRouter router("PCIeRouter0");
  router.upstream_port(pcieRootComplex.mAPI);

  gen_s.target_port.base_addr = (MAddr) 0x0000000000000000LL;
  gen_s.target_port.high_addr = (MAddr) 0x000000000000FFFFLL;
  
  // /////  connect Devices to the router ///////////////////
  router.downstream_port(gen_m.init_port); // port 1 = device 0
  router.downstream_port(gen_s.target_port); // port 2 = device 1

  // manually set address map
  cout << "------ force creating of ID map -------" << endl;
  router.m_PCIeAddressMap.generate_ID_map(); // force creating of ID map
  cout << "------ set manual addresses -------" << endl;
  router.add_Memory_address_range(gen_s.target_port);
  cout << "------ sc_start() ------------- -------" << endl;

# endif

// ifdef USE_GENERIC_ROUTER_CONNECT
#else 

  sillysort gen_m("Generic_Master_Sillysort");
  simplememory gen_s("Generic_Slave_Memory");

  tlm::PCIe::PCIe2GenericPortWrapper wrapper_pcie_m("PCIe2GenericPortWrapper_pcie_m");
  PCIeSendDevice3 pcie_m("PCIe_Master");
  Mem gen_s2("Generic_Slave_DDR_Memory");

  gen_s.target_port.base_addr  = (MAddr) 0x0000000000000000LL;
  gen_s.target_port.high_addr  = (MAddr) 0x000000000000FFFFLL;
  gen_s2.slave_port.base_addr  = (MAddr) 0x0000000000010000LL;
  gen_s2.slave_port.high_addr  = (MAddr) 0x000000000001FFFFLL;
  // set address range
  pcie_m.mAPI.base_addr        = (MAddr) 0x0000000000100000LL;
  pcie_m.mAPI.high_addr        = (MAddr) 0x00000000001FFFFFLL;

  SimpleBusProtocol p("Protocol", sc_time(10, SC_NS));
  fixedPriorityScheduler s("Scheduler");
  GenericRouter r("Router");

  r.protocol_port(p);
  p.router_port(r);
  p.scheduler_port(s);
  

  gen_m.init_port(r.target_port);

  // ** BEGIN Connect PCIe Device to Generic Router **
  // ** Choose one alternative: **

  // 1. PCIe Device with wrapper (as Master and Slave)
  r.init_port(wrapper_pcie_m.target_port);
  wrapper_pcie_m.init_port(r.target_port);
  wrapper_pcie_m.pcie_port(pcie_m.mAPI);

  // 2. PCIe Device only as master
  //pcie_m.mAPI(r.target_port);

  // 3. PCIe Device only as slave
  //r.init_port(pcie_m.mAPI);

  // ** END Connect PCIe Device to Generic Router **

  r.init_port(gen_s.target_port);
  r.init_port(gen_s2.slave_port);

#endif

  sc_start();

  cout << "------ simulation done -------- -------" << endl;

  return 0;
}
