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

#include "PCIeMaDevice.h"

void PCIeMaDevice::main_action() {
  cout << endl << "------------------------------------------" << endl << endl;
  cout << "PCIe Master action:"<< endl << endl;
  
  MAddr targetAddr = 0x0000001;
  PCIeMasterAccessHandle ah;

  unsigned int data_size;
  std::vector<gs_uint8> *dat;


  // ////////  Memory Write ///////////
  PCIeDEBUG("Perform Memory Write");
  data_size = 10;

  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose
  for (unsigned int i = 0; i < data_size; i++) {
    dat->at(i) = i+11;
  }

  ah = mAPI.create_transaction();
  ah->init_Memory_Write( targetAddr, *dat, data_size );
  mAPI.send_transaction(ah);
  
  PCIe_REGR_TEST("PASS/FAIL: Passed when transaction arrives successfully.");
  
  delete dat; dat = NULL;
  cout << endl<<endl;


  // //////// Memory Read ///////////
  PCIeDEBUG("Perform Memory Read");
  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose

  ah = mAPI.create_transaction();
  ah->init_Memory_Read( targetAddr, *dat, data_size ); // read data_size bytes from address targetAddr to vector dat
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

  PCIe_REGR_TEST("PASS/FAIL: Passed when transaction completed successfully.");
  delete dat; dat = NULL;
  cout << endl;

  // //////// IO Write ///////////
  PCIeDEBUG("Perform IO Write - should fail when received by generic device");
  data_size = 4;

  dat = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose
  for (unsigned int i = 0; i < data_size; i++) {
    dat->at(i) = i;
  }

  ah = mAPI.create_transaction();
  ah->init_IO_Write( targetAddr, *dat, data_size );
  mAPI.send_transaction(ah);
  
  PCIeDEBUG("process completion:");
  if (ah->get_Completion_Status() == SuccessfulCompletion) {
    cout << "  Completion status: successfull"<<endl;
    cout << "  Completion answered by completer's function no. "<< (unsigned int) ah->get_completers_Function_Number()<<endl;
  } else
    cout << "  Completion status: not successfull ("<<(unsigned int)ah->get_Completion_Status() <<dec<<")"<<endl;

  delete dat; dat = NULL;

  PCIe_REGR_TEST("PASS/FAIL: Passed when transaction arrives NOT successfully because of unknown command at the generic receiver.");
  cout << endl;

  cout << endl << "------------------------------------------" << endl << endl;
 
  
}

