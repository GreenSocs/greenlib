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

#ifndef __MyPCIeRootComplex_h__
#define __MyPCIeRootComplex_h__

#include <systemc.h>

#include "platform_globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/api/PCIe/PCIeRootComplex.h"

using namespace tlm;
using namespace tlm::PCIe;


class MyPCIeRootComplex
 : public tlm::PCIe::PCIeRootComplex // must be public for API checks,
{
friend class PCIeRootComplex;

public:

  SC_HAS_PROCESS(MyPCIeRootComplex);

  // Constructor
  MyPCIeRootComplex(sc_module_name name_) : 
    PCIeRootComplex(name_)
  {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
    cout<<"Constructor MyPCIeRootComplex " << name() <<endl;
#endif
    SC_THREAD(main_action);
  }
    
  void main_action();

protected:

  void do_MemoryRead(MAddr addr, unsigned int data_size, bool lock = false);
  void do_unlock();
  void do_MemoryWrite(MAddr addr, unsigned int data_size);
  
protected:
  
  sc_event m_received_PME_TO_Ack;
  
};


#endif
