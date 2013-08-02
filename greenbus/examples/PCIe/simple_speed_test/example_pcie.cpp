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


//
// Speedtest example:
// - Root complex does nothing
// - Sender sends transactions (configure in globals.h)
//   (no create_transaction, no data copy etc.)
// - Recevier receives but does nothing more
//
                              
// //////////////////////////////////////
//  main
// //////////////////////////////////////

int sc_main(int, char**)
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT);

  MySimplePCIeRootComplex pcieRootComplex("Simple_PCIe_Root_Complex");

  PCIeRecvDevice pcieRecv("PCIeRecv");
  PCIeSendDevice pcieSend("PCIeSender");

  PCIeRouter pcierouter("PCIeRouter");
  pcierouter.upstream_port(pcieRootComplex.mAPI);

  pcieSend.mAPI.base_addr = (MAddr)0x00000000F0000001LL;
  pcieSend.mAPI.high_addr = (MAddr)0x00000000FF000000LL;

  pcieRecv.mAPI.base_addr = (MAddr)0x0000000000000000LL;
  pcieRecv.mAPI.high_addr = (MAddr)0x000000000F000000LL;
  
  // connect PCIe Devices to the router ///////////////////
  pcierouter.downstream_port(pcieSend.mAPI);
  pcierouter.downstream_port(pcieRecv.mAPI);
  
  // manually set address map
  pcierouter.m_PCIeAddressMap.generate_ID_map(); // force creating of ID map
  pcierouter.add_Memory_address_range(pcieSend.mAPI);
  pcierouter.add_Memory_address_range(pcieRecv.mAPI);
  pcierouter.add_Memory_address_range(pcieRecv.mAPI);
  
  sc_start();

  return 0;
}
