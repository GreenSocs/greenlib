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

#include "TestMasterDevice.h"
#include "ReceiverSlaveDevice.h"


int sc_main(int argc, char** argv) {
  
  //sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  //sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DISPLAY);
  //sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DO_NOTHING);
  //sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/wrong_register_access", sc_core::SC_DO_NOTHING);

  // Configure warnings about accesses to write protected register bits:
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);
  
  GS_INIT_STANDARD_GREENCONTROL;
  
  ReceiverSlaveDevice slave_dev("ReceiverSlaveDevice");
  TestMasterDevice test_send_dev("TestMasterDevice");
  
  test_send_dev.m_master_port.get_bus_port()( slave_dev.m_slave_port.get_bus_port());
  
  std::cout << std::endl << "--------- START simulation ---------" << std::endl << std::endl;	
  sc_core::sc_start();
  std::cout << std::endl << "---------- END simulation ---------- " << std::endl << std::endl;	
  
  return 0;
}
