//   GreenControl framework
//
// LICENSETEXT
//
//   Copyright (C) 2007 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
//   Wolfgang Klingauf <klingauf@eis.cs.tu-bs.de>
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
// simplememory see usercode/example_simplebus
//

#ifndef __MEM_H__
#define __MEM_H__

#include <systemc.h>

#include "globals.h"
#include "greenbus/transport/GP/GP.h"

// SimpleBus API
#include "greenbus/api/simplebusAPI.h"

// GreenControl and GreenConfig
//#include "greencontrol/gcnf/apis/gcnf_param/gcnf_param.h"

using namespace tlm;
//using gc::config::gcnf_param;


#include "ddr_ram.h"


/// Memory for demonstration platform
/**
 *
 * Instead of using this class the simplebusSlavePort 
 * may be instatiated in the testbench directly:
 * \code
 * simplebusSlavePort *slaveport = new simplebusSlavePort("memoryport");
 * ddr *memory = new ddr("Mem");
 * slaveport->slave_port(*memory);
 * slaveport->base_addr = 0x200;
 * slaveport->high_addr = 0x300;
 * \endcode
 * But in that case the base and high addresses cannot be parameters.
 */
class Mem
: public sc_module
{
public:
  /// SimpleBus slave port
  simplebusSlavePort slave_port;

  SC_HAS_PROCESS(Mem);
	
  /// Constructor
  Mem(sc_module_name name)
    : sc_module(name),
      slave_port("simplebus_slaveport"),
      ddr_mem("memory")
  { 
    ddr_mem.size = 100000;
    slave_port.slave_port(ddr_mem);
  }

private:
  /// Memory
  ddr ddr_mem;

};


#endif
