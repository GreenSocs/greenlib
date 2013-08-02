// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
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

/*
 * This testbench tests the performance of return paths with a modified
 * payload event queue.
 *
 * The SC_IMMEDIATE notify method uses the return path, other calls don't.
 * 
 * Make settings in globals.h!!!!
 *
 */

#include <systemc.h>

// user settings!
#include "globals.h"

#include "Master.h"
#include "Slave.h"
#include "payload_event_queue_only_call_delta_boost_return_path.h"
#include "peqConcatenator.h"

int sc_main(int argc, char *argv[]) {  

  cout << "performance_test" <<endl;

  // Master and Slave
  Master master("Master");
  Slave  slave ("Slave"); 
  
  // Channels
  payload_event_queue<unsigned int> my_SlPEQ;
  payload_event_queue<unsigned int> my_MaPEQ;

  // ************** Connections **********************
  
  // ----------------
  // Direction Master->Slave
  
  peqConcatenator<unsigned int> *my_concMaSlPEQs[AMOUNT_PEQ_CONCATENATIONS-1];
  for (int i = 0; i < AMOUNT_PEQ_CONCATENATIONS-1; i++) {
    std::stringstream ss; ss<<"Master_Slave_peqConcatenator"<<i;
    peqConcatenator<unsigned int> *tmp = new peqConcatenator<unsigned int>(ss.str().c_str());
    my_concMaSlPEQs[i] = tmp;
  }
  
  master.peq_out_port(my_SlPEQ);

  for (int i = 0; i < AMOUNT_PEQ_CONCATENATIONS-1; i++) {
    // first connection with my_SlPEQ
    if (i==0) {
      my_SlPEQ.out_port(*my_concMaSlPEQs[0]);
    } 
    // connection between two concatenators
    else if (i < AMOUNT_PEQ_CONCATENATIONS-2) {
      my_concMaSlPEQs[i-1]->peq.out_port(*my_concMaSlPEQs[i]);
    }
    // last connection with slave
    else { // if (i==AMOUNT_PEQ_CONCATENATIONS-2)
      my_concMaSlPEQs[i-1]->peq.out_port(*my_concMaSlPEQs[i]);
      my_concMaSlPEQs[i]->peq.out_port(slave);
    } 
  }

  // if no concatenated peqs
  if (AMOUNT_PEQ_CONCATENATIONS == 1) {
    my_SlPEQ.out_port(slave);
  }
  // if only one concatenated peq
  if (AMOUNT_PEQ_CONCATENATIONS == 2) {
    my_concMaSlPEQs[0]->peq.out_port(slave);
  }

  // ----------------
  // Direction Slave->Master (without return path)
  slave.peq_out_port(my_MaPEQ);

  peqConcatenator<unsigned int> *my_concSlMaPEQs[AMOUNT_PEQ_CONCATENATIONS-1];
  for (int i = 0; i < AMOUNT_PEQ_CONCATENATIONS-1; i++) {
    std::stringstream ss; ss<<"Slave_Master_peqConcatenator"<<i;
    peqConcatenator<unsigned int> *tmp = new peqConcatenator<unsigned int>(ss.str().c_str());
    my_concSlMaPEQs[i] = tmp;
  }
  
  for (int i = 0; i < AMOUNT_PEQ_CONCATENATIONS-1; i++) {
    // first connection with my_MaPEQ
    if (i==0) {
      my_MaPEQ.out_port(*my_concSlMaPEQs[0]);
    } 
    // connection between two concatenators
    else if (i < AMOUNT_PEQ_CONCATENATIONS-2) {
      my_concSlMaPEQs[i-1]->peq.out_port(*my_concSlMaPEQs[i]);
    }
    // last connection with master
    else { // if (i==AMOUNT_PEQ_CONCATENATIONS-2)
      my_concSlMaPEQs[i-1]->peq.out_port(*my_concSlMaPEQs[i]);
      my_concSlMaPEQs[i]->peq.out_port(master);
    }
  }
  
  // if no concatenated peqs
  if (AMOUNT_PEQ_CONCATENATIONS == 1) {
    my_MaPEQ.out_port(master);
  }
  // if only one concatenated peq
  if (AMOUNT_PEQ_CONCATENATIONS == 2) {
    my_concSlMaPEQs[0]->peq.out_port(master);
  }
  
  // ----------------
  
  sc_start();

  return 0;

}
