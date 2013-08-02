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

#ifndef __PCIeAPI_h__
#define __PCIeAPI_h__

// define PRECHECK_ENABLE if the API should check some of the rules
#define PRECHECK_ENABLE

#include "greenbus/transport/PCIe/PCIe.h"

#include "greenbus/transport/PCIe/PCIeRouter_if.h"

#include "greenbus/api/PCIe/ConfReg.h"


namespace tlm {
namespace PCIe {

typedef tlm_b_if<PCIeTransactionHandle> PCIe_recv_if;

/// PCIe Endpoint API (unified for all PCIe Endpoint devices, not separated to master and slave)
/**
 * This API can be instantiated in any PCIe Endpoint device. It is a bidirectional PCIe Port
 * and pre-handles incoming transactions.
 * After pre-handling of transactions the b_transact method in the Endpoint device is called
 * (if needed).
 *
 * The API own the configuration registers of the PCIe device.
 *
 * Configuration Requests are handles without the need of processing in the device.
 *
 * A constructor parameter allows to disable the pre-handling. Then the owner has to bind to the
 * multi port. This is needed for routers. They use the handle_Configuration_Requests method to
 * make use of the included register memory.
 *
 */
class PCIeAPI
: public PCIeBidirectionalPort, // is an sc_module
  //needed if this owns ports: public tlm_b_if<PCIeTransactionHandle>, // b_transact method (PV) for slave
  public payload_event_queue_output_if<PCIeTransactionContainer> // notify method (call peq) for slave
{     

public:

  SC_HAS_PROCESS(PCIeAPI);

  /// PCIe slave API constructor.
  /**
   * Create a PCIe API. Use configuration framework
   * to set address range (base_addr, high_addr).
   *
   * Important: _If_ receiving of transactions should be supported, don't forget the device pointer!
   *
   * @param api_name  Name of the API
   * @param dev       Pointer to the receiving object. NULL if receiving should not be supported!
   * @param handle_transactions  If the API should handle the transaction (true=default) or the owner (false)
   */
  PCIeAPI ( sc_module_name api_name, PCIe_recv_if* dev, bool handle_transactions = true )
  : PCIeBidirectionalPort(api_name, true),
    m_locked(false),
    m_ConfReg(m_cnfg_register_memory = new gs_uint8[4096]),  // create configuration register memory
    device(dev),
    m_bus_no(-1),
    m_device_no(-1)
  {
    // bind blocking if (supported b_transact)
    if (handle_transactions) bind_b_if(*this); 

    // bind PEQ output port (not supported notify)
    if (handle_transactions) peq.out_port(*this);
    
    /// SC_METHOD sends a PME_TO_Ack message when triggered.
    SC_METHOD(send_PME_TO_Ack);
    sensitive << m_PME_action;
    dont_initialize();
  }


  // ///////////////////// API methods ///////////////////////
  
  // /// API methods
  
  /// Create transaction and return shared pointer to the PCIe Master Access.
  inline PCIeMasterAccessHandle create_transaction() {
    PCIeMasterAccessHandle ret = PCIeBidirectionalPort::create_transaction();
    CHECK_RULES(if (m_bus_no<0 || m_device_no<0) SC_REPORT_WARNING(name(), "This device has not yet received a bus and device number! Sending forbidden without Requester ID."); );
    ret->setMRequBusNo(m_bus_no);
    ret->setMRequDevNo(m_device_no);
    return ret;
  }

  /// Sends the transaction
  inline void send_transaction(PCIeMasterAccessHandle ah) {
#ifdef CHECK_RULES_ENABLE
    // Check for some rules
    // LOCKED: p.378
    if (m_locked && ah->getMTrafficClass() == 0/*TODO: VC0, not TC0!!*/) {
      PCIeDEBUG("Device is locked and may not send Requests in VC0 (only Completions). TODO:correct?");
      return;
    }
    // others
    unsigned int cmd = ah->getMPCIeCmd();
    switch (cmd) {
      case MemReadLocked: {
        if (ah->getMTrafficClass() != 0) SC_REPORT_ERROR(name(), "Locked transactions only allowed in Traffic Class 0!");
      } break;
      // TODO: maybe delete this warning later
      case CnfgReadTy1:
      case CnfgWriteTy1: {
        //SC_REPORT_WARNING(name(), "Configuration Requests only may be sent by Root.");
      } break;
      case CnfgReadTy0:
      case CnfgWriteTy0: {
        SC_REPORT_ERROR(name(), "Configuration Requests of Type 0 only may be generated by a switch.");
      } break;
    }
#endif
    Transact( ah ); //boost::dynamic_pointer_cast<PCIeTransaction>(ah) );
#ifdef CHECK_RULES_ENABLE
    if (ah->get_Completion_Status() > SuccessfulCompletion) {
      SC_REPORT_WARNING(name(), "Completion Status: ERROR");
    }
    if (cmd != ah->getMPCIeCmd()) {
      if (cmd == CnfgReadTy1 && ah->getMPCIeCmd() == CnfgReadTy0) { }
      else if (cmd == CnfgWriteTy1 && ah->getMPCIeCmd() == CnfgWriteTy0) { }
      else {
        SC_REPORT_WARNING(name(), "TLP Type changed during transmission!");
      }
    }
    // Check if completion was sent if one is needed
    // if no completion
    if (ah->getMComplStatus() == NO_COMPL_STATUS) {
      // if completion had been needed // TODO: check if list is complete
      if (/*reads:*/  cmd == MemRead or cmd == IORead or cmd == CnfgReadTy0
                      or cmd == CnfgReadTy1 or cmd == MemReadLocked
          /*writes:*/ or cmd == CnfgWriteTy0 or cmd == CnfgWriteTy1 or cmd == IOWrite) {
        SC_REPORT_WARNING(name(), "A transaction which needs completion was not completed!");
      }
    }
#endif
  }

  // /////////////////////  ///////////////////////

  /// Blocking transact method handles incoming transactions and forwards them to the device.
  /**
   * The tlm_b_if PV transaction implementation
   * directly calls the according b_transact method in the device
   *
   * Handles (some) configuration
   */
  virtual void b_transact(PCIeTransactionHandle th) {
    //PCIeDEBUG("b_transact call device's b_transact");
    PCIeSlaveAccessHandle ah = _getSlaveAccessHandle(th);
    
    bool call_transact = true; // if this transaction should be forwarded to the receiving device
    if (ah->getMIsIDbasedRouting()) {  // to make memory transactions fast!
      unsigned int cmd = ah->get_TLP_type();
#ifdef PRECHECK_ENABLE
      // check some rules
      switch(cmd) {
        case CnfgReadTy1:
        case CnfgWriteTy1: { // see p. 422
          PCIeDEBUG("Configuration Request of Type 1 not allowed at Endpoint. Unsupported Request.");
          ah->init_Unsupported_Request();
          call_transact = false;
        } break;
        // handle Configuration Register reads!
        case CnfgReadTy0: {
          if (ah->getMFuncNo() != 0) {
            SC_REPORT_WARNING(name(), "Other function number than '0' not yet supported for Configuration Register access!");
            // TODO: unsupported request??
            //call_transact = false;
          } else {
            PCIeDEBUG("Forward read order to Configuration Register.");
            std::vector<gs_uint8> *dat;
            dat = &(ah->getMData().getData());
            // call read function in register
            if (m_ConfReg.read(ah->getMRegNo(), ah->getMBurstLength(), dat))
              ah->init_Cnfg_Read_Completion(SuccessfulCompletion);
            else
              ah->init_Unsupported_Request();
            // Do not forward this transaction to user module
            call_transact = false;
          }
        } break;
        // handle Configuration Register writes!
        case CnfgWriteTy0: { // see p. 422
          if (ah->getMFuncNo() != 0) {
            SC_REPORT_WARNING(name(), "Other function number than '0' not yet supported for Configuration Register access!");
            // TODO: unsupported request??
            //call_transact = false;
          } else {
            PCIeDEBUG("Forward write order to Configuration Register.");
            std::vector<gs_uint8> *dat;
            dat = &(ah->getMData().getData());
            // call write funtion in register
            if (m_ConfReg.write(ah->getMRegNo(), ah->getMBurstLength(), dat))
            ah->init_Cnfg_Write_Completion(SuccessfulCompletion);
            else
              ah->init_Unsupported_Request();
            // Do not forward this transaction to user module
            call_transact = false;
          }
        } break;
      }
#endif
      // handle Configuration Requests
      handle_Configuration_Request(ah);
      
    } // end if ID based routing
    else if (ah->getMIsImplicitRouting()) { // to make memory transactions fast!
      unsigned int cmd = ah->get_TLP_type();
      switch(cmd) {
        // LOCKED: Unlock
        case Msg: {
          if (ah->getMMessageCode() == Unlock) {
            PCIeDEBUG("Unlock device");
            m_locked = false;
          }
          if (ah->getMMessageCode() == PME_Turn_Off) {
            PCIeDEBUG("Send Ack and turn off device!");
            // send PME_TO_Ack _after_ return!
            m_PME_action.notify(SC_ZERO_TIME);
          }
          // INTx Interrupts
          if (ah->getMMessageCode() == Assert_INTA || ah->getMMessageCode() == Assert_INTB ||
              ah->getMMessageCode() == Assert_INTC || ah->getMMessageCode() == Assert_INTD ||
              ah->getMMessageCode() == Deassert_INTA || ah->getMMessageCode() == Deassert_INTB ||
              ah->getMMessageCode() == Deassert_INTC || ah->getMMessageCode() == Deassert_INTD) {
            // see p. 71
            if (ah->getMTrafficClass() != 0) {
              PCIeDEBUG("Interrupt Messages only allowed in Traffic Class TC0!");
              SC_REPORT_ERROR(name(), "Interrupt Messages only allowed in Traffic Class TC0!");
              // TODO: reported Error instead of SC_REPORT_ERROR
            }
          }
        } break;
        default:
          //cout << "2"<<endl;
          break;
      }
    }

    if (device == NULL && call_transact == true) {
      call_transact = false;
      SC_REPORT_WARNING(name(), "User module is not able to receive transactions!");
    }

    // **** Forward transaction to device
    if (call_transact) device->b_transact(th);
    
    unsigned int cmd = ah->get_TLP_type();
    switch(cmd) {
      case MemReadLocked: {
        // LOCKED:
        // if a Successful Completion for a MemWriteLocked
        if (ah->getMComplStatus() == SuccessfulCompletion) {
          PCIeDEBUG("Lock device");
          m_locked = true;
        }
      } break;
    }
  }
 
      /// Handles Configuration Requests with the registers
      /**
       * This method may is called by b_transact 
       * (or may be called by the owning router if the constructor
       * variable handle_transactions is false.)
       */
      void handle_Configuration_Request(PCIeSlaveAccessHandle& ah) {
        // handle Configuration Requests
        unsigned int cmd = ah->get_TLP_type();
        switch(cmd) {
          case CnfgReadTy0:
          case CnfgWriteTy0: {
            // read out device and bus number and set variables m_device_no and m_bus_no
            if (m_device_no < 0) {
              m_bus_no = ah->getMBusNo();
              m_device_no = ah->getMDevNo();
              PCIeDEBUG("Updated my Bus (="<< m_bus_no<<") and Device Number (="<<m_device_no<<")");
            }
            
            // TODO: Check if the Request addresses a valid local Configuration space
    
            // ****** Add new address range to switch's address map if TLP sets a base address
            if ( m_ConfReg.is_Type_00_Header() ) {
              if (false) { // check register address if it is a base address (10h-24h)
                #ifdef PRECHECK_ENABLE
                // TODO: Check if this base address was already set. If now another value: warning!
                #endif
                if (false) { // if I/O Space
                  if (m_device_no >= 0 && upstream_switch) {
                    MAddr base = 0;// TODO: read base address out of transaction
                    MAddr high = 0;// TODO: read high address out of transaction
                    upstream_switch->add_IO_address_range_to_device(base, high, m_bus_no, m_device_no);
                  } else
                    SC_REPORT_ERROR(name(), "Software tried to set an address range of this device which is not connected to a switch's Downstream Port.");
                }
                else { // if Memory Space
                  if (m_device_no >= 0 && upstream_switch) {
                    MAddr base = 0;// TODO: read base address out of transaction
                    MAddr high = 0;// TODO: read high address out of transaction
                    upstream_switch->add_Memory_address_range_to_device(base, high, m_bus_no, m_device_no);
                  } else
                    SC_REPORT_ERROR(name(), "Software tried to set an address range of this device which is not connected to a switch's Downstream Port.");
                }
              }
              if (false) { // check register address if it is an Expansion ROM base address (30h)
                #ifdef PRECHECK_ENABLE
                // TODO: Check if this base address was already set. If now another value: warning!
                #endif
                // Expansion ROM is in Memory Address Space (see PCI 3.0 spec. p.228)
                if (m_device_no >= 0 && upstream_switch) {
                  MAddr base = 0;// TODO: read base address out of transaction
                  MAddr high = 0;// TODO: read high address out of transaction
                  upstream_switch->add_Memory_address_range_to_device(base, high, m_bus_no, m_device_no);
                } else
                  SC_REPORT_ERROR(name(), "Software tried to set an Expansion ROM address range of this device which is not connected to a switch's Downstream Port.");
              }
            }
            else if ( m_ConfReg.is_Type_01_Header() ) {
              if (false) { // check register address if it is a base address (10h - 14h)
                #ifdef PRECHECK_ENABLE
                // TODO: Check if this base address was already set. If now another value: warning!
                #endif
                if (false) { // if I/O Space
                  if (m_device_no >= 0 && upstream_switch) {
                    MAddr base = 0;// TODO: read base address out of transaction
                    MAddr high = 0;// TODO: read high address out of transaction
                    upstream_switch->add_IO_address_range_to_device(base, high, m_bus_no, m_device_no);
                  } else
                    SC_REPORT_ERROR(name(), "Software tried to set an address range of this device which is not connected to a switch's Downstream Port.");
                }
                else { // if Memory Space
                  if (m_device_no >= 0 && upstream_switch) {
                    MAddr base = 0;// TODO: read base address out of transaction
                    MAddr high = 0;// TODO: read high address out of transaction
                    upstream_switch->add_Memory_address_range_to_device(base, high, m_bus_no, m_device_no);
                  } else
                    SC_REPORT_ERROR(name(), "Software tried to set an address range of this device which is not connected to a switch's Downstream Port.");
                }
              }
            }
            //}
            //call_transact = false; // TODO: don't forward Configuration Requests to device
          } break;
        }
      }


  // /////////////////////  ///////////////////////

  /// Non-blocking PVT protocol not supported (called by the peq.)
  void notify(PCIeTransactionContainer& tp) {
    SC_REPORT_WARNING(name(), "Non-blocking communication is not supported!");
  }
  
  /// SystemC callback
  /**
   * Set pointer to upstream Switch.
   */
  void end_of_elaboration() {
    // set pointer to upstream switch
    sc_interface *inf         = this->out.get_interface();
    sc_object    *destination = dynamic_cast<sc_object*>(inf);
    sc_object *o = dynamic_cast<sc_object*>(destination->get_parent());
    /*const char* target_name;
    if (o->get_parent()) target_name = o->get_parent()->name();
    else target_name = o->name();
    cout << "this "<<this->name() << " is connected to target_name="<<target_name<<endl;*/
    PCIeRouter_if *owner = dynamic_cast<PCIeRouter_if*>(this->get_parent());
    if (!owner){
      upstream_switch = dynamic_cast<PCIeRouter_if*>(o->get_parent());
      if (!upstream_switch){
        SC_REPORT_WARNING(name(), "Device is not connected to a switch!");
      }
    }
  }

protected:

  /// SC_METHOD sends a PME_TO_Ack Message
  /**
   * This SC_METHOD is triggered when a PME_Turn_Off message was received.
   * The method is needed to send the Ack _after_ the Turn Off message
   * returned.
   */
  void send_PME_TO_Ack() {
    PCIeDEBUG("Send PME_TO_Ack");
    PCIeMasterAccessHandle ah = create_transaction();
    ah->init_Message(PME_TO_Ack);
    send_transaction(ah);
    PCIeDEBUG("Shutdown power of device "<< get_parent()->name());
  }

public:

  /// Whether the device is locked or not
  bool m_locked;
  
  /// raw Register Memory of this device. Access with ConfReg API
  gs_uint8* m_cnfg_register_memory;
  /// Configuration Register API to access the m_cnfg_register_memory (for default function no. 0)
  ConfReg m_ConfReg;
  
protected:

  /// Owner device which implements b_transact;
  PCIe_recv_if* device;

  /// Switch which is connected to the port (allow call of address adding methods)
  PCIeRouter_if* upstream_switch;

  /// The bus number where this device is connected to (initialized to -1, set when incoming ID routed TLP)
  int m_bus_no;
  /// The device number of this device (initialized to -1, set when incoming ID routed TLP)
  int m_device_no;

public:
  /// Event for send_PME_TO_Ack sc_method. Public to be notified by an owning router.
  sc_event m_PME_action;
  
};


} // end namespace PCIe
} // end namespace tlm


#endif
