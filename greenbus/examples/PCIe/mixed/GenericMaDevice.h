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

#ifndef __GenericMaDevice_h__
#define __GenericMaDevice_h__

#include <systemc.h>

#include "mixed_globals.h"
#include "greenbus/transport/GP/GP.h"

#define MEMSIZE 100000

using namespace tlm;


class GenericMaDevice
 : public sc_module,
   public payload_event_queue_output_if<GS_ATOM>
{
public:

  GenericMasterBlockingPort init_port;
  typedef GenericMasterBlockingPort::accessHandle accessHandle;
  typedef GenericMasterBlockingPort::phase phase;

  void notify(GS_ATOM&);

public:

  SC_HAS_PROCESS(GenericMaDevice);

  // Constructor
  GenericMaDevice(sc_module_name name_) : 
    sc_module(name_),
    init_port("generic_unidiectional_init_port")
  {
    cout << "Constructor GenericMaDevice " << name() << endl;
  
    init_port.out_port(*this);

    SC_THREAD(main_action);
  }
    
  void main_action();
  
  GSDataType data;

  unsigned char mem[MEMSIZE];

};

#endif
