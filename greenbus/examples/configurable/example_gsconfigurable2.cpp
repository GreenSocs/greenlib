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

#ifdef USE_DUST
#include "greenbus/utils/dust_model.h"
#endif

//#include "userAPI/shipAPI.h"
#include "greenbus/transport/GP/GP.h"
#include "greenbus/utils/gb_config.h"

using namespace tlm;

class master : public sc_module, public gb_configurable {  
public:

  GenericMasterPort out;

  SC_CTOR(master) : out("out") {
    GS_TRACE("I am the master %s", name);

    // register parameters
    GB_PARAM(addr, MAddr);
    GB_PARAM(cmd, MCmd);
    GB_PARAM(data, char[20]);
  }

  gb_param<MAddr> addr;
  gb_param<MCmd> cmd;
  gb_param<char[20]> data;
};


class baby : public sc_module, public gb_configurable {
public:
  
  SC_CTOR(baby) {
    GS_TRACE("I am the baby %s", name);

    GB_PARAM(type, char[20]);
    GB_PARAM(color, char[20]);
  }

  gb_param<char[20]> type;
  gb_param<char[20]> color;
};



class slave : public sc_module, public gb_configurable, public tlm_b_if<GenericTransactionHandle> {
public:

  GenericTargetPort in;

  SC_CTOR(slave) : in("in"), mybaby("erik") {
    GS_TRACE("I am the slave %s", name);

    GB_PARAM(addr_base, MAddr);
    GB_PARAM(addr_high, MAddr);    

    // bind PV interface
    in.bind_b_if(*this);
  }

  void b_transact(GenericTransactionHandle t)
  {
    cout << name() << " WARNING: PV b_transact not implemented by this slave." << endl;
  }  

  gb_param<MAddr> addr_base, addr_high;
  gb_param<char[5]> foo; 

  baby mybaby;
};


int sc_main(int argc, char *argv[]) {

#ifdef USE_DUST
  DUST_INITIALIZE;
#endif

  master m("master");
  slave s("slave");  

  // set some parameters manually
  m.setParam("data", "Hello,GreenBus!");

  // set the rest by configuration file
  gb_config("test3.cfg");

  sc_start();

  return EXIT_SUCCESS;
}
