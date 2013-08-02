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

/**
 * Example demonstrates the usage of GreenBus Configuraiton Framework
 */

//#define VERBOSE

#ifdef USE_DUST
#include "greenbus/utils/dust_model.h"
#endif

#include "greenbus/core/tlm.h"

using namespace tlm;

class master : public sc_module, public gb_configurable {  
public:

  SC_CTOR(master) {
    GS_TRACE(name(), "I am a master");

    // register parameters
    GB_PARAM(addr, MAddr);
    GB_PARAM(cmd, MCmd);
    GB_PARAM(data, char[20]);
  }

  gb_param<MAddr> addr;
  gb_param<MCmd> cmd;
  gb_param<char[20]> data;
};


class slave : public sc_module, public gb_configurable {
public:

  SC_CTOR(slave) {
    GS_TRACE(name(), "I am a slave");

    GB_PARAM(addr_base, MAddr);
    GB_PARAM(addr_high, MAddr);
  }

  gb_param<MAddr> addr_base, addr_high;
  gb_param<char[5]> foo; 
};


int sc_main(int argc, char *argv[]) {

#ifdef USE_DUST
  DUST_INITIALIZE;
#endif

  master m("master");
  slave s("slave");  

  // set parameters
  m.setParam("addr", "0x1000");
  m.setParam("cmd", "1"); 
  m.setParam("data", "Hello,GreenBus!");

  s.setParam("addr_base", "0x1000");
  s.setParam("addr_high", "0x10FF");

  // test: set a Parameter that has not been registered
  s.setParam("foo", "bar");
  

  sc_start(-1);

  return EXIT_SUCCESS;
}
