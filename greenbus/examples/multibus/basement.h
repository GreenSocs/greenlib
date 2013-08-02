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

#ifndef __basement_h__
#define __basement_h__

#include <systemc>

// includes to build a PLB GreenBus
#include "greenbus/protocol/PLB/PLB.h"
#include "greenbus/protocol/PLB/PLBBusProtocol.h"
#include "greenbus/scheduler/dynamicPriorityScheduler.h"

// master and slave
#include "simplememory.h"
#include "sillysort.h"

/**
 * The basement module
 */
SC_MODULE(basement) {
  SC_CTOR(basement) {
    
    // masters for the second floor GreenBus
    m2 = new sillysort("Master_0_basement",0x00ff, "I want a faster bus");
    m5 = new sillysort("Master_1_basement",0x0400, "We dumped the core");

    // slaves for the second floor GreenBus
    sl4 = new simplememory("Slave_0_basement",3);
    sl4->setAddress(0x0000, 0x11ff);

    // configure basement bus
    p2 = new PLBBusProtocol("PLB",10,SC_NS);
    s2 = new dynamicPriorityScheduler("PLB-Scheduler",10,SC_NS); 
    r2 = new GenericRouter<GenericTransaction, GenericPhase, GenericRouterAccess>("PLB-Router");  
    r2->protocol_port(*p2);
    p2->router_port(*r2);
    p2->scheduler_port(*s2);

    // connect masters to basement bus
    m2->init_port(r2->target_port);
    m5->init_port(r2->target_port);

    // connect slaves to second bus
    r2->init_port(sl4->target_port);
  }

  ~basement() {
    delete m2;
    delete m5;
    delete sl4;
    delete p2;
    delete s2;
    delete r2;
    SC_REPORT_INFO(name(), "Thank you for visiting the basement!");
  }

  sillysort *m2, *m5;
  simplememory *sl4;
  PLBBusProtocol *p2;
  dynamicPriorityScheduler *s2;
  GenericRouter<GenericTransaction, GenericPhase, GenericRouterAccess> *r2;
  

};

#endif
