// LICENSETEXT
// 
//   Copyright (C) 2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
// 
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT


      
#define GS_VERBOSE
// use chunking in master and slave
#define USE_CC
// write responses needed in master
#define WRITE_RESPONSE_NEEDED
// write responses sent by slave
#define WRITE_RESPONSE_SEND
// If master and slave use blocking API
//#define USE_BLOCKING_API


#define CLK_CYCLE 10

// GreenControl + GreenConfig
#include "greencontrol/config.h"

#include <systemc>

#include "TestMaster.h"
#include "TestSlave.h"


//
//
//  Test Scenario Example
//
//
// 


/// main
int sc_main(int argc, char* argv[])
{

  /// GreenControl Core instance
  gs::ctr::GC_Core       core();
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

  
  TestSlave sl("Slave",3);
  sl.setAddress(0x4000,0x400F);

  TestMaster ma("Master");
  ma.init_port.target_addr = 0x4000;
  
  ma.init_port(sl.target_port);
  sc_core::sc_start();

  std::cout << std::endl << "Simulation stopped"<<std::endl;
  
  return 0;
}

