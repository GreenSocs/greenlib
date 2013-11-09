// LICENSETEXT
// 
//   Copyright (C) 2008-2009 : GreenSocs Ltd
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


//#define USE_STATIC_CASTS --> do not define in the source code. Do a `make STATIC=yes` instead.

#define CLK_CYCLE 10

// GreenControl + GreenConfig
#include "greencontrol/config.h"

#include <systemc>

#include "Master.h"
#include "Slave.h"


//
//
//  Example with a GP master and a GP slave
//                 ---------       --------
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
//  define USE_CC (Master.h) to use multiple data phases (write)
//                              and multiple response phases (read) (this also affects the slave!)
//  define USE_SETSDATA_TO_WRITE_DATA (Slave.h) to use setSData(...) in the Slave
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

  
  SlaveMem s1("Slave1",3);
  s1.setAddress(0x4000,0x400F);
  //Slave s2("Slave2",3); // zweiter Parameter: ACK-Delay in Taktzyklen
  //s2.setAddress(0x5000,0x500F);

  Master m1("M1");
  m1.init_port.target_addr = 0x4000;
  
  m1.init_port(s1.target_port);
  sc_core::sc_start();

  std::cout << std::endl << "Simulation stopped"<<std::endl;
  
  return 0;
}

