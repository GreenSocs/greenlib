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

#ifndef __PCIeRootComplex_h__
#define __PCIeRootComplex_h__

#include <systemc.h>

#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/api/PCIe/PCIeAPI.h"
#include "greenbus/transport/PCIe/PCIeRouter.h"

namespace tlm {
namespace PCIe {



/**
 * This is a PCIe Root Complex prepared to be implemented by the user
 *
 * The user has to implement the method
 * void down_to_router_port_b_transact(PCIeTransactionHandle t);
 * to handle incoming TLPs.
 */
class PCIeRootComplex
 : public PCIeRouter // must be public for API checks,
{

  /// Connector handles incoming blocking and non-blocking transactions from Upstream router port
  /**
   * Wrapper class for slave PEQ output IF
   * Non-blocking is not supported.
   * Blocking forwarded to Root Complex.
   */
  class Connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>,
    //public tlm_b_if<PCIeTransactionHandle>,
    public PCIe_recv_if {
  public:
    Connector(PCIeRootComplex* boss):m_boss(boss) { }
    virtual void notify(PCIeTransactionContainer& tp) {
      SC_REPORT_WARNING("PCIeRouter Downstream connector", "Non-blocking communication is not supported!");
    }
    virtual void b_transact(PCIeTransactionHandle t) {
      m_boss->down_to_router_port_b_transact(t);
    }
  private:
    PCIeRootComplex* m_boss;
  };

public:

  SC_HAS_PROCESS(PCIeRootComplex);

  // Constructor
  PCIeRootComplex(sc_module_name name_) : 
    PCIeRouter(name_),//sc_module(name_),
    down_to_router_port("internal_RootComplex_to_router_port", &down_to_router_port_connector),
    down_to_router_port_connector(this)
  {
    PCIeDEBUG("Constructor PCIeRootComplex " << name());
  
    // bind PEQ output port (not supported notify)
    //down_to_router_port.peq.out_port(down_to_router_port_connector);
    // bind blocking
    //down_to_router_port.bind_b_if(down_to_router_port_connector);
    
    down_to_router_port(upstream_port);
  
    //SC_THREAD(main_action);
  }
    
  //void main_action();

protected:
  
  /// Blocking PV incoming transaction
  void down_to_router_port_b_transact(PCIeTransactionHandle t);

  /// Internal port to the Switch's Upstream Port
  PCIeAPI/*PCIeBidirectionalPort*/ down_to_router_port;
  
protected:

  /// Conector to handle incoming transactions from router's Upstream Port
  Connector down_to_router_port_connector;
  
};


} // end namespace PCIe
} // end namespace tlm


#endif
