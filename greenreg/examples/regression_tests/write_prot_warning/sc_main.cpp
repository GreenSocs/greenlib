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
// GreenReg regression test example testing warnings for
// write accesses to write protected register bits
//

#include <boost/config.hpp>
#include <systemc>
#include "greenreg/greenreg.h"
#include "greenreg/greenreg_socket/greenreg_socket.h"

#include "greencontrol/all.h"

#include "greenreg/greenreg_socket.h"									// DRF Port classes

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
    
    cw = 0xAD;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    m_master_port->write( 0x01, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x01, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    cw = 0xF0;
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
    
    cw = 0xBD;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    m_master_port->write( 0x02, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x02, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    cw = 0xF0;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    m_master_port->write( 0x02, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x02, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;

    // Switch warning behavior
    sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DO_NOTHING);
    sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DISPLAY);

    // This is ok (no warning) because we write a zero to write protected area
    cw = 0x0D;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    m_master_port->write( 0x01, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x01, 4 );						// read 4 bytes
    SHOW(name(), "     read value: " << cr << "="<< std::hex<<"0x"<<cr<<std::dec<< std::endl);
    std::cout << std::endl << std::endl;

    // This will cause a warning because we write the current value to the write protected area (instead of zero)
    cw = 0xAE;
    SHOW(name(), "---- SC_THREAD Bus Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    m_master_port->write( 0x01, cw, 4 );						// write 4 bytes
    SHOW(name(), "---- SC_THREAD Read back value");
    cr = m_master_port->read( 0x01, 4 );						// read 4 bytes
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
  : gr_device(name, gs::reg::INDEXED_ADDRESS, 6, NULL)
  , m_slave_port( "slave_port", r, 0x0, 0xFFFFFFFF)		// TLM Slave port
  {
    
		SC_THREAD(main_action_THREAD);
    
		// create register
    r.create_register( "Reg1", "Test Register1 for ReceiverSlaveDevice", /*offset*/0x01,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xaa, /*write mask*/0x0000000F, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created singleio register Reg1 = 0x" << std::hex << r[0x01] << std::dec<< std::endl<< std::endl;
    r[0x01].disable_events(); // this will be useless in future releases when event switch default is disabled
    
		// create register
    r.create_register( "Reg2", "Test Register2 for ReceiverSlaveDevice", /*offset*/0x02,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SPLIT_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xbb, /*write mask*/0x0000000F, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created splitio register Reg2 = 0x" << std::hex << r[0x02] << std::dec<< std::endl<< std::endl;
    r[0x02].disable_events(); // this will be useless in future releases when event switch default is disabled
    
    // create bit range in register Reg1
		r[0x01].br.create("bit_range_2_5", 2, 5);
		std::cout << "bit range bit_range_2_5 = 0x" <<std::hex<< r[0x01].br["bit_range_2_5"]<<std::dec << std::endl;

    // create bit range in register Reg1
		r[0x01].br.create("bit_range_0_3", 0, 3);
		std::cout << "bit range bit_range_0_3 = 0x" <<std::hex<< r[0x01].br["bit_range_0_3"]<<std::dec << std::endl;
    // create bit range in register Reg1
		r[0x01].br.create("bit_range_4_7", 4, 7);
		std::cout << "bit range bit_range_4_7 = 0x" <<std::hex<< r[0x01].br["bit_range_4_7"]<<std::dec << std::endl;
    
    // create register
    r.create_register( "Reg5", "Test Register5", /*offset*/0x05,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xaaaaaaaa, /*write mask*/0x0000FFFF, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created single IO register Reg5 = 0x" << std::hex << r[0x05] << std::dec<< std::endl<< std::endl;
    r[0x05].disable_events(); // this will be useless in future releases when event switch default is disabled

		// create bit range in register Reg5
		r[0x05].br.create("bit_range_4_15", 4, 15);
		r[0x05].br.create("bit_range_14_23", 14, 23);
       
    std::cout << std::endl;
  }
	~ReceiverSlaveDevice () {
    GC_UNREGISTER_CALLBACKS();
	}
  
	void end_of_elaboration() {
    std::cout << "Entered ReceiverSlaveDevice::end_of_elaboration" << std::endl;
    
    std::cout << "register GR_FUNCTION POST bus write notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_POST_buswrite_GR);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::POST_WRITE, "bus_post_write_to_reg1_GR", gs::reg::NOTIFY));

    std::cout << "register GR_FUNCTION POST write notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, reg2_split_synchronizer_bus_write_GR);
    GR_SENSITIVE(r[0x02].add_rule( gs::reg::POST_WRITE, "reg2_split_synchronizer_bus_write_GR", gs::reg::NOTIFY));
    
    std::cout << std::endl;
    
    unsigned int cw;

    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access Reg1

    SHOW(name(), "current value reg 0x01: " << r[0x01] << "="<< std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);

    cw = 0xFF;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    r[0x01] = cw;
    SHOW(name(), "     new value: " << r[0x01] << "="<< std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);

    cw = 0x0C;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x01]");
    r[0x01] = cw;
    SHOW(name(), "     new value: " << r[0x01] << "="<< std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);

    cw = 0xAA;
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
    
    cw = 0x0C;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    r[0x02] = cw;
    SHOW(name(), "     new value: " << r[0x02] << "="<< std::hex<<"0x"<<r[0x02]<<std::dec<< std::endl);
    
    cw = 0xBA;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg [0x02]");
    r[0x02] = cw;
    SHOW(name(), "     new value: " << r[0x02] << "="<< std::hex<<"0x"<<r[0x02]<<std::dec<< std::endl);
   
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access bits
    
    SHOW(name(), "current value reg 0x05: " << r[0x05] << "="<< std::hex<<"0x"<<r[0x05]<<std::dec<< std::endl);
    cw = 0x00;
    SHOW(name(), "---- Local Set bit 3 (allowed) of reg r[0x05] to "<<cw);
    r[0x05].b[3] = cw;
    SHOW(name(), "     new value: " << r[0x05] << "="<< std::hex<<"0x"<<r[0x05]<<std::dec<< std::endl);
    
    cw = 0x00;
    SHOW(name(), "---- Local Set bit 17 (NOT allowed) of reg r[0x05] to "<<cw);
    r[0x05].b[17] = cw;
    SHOW(name(), "     new value: " << r[0x05] << "="<< std::hex<<"0x"<<r[0x05]<<std::dec<< std::endl);
        
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access bit ranges

    cw = 0xFF;
    SHOW(name(), "---- Local Set (allowed) bit range r[0x05].br(\"bit_range_4_15\") to 0x"<<std::hex<<cw<<std::dec);
    r[0x05].br["bit_range_4_15"] = cw;
    SHOW(name(), "     new value: " << r[0x05] << "="<< std::hex<<"0x"<<r[0x05]<<std::dec<< std::endl);
    
    cw = 0xFF;
    SHOW(name(), "---- Local Set (NOT allowed) bit range r[0x05].br(\"bit_range_14_23\") to 0x"<<std::hex<<cw<<std::dec);
    r[0x05].br["bit_range_14_23"] = cw;
    SHOW(name(), "     new value: " << r[0x05] << "="<< std::hex<<"0x"<<r[0x05]<<std::dec<< std::endl);

    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access bit ranges (2)
    
    cw = 0xF;
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg bit_range [0x01].br[bit_range_2_5]");
    r[0x01].br["bit_range_2_5"] = cw;
    SHOW(name(), "     new value: " << std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg bit_range [0x01].br[bit_range_0_3]");
    r[0x01].br["bit_range_0_3"] = cw;
    SHOW(name(), "     new value: " << std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    SHOW(name(), "---- Local Write 0x"<<std::hex<<cw<<std::dec<<" to reg bit_range [0x01].br[bit_range_4_7]");
    r[0x01].br["bit_range_4_7"] = cw;
    SHOW(name(), "     new value: " << std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    // Manually access bits (2)
    
    SHOW(name(), "---- Local Write 1 to reg bit [0x01].b[0]");
    r[0x01].b[0] = 1;
    SHOW(name(), "     new value: " << std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    SHOW(name(), "---- Local Write 1 to reg bit [0x01].b[7]");
    r[0x01].b[7] = 1;
    SHOW(name(), "     new value: " << std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    SHOW(name(), "---- Local Write 1 to reg bit [0x01].b[6]");
    r[0x01].b[6] = 1;
    SHOW(name(), "     new value: " << std::hex<<"0x"<<r[0x01]<<std::dec<< std::endl);
    
    
    std::cout << "------------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Finished ReceiverSlaveDevice::end_of_elaboration" << std::endl;
	}
    
  void main_action_THREAD() {
    std::cout << std::endl << std::endl << "Entered ReceiverSlaveDevice::main_action_THREAD:" << std::endl;
    std::cout << "Finished ReceiverSlaveDevice::main_action" << std::endl;
	}
  

  void reg2_split_synchronizer_write_GR() {
    std::cout << "sync usr reg2" << std::endl;
    //std::cout << "r[0x02].i = " << r[0x02].i << std::endl;
    //std::cout << "r[0x02].o = " << r[0x02].o << std::endl;
    r[0x02].i.set(r[0x02].o, false); // Suppress write protected warning with false
    //std::cout << "r[0x02].i = " << r[0x02].i << std::endl;
    //std::cout << "r[0x02].o = " << r[0x02].o << std::endl;
	}

  void reg2_split_synchronizer_bus_write_GR() {
    std::cout << "sync bus reg2" << std::endl;
    //std::cout << "r[0x02].i = " << r[0x02].i << std::endl;
    //std::cout << "r[0x02].o = " << r[0x02].o << std::endl;
    r[0x02].o.set(r[0x02].i, false); // Suppress write protected warning with false
    //std::cout << "r[0x02].i = " << r[0x02].i << std::endl;
    //std::cout << "r[0x02].o = " << r[0x02].o << std::endl;
	}
  
  void show_notification_reg1_POST_busread_GR() {
    SHOW("show_notification_reg1_POST_busread_GR", "****** got register notification (GR_FUNCTION callback) for Reg1 bus read access");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_POST_buswrite_GR() {
    SHOW("show_notification_reg1_POST_buswrite_GR", "****** got register notification (GR_FUNCTION callback) for Reg1 bus write access");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_GR() {
    SHOW("show_notification_reg1_GR", "****** got register notification (GR_FUNCTION callback) for Reg1 usr out access");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
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
