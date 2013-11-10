// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
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


// uncomment the following line if you want 1000 runs...
//#define SPEEDTEST

  

#define MEMSIZE 10000

#ifdef USE_GPSOCKET
  // GreenControl + GreenConfig
# include "greencontrol/config.h"
#endif

#include <boost/config.hpp> // needed for SystemC 2.1
#include <systemc>

#include "sillysort.h"
#include "simplememory.h"

//
//
// Example using blocking GP communication on master side
//
//  Should work with the old GreenBus as well as with GPSocket
//  define USE_GPSOCKET when using the GPSocket
//
//

/// main
int sc_main(int, char**)
{
//  sc_report_handler::set_actions(SC_ERROR, SC_ABORT); 
  
#ifdef USE_GPSOCKET
  /// GreenControl Core instance
  gs::ctr::GC_Core       core();
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);
#endif

  sillysort m("sillysort");
  simplememory s("simplememory");

  m.init_port(s.target_port);

  sc_core::sc_start(-1);

  return 0;
}
