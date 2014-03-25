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
// GreenReg example which demonstrates the creation of an
//  additional register container.
//
// Remark that it is not possible to create a register 
//  container with a not gr_(sub)device, because the 
//  container constructor needs a parent reference.
//
// This is more a code example than a runnable example that
//  does meaningful things.
//


#include <boost/config.hpp>
#include <systemc>
#include <greenreg/greenreg.h>

#include "greencontrol/config.h"

#define SHOW(name, msg) \
{ std::printf("@%s /%d (%s): ", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count(), name); std::cout << msg << std::endl; }


/// Device owning registers being accessed by the Master device over the bus 
class Device 
: public gs::reg::gr_device
{
public:
  
  gs::reg::gr_register_container my_container;
  
  SC_HAS_PROCESS( Device );
  Device(sc_core::sc_module_name name) 
  : gr_device(name, gs::reg::INDEXED_ADDRESS, 2, NULL)
  , my_container("additional_register_container", gs::reg::INDEXED_ADDRESS, 4, *this) // this is an ADDITIONAL register container ("r" is existing by default)
  {
		// create register in std container
    r.create_register( "Reg1", "Test Register", /*offset*/0x01,
                       /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       /*init value*/0x00000001, /*write mask*/0xFFFFFFFF, /*reg width*/32 );
		std::cout << "created single IO register Reg1 = 0x" << std::hex << r[0x01] << std::dec<< " in std container "<< r.name() << std::endl<< std::endl;

		// create register in additional container
    my_container.create_register( "Reg2", "Add. Test Register", /*offset*/0x03,
                                 /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                                 /*init value*/0xaaaaaaaa, /*write mask*/0xFFFFFFFF, /*reg width*/32 );
		std::cout << "created single IO register Reg2 = 0x" << std::hex << my_container[0x03] << std::dec<< " in "<< my_container.name() << std::endl<< std::endl;
    
    std::cout << std::endl;
  }
  
	~Device () {
	}
  
	void end_of_elaboration() {
	}
  
};

int sc_main(int argc, char** argv) {
  
  sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DISPLAY);
  
  // Configure warnings about accesses to write protected register bits:
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);
  
  gs::ctr::GC_Core core(); 
  gs::cnf::ConfigPlugin configPlugin(); 
  
  Device dev("RegisterDevice");
  
  sc_core::sc_start();
  Sleep(3000);
  return EXIT_SUCCESS;
}
