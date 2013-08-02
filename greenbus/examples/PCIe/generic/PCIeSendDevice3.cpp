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

#include "PCIeSendDevice3.h"

void PCIeSendDevice3::main_action() {

  wait(10, SC_US);
  
  cout << endl << "********* PCIeSendDevice3 action ***********" << endl;
 
  do_MemoryRead(0x10010, 10);
  PCIe_REGR_TEST("PASS/FAIL Passed if completed successfully."); 

  cout << endl << "*** Memory Write to myself (to test routing)" << endl;
  do_MemWrite(0x100010, 10);
  PCIe_REGR_TEST("PASS/FAIL Passed if transaction went through the router back to this device (without processing it in detail)."); 
}



///  Memory Write
void PCIeSendDevice3::do_MemWrite(MAddr addr, unsigned int data_size) {
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
void PCIeSendDevice3::do_MemoryRead(MAddr addr, unsigned int data_size, bool lock) {
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
void PCIeSendDevice3::do_IO_Write(MAddr addr, unsigned int data_size) {
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
void PCIeSendDevice3::do_IO_Read(MAddr addr, unsigned int data_size) {
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
