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

#include "pseudo_master_simplebus.h"

pseudo_master_simplebus::pseudo_master_simplebus(sc_module_name name)
: sc_module(name),
  port("out")
{
  GS_TRACE(sc_module::name(), "I am a Pseudo-Master for the SimpleBus!");  
  
  GB_PARAM(init_size, MBurstLength, 0x100000); // 1MB
  GB_PARAM(write_size, MBurstLength, 1024);
  GB_PARAM(read_size, MBurstLength, 1024);
  GB_PARAM(write_addr, MAddr, 0);
  GB_PARAM(read_addr, MAddr, 0);
  GB_PARAM(clk_period, MBurstLength, 10);
  GB_PARAM(write_inc, MBurstLength, 0);
  GB_PARAM(read_inc, MBurstLength, 0);
  GB_PARAM(loops, MBurstLength, 2); 
  GB_PARAM(read_count, MBurstLength, 100000);
  GB_PARAM(write_count, MBurstLength, 100000);
  GB_PARAM(read_count_inc, MBurstLength, 0);
  GB_PARAM(write_count_inc, MBurstLength, 0);
  GB_PARAM(write_count_rnd, MBurstLength, 0);
  GB_PARAM(read_count_rnd, MBurstLength, 0);
  GB_PARAM(delay_max, MBurstLength, 10000);

  srand(rand()); // random seed to make multiple master behaviour differ
  //tcount = 0;

  SC_THREAD(action);
}

inline void pseudo_master_simplebus::read() {
  GS_TRACE(sc_module::name(), "Reading %d bytes from address 0x%x", (gs_uint32)read_size.value, (gs_uint32)read_addr.value);
  if (mData.size() < read_size.value) {
    mData.resize(init_size.value, 42);
    sc_assert(read_size.value<=mData.size());
  }
  tm.startMeasure();
  for (gs_uint32 i=0; i<read_count.value; i++) {
    port.read(mData, read_addr.value, read_size.value);
    sc_core::wait(getRandom(delay_max.value)*clk_period.value, SC_NS);     
    //sc_core::wait(delay_max.value*clk_period.value, SC_NS);
  }
  cout << name() << ": read (# transactions, bytes, us, cycles), " << dec << (gs_uint32)read_count.value << ", " << dec << (gs_uint32)read_size.value << ", ";
  tm.stopMeasure();
}

inline void pseudo_master_simplebus::write() {
  GS_TRACE(sc_module::name(), "Sending %d bytes to address 0x%x", (gs_uint32)write_size.value, (gs_uint32)write_addr.value);
  if (mData.size() < write_size.value) {
    mData.resize(init_size.value, 42);
    sc_assert(write_size.value<=mData.size());
  }
  tm.startMeasure();
  for (gs_uint32 i=0; i<write_count.value; i++) {
    port.write(mData, write_addr.value, write_size.value);
    sc_core::wait(getRandom(delay_max.value)*clk_period.value, SC_NS);
    //sc_core::wait(delay_max.value*clk_period.value, SC_NS);
  }
  static gs_uint32 write_count_overall = 0;
  write_count_overall+=write_count.value;
  cout << name() << ": write (# transactions, bytes, us, cycles), " << dec << (gs_uint32)write_count.value << "("<<write_count_overall<<"), " << dec << (gs_uint32)write_size.value << ", ";
  tm.stopMeasure();
}

inline bool pseudo_master_simplebus::doWrite() {
  return write_count.value>0;
}

inline bool pseudo_master_simplebus::doRead() {  
  return read_count.value>0;
}

/// update parameter values after write
inline void pseudo_master_simplebus::updateWrite() {
  loops.value-=1; // global transaction counter
  write_size.value+=write_inc.value;
  if (write_count_rnd.value==0) {
    write_count.value+=write_count_inc.value;  
  } else {
    write_count.value=getRandom(write_count_rnd.value);
  }
} 

/// update parameter values after read
inline void pseudo_master_simplebus::updateRead() {
  loops.value-=1;  // global transaction counter
  read_size.value+=read_inc.value;
  if (read_count_rnd.value==0) {
    read_count.value+=read_count_inc.value;
  } else {
    read_count.value=getRandom(read_count_rnd.value);
  }
}

/// get a random delay (configure range with parameter delay_max)
inline gs_uint32 pseudo_master_simplebus::getRandom(gs_uint32 max) {  
  return 1+ (gs_uint32)(max*(rand()/(RAND_MAX + 1.0)));
}

void pseudo_master_simplebus::end_of_simulation() {
  //cout << name() << ": Transaction count="<<tcount;
}

void pseudo_master_simplebus::action() {
  static gs_uint32 running=0;
  running++;

  updateWrite();
  updateRead();

  while(loops.value>0) {

    if (doWrite()) {
      write();
      updateWrite();
      cout << endl;
    }
    
    if (doRead()) {
      read();
      updateRead();
      cout << endl;
    }    
  }
  running--;
  if (running==0) sc_stop();
}

