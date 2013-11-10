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

//#define GS_VERBOSE
//#define SPEEDTEST

#define MEMSIZE 10000

#ifndef USE_GPSOCKET
# define USE_GPSOCKET
#endif

// GreenControl + GreenConfig
#include "greencontrol/config.h"

#include <systemc>

#include "gsgpsocket/examples/simple/sillysort.h"
#include "gsgpsocket/examples/simple/simplememory.h"
                                        
//#include "gsgpsocket/transport/genericRouter.h"
//#include "gsgpsocket/protocol/SimpleBus/simpleBusProtocol.h"
//#include "gsgpsocket/scheduler/fixedPriorityScheduler.h"
//#include "gsgpsocket/scheduler/dynamicPriorityScheduler.h"
               
#include "greenrouter/genericRouter.h"
#include "greenrouter/protocol/SimpleBus/simpleBusProtocol.h"
#include "greenrouter/scheduler/fixedPriorityScheduler.h"
#include "greenrouter/scheduler/dynamicPriorityScheduler.h"

////////////////////////////////////////
// main
////////////////////////////////////////

int sc_main(int, char**)
{
  /// GreenControl Core instance
  gs::ctr::GC_Core       core();
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);
	
  sillysort m1("MasterMark",0x4010, "Call me or I will call you");
  sillysort m2("MasterWolle",0x00ff, "I want a faster bus");
  sillysort m3("MasterRobert",0xffffff00, "My bus even can handle timeouts");
  sillysort m4("MasterMarcus",0x1202, "TLM rocks");
  sillysort m5("MasterOliver",0x4200, "We dumped the core");
  
  simplememory s1("Slave0",1);
  //s1.setAddress(0x4000, 0x4050);
  s1.target_port.base_addr=0x4000;
  s1.target_port.high_addr=0x47ff;

  simplememory s2("Slave1",19);
  //s2.setAddress(0xffffff00, 0xffffffff);
  s2.target_port.base_addr=0xffffff00;
  s2.target_port.high_addr=0xffffffff;

  simplememory s3("Slave2",4);
  //s3.setAddress(0x1200, 0x120f);
  s3.target_port.base_addr=0x1200;
  s3.target_port.high_addr=0x120f;

  simplememory s4("Slave3",3);
  //s4.setAddress(0x0000, 0x11ff);
  s4.target_port.base_addr=0x0000;
  s4.target_port.high_addr=0x11ff;

  gs::gp::SimpleBusProtocol<32> p("Protocol", 10); // the clkPeriod could be set using the gs_param m_clkPeriod
  gs::gp::fixedPriorityScheduler s("Scheduler");
  gs::gp::GenericRouter<32> r("Router");

  r.protocol_port(p);
  p.router_port(r);
  p.scheduler_port(s);
  

  m1.init_port(r.target_socket);

  m4.init_port(r.target_socket);
  m2.init_port(r.target_socket);
  m5.init_port(r.target_socket);
  m3.init_port(r.target_socket);

  r.init_socket(s1.target_port);
  r.init_socket(s3.target_port);
  r.init_socket(s2.target_port);
  r.init_socket(s4.target_port);

  sc_core::sc_start();
  std::cout<<"DONE"<<std::endl;
  
  return 0;
}
