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

#ifndef __PCIeRecvInfoDevice_h__
#define __PCIeRecvInfoDevice_h__

#include <systemc.h>

#include "regdemo_globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/api/PCIe/PCIeAPI.h"

using namespace tlm;
using namespace tlm::PCIe;


/// PCIe Device which prints out received messages
class PCIeRecvInfoDevice
 : public sc_module,
   public PCIe_recv_if
{
public:
  PCIeAPI mAPI;

public:

  SC_HAS_PROCESS(PCIeRecvInfoDevice);

  // Constructor
  PCIeRecvInfoDevice(sc_module_name name_) : 
    sc_module(name_),
    mAPI("PCIe_Recv_API", this)
  {
    cout <<endl<< "Constructor PCIeRecvInfoDevice " << name() <<endl;
    SC_THREAD(main_action);
    
    // Initialize configuration registers
    ConfReg::Register_init_struct ri;
    ri.header_type = 0; // device header
    mAPI.m_ConfReg.init(ri);

    // Initialize an I/O address space
    ConfReg::Base_address_struct ba;
    ba.BaseAddressSpaceIndicator = 1; // I/O space
    ba.BaseAddressPrefetchable = 1;
    ba.BaseAddressWidthType = 0; // 32 bit address
    ba.BaseAddressRequiredMemorySpaceAmount0HardwiredBits = 6;
    ba.address = 0;

    // Initialize a Memory address space
    mAPI.m_ConfReg.init_add_base_address(ba);
    ba.BaseAddressSpaceIndicator = 0; // Memory space
    ba.BaseAddressPrefetchable = 0;
    ba.BaseAddressWidthType = 0; // 32 bit address
    ba.BaseAddressRequiredMemorySpaceAmount0HardwiredBits = 4;
    ba.address = 0;
    mAPI.m_ConfReg.init_add_base_address(ba);
  
  }
  
  void main_action();
  
 /**
   * The PCIe_recv_if implementation
   */
  virtual void b_transact(PCIeTransactionHandle th);

private:
};

#endif