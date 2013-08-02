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

#ifndef __ConfReg_h__
#define __ConfReg_h__

#include "greenbus/utils/gs_datatypes.h"
#include <math.h>


namespace helpfunc {
  
  static const char* to_binary_string(tlm::gs_uint64 n) {
    std::ostringstream ss;
    //cout << "decimal: " << n << endl;
    
    // print binary with leading zeros
    //cout << "binary : ";
    for (int i=31; i>=0; i--) {
      int bit = ((n >> i) & 1);
      ss << bit;
    }
    //cout << endl;
    return ss.str().c_str();
  }
  
  /// Returns the next naturally aligned address greater equal the given one
  /**
   * TODO: correct but not optimal 
   */
  static tlm::gs_uint64 find_next_naturally_aligned_address(tlm::gs_uint64 addr) {
    //cout << "find_next_naturally_aligned_address("<<addr<<"="<<to_binary_string(addr)<<")"<<endl;
    // Check if all lower bits are '1's, if not fill up all with '1's
    tlm::gs_uint64 result = 0;
    int bit;
    bool now_only_ones = false;
    for (int i=31; i>=0; i--) {
      bit = ((addr >> i) & 1);
      if (bit) { 
        if (!now_only_ones) {
          result = pow(2, i+1);
          //cout << " result=2^(i+1)="<<pow(2, i+1);
          //cout << "  result = "<<result<<"="<<to_binary_string(result)<<endl;
          return result;
        }
        now_only_ones = true;
      }
    }
    return result;
  }
  
} // end namespace


#define ADD_REGISTER(name, byteIndex, bitSize) \
  void set_##name(gs_uint##bitSize val) { \
    memcpy(&register_memory[byteIndex], &val, sizeof(val)); \
  } \
  gs_uint##bitSize get_##name() { \
    gs_uint##bitSize ret; \
    memcpy(&ret, &register_memory[byteIndex], sizeof(ret)); \
    return ret; \
  } \

namespace tlm {
namespace PCIe {

using namespace tlm;

/// PCIe Configuration Register API for PCIe devices
/**
 * API to access the register memory array with convenience methods.
 */
class ConfReg
{ 
public:
  
  /// Initialization struct for one base address
  struct Base_address_struct {
    bool BaseAddressSpaceIndicator; // 0: Memory Space, 1: I/O Space
    bool BaseAddressPrefetchable; // 0: not prefetchable, 1: prefetchable (means no side-effects on reads)
    unsigned int BaseAddressWidthType; // 0: 32-bit access space, 2: 64-bit access space
    unsigned int BaseAddressRequiredMemorySpaceAmount0HardwiredBits; // Required Memory Space: Amount of hardwired bits including the already used ones 0,1 (I/O) or 0-3 (Mem)
    unsigned int BaseAddressDWORDposition; // on which DWORD position of the Base Address field in the Configuration Space Header
    gs_uint32 address; // contains 32 bit (TODO: not supported in demo: or 64 bit) base address
    
    std::string toString() {
      std::ostringstream s;
      s << "Base address = "<< address << "\n";
      s << "   Space indicator: ";
      if (BaseAddressSpaceIndicator) s << "0 (Memory Space)\n";
      else  s << "1 (I/O Space)\n";
      s << "   Prefetchable: ";
      if (BaseAddressPrefetchable) s << "1 (yes)\n";
      else  s << "0 (no)\n";
      s << "   width: ";
      if (BaseAddressWidthType == 0) s << "0 (32 bit)\n";
      else if (BaseAddressWidthType == 2) s << "2 (64 bit)\n";
      else s << "reserved\n";
      s << "   Hardwired bits = " << BaseAddressRequiredMemorySpaceAmount0HardwiredBits << "\n";
      s << "   BaseAddressDWORDposition = " << BaseAddressDWORDposition << " = conf. reg. addr 0x"<< hex<<(0x10 +BaseAddressDWORDposition*4)<<dec<<"("<<(0x10 +BaseAddressDWORDposition*4)<<"d)";
      return s.str();
    }
  };
  
  /// Specifies in the vector base_address_reg_dword_types of which type the data dword is
  enum BaseAddressRegDWordType {
    UNUSED = 0,
    HEAD32bit//,  // bits  0...31 of a 32 bit base address
    //HEAD64bit,  // bits  0...31 of a 64 bit base address
    //TAIL64bit   // bits 32...63 of a 64 bit base address
  };
  
  /// Initialization struct for one device function
  struct Register_init_struct {
    // TODO vedor id etc.
    unsigned int header_type; // 0: device header, 1: Switch, Root Complex, Bridge header
  };
  
  public:
  
  /// Constructor
  /**
   * @param register_memory  Pointer to the register memory the API should work on
   */
  ConfReg(gs_uint8 reg[]) {
    register_memory = reg;
  }
  
  // Register the different fields in the memory with their position and size
  ADD_REGISTER(headerType, 0x0E, 8)
  ADD_REGISTER(Memory_Limit, 0x22, 16)
  
  // ////// Initialize functions ///////// //

  // TODO: init all read-only and default fields in the Configuration Register Space
  //       (address space, vendor id etc.)
  //  To be called by the owning device!
  void init(Register_init_struct init) {
    set_headerType(init.header_type);
    if (get_headerType() == 0) {
      base_address_reg_dword_types.resize(6, UNUSED);
    } 
    else if (get_headerType() == 1) {
      base_address_reg_dword_types.resize(2, UNUSED);
    }
    else {
      assert(false);
    }
  } 
  /// Adds a base address init struct
  /**
   * Member 'address' should be set to 0!
   */
  void init_add_base_address(Base_address_struct init) {
    PCIeDEBUG2("PCIe ConfReg", "Add base address to register");
    if (init.BaseAddressSpaceIndicator == 0)
      assert(init.BaseAddressRequiredMemorySpaceAmount0HardwiredBits >= 3);
    else
      assert(init.BaseAddressRequiredMemorySpaceAmount0HardwiredBits >= 1);
    //** update vector base_address_reg_dword_types
    // find first unused position
    bool succ = false;
    unsigned int i;
    for (i = 0; i < base_address_reg_dword_types.size(); i++) {
      if (base_address_reg_dword_types[i] == UNUSED) {
        // set start position of this base address
        init.BaseAddressDWORDposition = i;
        cout << "  ConfReg: add new base register to address dword #"<<i <<endl;
        // set dword vector
        if (init.BaseAddressWidthType == 0) base_address_reg_dword_types[i] = HEAD32bit;
        else assert(false); // unsupported in demo
        /*else if (init.BaseAddressWidthType == 2) {
          base_address_reg_dword_types[i] = HEAD64bit;
          base_address_reg_dword_types[i+1] = TAIL64bit;
        }*/
        succ = true;
        break;
      }
    }
    sc_assert(succ);
    // ** Store base address
    base_addresses.push_back(init);
    // build registers:
    // write default address (the read-only registers are calculated there!)
    std::vector<gs_uint8> data_target(4, 0);
    memcpy(&data_target[0], &(init.address), 4);
    write(16 + (i*4), 4, &data_target);
    // show
    cout << "Existing base addresses in this PCIe ConfReg: " << endl;
    for (std::vector<Base_address_struct>::iterator iter = base_addresses.begin(); iter != base_addresses.end(); iter++) {
      cout << endl << " " << (*iter).toString();
    }
    cout << endl;
  }
  
  /// Helping function for write function: calculates the read-only bits
  gs_uint32 build_read_only_base_address_register_part(Base_address_struct &init) {
    gs_uint32 reg = 0;
    if (init.BaseAddressSpaceIndicator) { // Memory space 
      // Space indicator
      reg = reg | 0x0;
      // Type (32/64 bit)
      if (init.BaseAddressWidthType == 2) { // 64 bit
        reg = reg | 0x4; // b10x
        assert(false); // not supported in demo
      } // else: 32 bit b00x
      // Prefetchable
      if (init.BaseAddressPrefetchable)
        reg = reg | 0x8; // b1000
    } else { // I/O space
      reg = reg | 0x1;
    }
    return reg;
  }
  
  // //////// Access functions /////////// //
  
  /// If the PCI compatible Configuration Registers are of Header Type 0x00
  bool is_Type_00_Header() {
    // TODO
    if (get_headerType() == 0)
      return true;
    else return false;
  }

  /// If the PCI compatible Configuration Registers are of Header Type 0x01
  bool is_Type_01_Header() {
    // TODO
    if (get_headerType() == 1)
      return true;
    else return false;
  }
  
  /// Write function
  /**
   * Must ensure correct behavior of each register
   * (meaning write protected bits etc.)!
   * Must not overwrite read-only bits!
   *
   * @param register_addr  Address of the register (register number).
   * @param write_length   Amount of bytes that should be written
   * @param data           Pointer to vector containing the data (beginning at [0])
   * @return  If write was successfull or if an error (unsupported request) should be returned
   */
  bool write(unsigned int register_addr, unsigned int write_length, std::vector<gs_uint8>* data) {
    PCIeDEBUG2("ConfReg", "Register write to register %u, size = %u", register_addr, write_length);
    if (data->size() < write_length) {
      SC_REPORT_WARNING("PCIe ConfReg", "Configuration Register write fails because data is to small!");
      return false;
    }
    // Device Header Type
    if (is_Type_00_Header()) {
      // *** TODO **** THIS IS A DEMO, CODED BY HAND FOR EACH REGISTER *****
      // ** Base Address Registers:
      if (register_addr >= 16 /*10h*/ && register_addr <= 36 /*24h*/) {
        bool succ = false;
        assert(write_length == 4); // TODO: handle smaller writes, too??
        for (unsigned int i = 0; i < base_addresses.size(); i++) {
          // if this is the write target // 64-bit not supported in this demo
          if (base_addresses[i].BaseAddressDWORDposition *4 +0x10 == register_addr) { // Write into the correct word
            // simply write the 32 bit address
            if (base_address_reg_dword_types[i] == HEAD32bit) {
              gs_uint32 tmp = 0;
              // convert char to uint
              memcpy(&tmp, &(data->at(0)), 4);  //TODO: andersrum: for (unsigned int u=0; u<sizeof(unsigned int); ++u) tmp = (tmp << CHAR_BIT) | data->at(u);
              cout << "  write 32 bit address ["<< (unsigned int)data->at(0)<<"]["<< (unsigned int)data->at(1)<<"]["<< (unsigned int)data->at(2)<<"]["<< (unsigned int)data->at(3)<<"]= " << tmp ;
              cout << "  to the dword #"<<base_addresses[i].BaseAddressDWORDposition << endl;
              // calculate read-only bits with bit mask
              cout << "  original to write:              " << helpfunc::to_binary_string(tmp) <<" = "<<tmp<< endl;
              gs_uint32 bit_mask = pow(2, base_addresses[i].BaseAddressRequiredMemorySpaceAmount0HardwiredBits) -1;
              bit_mask = bit_mask ^ 0xFFFFFFFF; // negate
              cout << "  bitmask:                        " << helpfunc::to_binary_string(bit_mask) <<" = "<<bit_mask<< endl;
              tmp = tmp & bit_mask;
              cout << "  to write (ro not yet regarded): " << helpfunc::to_binary_string(tmp) <<" = "<<tmp<< endl;
              // add default read-only bits
              gs_uint32 read_only = build_read_only_base_address_register_part(base_addresses[i]);
              tmp = tmp ^ read_only;
              // store in base address struct
              base_addresses[i].address = tmp;
              cout << "  write, masked, read-only ored:  " << helpfunc::to_binary_string(tmp) <<" = "<<tmp<< endl;
              cout << endl;
              succ = true;
            }
            else assert(false); // not supported in demo
          }
        }
        if (!succ) {
          cout << " Write warning: Base address register does not exist!" << endl;
          //assert(false);
          return true;
        }
      }
      
      // *** END DEMO ******************************************************
    } 
    // Switch, Root Complex, Bridge Header Type
    else if (is_Type_01_Header()) {
      assert(false);
      // TODO
    }
    return true;
  }

  /// Read function
  /**
   * Must ensure correct behavior of each register
   * (meaning fixed bits etc.)!
   *
   * @param register_addr  Address of the register (register number).
   * @param read_length    Amount of bytes that should be read
   * @param data_target    Pointer to vector where the data should be written to (beginning at [0]).
   *                       The vector must already be resized!
   * @return  If read was successfull or if an error (unsupported request) should be returned
   */
  bool read(unsigned int register_addr, unsigned int read_length, std::vector<gs_uint8>* data_target) {
    PCIeDEBUG2("ConfReg", "Register read from register %u, size = %u", register_addr, read_length);
    if (data_target->size() < read_length) {
      SC_REPORT_WARNING("PCIe ConfReg", "Configuration Register read fails because data target is to small!");
      return false;
    }
    // Device Header Type
    if (is_Type_00_Header()) {
      // *** TODO **** THIS IS A DEMO, CODED BY HAND FOR EACH REGISTER *****
      // ** Base Address Registers:
      if (register_addr >= 16 /*10h*/ && register_addr <= 36 /*24h*/) {
        bool succ = false;
        assert(read_length == 4); // TODO: handle smaller writes, too??
        for (unsigned int i = 0; i < base_addresses.size(); i++) {
          // if this is the write target // 64-bit not supported in this demo
          if (base_addresses[i].BaseAddressDWORDposition*4 + 0x10 == register_addr) {
            // simply write the 32 bit address
            if (base_address_reg_dword_types[i] == HEAD32bit) {
              // convert base address to char data vector
              assert(data_target->size() >= 4);
              unsigned char ser[] = {0,0,0,0}; 
              memcpy(&ser, &(base_addresses[i].address), read_length); // TODO: directly memcpy to vector!!
              data_target->at(0) = ser[3];
              data_target->at(1) = ser[2];
              data_target->at(2) = ser[1];
              data_target->at(3) = ser[0];
              cout << "  read 32 bits: "<< base_addresses[i].address 
                   << " = ["<<(unsigned int)data_target->at(0)<<"]["<<(unsigned int)data_target->at(1)<<"]["<<(unsigned int)data_target->at(2)<<"]["<<(unsigned int)data_target->at(3)<<"] = "<< helpfunc::to_binary_string(base_addresses[i].address)<<")"<< endl;
              succ = true;
            }
            else assert(false); // not supported in demo
          }
        }
        if (!succ) {
          data_target->at(0) = 0;
          data_target->at(1) = 0;
          data_target->at(2) = 0;
          data_target->at(3) = 0;
          cout << " Read warning: Base address register does not exist!" << endl;
          return true;
        }
      }
      
      // *** END DEMO ******************************************************
    } 
    // Switch, Root Complex, Bridge Header Type
    else if (is_Type_01_Header()) {
      assert(false);
      // TODO
    }
    return true;
  }
  
  void show_registers() {
    cout << "Registers:";
    for (unsigned int i=0; i<0x40; i++) {
      if (i % 4 == 0) cout << endl<< " 0x"<< (hex)<<i<<(dec) << "   ";
      cout << (unsigned int) register_memory[i] << " ";
    }
    cout << endl;
  }
  
  gs_uint8 *register_memory;
  
  std::vector<Base_address_struct> base_addresses;
  std::vector<BaseAddressRegDWordType> base_address_reg_dword_types;
  
};


} // end namespace PCIe
} // end namespace tlm


#endif
