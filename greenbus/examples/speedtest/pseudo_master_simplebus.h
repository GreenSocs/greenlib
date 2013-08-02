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

#ifndef __pseudo_master_simplebus_h__
#define __pseudo_master_simplebus_h__

#include "globals.h"
#include "systemc.h"
#include "greenbus/utils/gb_configurable.h"
#include "greenbus/api/simplebusAPI.h"
#include "time_measure.h"

using namespace tlm;

class pseudo_master_simplebus
: public sc_module,
  public gb_configurable
{
public:
  simplebusMasterPort port;
  
  SC_HAS_PROCESS(pseudo_master_simplebus);
  pseudo_master_simplebus(sc_module_name name);  
  void action();
  void read();
  void write();
  bool doWrite();
  bool doRead();
  void updateWrite();
  void updateRead();
  gs_uint32 getRandom(gs_uint32);
  void end_of_simulation();
  
  gb_param<MBurstLength> init_size;    // data vector size (1MB)
  gb_param<MBurstLength> write_size;   // write bytes (1024)
  gb_param<MBurstLength> read_size;    // read bytes (1024)
  gb_param<MAddr> write_addr;          // write address (0)
  gb_param<MAddr> read_addr;           // read address (0)
  gb_param<MBurstLength> clk_period;   // clk period in ns (10)
  gb_param<MBurstLength> write_inc;    // increment write byte size per loop (0)
  gb_param<MBurstLength> read_inc;     // increment read byte size per loop (0)
  gb_param<MBurstLength> loops;        // # benchmark loops (10)
  gb_param<MBurstLength> read_count;   // # read transactions per measure (10)
  gb_param<MBurstLength> write_count;  // # write transactions per measure (10)
  gb_param<MBurstLength> write_count_inc; // increment number of write transaction per measure (0)
  gb_param<MBurstLength> read_count_inc; // increment number of write transaction per measure (0)
  gb_param<MBurstLength> write_count_rnd; // # write transactions is in given random range (0)
  gb_param<MBurstLength> read_count_rnd; // # read transactions is in given random range (0)
  gb_param<MBurstLength> delay_max;    // max. random clk cycle delay between transactions (100)
  
private:
  std::vector<gs_uint8> mData;
  timeMeasure tm;	
  //gs_uint32 tcount;
};


#endif
