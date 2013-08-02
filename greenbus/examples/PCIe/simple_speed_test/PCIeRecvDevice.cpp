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

#include "PCIeRecvDevice.h"

void PCIeRecvDevice::main_action() {

}

void PCIeRecvDevice::b_transact(PCIeTransactionHandle th) {

#ifndef ENABLE_SPEED_CHECK_SCENARIO
  cout << name() << ": b_transact: received transaction" << endl;
#endif

  PCIeSlaveAccessHandle ah = _getSlaveAccessHandle(th);

  /*unsigned int cmd = ah->get_TLP_type();
  switch(cmd) {
    case MemWrite:
      {
        PCIeDEBUG("received transaction of type 'MemWrite':");
        // if pointer data
        if (ah->getMData().getPointer()) {
          std::vector<gs_uint8> *dat;
          dat = static_cast<std::vector<gs_uint8>* >(ah->getMData().getPointer());
          cout << "                       Pointer data"<<endl;
          cout << "                            ";
          for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
            cout << (unsigned int)(dat->at(i)) << " ";
          }
          cout << endl;
        }
        // if not pointer data
        else {
          std::vector<gs_uint8> *dat;
          cout << "                       Not pointer data"<<endl;
          dat = &(ah->getMData().getData());
          cout << "                            ";
          for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
            cout << (unsigned int)(dat->at(i)) << " ";
          }
          cout << endl;
        }
      }
      break;
    
    case MemReadLocked:
      {
        PCIeDEBUG("received transaction of type 'MemReadLocked':");
        // device is locked automatically in mAPI.m_locked
      } // without break!
    case MemRead:
      {
        PCIeDEBUG("received transaction of type 'MemRead':");
        ah->init_Memory_Read_Completion(SuccessfulCompletion, 3);
        // if pointer data
        if (ah->getMData().getPointer()) {
          cout << "                       Pointer data"<<endl;
          std::vector<gs_uint8> *dat;
          dat = static_cast<std::vector<gs_uint8>*>(ah->getMData().getPointer());
          for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
            dat->at(i) = i+100;
          }
        }
        // if not pointer data
        else {
          cout << "                       Not pointer data"<<endl;
          std::vector<gs_uint8> *dat;
          dat = &(ah->getMData().getData());
          for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
            dat->at(i) = i+100;
          }
        }
      }
      break;
    
    case IOWrite:
      {
        PCIeDEBUG("received transaction of type 'IOWrite':");
        ah->init_IO_Write_Completion(SuccessfulCompletion, 3);
        std::vector<gs_uint8> *dat;
        dat = &(ah->getMData().getData());
        cout << "                            ";
        for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
          cout << (unsigned int)(dat->at(i)) << " ";
        }
        cout << endl;
      }
      break;

    case IORead:
      {
        PCIeDEBUG("received transaction of type 'IORead':");
        ah->init_IO_Read_Completion(SuccessfulCompletion, 3);
        std::vector<gs_uint8> *dat;
        dat = &(ah->getMData().getData());
        for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
          dat->at(i) = i+100;
        }
      }
      break;
    
      
    default:
      {
        PCIeDEBUG("Unsupported TLP type (mPCIeCmd)");
        // If Request requires a completion, send Completion Status = UR
        // TODO: check if list is complete; also see PCIeAPI.h
        if (  cmd == MemRead or cmd == IORead or cmd == CnfgReadTy0
                        or cmd == CnfgReadTy1 or cmd == MemReadLocked
              or cmd == CnfgWriteTy0 or cmd == CnfgWriteTy1 or cmd == IOWrite) {
          ah->init_Unsupported_Request();
        }
      }
  }*/
  
}

