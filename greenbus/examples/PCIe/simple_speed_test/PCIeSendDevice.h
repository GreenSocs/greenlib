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

#ifndef __PCIeSendDevice_h__
#define __PCIeSendDevice_h__

#include <systemc.h>

#include "globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/api/PCIe/PCIeAPI.h"

using namespace tlm;
using namespace ::PCIe;
using namespace tlm::PCIe;


class PCIeSendDevice
 : public sc_module
{
public:
  PCIeAPI mAPI;

public:

  SC_HAS_PROCESS(PCIeSendDevice);

  // Constructor
  PCIeSendDevice(sc_module_name name_) : 
    sc_module(name_),
    mAPI("Ma1_PCIeAPI", NULL)
  {
    SC_THREAD(main_action);
  }
    
  void main_action();
  void do_MemWrite(MAddr addr, unsigned int data_size);
  void do_MemoryRead(MAddr addr, unsigned int data_size);
  
};

#endif
