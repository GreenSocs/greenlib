//   GreenReg framework
//
// LICENSETEXT
//
//   Copyright (C) 2008 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
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

// test setting of register bits through config file etc

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg/greenreg.h>
#include <greenreg/greenreg_socket.h>

#include "greencontrol/config.h"
#include "greencontrol/gcnf/apis/configFileApi/configfile_tool.h"  // API Tool to read configuration file(s)

//#include "gr_gsgpsocket.h"

class MyMod : public gs::reg::gr_device {
  public:
    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS( MyMod );
    MyMod(sc_core::sc_module_name name, int opt) : gr_device(name, gs::reg::INDEXED_ADDRESS, 2, NULL), _opt(opt) {
      mApi = gs::cnf::GCnf_Api::getApiInstance(NULL);
      SC_THREAD(main_action);

      // create register
      r.create_register( "Reg0", "Test Register0 for MyMod", /*offset*/0x00,
        /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
        /*init value*/0xabc);// uses default write mask: 0xFFFFFFFF, reg width: 32, lock mask: 0x00
      r.create_register( "Reg1", "Test Register1 for MyMod", /*offset*/0x01,
        /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
        /*init value*/0xab);// uses default write mask: 0xFFFFFFFF, reg width: 32, lock mask: 0x00


      // access register
      std::cout << "created sharedio register Reg0 = 0x" << std::hex << r[0x00] << std::endl;
      std::cout << "created splitio register Reg1 = 0x" << std::hex << r[0x01] << std::endl;
    }
    ~MyMod () {
      //GC_UNREGISTER_CALLBACKS();
    }

    void end_of_elaboration() {
      std::cout << std::endl << "Entered MyMod::end_of_elaboration.." << std::endl;
      if (_opt==0) {
        GR_FUNCTION(MyMod, show_notification_reg0_DR);
        GR_SENSITIVE(r[0x00].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg0_DR", gs::reg::NOTIFY));
        
        GR_FUNCTION(MyMod, show_notification_reg1_DR);
        GR_SENSITIVE(r[0x01].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg1_DR", gs::reg::NOTIFY));
      }
      else if (_opt==1) {
        SC_METHOD( show_notification_reg0_SC );
        sensitive << r[0x00].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg0_SC", gs::reg::NOTIFY);
        dont_initialize();
        r[0x00].enable_events();

        SC_METHOD( show_notification_reg1_SC );
        sensitive << r[0x01].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg1_SC", gs::reg::NOTIFY);
        dont_initialize();
        r[0x01].enable_events();
      }
      std::cout << "..Finished MyMod::end_of_elaboration" << std::endl;
    }

    void main_action() {
      GC_REGISTER_PARAM_CALLBACK(mApi->getPar("MyMod.default_registers.Reg0"), MyMod, config_callback_reg0);

      GC_REGISTER_PARAM_CALLBACK(mApi->getPar("MyMod.default_registers.Reg1"), MyMod, config_callback_reg1);
      
      std::cout << std::endl << "Entered MyMod::main_action.." << sc_core::sc_time_stamp() << std::endl;
      std::cout << std::endl << "action r[0x01] = 12" << std::endl;
      r[0x01] = 12;
      wait(5, sc_core::SC_NS);

      std::cout << std::endl << "action r[0x01] = 0" << std::endl;
      r[0x01] = 0;
      wait(5, sc_core::SC_NS);

      std::cout << std::endl << "action set bit 5 of r[0x01] to true" << std::endl;
      r[0x01].b[5] = true;
      wait(5, sc_core::SC_NS);

      std::cout << std::endl << "action set bit 4 of r[0x01] to 1" << std::endl;
      r[0x01].b[4] = 1;

      wait(5, sc_core::SC_NS);
      std::cout << std::endl << "action set bit 3 of r[0x01] to true" << std::endl;
      r[0x01].b[3] = true;

      wait(5, sc_core::SC_NS);
      std::cout << std::endl << "action set bit 2 of r[0x01] to 1" << std::endl;
      r[0x01].b[2] = 1;

      wait(5, sc_core::SC_NS);
      std::cout << std::endl << "action set bit 5 of r[0x01] to 0" << std::endl;
      r[0x01].b[5] = 0;

      wait(5, sc_core::SC_NS);
      std::cout << std::endl << "action set bit 4 of r[0x01] to false" << std::endl;
      r[0x01].b[4] = false;

      wait(5, sc_core::SC_NS);
      std::cout << std::endl << "action set bit 3 of r[0x01] to 0" << std::endl;
      r[0x01].b[3] = 0;
      
      std::cout << "..Finished MyMod::main_action " << sc_core::sc_time_stamp() << "\n";
    }

    void show_notification_reg0_DR() {
      std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg0 (DR)" << "  value = 0x" << r[0x00] << std::endl;
    } 

    void show_notification_reg0_SC() {
      std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg0 (SC)" << "  value = 0x" << r[0x00] << std::endl;
    } 

    void show_notification_reg1_DR() {
      std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg1 (DR)" << "  value = 0x" << r[0x01] << std::endl;
    }

    void show_notification_reg1_SC() {
      std::cout << sc_core::sc_time_stamp() << " ****** got register notification for Reg1 (SC)" << "  value = 0x" << r[0x01] << std::endl;
    }

    void config_callback_reg0(gs::gs_param_base& par) {
      std::cout << "config_callback for " << par.getName() << " of type " << par.getTypeString();
      if (par.is_destructing()) {
        std::cout << " - Is just being destructed!" << std::endl;
      } 
      else {
        std::cout << " - new value: " << par.getString() << std::endl;
      }
    }

    void config_callback_reg1(gs::gs_param_base& par) {
      std::cout << "config_callback for " << par.getName() << " of type " << par.getTypeString();
      if (par.is_destructing()) {
        std::cout << " - Is just being destructed!" << std::endl;
      } 
      else {
        std::cout << " - new value: " << par.getString() << std::endl;
      }
    }

  protected:
    gs::cnf::cnf_api* mApi;
  private:
    int _opt;
    sc_event* m_reg0_change_event;
    sc_event* m_reg1_change_event;
};

int sc_main(int argc, char** argv) {
  sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  //sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  //sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DO_NOTHING);

  // Configure warnings about accesses to write protected register bits:
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);
  
  // Argument handling
  int i = 0;
  bool failure = false;
  if (argc == 2) {
    i = atoi(argv[1]);
    if (i != 1 && i != 2) failure = true;
  }
  if (argc > 2 || failure ) {
    std::cout << "usage: name <option 1-2>"<<std::endl;
    std::cout << "\t(default) 0 for registering GR_FUNCTION (function callbacks)"<<std::endl;
    std::cout << "\t          1 for registering SC_METHOD (events notification)"<<std::endl;
    return 1;
  }

  /// GreenControl and GreenConfig
  gs::ctr::GC_Core core("ControlCore");
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

  gs::cnf::ConfigFile_Tool configTool("ConfigFileTool");
  configTool.config("configFile.cfg");
  MyMod mod("MyMod", i);

  std::cout << "\nEntering sc_start : simulation starting\n";  
  sc_core::sc_start();    // (1000, SC_NS);            // Start the simulation
  std::cout << "Exited sc_start : simulation finished\n\n";  

  return 0;
}
