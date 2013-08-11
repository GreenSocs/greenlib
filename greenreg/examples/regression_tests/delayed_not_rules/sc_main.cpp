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
// raw GreenReg test example
//

//#define GS_PARAM_CALLBACK_VERBOSE

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
    unsigned int cw;
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Bus access Reg1
    
    SHOW(name(), "---- SC_THREAD Bus Read current value reg 0x01");
    cr = m_master_port->read( 0x01, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    cw = 0xFF;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    m_master_port->write( 0x01, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x01, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    cw = 0x0D;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    m_master_port->write( 0x01, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x01, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Bus access Reg2
    
    SHOW(name(), "---- SC_THREAD Bus Read current value reg 0x02");
    cr = m_master_port->read( 0x02, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    cw = 0xFF;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    m_master_port->write( 0x02, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x02, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    cw = 0x0D;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    m_master_port->write( 0x02, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x02, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
  }
  
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
  : gr_device(name, gs::reg::INDEXED_ADDRESS, 3, NULL)
  , m_slave_port( "slave_port", r, 0x0, 0xFFFFFFFF)		// TLM Slave port
  {
    
		SC_THREAD(main_action_THREAD);
    
		// create register
    r.create_register( "Reg5", "Test Register5", /*offset*/0x01,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xaaaaaaaa, /*write mask*/0xFFFFFFFF, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created single IO register Reg5 = 0x" << std::hex << r[0x01] << std::dec<< std::endl<< std::endl;
    r[0x01].disable_events(); // this will be useless in future releases when event switch default is disabled
    
		// create register
    r.create_register( "Reg2", "Test Register2 for ReceiverSlaveDevice", /*offset*/0x02,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SPLIT_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xbb, /*write mask*/0xFFFFFFFF, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created splitio register Reg2 = 0x" << std::hex << r[0x02] << std::dec<< std::endl<< std::endl;
    r[0x02].disable_events(); // this will be useless in future releases when event switch default is disabled
    
    std::cout << std::endl;
  }
	~ReceiverSlaveDevice () {
    GC_UNREGISTER_CALLBACKS();
	}
  
	void end_of_elaboration() {
    std::cout << "Entered ReceiverSlaveDevice::end_of_elaboration" << std::endl;
    
    std::cout << "register not delayed GR_FUNCTION without params for post write" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_not_delayed_no_params);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::POST_WRITE, "post_write_1", gs::reg::NOTIFY));
    
    std::cout << "register delayed GR_FUNCTION without params for post write" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_delayed_no_params);
    GR_DELAYED_SENSITIVE(r[0x01].add_rule( gs::reg::POST_WRITE, "post_write_2", gs::reg::NOTIFY),
                         sc_core::sc_time(20,sc_core::SC_NS));
    
    std::cout << "register not delayed GR_FUNCTION with params for post write" << std::endl;
    GR_FUNCTION_PARAMS(ReceiverSlaveDevice, show_notification_reg1_not_delayed_params);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::POST_WRITE, "post_write_3", gs::reg::NOTIFY));
    
    std::cout << "register delayed GR_FUNCTION with params for post write" << std::endl;
    GR_FUNCTION_PARAMS(ReceiverSlaveDevice, show_notification_reg1_delayed_params);
    GR_DELAYED_SENSITIVE(r[0x01].add_rule( gs::reg::POST_WRITE, "post_write_4", gs::reg::NOTIFY),
                         sc_core::sc_time(10, sc_core::SC_NS));
    
    std::cout << std::endl;
    
    unsigned int cw;
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access Reg1
    
    SHOW(name(), "current value reg 0x01: " << r[0x01] << "="<< std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    cw = 0xFF;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    r[0x01] = cw;
    SHOW(name(), "     new value: " << r[0x01] << "="<< std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access Reg2
    
    SHOW(name(), "current value reg 0x02: " << r[0x02] << "="<< std::hex<<"0x"<<r[0x02]<<std::dec<< std::endl);
    
    cw = 0xFF;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    r[0x02] = cw;
    SHOW(name(), "     new value: " << r[0x02] << "="<< std::hex<<"0x"<<r[0x02]<<std::dec<< std::endl);
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // ----
    
    std::cout << std::endl;
    
    std::cout << "Finished ReceiverSlaveDevice::end_of_elaboration" << std::endl;
	}
  
  void main_action_THREAD() {
    std::cout << std::endl << std::endl << "Entered ReceiverSlaveDevice::main_action_THREAD:" << std::endl;
    std::cout << "Finished ReceiverSlaveDevice::main_action" << std::endl;
	}
  
	void show_notification_reg1_not_delayed_no_params() {
    SHOW("show_notification_reg1_not_delayed_no_params", "****** got NOT DELAYED register notification (GR_FUNCTION callback) for Reg1 post write");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_delayed_no_params() {
    SHOW("show_notification_reg1_delayed_no_params", "****** got DELAYED register notification (GR_FUNCTION callback) for Reg1 post write");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_not_delayed_params(gs::reg::transaction_type* &tr, const sc_core::sc_time& delay) {
    SHOW("show_notification_reg1_not_delayed_params", "****** got NOT DELAYED (transaction, delay) register notification (GR_FUNCTION callback) for Reg1 post write");
    SHOW("show_notification_reg1_not_delayed_params", "       transaction ID = " << tr->getTransID());
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_delayed_params(gs::reg::transaction_type* &tr, const sc_core::sc_time& delay) {
    SHOW("show_notification_reg1_delayed_params", "****** got DELAYED (transaction, delay) register notification (GR_FUNCTION callback) for Reg1 post write");
    SHOW("show_notification_reg1_delayed_params", "       " << "delayed for "<< delay.to_string());
    SHOW("show_notification_reg1_not_delayed_params", "       transaction ID = " << tr->getTransID());
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
  
  
  void reg2_split_synchronizer_write_GR() {
    std::cout << "sync usr reg2" << std::endl;
    r[0x02].i.set(r[0x02].o, false);
	}
  void reg2_split_synchronizer_bus_write_GR() {
    std::cout << "sync bus reg2" << std::endl;
    r[0x02].o.set(r[0x02].i, false);
	}
  
};

int sc_main(int argc, char** argv) {
  
  //sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
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
