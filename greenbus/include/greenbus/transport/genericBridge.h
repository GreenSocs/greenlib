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

#ifndef __genericBridge_h__
#define __genericBridge_h__

#include <systemc.h>

#include "greenbus/core/tlm.h"
#ifndef USE_STATIC_CASTS
#include "greenbus/transport/generic.h"
#endif



/**
 * This is the generic bridge. You can use it two connect two GreenBusses.
 * It owns an initiator and a target port.
 */
template <class TRANSACTION,     /* the transaction */
          class PHASE,           /* the phases that are supported by the protocol */
          class GenericTransactionHandle = boost::shared_ptr<TRANSACTION>,
          class PHASE_P = PHASE,
          class GS_ATOM = unevenpair<GenericTransactionHandle, PHASE_P>,
          class INITPORT = initiator_port<TRANSACTION,GenericRouterAccess,PHASE>,
          class TARGETPORT = target_port<TRANSACTION,GenericRouterAccess,PHASE> >
class GenericBridge 
: public sc_module,
  public tlm_b_if<GenericTransactionHandle>,
  public tlm_slave_if<MAddr>
{
public:
  TARGETPORT target_port;
  INITPORT init_port;


  SC_HAS_PROCESS(GenericBridge);

  GenericBridge(sc_module_name name_) 
    : sc_module(name_),
      target_port("target_port"),
      init_port("init_port"),
      m_setAddress_has_been_called(false)
  {
    GS_TRACE(name(), "I am a generic bridge.");

    // DUST structure analysis
#ifdef USE_DUST
    DUST_BRIDGE("GenericBridge");
#endif

    // bind PV interface
    target_port.bind_b_if(*this);
    
    SC_METHOD( MasterAccessMonitor );
    sensitive << target_port.default_event();
    dont_initialize();

    SC_METHOD( SlaveAccessMonitor );
    sensitive << init_port.default_event();
    dont_initialize();
  }



  void MasterAccessMonitor()
  {
    GS_TRACE(name(), "Forwarding master atom");

    // forward payload to slave
    init_port.out->notify(target_port.get_payload());
  }


  void SlaveAccessMonitor() 
  {
    GS_TRACE(name(), "Forwarding slave atom");

    // forward payload to master
    target_port.out->notify(init_port.get_payload());
  }


  // PV blocking if ///////////////////////////////////////////////////////////

  void b_transact(GenericTransactionHandle t)
  {
    GS_TRACE(name(), "forwarding PV transaction directly to target bus.");
    init_port.b_out->b_transact(t);
  }


  // TLM slave if /////////////////////////////////////////////////////////////

  typedef MAddr ADDRTYPE;


  /**
   * Currently, this bridge only supports the configuration of _one_ 
   * address range. This range is used for forwarding into both directions,
   * i.e. from bus A to bus B and vice versa.
   */
  virtual void setAddress(ADDRTYPE  base, ADDRTYPE  high) 
  {
    if (m_setAddress_has_been_called)
      SC_REPORT_WARNING(name(), "genericBridge does not support multiple address ranges");

    m_setAddress_has_been_called = true;
    m_base = base;
    m_high = high;
  }

  /**
   * TODO: man kˆnnte den Slave eine Addressliste zur¸ckgeben lassen, so dass auch Slaves
   * mit mehreren Addressranges mˆglich werden (wie z.B. diese BRidge..)
   */
  virtual void getAddress(ADDRTYPE& base, ADDRTYPE& high) 
  {
    base = m_base;
    high = m_high;
  }

protected:

  bool m_setAddress_has_been_called;
  ADDRTYPE m_base, m_high;
  
};

#endif
