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
  cout << endl << "---------- Root Complex action --------------------------------" << endl << endl;


  // initialize bus and device numbers for each device
  /*cout << "------------------------------------------------------------" << endl;
  cout << "initialize bus and device numbers for each device" << endl;
  
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
*/

  cout << endl << endl << endl;
  cout << "------------------------------------------------------------" << endl;
  cout << "---------- read out base address space requirements" << endl;

  // TODO: Disable all command registers
  
  // TODO: Read and store all current base addresses
 
  PCIeMasterAccessHandle ah;
  cout << "------ write 0xFFFFFFFF to base address registers --------------" << endl;
  // send Configuration Request writing 1s to base address registers
  unsigned int data_size = 4;
  std::vector<gs_uint8> *dat;
  dat = new std::vector<gs_uint8>(data_size, 0xFF);
  for (unsigned int bus = 0; bus < 7; bus++) {
    for (unsigned int dev = 0; dev < 1; dev++) {
      for (unsigned int dword = 0; dword < 6; dword++) {
        if (dword > 0 && ah->get_Completion_Status() != SuccessfulCompletion) break; // stop for this device if first transaction was not completed
        ah = down_to_router_port.create_transaction();
        cout << endl<<"**** Send Configuration Write Request for base address field to **** bus "<<bus<<", dev "<<dev<< " conf. reg. address 0x"<<hex<<(0x10 +dword*4)<<dec<<"("<<(0x10 +dword*4)<<"d) ****" << endl;
        ah->init_Configuration_Write(bus, dev, 0,
                                    0x10 + dword*4, // Base Address
                                    *dat, data_size);
        down_to_router_port.send_transaction(ah);
      } // end for dword
    } // end for dev
  } // end for bus
  PCIe_REGR_TEST("PASS/FAIL Passed if each device receives one Configuration Request.");
  
  cout << endl << endl << endl;
  cout << "------------------------------------------------------------" << endl;
  cout << "------------- Calculate base addresses" << endl;
  cout << "------ read base address registers and calculate base addresses -----" << endl;
  gs_uint64 next_MEMORY_base_addr = 4096; // first base address for memory address space; must be naturally aligned!
  gs_uint64 next_IO_base_addr = 4096; // first base address for I/O address space; must be naturally aligned!
  unsigned int dev_mem_size_reg; // original register read out from the device
  unsigned int dev_mem_size;     // calculated device's mem size
  // send Configuration Request reading to base address registers
  data_size = 4;
  dat->clear(); dat->resize(data_size);
  for (unsigned int bus = 0; bus < 7; bus++) {
    for (unsigned int dev = 0; dev < 1; dev++) {
      for (unsigned int dword = 0; dword < 6; dword++) {
        if (dword > 0 && ah->get_Completion_Status() != SuccessfulCompletion) break; // stop for this device if first transaction was not completed
        ah = down_to_router_port.create_transaction();
        cout << endl<<"**** Send Configuration Read Request for base address field to **** bus "<<bus<<", dev "<<dev<< " conf. reg. address 0x"<<hex<<(0x10 +dword*4)<<dec<<"("<<(0x10 +dword*4)<<"d) ****" << endl;
        ah->init_Configuration_Read(bus, dev, 0,
                                     0x10 + dword*4, // Base Address
                                     *dat, data_size);
        down_to_router_port.send_transaction(ah);
        if (ah->get_Completion_Status() == SuccessfulCompletion) {
          // ** get mem size out of transaction
          dev_mem_size_reg = 0;
          //    convert char to uint
          for (unsigned int u=0; u<sizeof(unsigned int); ++u)
            dev_mem_size_reg = (dev_mem_size_reg << CHAR_BIT) | dat->at(u);
          cout << " received base address register (32 bit):"<< endl;
          cout << "  ["<< (unsigned int)dat->at(0)<<"]["<< (unsigned int)dat->at(1)<<"]["<< (unsigned int)dat->at(2)<<"]["<< (unsigned int)dat->at(3)<<"]"
               << " = " << helpfunc::to_binary_string(dev_mem_size_reg) << " = " << dev_mem_size_reg << endl;
          if (dev_mem_size_reg != 0) {
            // TODO ignore upper 16 bits if I/O base address and these bits are all 0: dann nur die unteren 16 bit berechnen
            //    clear encoding information
            if ((dev_mem_size_reg & 0x00000001) == 1) // if I/O
              dev_mem_size = dev_mem_size_reg & 0xFFFFFFFE; // make bit 0 = b0
            else if ((dev_mem_size_reg & 0x00000001) == 0) // if Mem
              dev_mem_size = dev_mem_size_reg & 0xFFFFFFF0; // make bits 0...3 = b0
            cout << " cleared:  " << helpfunc::to_binary_string(dev_mem_size) << " = " << dev_mem_size << endl;
            dev_mem_size = dev_mem_size ^ 0xFFFFFFFF; // invert
            cout << " inverted: " << helpfunc::to_binary_string(dev_mem_size) << " = " << dev_mem_size << endl;
            dev_mem_size++; // increment +1
            cout << " incremen: " << helpfunc::to_binary_string(dev_mem_size) << " = " << dev_mem_size << endl;
            cout << " calculate range size = "<<dev_mem_size<<endl;
            // ** calculate base address
            // TODO!! Beware of bit-alignments!
            // ** store address and size into local map
            // ** and increment base address to be assigned to the next device
            if (dev_mem_size > 0) {
              if ((dev_mem_size_reg & 0x00000001) == 1) { // if I/O address space
                // ** calculate I/O space
                // Beware: upper 16 bit are ignored by the devices if bits 16-32 returned zero on read!
                PCIeDEBUG2(name(), "** RESULT: Store I/O address range 0x%llx to 0x%llx (size %u) for device #%u (bus #%u)\n",  (long long unsigned int)next_IO_base_addr, (long long unsigned int)next_IO_base_addr+dev_mem_size-1, dev_mem_size, dev, bus);
                Dev_dword devdword; devdword.busNo = bus; devdword.devNo = dev; devdword.dwordNo = dword;
                m_IO_base_address_map.insert(  std::make_pair( devdword, std::make_pair(next_IO_base_addr, dev_mem_size) )  );
                // calculate naturally aligned addresses
                next_IO_base_addr = helpfunc::find_next_naturally_aligned_address(next_IO_base_addr + dev_mem_size); 
              } else if ((dev_mem_size_reg & 0x00000001) == 0) { // if Memory address space
                // ** calculate Memory space
                PCIeDEBUG2(name(), "** RESULT: Store MEMORY address range 0x%llx to 0x%llx (size %u) for device #%u (bus #%u)\n",  (long long unsigned int)next_MEMORY_base_addr, (long long unsigned int)next_MEMORY_base_addr+dev_mem_size-1, dev_mem_size, dev, bus);
                Dev_dword devdword; devdword.busNo = bus; devdword.devNo = dev; devdword.dwordNo = dword;
                m_MEMORY_base_address_map.insert(  std::make_pair( devdword, std::make_pair(next_MEMORY_base_addr, dev_mem_size) )  );
                // calculate naturally aligned addresses
                next_MEMORY_base_addr = helpfunc::find_next_naturally_aligned_address(next_MEMORY_base_addr + dev_mem_size);
              }
            } // end dev_mem_size > 0
          } // end if dev_mem_size_reg == 0
          else {
            cout << " Not processing not implemented address register at the target." << endl;
          }
        } // end if SuccessfulCompletion
      } // end for dword
    } // end for dev
  } // end for bus
  PCIe_REGR_TEST("PASS/FAIL Passed if each device receives one Configuration Request.");
  delete dat; dat = NULL;

  // TODO: Restore / set all base addresses
  
  cout << endl << endl << endl;
  cout << "------------------------------------------------------------" << endl;

  cout << "------------- write I/O base addressses to devices" << endl;
  { // local block
    unsigned int data_size = 4;
    std::vector<gs_uint8> *wdat;
    wdat = new std::vector<gs_uint8>(data_size);
    gs_uint32 addr;
    std::map<Dev_dword, std::pair<gs_uint64, unsigned int> >::iterator iter;
    for (iter = m_IO_base_address_map.begin(); iter != m_IO_base_address_map.end(); iter++) {
      addr = iter->second.first;
      assert(sizeof(addr) == data_size);
      unsigned char ser[] = {0,0,0,0}; 
      memcpy(&ser, &(addr), data_size); // TODO: directly memcpy to vector!!
      wdat->at(0) = ser[0];
      wdat->at(1) = ser[1];
      wdat->at(2) = ser[2];
      wdat->at(3) = ser[3];
      //memcpy(wdat, &addr, data_size);
      ah = down_to_router_port.create_transaction();
      cout << endl<<"**** Send Configuration Write Request for base address field to **** bus "<<iter->first.busNo<<", dev "<<iter->first.devNo<< " conf. reg. address 0x"<<hex<<(0x10 +iter->first.dwordNo*4)<<dec<<"("<<(0x10 +iter->first.dwordNo*4)<<"d) ****" << endl;
      cout << "     set address "<<addr<< endl;
      ah->init_Configuration_Write(iter->first.busNo, iter->first.devNo, 0,
                                   0x10 + iter->first.dwordNo*4, // Base Address
                                   *wdat, data_size);
      down_to_router_port.send_transaction(ah);
    }
  } // end block

  cout << "------------- write Memory base addressses to devices" << endl;
  { // local block
    unsigned int data_size = 4;
    std::vector<gs_uint8> *wdat;
    wdat = new std::vector<gs_uint8>(data_size);
    gs_uint32 addr;
    std::map<Dev_dword, std::pair<gs_uint64, unsigned int> >::iterator iter;
    for (iter = m_MEMORY_base_address_map.begin(); iter != m_MEMORY_base_address_map.end(); iter++) {
      addr = iter->second.first;
      assert(sizeof(addr) == data_size);
      unsigned char ser[] = {0,0,0,0}; 
      memcpy(&ser, &(addr), data_size); // TODO: directly memcpy to vector!!
      wdat->at(0) = ser[0];
      wdat->at(1) = ser[1];
      wdat->at(2) = ser[2];
      wdat->at(3) = ser[3];
      //memcpy(wdat, &addr, data_size);
      ah = down_to_router_port.create_transaction();
      cout << endl<<"**** Send Configuration Write Request for base address field to **** bus "<<iter->first.busNo<<", dev "<<iter->first.devNo<< " conf. reg. address 0x"<<hex<<(0x10 +iter->first.dwordNo*4)<<dec<<"("<<(0x10 +iter->first.dwordNo*4)<<"d) ****" << endl;
      cout << "     set address "<<addr<< endl;
      ah->init_Configuration_Write(iter->first.busNo, iter->first.devNo, 0,
                                   0x10 + iter->first.dwordNo*4, // Base Address
                                   *wdat, data_size);
      down_to_router_port.send_transaction(ah);
    }
  } // end block
  
  // TODO: Enable command registers

  
    
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
