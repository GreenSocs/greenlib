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


#define CLK_CYCLE 10

//#include "ShowSCObjects.h"

// GreenControl + GreenConfig
#include "greencontrol/config.h"

#include <systemc>

#include "BidirModule.h"
#include "SlaveMemBidir.h"


//
//
//  Example with bidirectional GP modules doing nb or b communication
//               ------------------------
//
// Master:
//  The master sends multiple writes and afterwards multiple reads
//  to the default target address (set in this testbench below).
//
// Slave:
//  The slaves stores the writen data and allows to read this data 
//  again.
//
// 
// Behaviour of master and slave is configurable:
//  define USE_PV (Master.h) to use blocking PV calls
//
//
//


/// main
int sc_main(int argc, char* argv[])
{

  //sc_report_handler::set_actions(SC_ERROR, SC_ABORT); 

  /// GreenControl Core instance
  gs::ctr::GC_Core       core();
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

  
  SlaveMemBidir s1("Slave1",3);
  s1.setAddress(0x4000,0x400F);

  BidirModule m1("M1");
  m1.bidir_port.target_addr = 0x4000;
  

  // connect initiators and targets

  m1.bidir_port(s1.target_port);

  //ShowSCObjects::showSCObjects();

  sc_core::sc_start();

  std::cout << std::endl << "Simulation stopped"<<std::endl;
  
  return 0;
}

