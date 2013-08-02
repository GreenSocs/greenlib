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

#define VERBOSE

#include <systemc.h>

#define USE_STATIC_CASTS
#include "greenbus/transport/GP/GP.h"

#ifdef USE_DUST
#include "greenbus/utils/dust_model_and_transactions.h"
#endif

#include "greenbus/transport/genericRouter.h"
#include "greenbus/protocol/SimpleBus/simpleBusProtocol.h"
#include "greenbus/scheduler/fixedPriorityScheduler.h"

#include "sillysort.h"
#include "simplememory.h"                                                      
                                                       
////////////////////////////////////////
// main
////////////////////////////////////////

int sc_main(int, char**)
{

#ifdef USE_DUST
  DUST_INITIALIZE;
#endif
	
  sillysort m1("MasterMark",0x4010, "Call me or I will call you");
  sillysort m2("MasterWolle",0x00ff, "I want a faster bus");
  sillysort m3("MasterRobert",0xffffff00, "My bus even can handle timeouts");
  sillysort m4("MasterMarcus",0x1202, "TLM rocks");
  sillysort m5("MasterOliver",0x4000, "We dumped the core");

  simplememory s1("Slave0",1);
  s1.setAddress(0x4000, 0x4050);
  simplememory s2("Slave1",19);
  s2.setAddress(0xffffff00, 0xffffffff);
  simplememory s3("Slave2",4);
  s3.setAddress(0x1200, 0x120f);
  simplememory s4("Slave3",3);
  s4.setAddress(0x0000, 0x11ff);

  SimpleBusProtocol p("Protocol", sc_time(10, SC_NS));
  fixedPriorityScheduler s("Scheduler");
  GenericRouter r("Router");

  r.protocol_port(p);
  p.router_port(r);
  p.scheduler_port(s);
  

  m1.init_port(r.target_port);
  m4.init_port(r.target_port);
  m2.init_port(r.target_port);
  m5.init_port(r.target_port);
  m3.init_port(r.target_port);

  r.init_port(s1.target_port);
  r.init_port(s3.target_port);
  r.init_port(s2.target_port);
  r.init_port(s4.target_port);

  sc_start(1, SC_MS);
  cout<<"DONE"<<endl<<flush;
  
  return 0;
}
