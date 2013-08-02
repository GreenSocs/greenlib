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

#include "speedtest_top.h"

speedtest_top::speedtest_top(sc_module_name _name, char* cfgfile, unsigned delay)
  : sc_module(_name),
    m_cfgfile(cfgfile),
    delay(delay)
{
  GB_PARAM(number_of_masters, gs_uint32, 1);
  GB_PARAM(number_of_slaves, gs_uint32, 1);
  GB_PARAM(busname, std::string, "SimpleBus");
}


void speedtest_top::before_end_of_elaboration() {    
  // create bus
  busmap *bmap = create_busmap();
  GenericRouter *bus;
  if ((bus = create_bus(bmap, busname.value.c_str())) == NULL)
    SC_REPORT_ERROR(name(), "Could not create bus");
  
  
  // create master nodes and attach them to the bus
  for (gs_uint32 m=0; m<number_of_masters; m++) {
    std::stringstream ss;
    ss << "m" << m;
    pseudo_master_simplebus *mst = new pseudo_master_simplebus(ss.str().c_str());
    mst->delay_max=delay;
    mst->write_size=16;
    //mst->write_count=10000/number_of_masters;
    mst->write_count_rnd=10000;
    mst->read_count=(SDataWidth)0;
    mst->loops=100;
    mst->port.simulation_mode=0; // BA mode
    mst->port(bus->target_port);
  }

  // create slave nodes and attach them to the bus
  for (gs_uint32 s=0; s<number_of_slaves; s++) {
    std::stringstream ss;
    ss << "s" << s;
    ddr *slv = new ddr(ss.str().c_str());
    ss << "_tport";
    simplebusSlavePort *p = new simplebusSlavePort(ss.str().c_str());    
    p->slave_port(*slv);
    bus->init_port(*p);
  }

  gb_config(m_cfgfile);
}


GenericRouter* speedtest_top::create_bus(busmap *m, const char* busname) {
  // find create bus function in busstd::map
  busmap::iterator pos = m->find(busname);
  if (pos == m->end()) {
    char ch[256];
    sprintf(ch, "Unknown bus type specified in BUS command. See router/gsrouter.h for available busses.");
    SC_REPORT_WARNING(name(), ch);
    return NULL;
  }

  // create the bus
  sc_module_name bname(busname);
  sc_time t(10, SC_NS);
  return pos->second(bname, t);

}
