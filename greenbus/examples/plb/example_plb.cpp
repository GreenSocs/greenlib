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


      
//#define USE_GENERIC_API
//#define USE_PV
#define USE_CC

#define LOOPS 20 /* number of loops */

#define VERBOSE


//#define PHASE_TRACE /* create VCD trace file with PLB signals */


#include <systemc.h>

//#define USE_STATIC_CASTS --> do not define in the source code. Do a `make STATIC=yes` instead.

#include "greenbus/transport/PLB/PLB.h"

#ifdef USE_DUST
// uncomment the following line if you want to enable DUST structure analysis
#include "greenbus/utils/dust_model.h"
// uncomment the following line if you want to enable DUST strucutre analysis and transaction recording
//#include "greenbus/utils/dust_model_and_transactions.h"
// uncomment the following line if you want to enable DUST strucutre analysis and full transaction introspection
//#include "greenbus/utils/dust_model_and_introspection.h"
#endif

//#include "greenbus/api/shipAPI.h"
#include "greenbus/transport/genericRouter.h"
#include "greenbus/protocol/PLB/PLBBusProtocol.h"
#include "greenbus/scheduler/dynamicPriorityScheduler.h"

#define CLK_CYCLE 10

#include "PLBMaster.h"
#include "PLBSimplememory.h"



                              
////////////////////////////////////////
// main
////////////////////////////////////////

int sc_main(int argc, char* argv[])
{
#ifdef USE_DUST
  DUST_INITIALIZE;
#endif
	
  //PLBMaster(sc_module_name name_, unsigned long long targetAddress, const char* data, bool rNw)
//SC_REPORT_ERROR("msg_type", "my error");
sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);

  PLBSimplememory s1("Slave1",3);
  s1.setAddress(0x4000,0x400F);
  PLBSimplememory s2("Slave2",3); // zweiter Parameter: ACK-Delay in Taktzyklen
  s2.setAddress(0x5000,0x500F);
/*
  PLBSimplememory s3("Slave3",0);
  s3.setAddress(0x1000,0x100F);
  PLBSimplememory s4("Slave4",23);
  s4.setAddress(0x3000,0x300F);
  PLBSimplememory s5("Slave5",2);
  s5.setAddress(0x2000,0x200F);
*/

  
  PLBMaster m1("M1", false); // do writes
  m1.init_port.target_addr = 0x4000;
  if (argc==4) {
    m1.burst_length = atoi(argv[2]);
    m1.loops = atoi(argv[3]);
  }
  else {
    m1.burst_length = 80;
    m1.loops = 0; 
  }
  m1.initial_delay = 92; // equal to Verilog testbench-1 (because enqueuing takes 1 cycle)

  
/*
  PLBMaster m3("Klausi3",0x2000,"Klausi ru",true);
  PLBMaster m4("Klausi4",0x3000,"Klausi rulez",false);
  PLBMaster m5("Klausi5",0x5000,"Klausz",true);
*/
  //PLBSimplememory(sc_module_name name_, int delay )


  // attach PLB protocol and scheduler to the generic router /////////////////
  PLBBusProtocol p("PLB",10,SC_NS);
  dynamicPriorityScheduler s("Scheduler",10,SC_NS);  //this one is only usefull in connection with PLB
  GenericRouter r("Router");

  r.protocol_port(p) ;
  p.router_port(r);
  p.scheduler_port(s);


  // connect initiators and targets to the generic router ///////////////////


  m1.init_port(r.target_port);
//  m3.init_port(r.target_port);
  PLBMaster* m2;
  if (argc<3) {
    m2=new PLBMaster("M2", false); // do writes
    m2->init_port.target_addr = 0x5000;
    m2->burst_length = 160;
    m2->initial_delay = 100; // equal to Verilog testbench-1 (because enqueuing takes 1 cycle)
    m2->init_port(r.target_port);
  }
//  m5.init_port(r.target_port);
//  m4.init_port(r.target_port);

  r.init_port(s1.target_port);
  r.init_port(s2.target_port);
//  r.init_port(s3.target_port);
//  r.init_port(s4.target_port);
//  r.init_port(s5.target_port);
  
  if (argc==2)
    sc_start(atoi(argv[1]), SC_US);
  else if (argc==4)
    sc_start();
  else {
    cout<<"Usage: "<<argv[0]<<" simulation_time_in_us  (master1_burstlength  num_transactions)"<<endl;
    cout<<"        Simulation time will be ignored when 2nd and 3rd parameter are given"<<endl;
  }

  sc_stop();  

  return 0;
}
