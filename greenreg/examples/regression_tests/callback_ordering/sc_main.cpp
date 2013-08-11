//   GreenReg framework
//
// LICENSETEXT
//
//   Copyright (C) 2008 : GreenSocs Ltd
//    http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
//   Puneet Arora
//     GreenSocs
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
// Regression test
//  Change order of notification rules
//
// regression test for task node 2179
// 'Need ability to block register updates in PRE_WRITE rule'
// Solved by allowing ordering notification rules
//

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg/greenreg.h>

#include "greencontrol/all.h"

class MyMod : public gs::reg::gr_device {
public:
  SC_HAS_PROCESS( MyMod );
  MyMod(sc_core::sc_module_name name) : gr_device(name, gs::reg::INDEXED_ADDRESS, 1, NULL) {
    SC_THREAD(main_action);

    r.create_register("Reg0", "Register0_MyMod", 0x00, gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH, 0xabc);
    r[0x00].disable_events();

    std::cout << "created sharedio register Reg0 = 0x" << std::hex << r[0x00] << std::endl;
  }

  void end_of_elaboration() {
    std::cout << "\nEntered MyMod::end_of_elaboration..\n";

    GR_FUNCTION(MyMod, show_notification_reg0_DR_1);
    GR_SENSITIVE(r[0x00].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg0_DR_1", gs::reg::NOTIFY));

    GR_FUNCTION(MyMod, show_notification_reg0_DR_2);
    GR_SENSITIVE(r[0x00].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg0_DR_2", gs::reg::NOTIFY));

    GR_FUNCTION(MyMod, show_notification_reg0_DR_3);
    GR_SENSITIVE(r[0x00].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg0_DR_3", gs::reg::NOTIFY));

    std::cout << "..Finished MyMod::end_of_elaboration\n";
  }

  void main_action() {
    wait(5, sc_core::SC_NS);
    r[0x00] = 12;
    std::cout << std::endl;
    
    wait(5, sc_core::SC_NS);
    r[0x00].get_user_obuf_write_rules().move_rule_to_back("written_to_reg0_DR_1");
    r[0x00] = 212;
    std::cout << std::endl;

    wait(5, sc_core::SC_NS);
    r[0x00].get_user_obuf_write_rules().move_rule_to_front("written_to_reg0_DR_3");
    r[0x00] = 412;
    std::cout << std::endl;

    wait(5, sc_core::SC_NS);
    std::cout << "..Finished MyMod::main_action " << sc_core::sc_time_stamp() << "\n";
  }

  void show_notification_reg0_DR_1() {
    std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg0 (DR)..1" << "  value = 0x" << r[0x00] << std::endl;
  } 
  void show_notification_reg0_DR_2() {
    std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg0 (DR)..2" << "  value = 0x" << r[0x00] << std::endl;
  } 
  void show_notification_reg0_DR_3() {
    std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg0 (DR)..3" << "  value = 0x" << r[0x00] << std::endl;
  } 
};

int sc_main(int argc, char** argv) {
  sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp

  // Configure warnings about accesses to write protected register bits:
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);

  GS_INIT_STANDARD_GREENCONTROL;

  MyMod mod("MyMod");

  std::cout << "\nEntering sc_start : simulation starting\n";  
  sc_core::sc_start(); 
  std::cout << "Exited sc_start : simulation finished\n";  

  return 0;
}
