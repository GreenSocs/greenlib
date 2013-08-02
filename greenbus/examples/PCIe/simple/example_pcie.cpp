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


#include <systemc.h>


#include "globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeRouter.h"

#include "MySimplePCIeRootComplex.h"
#include "PCIeSendDevice.h"
#include "PCIeRecvDevice.h"

//#include "ShowSCObjects.h"

                              
// //////////////////////////////////////
//  main
// //////////////////////////////////////

int sc_main(int, char**)
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT);
  //sc_report_handler::set_actions(SC_WARNING, SC_ABORT);

  cout << endl << "------------------------------------------" << endl << endl;
	
  MySimplePCIeRootComplex pcieRootComplex("Simple_PCIe_Root_Complex");

  PCIeRecvDevice pcieSl1("Recv_Slave");
  PCIeRecvDevice pcieSl2("Second_Recv_Slave");
  PCIeSendDevice pcieMa1("Sender_Master");

  PCIeRouter testrouter10("PCIeRouter10");
  testrouter10.upstream_port(pcieRootComplex.mAPI);

  PCIeRouter router("PCIeRouter0");
  router.upstream_port(testrouter10.downstream_port);

  //ShowSCObjects::showSCObjects();

  pcieMa1.mAPI.base_addr = (MAddr)0x00000000F0000001LL;
  pcieMa1.mAPI.high_addr = (MAddr)0x00000000FF000000LL;

  pcieSl1.mAPI.base_addr = (MAddr)0x0000000000000000LL;
  pcieSl1.mAPI.high_addr = (MAddr)0x00000000F0000000LL;
  
  pcieSl2.mAPI.base_addr = 0xFFFFFFFFFFFFFF00LL;
  pcieSl2.mAPI.high_addr = 0xFFFFFFFFFFFFFFFFLL;
  

  PCIeRouter testrouter1("PCIeRouter1");
  testrouter1.upstream_port(router.downstream_port);

  PCIeRouter testrouter2("PCIeRouter2");
  testrouter2.upstream_port(testrouter1.downstream_port);

  PCIeRouter testrouter3("PCIeRouter3");
  testrouter3.upstream_port(testrouter1.downstream_port);

  PCIeRouter testrouter4("PCIeRouter4");
  testrouter4.upstream_port(testrouter3.downstream_port);

  // connect PCIe Devices to the router ///////////////////
  testrouter3.downstream_port(pcieMa1.mAPI);
  router.downstream_port(pcieSl1.mAPI);

  testrouter4.downstream_port(pcieSl2.mAPI);

  
  // manually set address map
  cout << "------ force creating of ID map -------" << endl;
  router.m_PCIeAddressMap.generate_ID_map(); // force creating of ID map
  cout << "------ set manual addresses -------" << endl;
  testrouter3.add_Memory_address_range(pcieMa1.mAPI);
  router.     add_Memory_address_range(pcieSl1.mAPI);
  testrouter4.add_Memory_address_range(pcieSl2.mAPI);
  cout << "------ ------------------- -------" << endl;
  
  sc_start();

  cout << "------ ------------------- -------" << endl;

  //testrouter1.upstream_port.m_ConfReg.show_registers();
  //testrouter1.upstream_port.m_ConfReg.set_Memory_Limit(0xFFFF);
  //testrouter1.upstream_port.m_ConfReg.show_registers();

  // Test setting after end_of_elaboration
  // testrouter4.m_PCIeAddressMap.add_address_range_to_port((MAddr)0xFFFFFFFFFFFFFFF0LL, (MAddr)0xFFFFFFFFFFFFFFFFLL, 1);

  return 0;
}
