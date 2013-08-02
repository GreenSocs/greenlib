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

#include "PCIeSendDevice.h"

void PCIeSendDevice::main_action() {

#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << "********** Start PCIeSendDevice ***********" << endl;
#endif

#ifdef ENABLE_LOCKED_TEST_SCENARIO_OR_ENABLE_PME_SCENARIO
  MAddr addr_arr[] = {
    //0xA100LL, // sender
    //0xA200LL,
    //0xA300LL,
    0xB200LL,
    0xB300LL,
    0xB400LL,
    //0xC100LL, // router
    //0xC200LL,
    //0xC300LL,
    //0xC400LL,
    0xB100LL,
    0xD100LL
  };

  for (int z = 0; z<15; z++) {
    //cout << (z % (sizeof(addr_arr)/sizeof(MAddr))) << endl;
    do_MemWrite(addr_arr[(z % (sizeof(addr_arr)/sizeof(MAddr)))], 10);
    //do_MemRead(addr_arr[(z % (sizeof(addr_arr)/sizeof(MAddr)))], 10);
    wait(1, SC_US);
  }
#endif


#ifdef ENABLE_SPEED_CHECK_SCENARIO
  // send 1 000 000 transactions (333333 because of 3 senders)
  MAddr addr_arr[] = {
    0xB200LL,
    0xB300LL,
    0xB400LL,
    0xB100LL,
    0xD100LL
  };

  for (int z = 0; z < SPEED_CHECK_NUM_LOOPS; z++) {
    do_MemWrite(addr_arr[(z % (sizeof(addr_arr)/sizeof(MAddr)))], 10);
    //do_MemRead(addr_arr[(z % (sizeof(addr_arr)/sizeof(MAddr)))], 10);
  }
#endif
              
 
#ifdef ENABLE_INTERRUPT_SCENARIO
  if (std::string(name()).compare("send3") == 0) {
    PCIeMasterAccessHandle ah;

    cout << name() << ": *** Send Assert_INTA Interrupt Message" << endl;
    ah = mAPI.create_transaction();
    ah->init_Message( Assert_INTA );
    mAPI.send_transaction(ah);
    wait(1, SC_NS);
  
    cout << endl << endl << name() << ": *** Send Deassert_INTA Interrupt Message" << endl;
    ah = mAPI.create_transaction();
    ah->init_Message( Deassert_INTA );
    mAPI.send_transaction(ah);
    wait(1, SC_NS);

    cout << endl << endl << name() << ": *** Send Assert_INTD Interrupt Message" << endl;
    ah = mAPI.create_transaction();
    ah->init_Message( Assert_INTD );
    mAPI.send_transaction(ah);
    wait(1, SC_NS);

    cout << endl << endl << name() << ": *** Send Deassert_INTD Interrupt Message" << endl;
    ah = mAPI.create_transaction();
    ah->init_Message( Deassert_INTD );
    mAPI.send_transaction(ah);
  }
#endif
}

///  Memory Write
void PCIeSendDevice::do_MemWrite(MAddr addr, unsigned int data_size) {
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
void PCIeSendDevice::do_MemoryRead(MAddr addr, unsigned int data_size, bool lock) {
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
void PCIeSendDevice::do_IO_Write(MAddr addr, unsigned int data_size) {
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
void PCIeSendDevice::do_IO_Read(MAddr addr, unsigned int data_size) {
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
