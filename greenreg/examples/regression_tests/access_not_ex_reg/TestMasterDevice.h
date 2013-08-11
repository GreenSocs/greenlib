//   GreenReg framework
//
// LICENSETEXT
//
//   Copyright (C) 2009 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
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
// GreenReg regression test example testing 
// warnings for bus accesses to not existing registers
//

//#define GS_DEBUG

#include <boost/config.hpp>
#include <systemc>
#include <greenreg/greenreg.h>
#include <greenreg/greenreg_socket.h>

#include "greencontrol/all.h"

#include "greenreg/greenreg_socket/greenreg_socket.h"									// DRF Port classes

#define SHOW(name, msg) \
{ std::printf("@%s /%d (%s): ", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count(), name); std::cout << msg << std::endl; }



/// Device sending register accesses over the bus
class TestMasterDevice 
: public gs::reg::gr_device 
{
public:
  
  gs::reg::greenreg_socket< gs::gp::generic_master> m_master_port;	// TLM Master Port declaration
  
  GC_HAS_CALLBACKS();
	SC_HAS_PROCESS( TestMasterDevice );
	
  TestMasterDevice(sc_core::sc_module_name name) 
  : gr_device(name, gs::reg::INDEXED_ADDRESS, 2, NULL)
  , m_master_port( "master_port" )							// TLM bus master port
  {
    SC_THREAD( send_thread );
  }
  
  void send_thread() {
    wait(100, sc_core::SC_NS);
    std::cout << std::endl;
    SHOW(name(), "Start sending bus register accesses");
    
    unsigned int cr;
  
    // read existing register
    SHOW(name(), "---- SC_THREAD Reading reg [0x01]");
    cr = m_master_port->read( 0x01, 4 );						// Read 4 bytes
    SHOW(name(), "     read value: " << cr << std::endl);
    wait( 5, sc_core::SC_NS );

    // read not existing register
    SHOW(name(), "---- SC_THREAD Reading not existing reg [0x33]");
    cr = m_master_port->read( 0x33, 4 );						// Read 4 bytes
    SHOW(name(), "     Should have caused a warning");
    SHOW(name(), "     read value: " << cr << std::endl);
    wait( 5, sc_core::SC_NS );
  }
  
};

