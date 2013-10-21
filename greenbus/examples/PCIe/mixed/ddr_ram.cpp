// LICENSETEXT
//
//   Copyright (C) 2007 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
//   Wolfgang Klingauf <klingauf@eis.cs.tu-bs.de>
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

#include "ddr_ram.h"

ddr::ddr(sc_module_name name)
  : sc_module(name),
    clk_period(/*"clk_period", */DDR_CLK_PERIOD),
    size(/*"size",*/ DDR_SIZE),
    write_ws(/*"write_ws", */DDR_WRITE_WAIT_STATES),
    read_ws(/*"read_ws", */DDR_READ_WAIT_STATES),
    simulation_mode(/*"simulation_mode", */DDR_SIMULATION_MODE)

{
  DEMO_TRACE(sc_module::name(), "I am a ddr memory block!");
}


ddr::~ddr() {
}


bool ddr::read(std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
  sc_assert(length<=size/*.getValue()*/); // TODO: original < statt <=
  sc_assert(data.size()>=length); 
  if (length==0) return true;

  DEMO_TRACE(sc_module::name(), "Reading %d bytes from memory address 0x%x, takes %d ns",   
           length, addr, ((read_ws+length)>>3)*clk_period);
  wait((read_ws+length)*clk_period, SC_NS);
  if (memory.size() != size/*.getValue()*/)
    memory.resize(size/*.getValue()*/,0);

  if (simulation_mode==0) { // BA
    memcpy(&data[0], &memory[addr], length); // copy all data at once
    wait((read_ws+(length>>3))*clk_period, SC_NS); // estimate ram access time
  }
    
  else { // CT
    wait(read_ws*clk_period, SC_NS); // initial wait_states
    for (gs_uint32 i=0; i<length; i+=8) { // we assume a 64bit RAM interface
      memcpy(&data[i], &memory[addr+i], 8); // copy 64 bit
      wait(clk_period, SC_NS);
    }   
  }  

  return true;
}


bool ddr::write(const std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
  sc_assert(length<=size/*.getValue()*/); // TODO: original < statt <=
  sc_assert(data.size()>=length);
  if (length==0) return true;
  
  DEMO_TRACE(sc_module::name(), "Writing %d bytes to memory address 0x%x, takes %d ns", 
           length, addr, (write_ws+(length>>3))*clk_period);

  if (memory.size() != size/*.getValue()*/) 
    memory.resize(size/*.getValue()*/,0);

  // copy received data to memory
  //memory.assign(data.begin(), data.begin()+length); // very slow, speed independent of length!
  //for (gs_uint32 i=0; i<length; i++) { // faster
  //memory[i+addr] = data[i];
  //}

  if (simulation_mode==0) { // BA
    memcpy(&memory[addr], &data[0], length); // copy all data at once
    wait((write_ws+(length>>3))*clk_period, SC_NS); // estimate ram access time
  }
    
  else { // CT
    wait(write_ws*clk_period, SC_NS); // initial wait_states
    for (gs_uint32 i=0; i<length; i+=8) { // we assume a 64bit RAM interface
      memcpy(&memory[addr+i], &data[i], 8); // copy 64 bit
      wait(clk_period, SC_NS);
    }   
  }  
      

  return true;
}

