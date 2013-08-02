// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
//     GreenSocs Ltd
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
  : sc_module(name)
{
	GS_TRACE(sc_module::name(), "I am a ddr memory block!");
    
  GB_PARAM(clk_period, gs_uint32, DDR_CLK_PERIOD); 
  GB_PARAM(size, MBurstLength, DDR_SIZE);
  GB_PARAM(write_ws, gs_uint32, DDR_WRITE_WAIT_STATES);
  GB_PARAM(read_ws, gs_uint32, DDR_READ_WAIT_STATES);
  
  
  // fill with test data
  memory.resize(size.value);
  for (unsigned i=0; i<size.value; i++) {
    memory[i] = i;
  }  
}


ddr::~ddr() {
}


bool ddr::read(std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
  sc_assert(length<=size.value);
  sc_assert(data.size()>=length);
  if (length==0) return true;

  GS_TRACE(sc_module::name(), "Reading %d bytes from memory address 0x%x, takes %d ns", 
           length, addr, (read_ws+length>>3)*clk_period);
  if (memory.size() != size.value)
    memory.resize(size.value,0);

  memcpy(&data[0], &memory[addr], length); // copy all data at once
  wait((read_ws+(length>>3))*clk_period, SC_NS); // estimate ram access time
    
  return true;
}


bool ddr::write(const std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
  sc_assert(length<=size.value);
  sc_assert(data.size()>=length);
  if (length==0) return true;
  
  GS_TRACE(sc_module::name(), "Writing %d bytes to memory address 0x%x, takes %d ns", 
           length, addr, (write_ws+(length>>3))*clk_period);

  if (memory.size() != size.value) 
    memory.resize(size.value,0);

  // copy received data to memory
  //memory.assign(data.begin(), data.begin()+length); // very slow, speed independent of length!
  //for (gs_uint32 i=0; i<length; i++) { // faster
  //memory[i+addr] = data[i];
  //}

  memcpy(&memory[addr], &data[0], length); // copy all data at once
  wait((write_ws+(length>>3))*clk_period, SC_NS); // estimate ram access time      

  return true;
}

