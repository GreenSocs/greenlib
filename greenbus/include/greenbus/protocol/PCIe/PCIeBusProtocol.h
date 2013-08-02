// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf, Robert Guenzel
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

#ifndef __PCIeBusProtocol_h__
#define __PCIeBusProtocol_h__

#include <systemc.h>
#include "greenbus/transport/genericProtocol_if.h"
#include "greenbus/protocol/PCIe/PCIeRouter_if.h"
#include "greenbus/transport/genericScheduler_if.h"

#include "greenbus/core/tlm.h"
#include "greenbus/protocol/PCIe/PCIe.h"

#include <map>

/**
 *
 */

namespace tlm {
namespace PCIe {

class PCIeBusProtocol
: public GenericProtocol_if, 
  public sc_module
{
public:

  typedef tlm::unevenpair<GenericTransactionHandle,GenericPhase> pair_type;
  //typedef sc_export<payload_event_queue_if<tlm::unevenpair<GenericTransactionHandle,tlm::GenericPhase > > > * sourcePort;
  //typedef sc_export<payload_event_queue_if<tlm::unevenpair<GenericTransactionHandle,tlm::GenericPhase > > > * destinationPort;

  sc_port<PCIeRouter_if> router_port;

  sc_port<GenericScheduler_if> scheduler_port;

  SC_HAS_PROCESS(PCIeBusProtocol);
  PCIeBusProtocol(sc_module_name name, double time, sc_time_unit base);
  PCIeBusProtocol(sc_module_name name, sc_time &t);
  
  ~PCIeBusProtocol(){ 
  }

  // implements GenericProtocol_if
  virtual bool registerMasterAccess(pair_type& transaction);
  // implements GenericProtocol_if
  virtual bool processMasterAccess();
  // implements GenericProtocol_if
  virtual bool registerSlaveAccess(pair_type& transaction);
  // implements GenericProtocol_if
  virtual bool processSlaveAccess();

  // implements GenericProtocol_if
  virtual void assignProcessMasterAccessSensitivity(sc_process_b* pMethod);
  // implements GenericProtocol_if
  virtual void assignProcessSlaveAccessSensitivity(sc_process_b* pMethod);

  virtual void end_of_elaboration();


private:
  void init();
  
};

#include "PCIeBusProtocol.hpp"

} // end namespace PCIe
} // end namespace tlm

#endif
