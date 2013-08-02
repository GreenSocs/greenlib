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

#include "MySimplePCIeRootComplex.h"

void MySimplePCIeRootComplex::main_action() {
  cout << endl << "---------- Root Complex action --------------------------------" << endl << endl;
  
  MAddr targetAddr = 0x00000010;
  PCIeMasterAccessHandle ah;

  unsigned int data_size;
  std::vector<gs_uint8> *dat;

  cout << endl;
  
  // //////// Configuration Read ///////////
  PCIeDEBUG("Perform Configuration Read");
  
  data_size = 1;
  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose

  ah = mAPI.create_transaction();
  unsigned int busNo = 1, devNo = 1, funcNo = 5, registerAddr = 201;
  ah->init_Configuration_Read( busNo, devNo, funcNo, registerAddr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat

  mAPI.send_transaction(ah);

  PCIeDEBUG("process completion:");
  if (ah->get_Completion_Status() == SuccessfulCompletion) {
    cout << "  Completion status: successfull"<<endl;
    cout << "  Completion anwered by completer's function no. "<< (unsigned int) ah->get_completers_Function_Number()<<endl;
    sc_assert(funcNo == (unsigned int) ah->get_completers_Function_Number());
    
    PCIeDEBUG("read data:");
    cout << "                            ";
    for (unsigned int i = 0; i < data_size; i++) {
      cout << (unsigned int)(dat->at(i)) << " ";
    }
    cout << endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;

  delete dat; dat = NULL;
  cout << endl;

  // //////// Configuration Write ///////////
  PCIeDEBUG("Perform Configuration Write");
  data_size = 1;

  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose
  for (unsigned int i = 0; i < data_size; i++) {
    dat->at(i) = i+10;
  }

  ah = mAPI.create_transaction();
  ah->init_Configuration_Write( busNo, devNo, funcNo, registerAddr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat
  mAPI.send_transaction(ah);
  
  PCIeDEBUG("process completion:");
  if (ah->get_Completion_Status() == SuccessfulCompletion) {
    cout << "  Completion status: successfull"<<endl;
    cout << "  Completion answered by completer's function no. "<< (unsigned int) ah->get_completers_Function_Number()<<endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;

  delete dat; dat = NULL;
  cout << endl;

  // //////// Configuration Read to not existing Device ///////////
  PCIeDEBUG("Perform Configuration Read to not existing Device");
  
  data_size = 1;
  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose

  ah = mAPI.create_transaction();
  busNo = 1; devNo = 100;
  ah->init_Configuration_Read( busNo, devNo, funcNo, registerAddr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat

  mAPI.send_transaction(ah);

  PCIeDEBUG("process completion:");
  if (ah->get_Completion_Status() == UnsupportedRequest) {
    cout << "  Completion status: Unsupported Request"<<endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;

  delete dat; dat = NULL;
  cout << endl;

  // //////// Memory Read LOCKED ///////////
  PCIeDEBUG("Perform Memory Read LOCKED");
  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose

  ah = mAPI.create_transaction();
  ah->init_Memory_Read( targetAddr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat
  ah->lock_Memory_Read(); // LOCKED!
  mAPI.send_transaction(ah);

  PCIeDEBUG("process completion:");
  if (ah->get_Completion_Status() == SuccessfulCompletion) {
    cout << "  Completion status: successfull"<<endl;
    cout << "  Completion answered by completer's function no. "<< (unsigned int) ah->get_completers_Function_Number()<<endl;
  
    PCIeDEBUG("read data:");
    cout << "                            ";
    for (unsigned int i = 0; i < data_size; i++) {
      cout << (unsigned int)(dat->at(i)) << " ";
    }
    cout << endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;
  delete dat; dat = NULL;
  cout << endl;
  
  // /////// Unlock ////////////
  PCIeDEBUG("Perform Unlock");

  ah = mAPI.create_transaction();
  ah->unlock();
  mAPI.send_transaction(ah);

  cout << endl;

  cout << endl << "-----------END Root Complex action ----------------------" << endl << endl;
 
  
}

