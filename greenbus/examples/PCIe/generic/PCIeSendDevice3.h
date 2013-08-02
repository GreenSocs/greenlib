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

#ifndef __PCIeSendDevice3_h__
#define __PCIeSendDevice3_h__

#include <systemc.h>

#include "globals.h"
#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/api/PCIe/PCIeAPI.h"

#define PCIe_REGR_TEST(string) { cout <<"------------------------------------------------------------------------------------------"<<endl<<"    *** Regression test result: ***"<< endl<<"   @" << sc_time_stamp() << " /" << sc_delta_count() << " ("<<this->name() << "): " << string <<endl<<"------------------------------------------------------------------------------------------"<< endl << flush; }

using namespace tlm;
using namespace tlm::PCIe;


/// PCIe send class to test communication with generic devices over a generic router
/**
 * Send a PCIe Memory Read to a generic Memory Device (DDR RAM). 
 * That should work including the Completion.
 */
class PCIeSendDevice3
 : public sc_module,
   public PCIe_recv_if
{
public:
  PCIeAPI mAPI;

public:

  SC_HAS_PROCESS(PCIeSendDevice3);

  // Constructor
  PCIeSendDevice3(sc_module_name name_) : 
    sc_module(name_),
    mAPI("Ma1_PCIeAPI", this)
  {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
    cout<<"Constructor PCIeSendDevice3 " << name() <<endl;
#endif  
    SC_THREAD(main_action);
  }
    
  void main_action();
  
  void do_MemWrite(MAddr addr, unsigned int data_size);
  void do_MemoryRead(MAddr addr, unsigned int data_size, bool lock = false);
  void do_IO_Write(MAddr addr, unsigned int data_size);
  void do_IO_Read(MAddr addr, unsigned int data_size);

  /**
   * The PCIe_slave_if implementation
   */
  virtual void b_transact(PCIeTransactionHandle th) {
    PCIeDEBUG("b_transact: received transaction at PCIe Device");
    //PCIeSlaveAccessHandle ah = boost::dynamic_pointer_cast<PCIeSlaveAccess>(th);
    //cout << name() << ": Sender cannot receive Requests: send Unsupported Request."<< endl;
    //ah->init_Unsupported_Request();
  }

};

#endif
