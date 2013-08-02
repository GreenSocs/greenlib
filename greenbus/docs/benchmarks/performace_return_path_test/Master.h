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

#ifndef __Master_h__
#define __Master_h__


#include <systemc.h>

#include "globals.h"
#include "payload_event_queue_only_call_delta_boost_return_path.h"


using namespace tlm;

class Master
 : public sc_module,
   public payload_event_queue_output_if<unsigned int>
{
public:

  sc_port<payload_event_queue_if<unsigned int> > peq_out_port;

  SC_HAS_PROCESS(Master);

  // Constructor
  Master(sc_module_name name_) : 
    sc_module(name_)
  {
    SC_THREAD(main_action);
  }
  
  ~Master() {
  }

  void main_action();
  
  virtual bool notify (unsigned int& p, sc_time& t, bool use_return_path);

private:

  sc_event m_got_notify_event;
  
};

#endif
