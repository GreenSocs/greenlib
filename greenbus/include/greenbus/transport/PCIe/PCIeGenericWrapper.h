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

#ifndef __PCIeGenericWrapper_h__
#define __PCIeGenericWrapper_h__

#include "greenbus/transport/PCIe/PCIe.h"


namespace tlm {
namespace PCIe {


/// Wrapper PCIe bidirectional Port to Generic Initiator- and Slaveport and the other way round
/**
 * This wrapper can be used in both directions: 
 * - Connect a PCIe Device to a Generic Router mapping the bidirectional 
 *   PCIe port to the initiator and target ports.
 * - Connect a Generic Device to a PCIe Router mapping the initiator and
 *   target ports to one bidirectional PCIe port. For this direction the
 *   wrapper is not essential. A Generic Device may be connected to a PCIe
 *   Router via two downstream ports - if there are enough ports (max. 32)
 *   and if there are no other disturbing side effects.
 *
 * The ports used by the Wrapper are tlm ports templated with PCIe datatypes.
 */
class PCIe2GenericPortWrapper 
 : public sc_module
{
  typedef tlm_port<tlm_b_if<GenericTransactionHandle>, GS_ATOM> port_type;
  
protected:

  class Init_Connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>, 
    public tlm_b_if<PCIeTransactionHandle> {
  public:
    Init_Connector(PCIe2GenericPortWrapper* boss):m_boss(boss) { }
    virtual void notify(PCIeTransactionContainer& tp) {
      m_boss->pcie_port.out->notify(tp);
    }
    virtual void b_transact(PCIeTransactionHandle t) {
      m_boss->pcie_port.b_out->b_transact(t);
    }
  private:
    PCIe2GenericPortWrapper* m_boss;
  };

  class Target_Connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>,
    public tlm_b_if<PCIeTransactionHandle> {
  public:
    Target_Connector(PCIe2GenericPortWrapper* boss):m_boss(boss) { }
    virtual void notify(PCIeTransactionContainer& tp) {
      m_boss->pcie_port.out->notify(tp);
    }
    virtual void b_transact(PCIeTransactionHandle t) {
      m_boss->pcie_port.b_out->b_transact(t);
    }
  private:
    PCIe2GenericPortWrapper* m_boss;
  };

  class PCIe_Connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>,
    public tlm_b_if<PCIeTransactionHandle> {
  public:
    PCIe_Connector(PCIe2GenericPortWrapper* boss):m_boss(boss) { }
    virtual void notify(PCIeTransactionContainer& tp) {
      m_boss->init_port.out->notify(tp);
    }
    virtual void b_transact(PCIeTransactionHandle t) {
      m_boss->init_port.b_out->b_transact(t);
    }
  private:
    PCIe2GenericPortWrapper* m_boss;
  };
  
  typedef GenericInitiatorPort INITPORT;
  typedef GenericSlavePort SLAVEPORT;

public:

  PCIe2GenericPortWrapper(sc_module_name name_)
   : sc_module(name_),
     init_port("Wrapper_Init_Port"),
     target_port("Wrapper_Target_Port"),
     pcie_port("Wrapper_PCIe_Port"),
     m_tConn(this),
     m_iConn(this),
     m_pConn(this)
  {
    // connect generic ports to connectors
    target_port.peq.out_port(m_tConn);
    target_port.bind_b_if(m_tConn);
    init_port.peq.out_port(m_iConn);
    init_port.bind_b_if(m_iConn);

    // connect PCIe port to connector
    pcie_port.peq.out_port(m_pConn);
    pcie_port.bind_b_if(m_pConn);
  }

  /// Set addresses before end of elaboration to have them set when the address map is created
  void before_end_of_elaboration() {
    if (target_port.base_addr.value != 0 || target_port.high_addr.value != 0) {
      PCIeDEBUG("Target port address will not be set to the PCIe port's address range.");
    } else {
      
      sc_interface *inf         = (pcie_port.out.get_interface());
      sc_object    *destination = dynamic_cast<sc_object*>(inf);
      sc_object *o = dynamic_cast<sc_object*>(destination->get_parent());
      port_type* r =dynamic_cast<port_type*>(o);
      if (r) {
        target_port.base_addr = r->base_addr;
        target_port.high_addr = r->high_addr;
        PCIeDEBUG2(name(), "Set address range to base='0x%llx', high='0x%llx'", 
          (long long unsigned int) r->base_addr.value, (long long unsigned int) r->high_addr.value);
      }
      else {
        SC_REPORT_ERROR(name(), "Wrapper is not connected to a PCIe bidirectional port with base and high address");
      }

    }
  }

public:
  /// generic init port
  port_type  init_port;
  
  /// generic target port
  port_type  target_port;
  
  /// PCIe bidirectional port
  port_type  pcie_port;

protected:
  Target_Connector m_tConn;
  Init_Connector   m_iConn;
  PCIe_Connector   m_pConn;

};


} // end namespace PCIe
} // end namespace tlm


#endif
