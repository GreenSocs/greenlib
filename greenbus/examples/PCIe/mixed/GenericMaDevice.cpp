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

#include "GenericMaDevice.h"

void GenericMaDevice::main_action() {
  unsigned int data_size = 10;
  
  wait(1, SC_SEC);
  
  cout << "Generic Master action:" <<endl<<endl;
  
  // set things up at the slave
  //strcpy((char *)(mem), "As molt...");
  //data.setPointer((void*)&mem[0]);
  std::vector<gs_uint8> *vec = new std::vector<gs_uint8>(data_size+100); // +100 for test purpose
  for (unsigned int i = 0; i < data_size; i++) {
    vec->at(i) = i+12;
  }
  data.setPointer(vec);

  accessHandle t1 = init_port.create_transaction();
  t1->setMCmd(Generic_MCMD_WR);
  t1->setMAddr(0x10000);
  t1->setMData(data);
  //t1->setMBurstLength(strlen((char *)mem)+1);
  t1->setMBurstLength(data_size);

  cout << "run: start writing "<< data_size<<" bytes to slave" << endl;
  init_port.Transact(t1);
  if (t1->getSError() != GenericError::Generic_Error_NoError)
    SC_REPORT_WARNING(name(), "received error!");

  cout << "run: finished writing to slave" << endl;
  PCIe_REGR_TEST("PASS/FAIL: Passed when transaction arrives successfully at PCIe receiver.");
  cout << endl;


  // get the data
  std::vector<gs_uint8> *recvvec = new std::vector<gs_uint8>(data_size);

  t1 = init_port.create_transaction();
  t1->setMCmd(Generic_MCMD_RD);
  t1->setMAddr(0x10000);
  t1->setMBurstLength(data_size);
  t1->setMData(recvvec);
  cout << "run: PV read from slave" << endl;
  init_port.Transact(t1);
  if (t1->getSError() != GenericError::Generic_Error_NoError)
    SC_REPORT_WARNING(name(), "received error!");
  else {
    cout << "run: PV got "<<(dec)<<(unsigned int)t1->getMSBytesValid()<<" bytes from slave" << endl;
    sc_assert(t1->getMSBytesValid()==data_size);
    cout << "                            ";
    for (unsigned int i = 0; i < data_size; i++) {
      cout << (unsigned int)(recvvec->at(i)) << " ";
    }
  }
  cout << endl;
  PCIe_REGR_TEST("PASS/FAIL: Passed when transaction returned successfully with data.");
  cout << endl;


  cout << "run: PV send transaction which is not understood by PCIe: Generic_MCMD_RDEX" << endl;
  t1 = init_port.create_transaction();
  t1->setMCmd(Generic_MCMD_RDEX);
  t1->setMAddr(0x10000);
  t1->setMBurstLength(0);
  init_port.Transact(t1);
  PCIe_REGR_TEST("PASS/FAIL: Passed when PCIe receiver complains about unsupported TLP type.");
  cout << endl;


}

void GenericMaDevice::notify(GS_ATOM& tc)
{
  SC_REPORT_ERROR(name(), "Non-blocking notify(...) not supported.");
}

