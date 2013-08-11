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

// greenreg test example

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg/greenreg.h>
#include <greenreg/greenreg_socket.h>

#include "greencontrol/config.h"
#include "greencontrol/gcnf/apis/configFileApi/configfile_tool.h"  // API Tool to read configuration file(s)
#include "greencontrol/gcnf/apis/utils/show_database.h"

//#include "gr_gsgpsocket.h"

//# include "gsgpsocket/transport/GSGPSlaveSocket.h"

//using namespace gs;
//using namespace gs::gp;

class MyMod : public gs::reg::gr_device {
public:
	GC_HAS_CALLBACKS();
	SC_HAS_PROCESS( MyMod );
	MyMod(sc_core::sc_module_name name) 
  : gr_device(name, gs::reg::INDEXED_ADDRESS, 2, NULL) {
    
		SC_THREAD(main_action);
    
		// create register
		r.create_register( "Reg0", "Test Register0 for MyMod", /*offset*/0x00,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xabc);//, /*write mask*/0xFF, /*reg width*/32, /*lock mask*/0x00 );
		r.create_register( "Reg1", "Test Register1 for MyMod", /*offset*/0x01,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SPLIT_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xab, /*write mask*/0xFFFFFFFF, /*reg width*/32, /*lock mask*/0x00 );
    
    // enable events in register notifications
    r[0x00].enable_events();
    r[0x01].enable_events();
    
		// access register
		std::cout << "created sharedio register Reg0 = 0x" << std::hex << r[0x00] << std::dec << std::endl;
		std::cout << "created splitio register Reg1 = 0x" << std::hex << r[0x01] << std::dec << std::endl;
    
		// create bit in register Reg1
		/*std::cout << "Reg0 = ";
     for (int i = 31; i >= 0; i--) {
     std::cout << r[0x00].b[i];
     }
     std::cout << std::endl;
     std::cout << "Reg1 = ";
     for (int i = 31; i >= 0; i--) {
     std::cout << r[0x01].b[i];
     }
     std::cout << std::endl;*/
    
		/*std::cout << std::endl << "bit ";
     for (int i = 31; i >= 0; i--) {
     std::ostringstream intstr;
     intstr << (std::dec) << i;
     std::cout << intstr.str();
     if (intstr.str().length() == 1)
     std::cout << "   ";
     else if (intstr.str().length() == 2)
     std::cout << "  ";
     else assert (false);
     }
     std::cout << std::endl << "val ";
     for (int i = 31; i >= 0; i--) {
     std::cout << r[0x01].b[i] << "   ";
     }
     std::cout << std::endl;*/
    
		// create bit range in register Reg1
		r[0x01].br.create("bit_range_4_6", 4, 6);
		std::cout << "bit range bit_range_4_6 = " << r[0x01].br["bit_range_4_6"] << std::endl;
		std::cout << std::endl;
  }
	~MyMod () {
    GC_UNREGISTER_CALLBACKS();
	}
  
	void end_of_elaboration() {
    std::cout << "Entered MyMod::end_of_elaboration.."<< std::endl;
    //std::cout << "register notification rule" << std::endl;
    SC_METHOD( show_notification_reg0 );
    sensitive << r[0x00].add_rule( gs::reg::USR_OUT_WRITE /*gs::reg::PRE_WRITE*/ /*gs::reg::USR_IN_WRITE*/ /*gs::reg::USR_OUT_WRITE*/, "written_to_reg0", gs::reg::NOTIFY);
    dont_initialize();											// Don't run this process during init'zn

    SC_METHOD( show_notification_reg1 );
    sensitive << r[0x01].add_rule( gs::reg::USR_OUT_WRITE /*gs::reg::PRE_WRITE*/ /*gs::reg::USR_IN_WRITE*/ /*gs::reg::USR_OUT_WRITE*/, "written_to_reg1", gs::reg::NOTIFY);
    dont_initialize();											// Don't run this process during init'zn
    
    //std::cout << "register bit range notification rule" << std::endl;
    SC_METHOD( show_bitr46_notification );
    sensitive << r[0x01].br["bit_range_4_6"].add_rule( gs::reg::POST_WRITE /*gs::reg::PRE_WRITE*/ /*gs::reg::USR_IN_WRITE*/ /*gs::reg::USR_OUT_WRITE*/, "written_to_reg1_bit4", gs::reg::NOTIFY);
    dont_initialize();											// Don't run this process during init'zn
    //std::cout << std::endl;
    
    // THIS IS JUST A TEST, THIS IS __NOT__ THE RECOMMENDED WAY TO GET NOTIFIED ABOUT CONFIG CHANGES!! USE AN CALLBACK INSTEAD!
    mApi = gs::cnf::GCnf_Api::getApiInstance(this);
    m_reg0_change_event = &mApi->getPar("MyMod.default_registers.Reg0")->getUpdateEvent();
    SC_METHOD(event_reg0_change);
    sensitive << *m_reg0_change_event;
    dont_initialize();
    
    // THIS IS JUST A TEST, THIS IS __NOT__ THE RECOMMENDED WAY TO GET NOTIFIED ABOUT CONFIG CHANGES!! USE AN CALLBACK INSTEAD, SEE BELOW!
    m_reg1_change_event = &mApi->getPar("MyMod.default_registers.Reg1")->getUpdateEvent();
    SC_METHOD(event_reg1_change);
    sensitive << *m_reg1_change_event;
    dont_initialize();

    std::cout << "..Finished MyMod::end_of_elaboration"<< std::endl;
	}
  
	void main_action() {
    std::cout << "\nEntered MyMod::main_action.." << std::endl;
    // THIS IS THE RECOMMENDED WAY TO GET CALLED ON CONFIG PARAM CHANGES!!
    GC_REGISTER_PARAM_CALLBACK(mApi->getPar("MyMod.default_registers.Reg0"), MyMod, config_callback_reg0);
    GC_REGISTER_PARAM_CALLBACK(mApi->getPar("MyMod.default_registers.Reg1"), MyMod, config_callback_reg1);
    wait(5, sc_core::SC_NS);
    
    std::cout  << std::endl << "action r[0x01] = 5" << std::endl;
    r[0x01] = 5;
    wait(5, sc_core::SC_NS);
    
    std::cout << std::endl << "action r[0x01].br[\"bit_range_4_6\"] = 1" << std::endl;
    r[0x01].br["bit_range_4_6"] = 1;
    wait(5, sc_core::SC_NS);
    
    std::cout << std::endl << "action r[0x01].b[31] = true" << std::endl;
    r[0x01].b[31] = true;
    wait(5, sc_core::SC_NS);
    std::cout << "value register Reg1 = " << r[0x01] << std::endl;
    wait(5, sc_core::SC_NS);
    
    std::cout << std::endl << "action r[0x00] = 16" <<std::endl;
    r[0x00] = 16;
    std::cout << "..Finished MyMod::main_action\n";
	}
  
	void show_notification_reg0() {
    std::cout << sc_core::sc_time_stamp() << "****** got register notification for Reg0" << std::endl;
    std::cout << "   value = " << r[0x01] << std::endl;
	}
	void show_notification_reg1() {
    std::cout << sc_core::sc_time_stamp() << "****** got register notification for Reg1" << std::endl;
    std::cout << "   value = " << r[0x01] << std::endl;
	}
  
	void show_bitr46_notification() {
    std::cout << sc_core::sc_time_stamp() << "****** got bit notification for Reg1 bit range 4-6" << std::endl;
    std::cout << "   value = " << r[0x01].br["bit_range_4_6"] << std::endl;
	}
	/// SC_METHOD for parameter update event tests
	void event_reg0_change() {
    std::cout << "CHANGED MyMod.default_registers.Reg0, notified by event" << std::endl;
	}
	void event_reg1_change() {
    std::cout << "CHANGED MyMod.default_registers.Reg1, notified by event" << std::endl;
	}
	void config_callback_reg0(gs::gs_param_base& par) {
    std::cout << "config_callback for " << par.getName() << " of type " << par.getTypeString() << std::endl;
    if (par.is_destructing()) {
      std::cout << "Is just being destructed!" << std::endl;
    } else {
      std::cout << "Changed to (decimal) value: " << par.getString() << " = 0x" << std::hex << par.getValue<unsigned int>() << std::dec << std::endl;
    }
	}
	void config_callback_reg1(gs::gs_param_base& par) {
    std::cout << "config_callback for " << par.getName() << " of type " << par.getTypeString() << std::endl;
    if (par.is_destructing()) {
      std::cout << "Is just being destructed!" << std::endl;
    } else {
      std::cout << "Changed to (decimal) value: " << par.getString() << " = 0x" << std::hex << par.getValue<unsigned int>() << std::dec<< std::endl;
    }
	}
  
private:
  sc_core::sc_event* m_reg0_change_event;
	sc_core::sc_event* m_reg1_change_event;
protected:
	gs::cnf::cnf_api* mApi;
};

int sc_main(int argc, char** argv) {
  
  //sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  //sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  
  // Configure warnings about accesses to write protected register bits:
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);

  /// GreenControl and GreenConfig
  gs::ctr::GC_Core core("ControlCore");
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);
  
  gs::cnf::cnf_api *mApi = gs::cnf::GCnf_Api::getApiInstance(NULL);
  MyMod mod("MyMod");
  gs::gs_param_greenreg<uint_gr_t>* reg0Param = static_cast<gs::gs_param_greenreg<uint_gr_t>*>(mApi->getPar("MyMod.default_registers.Reg0"));
  gs::gs_param_greenreg<uint_gr_t>* reg1Param = static_cast<gs::gs_param_greenreg<uint_gr_t>*>(mApi->getPar("MyMod.default_registers.Reg1"));
  
  if (reg0Param) {
    std::cout << "found the reg0Param " << (*reg0Param).getName() << "\t0x" << std::hex << ((*reg0Param).getValue()) << "\n";
    (*reg0Param).setValue(1024);
    std::cout << "new value of reg0Param 0x" << std::hex << ((*reg0Param).getValue()) << "\n";
    (*reg0Param).setString("2048");
    std::cout << "newer value of reg0Param 0x" << std::hex << ((*reg0Param).getValue()) << "\n";
    if (reg1Param) {
	    (*reg0Param) = (*reg1Param);
	    std::cout << "updated value of reg0Param 0x" << std::hex << ((*reg0Param).getValue()) << "\n";
	    std::cout << "again value of reg0Param 0x" << atoi(((*reg0Param).getString()).c_str()) << "\n";
    }
    else
	    std::cout << "DID NOT find the reg1Param\n";
  }
  else
    std::cout << "DID NOT find the reg0Param\n";
  
  gs::cnf::ConfigFile_Tool configTool("ConfigFileTool");
  configTool.config("configFile.cfg");
  if (reg0Param)
    std::cout << "found the reg0Param 0x" << std::hex << ((*reg0Param).getValue()) << "\n";
  else
    std::cout << "DID NOT find the reg0Param\n";
  if (reg1Param)
    std::cout << "found the reg1Param 0x" << std::hex << ((*reg1Param).getValue()) << "\n";
  else
    std::cout << "DID NOT find the reg1Param\n";
  std::cout << "\nEntering sc_start : simulation starting\n";	
  sc_core::sc_start();		// (1000, SC_NS);						// Start the simulation
  std::cout << "Exited sc_start : simulation finished\n\n";	
  
  if (reg0Param)
    (*reg0Param).setValue(256);
  if (reg0Param)
    (*reg0Param).setValue(256);
  
  gs::cnf::show_database();

  return( 0);
}
