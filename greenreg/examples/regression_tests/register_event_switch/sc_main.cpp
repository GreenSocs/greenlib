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
// GreenReg enable events test example
//
// Tests the notification rule event switch (for GR_FUNCTIONs and SC_METHODs etc.).
// Mainly reg1 (0x01) is accessed by the owner module and by another Module via bus.
// The enable event switch of that register is toggled to check the notification of the 
// SC_METHODs being sensitive to notification rule events of that register.
//

#define GS_PARAM_CALLBACK_VERBOSE

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
    SC_METHOD( send_method );
    sensitive << ev;
  }
  
  void send_thread() {
    wait(100, sc_core::SC_NS);
    std::cout << std::endl;
    SHOW(name(), "Start sending bus register accesses");
    
    unsigned int cr;
    
    SHOW(name(), "---- SC_THREAD Reading reg [0x01]");
    cr = m_master_port->read( 0x01, 4 );						// Read 4 bytes
    SHOW(name(), "     read value: " << cr << std::endl);
    wait( 5, sc_core::SC_NS );

    // This will run the send_method - which should result in a warning 
    // of not processed pre rules because of events not making sense within methods
    ev.notify();
    
    wait( 50, sc_core::SC_NS );

    SHOW(name(), "---- SC_THREAD Reading reg [0x01]");
    cr = m_master_port->read( 0x01, 4 );						// Read 4 bytes
    SHOW(name(), "     read value: " << cr << std::endl);
    wait( 5, sc_core::SC_NS );

    // This will run the send_method - which should work fine because of callbacks
    ev.notify();
    
  }
  
  void send_method() {
    unsigned int cr;

    SHOW(name(), "---- SC_METHOD Reading reg [0x01]");
    cr = m_master_port->read( 0x01, 4 );						// Read 4 bytes
    SHOW(name(), "     read value: " << cr << std::endl);
    next_trigger();
  }
  
  sc_core::sc_event ev;
  
};



/// Device owning registers being accessed a) by the Master device over the bus b) by itself
class ReceiverSlaveDevice 
: public gs::reg::gr_device 
{
public:
  
  gs::reg::greenreg_socket< gs::gp::generic_slave> m_slave_port;

  GC_HAS_CALLBACKS();
	SC_HAS_PROCESS( ReceiverSlaveDevice );
	
  ReceiverSlaveDevice(sc_core::sc_module_name name) 
  : gr_device(name, gs::reg::INDEXED_ADDRESS, 2, NULL)
  , m_slave_port( "slave_port", r, 0x0, 0xFFFFFFFF)		// TLM Slave port
  {

		SC_THREAD(main_action_THREAD);

		SC_METHOD(main_action_METHOD);
    
		// create register
    r.create_register( "Reg0", "Test Register0 for ReceiverSlaveDevice", /*offset*/0x00,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SPLIT_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xabc);// uses default write mask: 0xFFFFFFFF, reg width: 32, lock mask: 0x00
    std::cout << "created sharedio register Reg0 = 0x" << std::hex << r[0x00] << std::dec << std::endl<< std::endl;
    r[0x00].disable_events(); // useless in future releases when the event switch default is disabled
    
    r.create_register( "Reg1", "Test Register1 for ReceiverSlaveDevice", /*offset*/0x01,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xab, /*write mask*/0xFFFFFFFF, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created splitio register Reg1 = 0x" << std::hex << r[0x01] << std::dec<< std::endl<< std::endl;
    r[0x01].disable_events(); // useless in future releases when the event switch default is disabled
    
		// create bit range in register Reg1
		r[0x01].br.create("bit_range_4_6", 4, 6);
		std::cout << "bit range bit_range_4_6 = " << r[0x01].br["bit_range_4_6"] << std::endl;
		
    std::cout << std::endl;
  }
	~ReceiverSlaveDevice () {
    GC_UNREGISTER_CALLBACKS();
	}
  
	void end_of_elaboration() {
    std::cout << "Entered ReceiverSlaveDevice::end_of_elaboration" << std::endl;

    std::cout << "register GR_FUNCTION usr out notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg0_GR);
    GR_SENSITIVE(r[0x00].add_rule( gs::reg::USR_OUT_WRITE /*gs::reg::PRE_WRITE*/ /*gs::reg::USR_IN_WRITE*/ /*gs::reg::USR_OUT_WRITE*/, "written_to_reg0_GR", gs::reg::NOTIFY));
    
    std::cout << "register GR_FUNCTION usr_out notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_GR);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg1_GR", gs::reg::NOTIFY));
    
    std::cout << "register GR_FUNCTION PRE bus read notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_PRE_busread_GR);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::PRE_READ, "bus_pre_read_from_reg1_GR", gs::reg::NOTIFY));

    std::cout << "register SC_METHOD usr_out notification rule" << std::endl;
    SC_METHOD(show_notification_reg1_SC);
    sensitive << r[0x01].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg1_SC", gs::reg::NOTIFY);

    std::cout << "register GR_FUNCTION POST bus read notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_POST_busread_GR);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::POST_READ, "bus_post_read_from_reg1_GR", gs::reg::NOTIFY));

    //std::cout << std::endl;
    
    std::cout << "register GR_FUNCTION bit range notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_bitr46_notification_GR);
    GR_SENSITIVE(r[0x01].br["bit_range_4_6"].add_rule( /*gs::reg::POST_WRITE*/ /*gs::reg::PRE_WRITE*/ gs::reg::USR_IN_WRITE /*gs::reg::USR_OUT_WRITE*/, "written_to_reg1_bit4_GR", gs::reg::NOTIFY));

    std::cout << "register SC_METHOD bit range notification rule" << std::endl;
    SC_METHOD(show_bitr46_notification_SC);
    sensitive << r[0x01].br["bit_range_4_6"].add_rule( /*gs::reg::POST_WRITE*/ /*gs::reg::PRE_WRITE*/ gs::reg::USR_IN_WRITE /*gs::reg::USR_OUT_WRITE*/, "written_to_reg1_bit4_SC", gs::reg::NOTIFY);
    dont_initialize();											// Don't run this process during init'zn
    
    std::cout << std::endl;
    
    std::cout << "Finished ReceiverSlaveDevice::end_of_elaboration" << std::endl;
	}
  
	void main_action_METHOD() {
    std::cout << std::endl << std::endl << "ReceiverSlaveDevice::main_action_METHOD:" << std::endl;
    std::cout << "  action r[0x00] = 50" << std::endl;
    r[0x00] = 50;
    std::cout << "  action r[0x01] = 150" << std::endl;
    r[0x01] = 150;
  }

  void main_action_THREAD() {
    std::cout << std::endl << std::endl << "Entered ReceiverSlaveDevice::main_action_THREAD:" << std::endl;
    wait(5, sc_core::SC_NS);
    std::cout << "  action r[0x00] = 50" << std::endl;
    r[0x00] = 50;
    wait(5, sc_core::SC_NS);
    
    std::cout << "  action r[0x01] = 5" << std::endl;
    r[0x01] = 5;
    wait(5, sc_core::SC_NS);
    std::cout << std::endl << "  action reset reg to 0" << std::endl;
    r[0x01] = 0;
    std::cout << std::endl << "  action set bit 5 to 1" << std::endl;
    r[0x01].b[5] = true;

    std::cout << "  action r[0x01].br[\"bit_range_4_6\"] = 1" << std::endl;
    r[0x01].br["bit_range_4_6"] = 1;
    wait(5, sc_core::SC_NS);
    std::cout << "  action r[0x01].b[31] = true" << std::endl;
    r[0x01].b[31] = true;
    wait(2, sc_core::SC_NS);
    std::cout << "  action r[0x01].b[31] = false" << std::endl;
    r[0x01].b[31] = false;
    wait(3, sc_core::SC_NS);
    std::cout << "   value register Reg1 = " << r[0x01] << std::endl;
    wait(5, sc_core::SC_NS);
    std::cout << "  action r[0x00] = 16" << std::endl;
    r[0x00] = 16;
    
    SHOW(name(), "Finished ReceiverSlaveDevice::main_action local access, now switching");
    std::cout << std::endl << std::endl << std::endl;
    wait (50, sc_core::SC_NS);
    
    SHOW(name(), "enable events for reg 0x01 (current value = "<< r[0x01] <<")");
    r[0x01].enable_events();

    std::cout << "  action r[0x01].br[\"bit_range_4_6\"] = 4" << std::endl;
    r[0x01].br["bit_range_4_6"] = 4;

    wait (50, sc_core::SC_NS);

    SHOW(name(), "disable events for reg 0x01 (current value = "<< r[0x01] <<")");
    r[0x01].disable_events();

    std::cout << "Finished ReceiverSlaveDevice::main_action" << std::endl;
	}
  
	void show_notification_reg0_GR() {
    SHOW("show_notification_reg0_GR", "****** got register notification (GR_FUNCTION callback) for Reg0");
    std::cout << "   value = " << r[0x00] <<"=0x"<<std::hex << r[0x00] <<std::dec<< std::endl;
	}
	void show_notification_reg1_GR() {
    SHOW("show_notification_reg1_GR", "****** got register notification (GR_FUNCTION callback) for Reg1");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_SC() {
    SHOW("show_notification_reg1_Sc", "****** got register notification (SC_METHOD event notification) for Reg1");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
  
	void show_bitr46_notification_GR() {
    SHOW("show_bitr46_notification_GR", "****** got bit notification (GR_FUNCTION callback) for Reg1 bit range 4-6");
    std::cout << "   value = " << r[0x01].br["bit_range_4_6"] << std::endl;
	}
	void show_bitr46_notification_SC() {
    SHOW("show_bitr46_notification_SC", "****** got bit notification (SC_METHOD event notification) for Reg1 bit range 4-6");
    std::cout << "   value = " << r[0x01].br["bit_range_4_6"] << std::endl;
	}
  
  void show_notification_reg1_PRE_busread_GR() {
    SHOW("show_notification_reg1_PRE_busread_GR", "****** got PRE bus read notification (GR_FUNCTION callback) for Reg1");
    std::cout << "   value = " << r[0x01] << std::endl;
  }
  void show_notification_reg1_POST_busread_GR() {
    SHOW("show_notification_reg1_POST_busread_GR", "****** got POST bus read notification (GR_FUNCTION callback) for Reg1");
    std::cout << "   value = " << r[0x01] << std::endl;
  }
  
};

int sc_main(int argc, char** argv) {
  
  sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  //sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DISPLAY);
  //sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DO_NOTHING);
  
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
