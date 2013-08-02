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

#ifndef __shipAPI_h__
#define __shipAPI_h__

   /// Use this define to enable various debug messages 
   //#define VERBOSE

#include <systemc.h>
#ifndef USE_STATIC_CASTS
# include "greenbus/transport/generic.h"
#endif
#include "greenbus/utils/gb_config.h"

#include "greenbus/api/ship/ship_datatypes.h"
#include "greenbus/api/ship/ship_serializable_if.h"
#include "greenbus/api/ship/ship_masterport.h"
#include "greenbus/api/ship/ship_slaveport.h"

namespace tlm {  

template <class T>
class shipMasterAPIForward: public basic_port_forwarder<shipMasterAPI<T>, shipSlaveAPI<T>, GenericTransaction, GenericPhase> {
public:
  shipMasterAPIForward(sc_module_name name): basic_port_forwarder<shipMasterAPI<T>, shipSlaveAPI<T>, GenericTransaction, GenericPhase>(name){}
};

template <class T>
class shipSlaveAPIForward: public basic_port_forwarder<shipSlaveAPI<T>, shipMasterAPI<T>, GenericTransaction, GenericPhase> {
public:
  shipSlaveAPIForward(sc_module_name name): basic_port_forwarder<shipSlaveAPI<T>, shipMasterAPI<T>, GenericTransaction, GenericPhase>(name){}
};

} // namespace tlm

#endif
