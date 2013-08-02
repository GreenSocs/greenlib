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
 * Example demonstrates a problem with module hierarchy.
 *
 * Problem definition: 
 * The default constructor of sc_module master (master.h) sets its module_name 
 * statically as a constructor initializer: 
 * 
 *     master() : sc_module_name("master").
 *
 + A SystemC error is reported when the module is created with memory type "auto":
 * 
 *     master m; --> "Invalid use of sc_module_name".
 *
 * However, if the module is created with the new operator, elaboration will success: 
 *
 *     master *m = new master();
 *
 * But in this case, all modules that are created after the master module 
 * will be reported as SUBMODULES of master in the simulation context!
 *
 * In the SystemC documentation there seems to be no explanation for this
 * behaviour.
 */

#include "greenbus/transport/GP/GP.h"
#include "greenbus/utils/gb_config.h"

#include "master.h"

using namespace tlm;


class baby : public sc_module, public gb_configurable {
public:
  
  SC_CTOR(baby) {
    GS_TRACE("I am the baby %s", this->name());

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
    GS_TRACE("I am the slave %s", this->name());

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


void dump_hierarchy(const vector<sc_object *> &objs) {
  vector<sc_object *>::const_iterator p = objs.begin();
  while (p != objs.end()) {
    cout << (*p)->name() << endl;
    sc_module *m = dynamic_cast<sc_module*>(*p);
    if (m)
      dump_hierarchy(m->get_child_objects());
    ++p;
  }
}

int sc_main(int argc, char *argv[]) {

#ifdef USE_DUST
  DUST_INITIALIZE;
#endif

  //master m("master");
  master *m = new master(); // --> forces following modules to be submodules of master
  slave s("slave");  

  // dump object hierarchy
  cout << endl << "-- object hierarchy --" << endl;
  dump_hierarchy(sc_get_curr_simcontext()->get_child_objects());    
  
  sc_start(-1);

  return EXIT_SUCCESS;
}
