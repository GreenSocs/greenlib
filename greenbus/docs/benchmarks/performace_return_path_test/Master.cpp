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

#include "Master.h"

bool Master::notify (unsigned int& p, sc_time& t, bool use_return_path) {
  PERF_TRACE("Master got notify");
  if (use_return_path) {
    // failure
    SC_REPORT_WARNING(name(), "Got notify with enabled return path from slave.");
  }
  else {
    m_got_notify_event.notify();
    PERF_TRACE("got acked transaction (p="<<p<<", t="<<t.to_string()<<")");
  }
  return false;
}

void Master::main_action() {
  cout << "Master main action" << endl;
  
  unsigned int p = 1000;
  
// if NOT return path
#ifndef USE_RETURN_PATH 

  // make transactions without return path
  PERF_TRACE("Create and send normal transactions without return path:"<<endl);
  for (int i=0; i< AMOUNT_TRANSACTIONS; i++) {
    p = 10000 + i*10;
    PERF_TRACE("send normal transaction (p="<<p<<")");
    peq_out_port->notify(p, sc_time(10, SC_MS));
    wait(m_got_notify_event);
    PERF_TRACE(endl);
  }

// if return path
#else
  
  // make transactions with return path
  PERF_TRACE("Create and send return path transactions:"<<endl);
  for (int i=0; i<AMOUNT_TRANSACTIONS; i++) {
    p = 20000 + i*10;
    PERF_TRACE("send return path transaction (p="<<p<<")");
    sc_time t(10, SC_MS);
    if (peq_out_port->notify(p, t, SC_IMMEDIATE))
    {
      PERF_TRACE("got acked transaction (p="<<p<<", t="<<t.to_string()<<")");
      wait(t);
      PERF_TRACE("waited for t");
    } else {
      PERF_TRACE("got NOT acked transaction (p="<<p<<")");
    }
    PERF_TRACE(endl);
  }

#endif
  
}
