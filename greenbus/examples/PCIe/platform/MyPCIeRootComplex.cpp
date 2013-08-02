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

#include "MyPCIeRootComplex.h"

void MyPCIeRootComplex::main_action() {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl << "---------- Root Complex action --------------------------------" << endl << endl;
#endif


#ifdef ENABLE_INTERRUPT_SCENARIO
# warning Use interrupt Scenario
  // initialize bus and device numbers for each device
  cout << "------------------------------------------------------------" << endl;

  PCIeMasterAccessHandle ah;
  ah = down_to_router_port.create_transaction();
  // send initializing Configuration Request to all devices to set their bus and dev numbers
  unsigned int data_size = 1;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);
  for (unsigned int bus = 0; bus < 16; bus++) {
    for (unsigned int dev = 0; dev < 3; dev++) {
      cout << endl<<"send Configuration Read Request for Header Type field to bus "<<bus<<", dev "<<dev<<endl;
      ah->init_Configuration_Read(bus, dev, 0,
                                  0x0E, // Header Type field 
                                  *dat, data_size);
      down_to_router_port.send_transaction(ah);
    }
  }
  PCIe_REGR_TEST("PASS/FAIL Passed if each device receives one Configuration Request.");
  delete dat; dat = NULL;

  cout << "------------------------------------------------------------" << endl;
#endif 

  
#ifdef ENABLE_LOCKED_TEST_SCENARIO
# warning Use Locked Test Scenario

  cout << "****** LOCK / UNLOCK Tests ********" << endl;
  // Wait for some normal communication
  wait(3, SC_US); 
  
  PCIe_REGR_TEST("PASS/FAIL Passed if all transactions performed normally.");
  
  cout << "********************************************************************" << endl;
  // now initiate LOCKED
  do_MemoryRead(0xB400, 10, true); // recv4
  PCIe_REGR_TEST("PASS/FAIL Passed if all transactions using the locked path (rootComplex-router2-router3-recv4 are blocked if they do not use the communication path between rootComplex and recv4.")

  // neg. regression test: Locked Mem Reads have to be used!
# ifdef ENABLE_NEG_LOCKED_TEST_SCENARIO
  PCIe_REGR_TEST("PASS/FAIL Pass if the NEXT transaction results in an error:");
  do_MemoryRead(0xB400, 10); // recv4
# endif
  
  do_MemoryWrite(0xB400, 10); // recv4
  PCIe_REGR_TEST("PASS/FAIL Passed if this transaction performed normally.");

  // not allowed write apart from the locked path
  do_MemoryWrite(0xB200, 10); // recv2
  PCIe_REGR_TEST("PASS/FAIL Passed if this transaction was not received by recv2.");

  // Wait for some further communication (done by other devices) which are blocked on the path to recv4
  wait(7, SC_US); 
  
  cout << "********************************************************************" << endl;
  // UNLOCK
  do_unlock();
  PCIe_REGR_TEST("PASS/FAIL Passed if this transaction unlocks all routers and devices.");
  cout << "********************************************************************" << endl;

  wait(10, SC_US);
  PCIe_REGR_TEST("PASS/FAIL Passed if all transactions performed normally.");

#endif

#ifdef ENABLE_PME_SCENARIO
# warning Use PME Test Scenario

  cout << "****** PME_Turn_Off / PME_TO_Ack Tests ********" << endl;

  // Wait for some normal communication
  wait(5, SC_US);
  PCIe_REGR_TEST("PASS/FAIL Passed if all transactions performed normally.");
  
  // Send PME_Turn_Off Message
  cout << name() << "****** Send PME_Turn_Off" << endl;
  PCIeMasterAccessHandle ah = down_to_router_port.create_transaction();
  ah->init_Message(PME_Turn_Off);
  down_to_router_port.send_transaction(ah);
  PCIe_REGR_TEST("PASS/FAIL Passed if all devices (APIs) received the message.");
  
  // Wait for Acknowledgement
  cout << name() << "****** Wait for PME_TO_Ack" << endl;
  wait(m_received_PME_TO_Ack);
  cout << "Got PME_TO_Ack - Shutdown the system power" << endl;
  PCIe_REGR_TEST("PASS Passed: All Acks received!!");
  sc_stop();

#endif

#ifdef USE_VENDOR_DEFINED_MESSAGE_SCENARIO
# warning Use Vendor defined Message Scenario
  // send Vendor Defined Messages

  unsigned int data_size = 10;
  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);

  // create Vendor defined Message
  ah = down_to_router_port.create_transaction();
  ah->init_Vendor_Defined_Message(BroadcastFromRootComplex, true,
                                     1234, 1234,
                                     *dat, data_size,
                                     3,0,0,0);
  down_to_router_port.send_transaction(ah);

  // create Vendor defined Message
  ah = down_to_router_port.create_transaction();
  ah->init_Vendor_Defined_Message(BroadcastFromRootComplex, true,
                                     1234, 1234);                                     
  down_to_router_port.send_transaction(ah);

  delete dat; dat=NULL;
#endif

}

void PCIeRootComplex::down_to_router_port_b_transact(PCIeTransactionHandle th) {
  PCIeDEBUG("down_to_router_port_b_transact: received transaction from internal router's Upstream Port");
  PCIeSlaveAccessHandle ah = _getSlaveAccessHandle(th);
  
  if (ah->getMPCIeCmd() == Msg && ah->getMMessageCode() == PME_TO_Ack) {
    cout << name() << " received PME_TO_Ack." << endl;
    PCIeRootComplex *this_p = this;
    static_cast<MyPCIeRootComplex*>(this_p)->m_received_PME_TO_Ack.notify();
  }
}

/// Memory Read
void MyPCIeRootComplex::do_MemoryRead(MAddr addr, unsigned int data_size, bool lock) {
  cout << endl;
  cout << name() << ": Perform "; if (lock) cout << "locked "; cout << "Memory Read";
  printf(" to address 0x%llx\n", (gs_uint64)addr);

  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);

  ah = down_to_router_port.create_transaction();
  ah->init_Memory_Read( addr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat
  if (lock) ah->lock_Memory_Read();
  down_to_router_port.send_transaction(ah);

  cout << "process completion:" << endl;
  if (ah->get_Completion_Status() == SuccessfulCompletion) {
    cout << "  Completion status: successfull"<<endl;
    cout << "  Completion answered by completer's function no. "<< (unsigned int) ah->get_completers_Function_Number()<<endl;
  
    cout << "read data:" << endl;
    cout << "                            ";
    for (unsigned int i = 0; i < data_size; i++) {
      cout << (unsigned int)(dat->at(i)) << " ";
    }
    cout << endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;
  delete dat; dat = NULL;
  cout << endl;
}

void MyPCIeRootComplex::do_unlock() {
  cout << endl;
  cout << name() << ": Send Unlock Message" << endl;

  PCIeMasterAccessHandle ah = down_to_router_port.create_transaction();
  ah->unlock();
  down_to_router_port.send_transaction(ah);

  cout << endl;
}


///  Memory Write
void MyPCIeRootComplex::do_MemoryWrite(MAddr addr, unsigned int data_size) {
  cout << endl;
  printf("%s: Perform Memory Write to address 0x%llx\n", name(), (gs_uint64)addr);

  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);
  for (unsigned int i = 0; i < data_size; i++) {
    dat->at(i) = i;
  }

  ah = down_to_router_port.create_transaction();
  ah->init_Memory_Write( addr, *dat, data_size );
  down_to_router_port.send_transaction(ah);
  
  delete dat; dat = NULL;
  cout << endl;
}
