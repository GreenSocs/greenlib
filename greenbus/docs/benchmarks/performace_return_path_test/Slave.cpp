// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf
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

#include "Slave.h"

bool Slave::notify (unsigned int& p, sc_time& t, bool use_return_path) {
  PERF_TRACE("Slave got notify (p="<<p<<", t="<<t.to_string()<<")");

  // We do something with the payload:
  p = p + 1;

  if (use_return_path) {
    PERF_TRACE("      return path ENABLED: return modified payload (="<<p<<") with 30ms time increment");
    t += sc_time(10, SC_MS);
    // Think if we were adding here 30 ms waiting time to the payload
    return true;
  }
  else {
    PERF_TRACE("      return path DISABLED: return modified payload (="<<p<<") after 30ms");
    peq_out_port->notify(p, sc_time(10, SC_MS));
    return false;
  }
}
