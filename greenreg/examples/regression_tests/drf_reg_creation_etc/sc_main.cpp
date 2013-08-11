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

// greenreg tests (against various bugs filed)
//(1) register_block are properly created and accessible in the INDEXED_ADDRESS mode of device,
//(2) Proper error message is given out when the type of register is skipped while creation,
//(3) Proper error message is given out when bus read fail due to incorrect offset,
//    node 2171
//(4) Proper error message is given out when bus write fail due to incorrect offset.
//    node 2171

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg/greenreg.h>
#include <greenreg/greenreg_socket.h>

#include "greencontrol/config.h"
#include "greencontrol/gcnf/apis/configFileApi/configfile_tool.h"  // API Tool to read configuration file(s)
//#include "gr_gsgpsocket.h"

class newMod : public gs::reg::gr_device {
public:
  SC_HAS_PROCESS(newMod);
  newMod(sc_core::sc_module_name name, int opt) : gr_device(name, gs::reg::INDEXED_ADDRESS, (opt==1)?14:1, NULL), _opt(opt) {
    SC_THREAD(main_action);
    if (opt==1) {
      r.create_register_block("register block0 in newMod", 0, 7, gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH, 0xcab);
      r.create_register_block("register block1 in newMod", 7, 14, gs::reg::STANDARD_REG | gs::reg::SPLIT_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH, 0xfab);
      std::cout << "Created two register blocks of size 7 each and type SINGLE and SPLIT\n";
    }
    else if (opt==2) {
      std::cout << "Trying to create a register without specifying the type bit..\n";
      r.create_register("Reg0", "Test Register0 for newMod", /*offset*/0x0,
			/*config*/gs::reg::STANDARD_REG | /*gs::reg::SINGLE_IO |*/ gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
			/*init value*/0xabc);//, /*write mask*/0xFF, /*reg width*/32, /*lock mask*/0x00 );
    }
    else {
      r.create_register("Reg0", "Test Register0 for newMod", /*offset*/0x0,
			/*config*/gs::reg::STANDARD_REG | gs::reg::SPLIT_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
			/*init value*/0xabc);//, /*write mask*/0xFF, /*reg width*/32, /*lock mask*/0x00 );
      std::cout << "Created a single register of type SPLIT_IO\n";
    }
  }
  void main_action() {
    std::cout << "\nEntered newMod::main_action.." << std::endl;
    wait(5, sc_core::SC_NS);
    std::cout << "value of r[0x0] 0x" << std::hex << r[0x0] << "\n";
    if (_opt==1) {
      std::cout << "Value of r[0xd] 0x" << std::hex << r[0xd] << "\n";
      std::cout << "Setting values at various registers\n";
      //r[0x0] = 5;
      r[0x1] = 15;
      r[0x2] = 50;
      r[0x3] = 51;
      r[0x4] = 52;
      r[0x5] = 53;
      r[0x6] = 54;
      r[0x7] = 45;
      r[0x8] = 55;
      r[0x9] = 75;
      r[0xa] = 85;
      r[0xb] = 65;
      r[0xc] = 95;
      //r[0xd] = 115;
    }
    else if (_opt==3) {
      unsigned int param = 11;
      if (r.bus_read(param, 0x0, 1))
        std::cout << "Bus read success at offset 0x0, " << std::dec << param << "\n";
      else
        std::cout << "Bus read failure at offset 0x0\n";
      if (r.bus_read(param, 0x1, 1))
        std::cout << "Bus read success at offset 0x1, " << std::dec << param << "\n";
      else
        std::cout << "Bus read failure at offset 0x1\n";
    }
    else if (_opt==4) {
      unsigned int param = 14;
      if (r.bus_write(param, 0x0, 1))
        std::cout << "Bus write success at offset 0x0 " << std::dec << param << "\n";
      else
        std::cout << "Bus write failure at offset 0x0\n";
      if (r.bus_write(param, 0x1, 1))
        std::cout << "Bus write success at offset 0x1" << std::dec << param << "\n";
      else
        std::cout << "Bus write failure at offset 0x1\n";
    }
    std::cout << "\nExiting newMod::main_action.." << std::endl;
  }
  void end_of_elaboration () {
    std::cout << "\nEntered newMod::end_of_elaboration..\n";
    if (_opt==1) {
      std::cout << "value of r[0xd] 0x" << std::hex << r[0xd] << "\n";
      r[0xd] = 50;
    }
    std::cout << "value of r[0x0] 0x" << std::hex << r[0x0] << "\n";
    r[0x0] = 5;
    std::cout << "\nExiting newMod::end_of_elaboration..\n";
  }

protected:
  int _opt;
};

#define helpMsg {\
    std::cout << "Please specify the flag to test one of the following three conditions (and rerun)\n";\
    std::cout << "\t (1) register_block are properly created and accessible in the INDEXED_ADDRESS mode of device,\n";\
    std::cout << "\t (2) Proper error message is given out when the type of register is skipped while creation,\n";\
    std::cout << "\t (3) Proper error message is given out when bus read fail due to incorrect offset,\n";\
    std::cout << "\t (4) Proper error message is given out when bus write fail due to incorrect offset.\n";\
    std::cout << "Please specify one of (1 2 3 4), as command line argument\n";\
    return 0; }

int sc_main(int argc, char** argv) {
  //sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
  //sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp

  // Configure warnings about accesses to write protected register bits:
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);
  
  int opt = 0;
  if (argc!=2) {helpMsg;}
  else {
    opt = atoi(argv[1]);
    std::cout << "You gave " << opt << "\n";
    if ((opt!=1) && (opt!=2) && (opt!=3) && (opt!=4)) helpMsg;
  }
  /// GreenControl and GreenConfig
  gs::ctr::GC_Core core("ControlCore");
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

  newMod nmod("newMod", opt);

  std::cout << "\nEntering sc_start : simulation starting\n";	
  sc_core::sc_start();		// (1000, SC_NS);						// Start the simulation
  std::cout << "Exited sc_start : simulation finished\n\n";	

  return( 0);
}
