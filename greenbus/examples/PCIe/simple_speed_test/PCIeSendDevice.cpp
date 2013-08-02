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

  MAddr addr = 0x10;
  PCIeMasterAccessHandle ah = mAPI.create_transaction();

  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(DATA_SIZE);

  for (unsigned int i = 0; i < TRANSACTION_COUNT; i++) {
#ifndef ENABLE_SPEED_CHECK_SCENARIO
    cout << name() << ": send "<< DATA_SIZE<<" bytes transaction #" << i<<endl;
#endif
    //    ah = mAPI.create_transaction(); 
    ah->reset(); // faster than create_transaction
    ah->init_Memory_Write( addr, *dat, DATA_SIZE );
    mAPI.send_transaction(ah);
  }
  
}
