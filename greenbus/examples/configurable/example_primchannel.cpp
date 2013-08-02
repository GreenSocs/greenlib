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

#include "systemc.h"
#include <vector>

#ifdef USE_DUST
#include "greenbus/utils/dust_model.h"
#endif

#include "greenbus/transport/GP/GP.h"
#include "greenbus/utils/gb_config.h"


using namespace tlm;
using namespace std;


class baby : public sc_module, public gb_configurable {
public:
  
  SC_CTOR(baby) {
    GS_TRACE("I am the baby %s", this->name());
  }

};



class master : public sc_module, public gb_configurable {  
public:

  sc_port<sc_fifo_out_if<int> > out;

  SC_CTOR(master) : out("out") {
    GS_TRACE("I am the master %s", name);

    // register parameters
    GB_PARAM(addr, MAddr);
  }

  gb_param<MAddr> addr;
};


class slave : public sc_module, public gb_configurable {
public:

  sc_port<sc_fifo_in_if<int> > in;

  SC_CTOR(slave) : in("in"), mybaby("erik") {
    GS_TRACE("I am the slave %s", this->name());

    GB_PARAM(addr_base, MAddr);
    GB_PARAM(addr_high, MAddr);    
  }

  void b_transact(GenericTransactionHandle t)
  {
    cout << name() << " WARNING: PV b_transact not implemented by this slave." << endl;
  }  

  gb_param<MAddr> addr_base, addr_high;
  gb_param<char[5]> foo; 

  baby mybaby;
};


static void dump_hierarchy(const vector<sc_object *> &objs) {
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

  master m("master");
  slave s("slave");

  sc_fifo<int> f("fifo");
  m.out(f);
  s.in(f);

  // dump object hierarchy
  cout << endl << "-- object hierarchy --" << endl;
  dump_hierarchy(sc_get_curr_simcontext()->get_child_objects());    

  model_builder mb();
  
  sc_start(-1);

  return EXIT_SUCCESS;
}
