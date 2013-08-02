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

#include "regdemo_globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeRouter.h"

#include "MyPCIeRootComplex.h"
#include "PCIeSendDevice.h"
#include "PCIeRecvInfoDevice.h"

 
//
// This example demonstrates the usage of registers in PCIe peripherals
// - as far as this is implemented:
//   - TODO: read memory space size
//   - TODO: read I/O space size
//   - TODO: set memory address
//   - TODO: set I/O address
//   - TODO: use enable register (config reg 0)
//
//
/* 
   Topology:

          rootComplex
              |
              |
           router1
           /     \
          |       |
      router2   send1
        |
        | 
      recv1
 
 */

///  main
int sc_main(int, char**)
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT);

  cout << endl << "------------------------------------------" << endl << endl;

  // instantiate partizipants
  MyPCIeRootComplex rootComplex("rootComplex");

  PCIeRecvInfoDevice recv1("recv1");
  PCIeSendDevice     send1("send1");

  PCIeRouter router1("router1");
  PCIeRouter router2("router2");

  // connect topology
  router1.upstream_port(rootComplex.downstream_port);
  router1.downstream_port(send1.mAPI);
  router2.upstream_port(router1.downstream_port);
  router2.downstream_port(recv1.mAPI);

  cout << "------ ------------------- -------" << endl;
  
  sc_start();

  cout << "Addresses used:"<<endl;
  cout << "  send1 base address = " << send1.mAPI.base_addr.get() << endl;
  cout << "        high address = " << send1.mAPI.high_addr.get() << endl;
  cout << "  recv1 base address = " << recv1.mAPI.base_addr.get() << endl;
  cout << "        high address = " << recv1.mAPI.high_addr.get() << endl;
  
  return 0;
}
