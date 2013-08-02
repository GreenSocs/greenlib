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

#ifndef __peqConcatenator_h__
#define __peqConcatenator_h__


#include <systemc.h>

#include "globals.h"
#include "payload_event_queue_only_call_delta_boost_return_path.h"

using namespace tlm;

template<typename PAYLOAD>
class peqConcatenator
 : public sc_module,
   public payload_event_queue_output_if<PAYLOAD>
{
public:

  payload_event_queue<PAYLOAD> peq;

  sc_port<payload_event_queue_if<PAYLOAD> > out_port;

  SC_HAS_PROCESS(peqConcatenator);

  // Constructor
  peqConcatenator(sc_module_name name_) : 
    sc_module(name_),
    peq("PEQ_inside_peqConcatenator")
  {
    PERF_TRACE("Construct new peq concatenator    ");
    out_port(peq);
  }
  // Constructor
  peqConcatenator() : 
    sc_module("DefaultPeqConcatenator"),
    peq("PEQ_inside_peqConcatenator")
  {
    PERF_TRACE("Construct new peq concatenator    ");
    out_port(peq);
  }
  
  virtual bool notify (PAYLOAD& p, sc_time& t, bool use_return_path) {
    PERF_TRACE("notify translation in concatenator peq");
    bool succ = false;
    if (use_return_path) {
      t += sc_time(10, SC_MS);
      succ = out_port->notify(p, t, SC_IMMEDIATE);
      t += sc_time(10, SC_MS);
    } else {
      out_port->notify(p, sc_time(10, SC_MS));
    }
    return succ;
  }

};



#endif