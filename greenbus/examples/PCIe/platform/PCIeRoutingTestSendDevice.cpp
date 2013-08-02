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

#include "PCIeRoutingTestSendDevice.h"

void PCIeRoutingTestSendDevice::main_action() {
  cout << "***** Routing Test Scenario *****" << endl;

  // ** Memory Address Space tests
  cout << endl << "Memory Address Space tests" << endl << endl;
  
  // Test sending to base address
  cout << endl << "***** Test sending to base address:" << endl;
  do_MemWrite(0xB100, 10); // recv1
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");

  // Test sending to high address
  cout << endl << "***** Test sending to high address (one byte):" << endl;
  do_MemWrite(0xB1FF, 1); // exact one byte
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");

  cout << endl << "***** Test sending to high address (more bytes than allowed!):" << endl;
  do_MemWrite(0xB1FF, 10); // more bytes than allowed!
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");

  // Test sending to address in between base and high
  cout << endl << "***** Test sending to address in between base and high:" << endl;
  do_MemWrite(0xB110, 10); // recv1
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");
  
  // Test routing to the correct devices
  cout << endl << "***** Test sending to recv2:" << endl;
  do_MemWrite(0xB201, 10); // device recv2
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv2'.");

  cout << endl << "***** Test sending to recv3:" << endl;
  do_MemWrite(0xB3A1, 10); // device recv3
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv3'.");

  cout << endl << "***** Test sending to recv4:" << endl;
  do_MemWrite(0xB4B1, 10); // device recv4
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv4'.");

  cout << endl << "***** Test sending to send3:" << endl;
  do_MemWrite(0xA3B0, 10); // device send3
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'send3'.");


  // ** IO Address Space tests
  cout << endl<<endl << "IO Address Space tests" << endl << endl;
  
  // Test sending to base address
  cout << endl << "***** Test sending to I/O base address:" << endl;
  do_IO_Write(0xB050, 4); // recv1
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");

  // Test sending to high address
  cout << endl << "***** Test sending to I/O high address (one byte):" << endl;
  do_IO_Write(0xB250, 4); // exact one byte
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");

  // Test sending to address in between base and high
  cout << endl << "***** Test sending to I/O address in between base and high:" << endl;
  do_IO_Write(0xB100, 4); // recv1
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv1'.");
  
  // Test routing to the correct devices
  cout << endl << "***** Test sending to recv3:" << endl;
  do_IO_Write(0xB310, 4); // device recv3
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv3'.");
  
  // Test routing to the correct devices
  cout << endl << "***** Test sending to recv4:" << endl;
  do_IO_Write(0xE111, 4); // device recv4
  PCIe_REGR_TEST("PASS/FAIL Passed if TLP arrived at device 'recv4'.");
  
  //wait(1, SC_US);
}

///  Memory Write
void PCIeRoutingTestSendDevice::do_MemWrite(MAddr addr, unsigned int data_size) {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl;
  printf("%s: Perform Memory Write to address 0x%llx\n", name(), (gs_uint64)addr);
#endif

  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  for (unsigned int i = 0; i < data_size; i++) {
    dat->at(i) = i;
  }
#endif

  ah = mAPI.create_transaction();
  ah->init_Memory_Write( addr, *dat, data_size );
  mAPI.send_transaction(ah);
  
  delete dat; dat = NULL;
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl;
#endif
}

/// Memory Read
void PCIeRoutingTestSendDevice::do_MemoryRead(MAddr addr, unsigned int data_size, bool lock = false) {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl;
  cout << name() << ": Perform "; if (lock) cout << "locked "; cout << "Memory Read";
  printf(" to address 0x%llx\n", (gs_uint64)addr);
#endif

  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);

  ah = mAPI.create_transaction();
  ah->init_Memory_Read( addr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat
  if (lock) ah->lock_Memory_Read();
  mAPI.send_transaction(ah);

#ifndef ENABLE_SPEED_CHECK_SCENARIO
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
  cout << endl;
#endif
  delete dat; dat = NULL;
}

/// IO Write
void PCIeRoutingTestSendDevice::do_IO_Write(MAddr addr, unsigned int data_size) {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl;
  printf("%s: Perform IO Write to address 0x%llx\n", name(), (gs_uint64)addr);
#endif

  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  for (unsigned int i = 0; i < data_size; i++) {
    dat->at(i) = i;
  }
#endif

  ah = mAPI.create_transaction();
  ah->init_IO_Write( addr, *dat, data_size );
  mAPI.send_transaction(ah);
  
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << "process completion:" << endl;
  if (ah->get_Completion_Status() == SuccessfulCompletion) {
    cout << "  Completion status: successfull"<<endl;
    cout << "  Completion answered by completer's function no. "<< (unsigned int) ah->get_completers_Function_Number()<<endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;
  cout << endl;
#endif

  delete dat; dat = NULL;
}

/// IO Read
void PCIeRoutingTestSendDevice::do_IO_Read(MAddr addr, unsigned int data_size) {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << endl;
  printf("%s: Perform IO Read to address 0x%llx\n", name(), (gs_uint64)addr);
#endif

  PCIeMasterAccessHandle ah;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size);

  ah = mAPI.create_transaction();
  ah->init_IO_Read( addr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat
  mAPI.send_transaction(ah);

#ifndef ENABLE_SPEED_CHECK_SCENARIO
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
  cout << endl;
#endif

  delete dat; dat = NULL;
}
