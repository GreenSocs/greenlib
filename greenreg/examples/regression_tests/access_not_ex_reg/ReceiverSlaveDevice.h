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
    
		// create register
    r.create_register( "Reg1", "Test Register1 for ReceiverSlaveDevice", /*offset*/0x01,
                      /*config*/gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      /*init value*/0xab, /*write mask*/0xFFFFFFFF, /*reg width*/32, /*lock mask*/0x00 );
		std::cout << "created splitio register Reg1 = 0x" << std::hex << r[0x01] << std::dec<< std::endl<< std::endl;
    r[0x01].disable_events();
    
    std::cout << std::endl;
  }
	~ReceiverSlaveDevice () {
    GC_UNREGISTER_CALLBACKS();
	}
  
	void end_of_elaboration() {
    std::cout << "Entered ReceiverSlaveDevice::end_of_elaboration" << std::endl;
    
    std::cout << "register GR_FUNCTION usr_out notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_GR);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::USR_OUT_WRITE, "written_to_reg1_GR", gs::reg::NOTIFY));
    
    std::cout << "register GR_FUNCTION POST bus read notification rule" << std::endl;
    GR_FUNCTION(ReceiverSlaveDevice, show_notification_reg1_POST_busread_GR);
    GR_SENSITIVE(r[0x01].add_rule( gs::reg::POST_READ, "bus_post_read_from_reg1_GR", gs::reg::NOTIFY));
    
    //std::cout << std::endl;
    
    std::cout << std::endl;
    
    std::cout << "Finished ReceiverSlaveDevice::end_of_elaboration" << std::endl;
	}
    
  void main_action_THREAD() {
    std::cout << std::endl << std::endl << "Entered ReceiverSlaveDevice::main_action_THREAD:" << std::endl;
    std::cout << "Finished ReceiverSlaveDevice::main_action" << std::endl;
	}
  
	void show_notification_reg1_POST_busread_GR() {
    SHOW("show_notification_reg1_POST_busread_GR", "****** got register notification (GR_FUNCTION callback) for Reg1 bus access");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
	void show_notification_reg1_GR() {
    SHOW("show_notification_reg1_GR", "****** got register notification (GR_FUNCTION callback) for Reg1 usr out access");
    std::cout << "   value = " << r[0x01] <<"=0x"<<std::hex << r[0x01] <<std::dec<< std::endl;
	}
  
};

