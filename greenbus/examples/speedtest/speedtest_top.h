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

#ifndef __speedtest_top_h__
#define __speedtest_top_h__

#include "globals.h"

#include "ddr_ram.h"
#include "pseudo_master_simplebus.h"
#include "greenbus/api/simplebusAPI.h"

#include "greenbus/utils/gb_config.h"
#include "greenbus/utils/gb_configurable.h"
#include "greenbus/utils/router_fabric.h"
#include "greenbus/utils/gs_datatypes.h"

#include <sstream>

#include "systemc.h"

/**
 * GreenBus speedtest top-level module.
 */
class speedtest_top
: public sc_module,
  public gb_configurable
{
 public:
  gb_param<gs_uint32> number_of_masters; // masters will be named m0, m1, m2, m3, ..
  gb_param<gs_uint32> number_of_slaves;  // slaves will be named s0, s1, s2, s3, ..
  gb_param<std::string> busname; // chose SimpleBus, PLB, or whatever

  SC_HAS_PROCESS(speedtest_top);
  speedtest_top(sc_module_name, char*, unsigned);
  void before_end_of_elaboration();
  GenericRouter* create_bus(busmap *m, const char* busname);

protected:
  char *m_cfgfile;
  unsigned delay;
};


#endif

