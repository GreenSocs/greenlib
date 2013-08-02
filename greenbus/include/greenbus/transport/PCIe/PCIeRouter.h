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

#ifndef __PCIeRouter_h__
#define __PCIeRouter_h__


#include <systemc.h>

#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeProtocol_if.h"
#include "greenbus/transport/PCIe/PCIeRouter_if.h"
#include "greenbus/transport/PCIe/PCIeAddressMap.h"

#include "greenbus/transport/PCIe/PCIeRouterConfRegTy1.h"

namespace tlm {
namespace PCIe {

// defines for Interrupt wires (see method calculate_Interrupt_Message(ah))
#define INTA 0
#define INTB 1
#define INTC 2
#define INTD 3

/// PCIe router
/**
 * The PCIeRouter. It owns two ports:
 * One multi-bidirectional downstream port for connecting arbitrary
 * numbers of PCIe bidirectional modules.
 * One bidirectional upstream port to connect to the root complex or 
 * other routers.
 */
class PCIeRouter
: public sc_module,
  public PCIeRouter_if
{
protected:

  // ///////////////////////////////////////////////////// //
  // //// connectors for bindings and                ///// //
  // //// handling of incoming transactions / phases ///// //

  /// Connector handles incoming blocking and non-blocking transactions from Upstream
  /**
   * Wrapper class for slave PEQ output IF
   * Non-blocking is not supported.
   * Blocking forwarded to router.
   */
  class Upstream_connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>,
    public tlm_b_if<PCIeTransactionHandle> {
  public:
    Upstream_connector(PCIeRouter* boss):m_boss(boss) { }
    virtual void notify(PCIeTransactionContainer& tp) {
      SC_REPORT_WARNING("PCIeRouter Upstream connector", "Non-blocking communication is not supported!");
      //m_boss->protocol_port[m_boss->m_protocol_port_index]->registerMasterAccess(tp);
    }
    virtual void b_transact(PCIeTransactionHandle t) {
      m_boss->b_transact_Upstream(t);
    }
  private:
    PCIeRouter* m_boss;
  };
  
  /// Connector handles incoming blocking and non-blocking transactions from Downstream
  /**
   * Wrapper class for slave PEQ output IF
   * Non-blocking is not supported.
   * Blocking forwarded to router.
   */
  class Downstream_connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>,
    public tlm_b_if<PCIeTransactionHandle> {
  public:
    Downstream_connector(PCIeRouter* boss):m_boss(boss) { }
    virtual void notify(PCIeTransactionContainer& tp) {
      SC_REPORT_WARNING("PCIeRouter Downstream connector", "Non-blocking communication is not supported!");
      //m_boss->protocol_port[m_boss->m_protocol_port_index]->registerMasterAccess(tp);
    }
    virtual void b_transact(PCIeTransactionHandle t) {
      m_boss->b_transact_Downstream(t);
    }
  private:
    PCIeRouter* m_boss;
  };
  
  
  /// PCIe dummy connector 
  /**
   * Dummy class which has a bidirectional PCIe initiator port.
   * Can be bound to the PCIe multi port (downstream_port) of the 
   * PCIeRouter to avoid port-not-bound-warning.
   */
  class dummy_pcie_connector
  : public payload_event_queue_output_if<PCIeTransactionContainer>,
    public tlm_b_if<PCIeTransactionHandle> {
  public:
    dummy_pcie_connector ()
    : i_port("dummy_bidirect_port", true)
    {
      i_port.bind_b_if(*this); 
      // bind PEQ output port (not supported notify)
      i_port.peq.out_port(*this);
    }
    virtual void b_transact(PCIeTransactionHandle th) { // tlm_b_if
      SC_REPORT_ERROR("PCIeRouter PCIe dummy connector", "b_transact!");
    }
    virtual void notify (PCIeTransactionContainer& p) { // payload_event_queue_output_if
      SC_REPORT_ERROR("PCIeRouter PCIe dummy connector", "notify!");
    } 
    PCIeBidirectionalPort i_port;
  };

public:

  // ////////////////////////////////////////////// //
  // //////////// Contructor / Methods //////////// //


  SC_HAS_PROCESS(PCIeRouter);
  /// Constructor; bind ports and register SC_METHODs with the kernel.
  PCIeRouter(sc_module_name name_)
    : sc_module(name_),
      downstream_port("downstream_port", true), // bidrectional multiport
      upstream_port("upstream_port", true),  // bidrectional port
      //protocol_port("protocol_port"), // protocol port (for feature use, not PV)
      m_PCIeAddressMap("address_map", getDownstreamPort(), getUpstreamPort()),
      m_upstream_conn(this),
      m_downstream_conn(this),
      m_EOEdone(false),
      m_received_PME_TO_Ack(0),
      m_Interrupt_to_send_Upstream(NO_MESSAGE_CODE),
      m_upstream_bus_no(-1),
      m_dev_no(-1)
  {
    PCIeDEBUG2(name(), "I am a PCIe router.");

    // DUST structure analysis
    #ifdef USE_DUST
    DUST_BUS("PCIeRouter");
    #endif

    // ** future use: this is needed if non-blocking communication is needed
    // DUMMY connect downstream to PEQ output IFs
    downstream_port.peq.out_port(m_downstream_conn);
    // DUMMY connect upstream to PEQ output IFs
    upstream_port.peq.out_port(m_upstream_conn);

    // ** Connect blocking if
    // Connect downstream handling of blocking comm.
    downstream_port.bind_b_if(m_downstream_conn);
    // Connect upstream handling of blocking comm.
    upstream_port.bind_b_if(m_upstream_conn);

    // ** These connects avoid warning about not-bound ports:
    // DUMMY connect downstream port
    this->downstream_port(m_pcie_dummy.i_port);
    
    SC_METHOD( ProcessDeviceAccess );
    p_PDEMethod = sc_get_last_created_process_handle();
    dont_initialize();
    
    /// SC_METHOD sends a PME_TO_Ack message when triggered.
    SC_METHOD( send_Router_PME_TO_Ack );
    sensitive << m_PME_action;
    dont_initialize();
    
    /// SC_METHOD sends a Interrupt Message of type m_Interrupt_to_send_Upstream when triggered.
    //SC_METHOD( send_Interrupt_Message );
    //sensitive << m_Interrupt_action;
    //dont_initialize();
    
  }
  
  /// Destructor deletes the Config. Registers
  ~PCIeRouter() {
    delete upstream_conf_reg; upstream_conf_reg = NULL;
    for (unsigned int i = 0; i<downstream_port.connected_in_ports.size()-1; i++) {
      delete downstream_conf_reg.back();
      downstream_conf_reg.pop_back();
    }
  }
  
  
  /// Protocol handling
  /**
   * Future use, not in PV.
   * This method can be made sensitive to an event in the bus protocol to control
   * the processing of transfers from master to slave.
   */
  void ProcessDeviceAccess()
  {
    cout << "ProcessDeviceAccess" << endl;
    // tell protocol that it's time to process the master accesses
    // this could include arbitrate between accesses that were enqueued earlier with registerMasterAccess...
    //protocol_port[m_protocol_port_index]->processMasterAccess();
  }

  /// Get the Downstream device Port
  /** 
   * Provide access to the Downstream device Port (used by bus protocol).
   */
  virtual PCIeRouterDevicePort* getDownstreamPort(){ return &downstream_port;}

  /// Get the Upstream device Port
  /** 
   * Provide access to the Upstream device Port (used by bus protocol).
   */
  virtual PCIeBidirectionalPort* getUpstreamPort(){ return &upstream_port;}

  /// Add an address range to the Memory Address map.
  /**
   * @param base  Base address
   * @param high  High address
   * @param bus_no     Bus number of the specified device
   * @param device_no  Device (which has to be connected to this router) which should get the address range assigned.
   *
   * Implements PCIeRouter_if to give the API access to the address map setting.
   * Needed for debugging to set (multiple) address ranges of a PCIe Device.
   */
  void add_Memory_address_range_to_device(MAddr base, MAddr high, 
                                          unsigned int bus_no, unsigned int device_no) {
    m_PCIeAddressMap.add_address_range(false, base, high, bus_no, device_no);
  }

  /// Add an address range to the IO Address map.
  /**
   * @param base  Base address
   * @param high  High address
   * @param bus_no     Bus number of the specified device
   * @param device_no  Device (which has to be connected to whis router) which should get the address range assigned.
   *
   * Implements PCIeRouter_if to give the API access to the address map setting.
   * Needed for debugging to set (multiple) address ranges of a PCIe Device.
   */
  void add_IO_address_range_to_device(MAddr base, MAddr high, 
                                      unsigned int bus_no, unsigned int device_no) {
    m_PCIeAddressMap.add_address_range(true, base, high, bus_no, device_no);
  }
  
  /// Registers the specified Memory address range to the device of the overgiven port which has to be connected to the downstream_port of this router.
  /**
   * This method does _not_ use the base_addr and high_addr parameters of the port.
   * This method adds an address ranges of PCIe modules for debugging and may be called
   * multiple times for multiple address ranges!
   */
  void add_Memory_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port) {
    add_address_range(false, port, port.base_addr, port.high_addr);
  }
  /// Registers the specified I/O address range to the device of the overgiven port which has to be connected to the downstream_port of this router.
  /**
   * This method does _not_ use the base_addr and high_addr parameters of the port.
   * This method adds an address ranges of PCIe modules for debugging and may be called
   * multiple times for multiple address ranges!
   */
  void add_IO_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port) {
    add_address_range(true, port, port.base_addr, port.high_addr);
  }

  /// Registers the port's address range to the device of the overgiven port which has to be connected to the downstream_port of this router.
  /**
   * Useful to connect generic devices which are not able to handle the
   * address settings which are sent by the Root Complex.
   *
   * The base_addr and high_addr parameters of the port are read and
   * added to the corresponding downstream port.
   *
   * May also be used to set address ranges of PCIe modules for debugging
   * if the device has not multiple address ranges.
   *
   * The target address map is specified with is_IO_map.
   */
  void add_Memory_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port, MAddr base, MAddr high) {
    add_address_range(false, port, base, high);
  }
  /// @see tlm::PCIe::PCIeRouter::add_Memory_address_range
  void add_IO_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port, MAddr base, MAddr high) {
    add_address_range(true, port, base, high);
  }
  
private:
  /// Help function for add_address_range Memory or IO
  void add_address_range(bool is_IO_map, tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port, MAddr base, MAddr high) {
    // check which connected downstream port this call belongs to
    int port_no = -1;
    // start at port 1 (not 0) to ignore dummys
    for (unsigned int i=1; i<downstream_port.connected_in_ports.size(); i++) {
      // get device_port
      PCIeAddressMap<PCIeRouter>::destinationPCIePort destination = downstream_port.connected_in_ports[i];
      if (&port == dynamic_cast<sc_object*> (destination->get_parent())) {
        port_no = i;
        PCIeDEBUG("Address range belongs to the device with bus no. "<<m_PCIeAddressMap.decode_port(port_no)<<" (=port "<<i<<")");
        break;
      }
    }
    if (port_no >= 0) {
      m_PCIeAddressMap.add_address_range(is_IO_map, base, high, m_PCIeAddressMap.decode_port(port_no));
    }
    else {
      SC_REPORT_ERROR(name(), "add_address_range called with a port which is not connected to this switch! Call this method only after connecting the device to the switch and call it on the correct switch.");
    }
  }

protected:

  // ////////////////////////////////////////////////////////////// //
  // /////////// Handling of blocking transactions //////////////// //

  /// Process blocking PCIe transactions from Upstream Port
  /**
   * - enable lock if needed (disable in handle_Upstream_transaction())
   * if check rules are enabled:
   *  - check if transaction should be sent backwards to incoming port
   */
  void b_transact_Upstream(PCIeTransactionHandle t) {
  
#ifdef USE_STATIC_CASTS
    PCIeRouterAccessHandle ah = _getRouterAccessHandle(t);//boost::static_pointer_cast<PCIeRouterAccess>(t);
#else
    PCIeRouterAccessHandle ah = boost::static_pointer_cast<PCIeRouterAccess>(t);
#endif

    // perform router action on transactions from Upstream
    bool forward = handle_Upstream_transaction(ah);
    
    if (forward) {
      PCIeDEBUG2(name(), "Forwarding PV TLP");// from sender id=0x%x.", (unsigned int)ah->getMID());

      // switch addressing method
      RouterPort outPort;
      
      // *** ID based routing
      if (ah->getMIsIDbasedRouting()) {
        PCIeDEBUG2(name(), "ID based routing");
        outPort = m_PCIeAddressMap.decode_ID_based(ah->getMBusNo(), ah->getMDevNo());
        if (outPort.outgoing_port == OutgoingPortError) {
          PCIeDEBUG("Decoding failed, Unsupported Request.");
          ah->init_Unsupported_Request();
        } else {
          if (outPort.outgoing_port == PCIeUpstreamPort) {
            SC_REPORT_WARNING(name(), "The transaction's outgoing port is equal its incoming port (Upstream)! Unsupported Request."); 
            ah->init_Unsupported_Request();
          }
          else
          {
            if (check_locked_rule(outPort, ah, false))
              send_transact(outPort, t);
          }
        }
      } // end ID based routing
      
      // *** implicit routing
      else if (ah->getMIsImplicitRouting()) {
        PCIeDEBUG("Implicit routing");
        outPort = m_PCIeAddressMap.decode_implicit(ah->getMMessageType());
#ifdef CHECK_RULES_ENABLES
        if (outPort.outgoing_port == PCIeUpstreamPort) {
          SC_REPORT_WARNING(name(), "The transaction's outgoing port is equal its incoming port (Upstream)! Unsupported Request."); 
          ah->init_Unsupported_Request();
        }
#endif
        if (/*outPort.outgoing_port == PCIeUpstreamPort || */
            outPort.outgoing_port == PCIeDownstreamPort) {
          if (check_locked_rule(outPort, ah, false))
            send_transact(outPort, t);
        }
        else if (outPort.outgoing_port == PCIeDownstreamBroadcast) {
          // Broadcast to all Downstream Ports
          outPort.outgoing_port = PCIeDownstreamPort;
          for (unsigned int i=1; i<downstream_port.connected_in_ports.size(); i++) {
            outPort.connected_port = i;
            // warning: transaction is _not_ copied!
            if (check_locked_rule(outPort, ah, false))
              send_transact(outPort, t);
          }
        }
      } // end impicit routing
      
      // *** address based routing
      else {
        PCIeDEBUG2(name(), "Address based routing");
        if (ah->getMPCIeCmd() == IORead || ah->getMPCIeCmd() == IOWrite)
          outPort = m_PCIeAddressMap.decode_IO_Address(ah->getMAddr());
        else
          outPort = m_PCIeAddressMap.decode_Memory_Address(ah->getMAddr());
#ifdef CHECK_RULES_ENABLES
        if (outPort.outgoing_port == PCIeUpstreamPort) {
          SC_REPORT_WARNING(name(), "The transaction's outgoing port is equal its incoming port (Upstream)! Unsupported Request."); 
          ah->init_Unsupported_Request();
        }
        else
#endif
        {
#ifdef CHECK_RULES_ENABLES
          // LOCKED: check if already locked with other path
          if (ah->get_TLP_type() == MemReadLocked && m_upstream_port_locked) {
            if (!m_downstream_port_locked[outPort.connected_port])
              SC_REPORT_ERROR(name(), "Lock failed: There is already another locked path. Connot lock two different paths.");
          }
#endif
          if (check_locked_rule(outPort, ah, false))
            send_transact(outPort, t);

          // LOCKED: Lock if MemReadLocked and successful completion (p.376)
          if (ah->get_TLP_type() == MemReadLocked
              && ah->get_Completion_Status() == SuccessfulCompletion) {
            PCIeDEBUG("Lock");
            m_upstream_port_locked = true;
            m_downstream_port_locked[outPort.connected_port] = true;
          }
        }
      } // end address based routing

    } // end forward
    else
      PCIeDEBUG("Not forwarded");

  }
  
  /// Process blocking PCIe transactions from Downstream Port
  /**
   * 
   */
  void b_transact_Downstream(PCIeTransactionHandle t) {

#ifdef USE_STATIC_CASTS
    PCIeRouterAccessHandle ah = _getRouterAccessHandle(t);
#else
    PCIeRouterAccessHandle ah = boost::static_pointer_cast<PCIeRouterAccess>(t);
#endif

    // perform router action on transactions from Upstream
    bool forward = handle_Downstream_transaction(ah);

    if (forward) {
      PCIeDEBUG2(name(), "Forwarding PV TLP");// from sender id=0x%x.", (unsigned int)ah->getMID());

      // switch addressing method
      RouterPort outPort;
      
      // *** ID based routing
      if (ah->getMIsIDbasedRouting()) {
        PCIeDEBUG2(name(), "ID based routing");
        outPort = m_PCIeAddressMap.decode_ID_based(ah->getMBusNo(), ah->getMDevNo());
        if (outPort.outgoing_port == OutgoingPortError) {
          PCIeDEBUG("Decoding failed, Unsupported Request.");
          ah->init_Unsupported_Request();
        } else {
          if (check_locked_rule(outPort, ah, true))
            send_transact(outPort, t);
        }
      }
      
      // *** implicit routing
      else if (ah->getMIsImplicitRouting()) {
        PCIeDEBUG("Implicit routing");
        outPort = m_PCIeAddressMap.decode_implicit(ah->getMMessageType());
#ifdef CHECK_RULES_ENABLES
        if (outPort.outgoing_port == PCIeDownstreamBroadcast) {
          PCIeDEBUG("Warning: Incoming broadcast at a Downstream Port: Broadcast only may be sent by Root Complex.");
        } else if (outPort.outgoing_port == PCIeDownstreamPort) {
          PCIeDEBUG("Warning: Incoming TLP at a Downstream Port should be routed to Downstream Port.");
        }
#endif
        if (outPort.outgoing_port == PCIeUpstreamPort/* || outPort.outgoing_port == PCIeDownstreamPort*/) {
          if (check_locked_rule(outPort, ah, true))
            send_transact(outPort, t);
        }
        else if (outPort.outgoing_port != OutgoingPortError) {
          cout << "Port="<<outPort.to_string()<< endl;
          SC_REPORT_ERROR(name(), "Incoming at Downstream Port: only may send Upstream (correct?).");
        }
      }
      
      // *** address based routing
      else {
        PCIeDEBUG2(name(), "Address based routing");
        if (ah->getMPCIeCmd() == IORead || ah->getMPCIeCmd() == IOWrite)
          outPort = m_PCIeAddressMap.decode_IO_Address(ah->getMAddr());
        else
          outPort = m_PCIeAddressMap.decode_Memory_Address(ah->getMAddr());
        if (check_locked_rule(outPort, ah, true))
          send_transact(outPort, t);
      }

    } // end forward
  
  }

protected:

  /// Perform router action on transactions from Upstream
  /**
   * - Handle Type 1/0 Configuration Requests
   * - Handle LOCK TLPs, UNLOCK Messages
   * - Handle PME_TO_Ack TLPs
   * - ...
   *
   * @return If the transaction sould be forwarded (e.g. false when answered with Unsupported Reequest)
   */
  bool handle_Upstream_transaction(PCIeRouterAccessHandle &ah) {
    PCIeDEBUG("Handle TLP from Upstream Port");
  
    CHECK_RULES(if (m_received_PME_TO_Ack > 0) SC_REPORT_WARNING(name(), "Incoming transaction resets mechanism for PME_TO_Ack."););
    m_received_PME_TO_Ack = 0; // reset mechanism for PME_TO_Ack
    
    unsigned int cmd = ah->get_TLP_type();
    switch(cmd) {
      
      case MemRead: {
        // LOCKED:
        CHECK_RULES(if (m_upstream_port_locked && ah->getMTrafficClass() == 0/*TODO: VC0, not TC0!!*/) \
                    SC_REPORT_ERROR(name(), "Switch is locked! Use MemReadLocked instead of MemRead!"););
      } break;
      
      case CnfgReadTy0: // p.422
      // NO BREAK!!
      case CnfgWriteTy0: {
        PCIeDEBUG("Handle Configuration Request");
        // TODO: Check if the Request addresses a valid local Configuration space
        // if yes, handle it: process
        process_RouterConfigRequest(ah);
        // if not, Unsupported Request
        return false;
      } break;

      case CnfgWriteTy1: // p.423
      // NO BREAK!!
      case CnfgReadTy1: {
        // Process if target is internal this switch
        if ( m_PCIeAddressMap.is_internal_switch_bus(ah->getMBusNo()) )  {
          // Don't change 1->0 to be able to differentiate between ConfigRequ for Upstream and Downstream Ports
          process_RouterConfigRequest(ah);
          return false;
        }
        // Change Type 1 -> 0 if target is connected to this switch
        else if ( m_PCIeAddressMap.is_served_bus         (ah->getMBusNo()) )  {
          PCIeDEBUG("Handle: Change Configuration Request Type 1->0");
          if (cmd == CnfgWriteTy1) ah->setMPCIeCmd(CnfgWriteTy0);
          else ah->setMPCIeCmd(CnfgReadTy0);
          // Check if addressed device number is valid
          if (ah->getMDevNo() != 0) {
            // At Switch's Downstream Ports only one device is allowed (dev no=0)
            ah->init_Unsupported_Request();
            return false;
          }
        }
        return true;
      } break;
      
      #ifdef CHECK_RULES_ENABLED
      // LOCKED:
      case MemReadLocked: {
        if (ah->getMTrafficClass() != 0) 
          SC_REPORT_ERROR(name(), "Locked transactions only allowed in Traffic Class 0."););
      } break;
      #endif
      
      case Msg: {
        // LOCKED: Unlock
        if (ah->getMMessageCode() == Unlock) {
          PCIeDEBUG("Unlock Message: unlock router");
          if (ah->getMTrafficClass() != 0) 
            SC_REPORT_ERROR(name(), "Unlock message only allowed in Traffic Class 0.");
          unlock_router();
        }
        #ifdef CHECK_RULES_ENABLED
        // INTx Interrupts, see p. 70
        if (ah->getMMessageCode() == Assert_INTA || ah->getMMessageCode() == Assert_INTB ||
            ah->getMMessageCode() == Assert_INTC || ah->getMMessageCode() == Assert_INTD ||
            ah->getMMessageCode() == Deassert_INTA || ah->getMMessageCode() == Deassert_INTB ||
            ah->getMMessageCode() == Deassert_INTC || ah->getMMessageCode() == Deassert_INTD) {
          PCIeDEBUG("Incoming Interruput at Upstream Port: not allowed! Malformed TLP.");
          SC_REPORT_ERROR(name(), "Incoming Interruput at Upstream Port: not allowed! Malformed TLP.");
          // TODO: handle Malformed TLP instead of SC_REPORT_ERROR!
        }
        #endif
      } break;

    }

    return true;
  }

  /// Checks whether the locked rule is broken
  /**
   * @param   p   Target port of the TLP
   * @param   ah  TLP handle
   * @param   from_downstream  If this TLP came from a Downstram Port (in that case it must not be forwarded to the locked Downstream Port)
   * @return  true if transaction needs NOT to be blocked, false if transaction has to be blocked because it hurts the blocked path.
   */
  inline bool check_locked_rule(RouterPort p, PCIeRouterAccessHandle &ah, bool from_downstream) {
    // LOCKED:
    if (m_upstream_port_locked) { // if something is locked
      if (ah->getMPCIeCmd() == Msg  &&  ah->getMMessageCode() == Unlock) return true;
      if (ah->getMTrafficClass() == 0 /*TODO: VC0, not TC0!!*/) {
        // if transaction for downstream port
        if (p.outgoing_port == PCIeDownstreamPort) {
          if (from_downstream && m_downstream_port_locked[p.connected_port]) {
            PCIeDEBUG("TLP will be blocked because Switch is locked.");
            return false;
          }
          else if (!from_downstream && !m_downstream_port_locked[p.connected_port]) {
            PCIeDEBUG("TLP will be blocked because TLPs from the locked Upstream Port only may be forwarded to the locked Downstream Port.");
            return false;
          }
        }
        else if (p.outgoing_port == PCIeUpstreamPort) {
          // No transaction (request) may pass down->up, only Completions!
          PCIeDEBUG("Upstream TLP will be blocked because Switch is locked.");
          return false;
        }
      }
    }
    // no rule broken: needs not to be blocked
    return true;
  }

  /// Perform router action on transactions from Downstream
  /**
   * - Handle LOCK TLPs, UNLOCK Messages
   * - Handle PME_TO_Ack TLPs
   * - ...
   *
   * @return If the transaction sould be forwarded (e.g. false when answered with Unsupported Reequest)
   */
  bool handle_Downstream_transaction(PCIeRouterAccessHandle &ah) {
    PCIeDEBUG("Handle TLP from Downstream Port");

    unsigned int cmd = ah->get_TLP_type();
    switch(cmd) {
      // Configuration Requests Downstream->Upstream not supported
      case CnfgReadTy0:
      case CnfgReadTy1:
      case CnfgWriteTy0:
      case CnfgWriteTy1: {
        PCIeDEBUG("Configuration Requests Downstream->Upstream not supported");
        ah->init_Unsupported_Request();
        return false;
      } break;
      case Msg: {
        // Handle PME_TO_Ack Messages: 
        //  collect all of the incoming and send upstream when all arrived
        if (ah->getMMessageCode() == PME_TO_Ack) {
          // TODO: If this Downstream Port was the point of origin of the
          //       PME_Turn_Off message don't forward!
          m_received_PME_TO_Ack++; // increase number of received Acks
          // check if all ACKs received
          if (m_received_PME_TO_Ack == downstream_port.connected_in_ports.size()-1) {
            // create an PME_TO_Ack to send Upstream
            PCIeDEBUG("Received PME_TO_Ack on all Downstream Ports: send PME_TO_Ack Upstream.");
            m_PME_action.notify(SC_ZERO_TIME);
          }
          else
            PCIeDEBUG("Increase PME_TO_Ack counter (="<<m_received_PME_TO_Ack<<").");
        }
        // INTx Interrupts
        if (ah->getMMessageCode() == Assert_INTA || ah->getMMessageCode() == Assert_INTB ||
            ah->getMMessageCode() == Assert_INTC || ah->getMMessageCode() == Assert_INTD ||
            ah->getMMessageCode() == Deassert_INTA || ah->getMMessageCode() == Deassert_INTB ||
            ah->getMMessageCode() == Deassert_INTC || ah->getMMessageCode() == Deassert_INTD) {
          // see p. 71
          if (ah->getMTrafficClass() != 0) {
            PCIeDEBUG("Interrupt Messages only allowed in Traffic Class TC0 (not in "<<ah->getMTrafficClass()<<")!");
            SC_REPORT_ERROR(name(), "Interrupt Messages only allowed in Traffic Class TC0!");
            // TODO: reported Error instead of SC_REPORT_ERROR
          }
          // error if wrong Message routing type
          CHECK_RULES(if (ah->getMMessageType() != LocalTerminateAtReceiver) SC_REPORT_ERROR(name(), "Wrong Message routing type for Interrupt Message: not LocalTerminateAtReceiver."););
          // calculate and send new Interrupt Message Upstream
          CHECK_RULES(if (m_Interrupt_to_send_Upstream != NO_MESSAGE_CODE) SC_REPORT_ERROR(name(), "Missed sending Interrupt before getting new one!"););
          PCIeDEBUG("Got Interrupt "<< convert_MessageCode_to_string( (MessageCode)(unsigned int)ah->getMMessageCode() ) );
          m_Interrupt_to_send_Upstream = calculate_Interrupt_Message(ah);
          send_Interrupt_Message(); // instead of: m_Interrupt_action.notify(SC_ZERO_TIME);
          return false; // don't forward
        }
      } break;
      #ifdef CHECK_RULES_ENABLES
      // LOCKED:
      case MemReadLocked: {
        PCIeDEBUG("Memory Read Locked Downstream->Upstream not supported: only Root Complex may send this TLP type.");
        SC_REPORT_ERROR(name(), "Memory Read Locked Downstream->Upstream not supported: only Root Complex may send this TLP type.");
        return false;
      } break;
      #endif      
    }

    return true;
  }

  /// Sends a transaction to a port, used by for b_transact
  void send_transact(RouterPort &outPort, PCIeTransactionHandle &t) {
    switch (outPort.outgoing_port) {
      case PCIeDownstreamPort:
        {
          PCIeDEBUG2(name(), "Outgoing multiport: PCIe downstream_port, port %u", outPort.connected_port);
          CHECK_RULES(if (outPort.connected_port == 0) SC_REPORT_WARNING(name(), "Outgoing port within multi port is '0'."););
          (*(downstream_port.connected_b_in_ports[outPort.connected_port]))->b_transact(t);
        }
        break;
      case PCIeUpstreamPort:
        {
          PCIeDEBUG2(name(), "Outgoing port: PCIe upstream_port");
          CHECK_RULES(if (outPort.connected_port != 0) SC_REPORT_WARNING(name(), "Outgoing Upstream Port is no muulti port but connected_port is set."););
          upstream_port.Transact(t);
        }
        break;
      default:
        SC_REPORT_ERROR(name(), "Routing error: No outgoing port!");
    }
  }
  
  // LOCKED:
  /// Unlocks all Ports
  /**
   * Used for initialization and Unlock messages
   */
  inline void unlock_router() {
    m_upstream_port_locked = false;
    m_downstream_port_locked.resize(downstream_port.connected_in_ports.size());
    for (unsigned int i=0; i<m_downstream_port_locked.size(); i++) m_downstream_port_locked[i] = false;
  }

  /// Calculates which Interrupt message has to be sent as result of the incoming one
  /** 
   * Simulates two PCI-to-PCI Bridges (see p. 73).
   *
   * @param ah  Access Handle of the incoming Interrupt Message
   * @return    Interrupt (De)Assert Message to be sent Upstream
   *
   * Time critical but not important!!!
   */
  inline MessageCode calculate_Interrupt_Message(PCIeRouterAccessHandle ah) {
    unsigned int intx_in_downstream;
    switch ((gs_uint32)ah->getMMessageCode()) {
      case Assert_INTA:
      case Deassert_INTA:
        intx_in_downstream = INTA;
      break;
      case Assert_INTB:
      case Deassert_INTB:
        intx_in_downstream = INTB;
      break;
      case Assert_INTC:
      case Deassert_INTC:
        intx_in_downstream = INTC;
      break;
      case Assert_INTD:
      case Deassert_INTD:
        intx_in_downstream = INTD;
      break;
      default:
        sc_assert(false);
    }
    // calculate mapping
    unsigned int intx_out;

    // *** first PCI-to-PCI Bridge (Downstream Bridge) ***
    //  no change because dev0 -> devX
    
    // *** second PCI-to-PCI Bridge (Upstream Bridge) ***
    unsigned int devNo;
    RouterPort p = m_PCIeAddressMap.decode_ID_based(ah->getMRequBusNo(), ah->getMRequDevNo());
    if (p.outgoing_port != PCIeDownstreamPort) {
      PCIeDEBUG("Failed to decode source of Interrupt Message: source bus "<< dec <<(int)ah->getMRequBusNo()<<", dev "<<(int)ah->getMRequDevNo());
      SC_REPORT_ERROR(name(), "Failed to decode source of Interrupt Message");
    }
    devNo = p.connected_port-1; // device number on the interal bus
    intx_out = (intx_in_downstream + (devNo%4) )%4;

    // if Assert
    if (ah->getMMessageCode() == Assert_INTA || ah->getMMessageCode() == Assert_INTB ||
        ah->getMMessageCode() == Assert_INTC || ah->getMMessageCode() == Assert_INTD ) {
      switch (intx_out) {
        case INTA:  return Assert_INTA;  break;
        case INTB:  return Assert_INTB;  break;
        case INTC:  return Assert_INTC;  break;
        case INTD:  return Assert_INTD;  break;
        default:    sc_assert(false);
      }
    } 
    // if Deassert
    else {
      switch (intx_out) {
        case INTA:  return Deassert_INTA;  break;
        case INTB:  return Deassert_INTB;  break;
        case INTC:  return Deassert_INTC;  break;
        case INTD:  return Deassert_INTD;  break;
        default:    sc_assert(false);
      }
    }
    return NO_MESSAGE_CODE;
  }
  
public:

  // /////////////////////////////////////////// //
  // /////////// other methods //////////////// //

  /// Callback
  /**
   * End of elab simulation kernel callback.
   * 
   * - Initialize the ports unlocked
   */
  virtual void end_of_elaboration()
  {
    if (m_EOEdone) return;
    m_EOEdone=true;
    PCIeDEBUG2(name(), "end_of_elaboration called.");
    
    // LOCKED: init locked = false
    unlock_router();
    
    /*if (protocol_port.size()>1) { // user protocol bound
      PCIeDEBUG2(name(), "User protocol detected.");
      m_protocol_port_index = 1;
    }
    else {
      PCIeDEBUG2(name(), "No user protocol detected. Using dummy protocol.");
    }*/
    
    // tell the protocol to assign sensitivies to the "process" methods
    //protocol_port[m_protocol_port_index]->assignProcessMasterAccessSensitivity(p_PMAMethod);
    //protocol_port[m_protocol_port_index]->assignProcessSlaveAccessSensitivity(p_PSAMethod);

    // create Configuration Registers for the logical Upstream PCI-to-PCI Bridge
    upstream_conf_reg = new PCIeRouterConfRegTy1("Downstream_PCI-to-PCI_Bridge_ConfReg");

    // create Configuration Registers for each logical Downstream PCI-to-PCI Bridge
    PCIeRouterConfRegTy1 *confreg;
    std::ostringstream ss;
    for (unsigned int i = 0; i<downstream_port.connected_in_ports.size()-1; i++) {
      ss.str("Downstream_PCI-to-PCI_Bridge_ConfReg_");
      ss << i;
      confreg = new PCIeRouterConfRegTy1(ss.str().c_str());
      downstream_conf_reg.push_back(confreg);
    }

    // create the id map 
    m_PCIeAddressMap.generate_ID_map();
    // create the address map (if there are manually set addresses)
    m_PCIeAddressMap.generate_address_map();
    // show map
#ifdef PCIeDEBUG_ON
    m_PCIeAddressMap.dump_map();
#endif

    cout << endl;
  }

  /// Reset method for fundamental reset
  /**
   * @todo test reset
   */
  void reset() {
    PCIeDEBUG("*** RESET PCIeRouter ***");
    m_received_PME_TO_Ack = 0;
    m_PCIeAddressMap.reset();
    // TODO: more resets?
    // TODO: reset conf. registers?
  }
  
  /// SC_METHOD sends a PME_TO_Ack Message
  /**
   * This SC_METHOD is triggered when a PME_Turn_Off message was received.
   * The method is needed to send the Ack _after_ the Turn Off message
   * returned.
   */
  void send_Router_PME_TO_Ack() {
    PCIeDEBUG("Send PME_TO_Ack");
    PCIeMasterAccessHandle ah = upstream_port.create_transaction();
    CHECK_RULES(if (m_upstream_bus_no<0 || m_dev_no<0) SC_REPORT_WARNING(name(), "This device has not yet received a bus and device number! Sending forbidden without Requester ID."); );
    ah->setMRequBusNo(m_upstream_bus_no);
    ah->setMRequDevNo(m_dev_no);
    ah->init_Message(PME_TO_Ack);
    //PCIeTransactionHandle th = _getPCIeTransactionHandle(ah);
    upstream_port.Transact(ah);
    PCIeDEBUG("Shutdown router "<< name());
    reset();
  }

  /// Sends an Interrupt Message
  void send_Interrupt_Message() {
    PCIeDEBUG("Send Interrupt Message "<< convert_MessageCode_to_string(m_Interrupt_to_send_Upstream) );
    PCIeMasterAccessHandle ah = upstream_port.create_transaction();
    CHECK_RULES(if (m_upstream_bus_no<0 || m_dev_no<0) SC_REPORT_WARNING(name(), "This device has not yet received a bus and device number! Sending forbidden without Requester ID."); );
    ah->setMRequBusNo(m_upstream_bus_no);
    ah->setMRequDevNo(m_dev_no);
    ah->init_Message(m_Interrupt_to_send_Upstream);
    upstream_port.Transact(ah);
    m_Interrupt_to_send_Upstream = NO_MESSAGE_CODE; // reset
  }
  /* OLD / FOR FUTURE USE: If this is a sc_method
   * This method is triggered when a Interrupt has to be forwarded.
   * The method is needed to send the Interrupt _after_ the originating
   * Interrupt Message returned.
   *
   * TODO: Implement a queue instead of m_Interrupt_to_send_Upstream to
   *       allow Interruts arriving simultaneous while sending other Interrupt
   */
    
protected:

  /// Processes Configuration Requests that are directed to PCI-to-PCI Bridges of this router.
  void process_RouterConfigRequest(PCIeRouterAccessHandle &ah) {
    PCIeDEBUG("Process Conf. Request directed to this router:");
    // handle Configuration Requests
    unsigned int cmd = ah->get_TLP_type();
    switch(cmd) {
      // TLP addresses to Upstream Port PCI-to-PCI Bridge
      case CnfgReadTy0:
      case CnfgWriteTy0: {
        PCIeDEBUG("Configuration Request was addresses to the Upstream PCI-to-PCI Bridge (on Bus no. "<<(int)ah->getMBusNo()<<")");
        // read out device and bus number and set variables m_dev_no and m_upstream_bus_no
        if (m_dev_no < 0) {
          m_upstream_bus_no = ah->getMBusNo();
          m_dev_no = ah->getMDevNo();
          PCIeDEBUG("Updated my Bus (="<< m_upstream_bus_no<<") and Device Number (="<<m_dev_no<<")");
        }
      }
      break;
      // TLP addresses to one Downstream Port PCI-to-PCI Bridge (handle like Ty0!!)
      case CnfgReadTy1:
      case CnfgWriteTy1: {
        PCIeDEBUG("Configuration Request was addresses to the Downstream PCI-to-PCI Bridge (on Bus no. "<<(int)ah->getMBusNo()<<") with the Device no. "<< (int)ah->getMDevNo());
      }
      break;
    }
  }

public:

  // ////////////////////////////////////////////// //
  // //////////// Input/Output ports ////////////// //
  
  /// Bidirectional PCIe Downstream Multiport for PCIe peripherals
  PCIeRouterDevicePort downstream_port;

  /// Bidirectional PCIe Upstream Port
  PCIeBidirectionalPort upstream_port;

  /// port to the protocol (for feature use, not PV)
  //sc_port<PCIeProtocol_if,0> protocol_port;


  // ////////////////////////////////////////////// //
  // //////////// Other public members //////////// //

  PCIeAddressMap<PCIeRouter> m_PCIeAddressMap;

protected:

  /**
   * Process handle to SC_METHOD processDeviceAccess 
   * that will be triggered by the bus protocol class.
   */
  sc_process_b* p_PDEMethod;

  Upstream_connector   m_upstream_conn;
  Downstream_connector m_downstream_conn;

  dummy_pcie_connector     m_pcie_dummy;

  bool m_EOEdone; // if end of elaboration was done
  //unsigned m_protocol_port_index;
  
  /// Locked indicator for the Downstream Ports
  std::vector<bool> m_downstream_port_locked;

  /// Indicator for the Downstream Ports which stores how many PME_TO_Ack message were received
  unsigned int m_received_PME_TO_Ack;

  /// Locked indicator for the Switch
  bool m_upstream_port_locked;

  /// Event for send_PME_TO_Ack sc_method. Public to be notified by an owning router.
  sc_event m_PME_action;

  /// Configuration Register (Header Type 1) for the logical Upstream PCI-to-PCI Bridge
  PCIeRouterConfRegTy1 *upstream_conf_reg;

  /// Configuration Register map (Header Type 1) for the logical Downstream PCI-to-PCI Bridges
  std::vector<PCIeRouterConfRegTy1*> downstream_conf_reg;

  /// Type of Interrupt that should be sent by the sc_method send_Interrupt_Message
  MessageCode m_Interrupt_to_send_Upstream;
  
  /// Event for send_Interrupt_Message sc_method.
  sc_event m_Interrupt_action;
  
  /// The bus number this router is connected to at the Upstream Port.
  int m_upstream_bus_no;
  /// The device number this router's Upstream Port has at the Upstream Router
  int m_dev_no;
  
};

} // end namespace PCIe
} // end namespace tlm


#endif

