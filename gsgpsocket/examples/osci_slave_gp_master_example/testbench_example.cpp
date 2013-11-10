// LICENSETEXT
// 
//   Copyright (C) 2007-2009 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
//     GreenSocs Ltd
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

#include "../nb_example/Master.h"
#include "../../unit_test/tlm/common/include/models/SimpleATTarget1.h" // target from the tlm2.0 sources


//
//
// Example with an  GPSocket Master  which sends to an  OSCI Slave
//                  ---------------                     ----------
// 
//
//

/// Main
int sc_main(int argc, char* argv[])
{
 
  /// GreenControl Core instance
  gs::ctr::GC_Core       core();
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);
  
  
  // use one master of the two:
  Master m1("M1");
  m1.burst_length = 4;
  
  //OSCI_Slave s1("OSCI_Slave");
  SimpleATTarget1 s1("OSCI_Slave");

  
  /// bind TLM2 initiators to TLM2 target sockets on SimpleBus
  m1.init_port(s1.socket);
   
  sc_core::sc_start();
  
  std::cout << std::endl << "Simulation stopped"<<std::endl;
  
  return 0;
}
