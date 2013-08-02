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
// PCIe test platform with multiple Sender and Receiver Devices
//
//
//

#include <systemc.h>

//#include "ShowSCObjects.h"

#include "platform_globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeRouter.h"

#include "MyPCIeRootComplex.h"
#include "PCIeSendDevice.h"
#include "PCIeRecvInfoDevice.h"

#ifdef ENABLE_ROUTING_TEST_SCENARIO
# include "PCIeRoutingTestSendDevice.h"
#endif
                              
///  main
int sc_main(int, char**)
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT);

#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl << "------------------------------------------" << endl << endl;
#endif

  // instantiate partizipants
  MyPCIeRootComplex rootComplex("rootComplex");

  PCIeRecvInfoDevice recv1("recv1");
  PCIeRecvInfoDevice recv2("recv2");
  PCIeRecvInfoDevice recv3("recv3");
  PCIeRecvInfoDevice recv4("recv4");
#ifdef ENABLE_ROUTING_TEST_SCENARIO
  PCIeRoutingTestSendDevice send1("send1_RoutingTester");
#else
  PCIeSendDevice     send1("send1");
#endif
  PCIeSendDevice     send2("send2");
  PCIeSendDevice     send3("send3");

  PCIeRouter router1("router1");
  PCIeRouter router2("router2");
  PCIeRouter router3("router3");
  PCIeRouter router4("router4");

  // connect Devices
  rootComplex.downstream_port(send1.mAPI);

  router1.upstream_port(rootComplex.downstream_port);
  router1.downstream_port(send2.mAPI);
  router1.downstream_port(recv1.mAPI);

  router2.upstream_port(rootComplex.downstream_port);
  router2.downstream_port(recv2.mAPI);

  router3.upstream_port(router2.downstream_port);
  router3.downstream_port(recv3.mAPI);
  router3.downstream_port(recv4.mAPI);

  router4.upstream_port(router2.downstream_port);
  router4.downstream_port(send3.mAPI);

  // set addresses
  send1.mAPI.base_addr = (MAddr) 0x000000000000A100LL;
  send1.mAPI.high_addr = (MAddr) 0x000000000000A1FFLL;
  send2.mAPI.base_addr = (MAddr) 0x000000000000A200LL;
  send2.mAPI.high_addr = (MAddr) 0x000000000000A2FFLL;
  send3.mAPI.base_addr = (MAddr) 0x000000000000A300LL;
  send3.mAPI.high_addr = (MAddr) 0x000000000000A3FFLL;
  //recv1 has multiple address ranges, see above
  recv2.mAPI.base_addr = (MAddr) 0x000000000000B200LL;
  recv2.mAPI.high_addr = (MAddr) 0x000000000000B2FFLL;
  recv3.mAPI.base_addr = (MAddr) 0x000000000000B300LL;
  recv3.mAPI.high_addr = (MAddr) 0x000000000000B3FFLL;
  recv4.mAPI.base_addr = (MAddr) 0x000000000000B400LL;
  recv4.mAPI.high_addr = (MAddr) 0x000000000000B4FFLL;

  //ShowSCObjects::showSCObjects();

  
  // manually set address map
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << "------ force creating of ID map -------" << endl;
#endif
  rootComplex.m_PCIeAddressMap.generate_ID_map(); // force creating of ID map
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << "------ set manual addresses -------" << endl;
#endif

  // Set Memory Address Space
  rootComplex.add_Memory_address_range(send1.mAPI);
  router1.add_Memory_address_range(send2.mAPI);
  router1.add_Memory_address_range(recv1.mAPI, 0x000000000000B100LL, 0x000000000000B1FFLL);
  router1.add_Memory_address_range(recv1.mAPI, 0x000000000000D100LL, 0x000000000000D1FFLL);
  router2.add_Memory_address_range(recv2.mAPI);
  router3.add_Memory_address_range(recv3.mAPI);
  router3.add_Memory_address_range(recv4.mAPI);
  // router2.add_Memory_address_range(send3.upstream_port, 0x000000000000D100LL, 0x000000000000D1FFLL); // Regression test address range already registered for other device
  // router3.add_Memory_address_range(recv4.mAPI,          0x000000000000D0FFLL, 0x000000000000D200LL); // Regression test overlapping regions
  router4.add_Memory_address_range(send3.mAPI);
  
  // Set I/O Address Space
  router1.add_IO_address_range(recv1.mAPI, 0x000000000000B050LL, 0x000000000000B250LL);
  router3.add_IO_address_range(recv3.mAPI, 0x000000000000B300LL, 0x000000000000B3FFLL);
  router3.add_IO_address_range(recv4.mAPI, 0x000000000000E000LL, 0x000000000000E1FFLL);

#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << "------ ------------------- -------" << endl;
#endif
  
  sc_start();

  return 0;
}
