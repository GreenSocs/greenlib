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
// Simple example with a PCIe master which sends and reads data
// from a generic memory slave device via PCIeTransactions.
//

#include <systemc.h>

// ////////////// User defines: //////////////////////////// //

// see
#include "mixed_globals.h"

// ///////////////////////////////////////////////////////// //

#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeRouter.h"

#include "PCIeRootComplex2.h"

// for test PCIe Master -> generic Slave
#include "PCIeMaDevice.h"
#include "Mem.h"

// for test Generic Master -> PCIe Slave
#include "GenericMaDevice.h"
#include "PCIeRecvDevice2.h"
                              
///  main
int sc_main(int, char**)
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT);

  cout << endl << "------------------------------------------" << endl << endl;

  // for test PCIe Master -> generic Slave
  PCIeMaDevice pcieMa1("PCIe_Master");
  Mem genMemSl("Generic_Receiver_Memory");

  // for test Generic Master -> PCIe Slave
  GenericMaDevice genMa2("Generic_Master");
  PCIeRecvDevice2 pcieSl2("PCIe_Receiver");

  PCIeRootComplex2 pcieRootComplex("PCIe_Root_Complex");
  PCIeRouter router("PCIeRouter");
  router.upstream_port(pcieRootComplex.mAPI);

  genMemSl.slave_port.base_addr = (MAddr) 0x0000000000000000LL;
  genMemSl.slave_port.high_addr = (MAddr) 0x000000000000FFFFLL;
  pcieSl2.mAPI.base_addr = (MAddr) 0x0000000000010000LL;
  pcieSl2.mAPI.high_addr = (MAddr) 0x00000000FFFFFFFFLL;
  
  // /////  connect Devices to the router ///////////////////
  // for test PCIe Master -> generic Slave
  router.downstream_port(pcieMa1.mAPI); // port 1 = device 0
  router.downstream_port(genMemSl.slave_port); // port 2 = device 1
  // for test Generic Master -> PCIe Slave
  router.downstream_port(genMa2.init_port);
  router.downstream_port(pcieSl2.mAPI);

  // manually set address map
  cout << "------ force creating of ID map -------" << endl;
  router.m_PCIeAddressMap.generate_ID_map(); // force creating of ID map
  cout << "------ set manual addresses -------" << endl;
  // Set Memory address space
  router.add_Memory_address_range(genMemSl.slave_port);
  router.add_Memory_address_range(pcieSl2.mAPI);
  // Set identical I/O address space
  router.add_IO_address_range(genMemSl.slave_port);
  router.add_IO_address_range(pcieSl2.mAPI);
  cout << "------ sc_start() ------------- -------" << endl;


  sc_start();

  cout << "------ ------------------- -------" << endl;

  return 0;
}
