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

#include "PCIeRecvInfoDevice.h"

void PCIeRecvInfoDevice::main_action() {

}

void PCIeRecvInfoDevice::b_transact(PCIeTransactionHandle th) {
  //PCIeDEBUG("b_transact: received transaction");
  PCIeSlaveAccessHandle ah = _getSlaveAccessHandle(th);

  unsigned int cmd = ah->get_TLP_type();
  switch(cmd) {
    case MemWrite:
      {
        PCIeDEBUG("received TLP of type 'MemWrite':");
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
        PCIeDEBUG("received TLP of type 'MemReadLocked':");
        // device is locked automatically in mAPI.m_locked
      } // without break!
    case MemRead:
      {
        PCIeDEBUG("received TLP of type 'MemRead':");
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
        PCIeDEBUG("received TLP of type 'IOWrite':");
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
        PCIeDEBUG("received TLP of type 'IORead':");
        ah->init_IO_Read_Completion(SuccessfulCompletion, 3);
        std::vector<gs_uint8> *dat;
        dat = &(ah->getMData().getData());
        for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
          dat->at(i) = i+100;
        }
      }
      break;
    
    case CnfgReadTy0:
      {
        PCIeDEBUG("received TLP of type 'CnfgReadTy0':");
        ah->init_Cnfg_Read_Completion(SuccessfulCompletion);
        std::vector<gs_uint8> *dat;
        dat = &(ah->getMData().getData());
        for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
          dat->at(i) = i+100;
        }
      }
      break;
    
    
    case CnfgWriteTy0:
      {
        PCIeDEBUG("received TLP of type 'CnfgWriteTy0':");
        ah->init_Cnfg_Write_Completion(SuccessfulCompletion);
        std::vector<gs_uint8> *dat;
        dat = &(ah->getMData().getData());
        cout << "                            ";
        for (unsigned int i = 0; i < ah->getMBurstLength(); i++) {
          cout << (unsigned int)(dat->at(i)) << " ";
        }
        cout << endl;
      }
      break;
    
    case CnfgReadTy1:
      {
        PCIeDEBUG("received TLP of type 'CnfgReadTy1':");
        // only allowed in switches!
        ah->init_Unsupported_Request();
        SC_REPORT_WARNING(name(), "Endpoint not allowed to get Type 1 Configuration Request: Unsupported Request.");
      }
      break;

    case CnfgWriteTy1:
      {
        PCIeDEBUG("received TLP of type 'CnfgWriteTy1':");
        // only allowed in switches!
        ah->init_Unsupported_Request();
        SC_REPORT_WARNING(name(), "Endpoint not allowed to get Type 1 Configuration Request: Unsupported Request.");
      }
      break;

    case Msg:
      {
        PCIeDEBUG("received TLP of type 'Msg':");
        unsigned int code = ah->getMMessageCode();
        switch(code) {
          case Unlock:
            {
              PCIeDEBUG("Unlock");
              // device is unlocked automatically in mAPI.m_locked
            }
            break;
            
          default:
            {
              SC_REPORT_WARNING(name(), "received not supported Message Code");
            }
        }
      }
      break;
      
    case NO_CMD:
      {
        PCIeDEBUG("Undefined TLP type (mPCIeCmd)");
        PCIeDEBUG("Report Malformed Packet");
        SC_REPORT_WARNING(name(), "Received Malformed Packet.");
        // TODO: create error TLP
      }
      break;
      
    default:
      {
        PCIeDEBUG("Unsupported TLP type (mPCIeCmd)");
        // If Request requires a completion, send Completion Status = UR
        // TODO: check if list is complete; also see PCIeAPI.h
        if (/*reads:*/  cmd == MemRead or cmd == IORead or cmd == CnfgReadTy0
                        or cmd == CnfgReadTy1 or cmd == MemReadLocked
            /*writes:*/ or cmd == CnfgWriteTy0 or cmd == CnfgWriteTy1 or cmd == IOWrite) {
          ah->init_Unsupported_Request();
        }
      }
  }
  
}

