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

#ifndef __ddrControllerAPI_h__
#define __ddrControllerAPI_h__

/**
 * \file simplebusAPI.h 
 * The simplebusAPI provides two blocking bus access methods: 
 * read, write.
 * It is a simple example for a GreenBus User-API.
 * Furthermore, it shows how slaves that _themselfes_ implement
 * a TLM API (rather than waiting for a bus event) 
 * can be connected to GreenBus.
 */

#include "systemc.h"
#include "greenbus/utils/gs_datatypes.h"
#include "greenbus/utils/gs_trace.h"
#include "greenbus/transport/generic.h"
#include "greenbus/transport/PLB.h"
#include "greenbus/api/simplebusAPI.h"

namespace tlm {

///////////////////////////////////////////////////////////////////////////////
/// The simplebus slave port
///////////////////////////////////////////////////////////////////////////////
/**
 * This is the slave port. Bind this port with a GreenBus router, and bind a
 * SimpleBus slave to this port.
 */
class ddrControllerPort 
: public GenericSlavePort,
  public tlm_b_if<GenericTransactionHandle>
{
public:
  typedef GenericSlavePort PORT;
  typedef PORT::accessHandle accessHandle;
  typedef PORT::phase phase;

  /// slave module socket
  sc_port<simplebus_if> slave_port;

  SC_HAS_PROCESS(ddrControllerPort);

  /**
   * Create a SimpleBus slave port. Use configuration framework
   * to set address range (base_addr, high_addr) of the slave.
   */
  ddrControllerPort(sc_module_name _name) 
    : PORT(_name),
      eventDetected(false)
  {
#ifdef USE_DUST
    DUST_SLAVE_PORT("ddrControllerPort", "SimpleBus");
#endif
      
    PORT::bind_b_if(*this);

    SC_THREAD(react);

    SC_METHOD(react_mth);
    sensitive << PORT::default_event();
    dont_initialize();
  }

  /**
   * The tlm_b_if PV transaction method
   */
  virtual void b_transact(transactionHandle th) {
    //sc_assert(th->getMBurstLength()>0);
    if (th->getMCmd() == Generic_MCMD_WR) { 
      if (slave_port->write(th->getMData().getData(), 
                            static_cast<gs_uint32>(th->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                            static_cast<gs_uint32>(th->getMBurstLength()))) {
        th->setSBurstLength(th->getMBurstLength());
      }
      else { // error
        th->setSError(GenericError::Generic_Error_AccessDenied);
        SC_REPORT_WARNING(name(), "b_transact() slave_port->write() returned false. Setting SError=Generic_Error_AccessDenied.");
      }
    }

    else if (th->getMCmd() == Generic_MCMD_RD) {      
      if (slave_port->read(th->getSData().getData(), 
                           static_cast<gs_uint32>(th->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                           static_cast<gs_uint32>(th->getMBurstLength()))) {
        th->setSBurstLength(th->getMBurstLength());
      }
      else { // error
        th->setSError(GenericError::Generic_Error_AccessDenied);
        SC_REPORT_WARNING(name(), "b_transact() slave_port->read() returned false. Setting SError=Generic_Error_AccessDenied.");
      }
    }

    else {
      SC_REPORT_WARNING(name(), "b_transact() got unknown MCmd. Ignoring. Setting SError=Generic_Error_AccessDenied.");
      th->setSError(GenericError::Generic_Error_AccessDenied);
    }
    sc_core::wait(SC_ZERO_TIME); //synch
  }

  void react_mth(){
    phase ph_check = PORT::get_phase();
    GS_TRACE(name(), "Got a TXN with phase: %s", ph_check.toString().c_str());
    if (ph_check.state!=PLB::PLBPhase::SecondaryRequest){ //ignore secondary requests
      GS_TRACE(name(), "Setting detect to true");
      eventDetected=true;
      ah_int = PORT::get_transaction();
      ph_int = ph_check;
      if (ph_int.state==GenericPhase::RequestValid){ //always accept requests to prevent timeouts
        if (ah_int->getMCmd() != Generic_MCMD_WR && ah_int->getMCmd() != Generic_MCMD_RD) {
          SC_REPORT_WARNING(name(), "react() got unknown MCmd in RequestValid phase. Ignoring. Sending Request Error with SError=Generic_Error_AccessDenied.");
          ah_int->setSError(GenericError::Generic_Error_AccessDenied);
          PORT::ErrorRequest(ah_int, ph_int, SC_ZERO_TIME, ph_int.getSimulationMode());
        }
        else {
          GS_TRACE(name(), "Got a Request: %i", (gs_uint32)ah_int->getMCmd());
          PORT::AckRequest(ah_int, ph_int, SC_ZERO_TIME, ph_int.getSimulationMode());
          cmd_int=ah_int->getMCmd();
        }
      }
      startReact.notify();
    }
  }

  /**
   * Play the generic protocol with GreenBus.
   */
  void react() {

    MCmd cmd = Generic_MCMD_IDLE;
    cmd_int = Generic_MCMD_IDLE;

    while (1) {
      if(!eventDetected) sc_core::wait(startReact);
      accessHandle ah = ah_int;
      phase ph = ph_int;
      eventDetected=false;
      GS_TRACE(name(), "Setting detect to false");
      switch(ph.state) {
      case GenericPhase::RequestValid: // master sends request
        {
          cmd = ah->getMCmd();
          cmd_int=Generic_MCMD_IDLE;
          if (cmd == Generic_MCMD_RD) { // read request
            GS_TRACE(name(), "react() got RequestValid with MCmd=Generic_MCMD_RD, MBurstLength=%d. Calling slave_port->read().", (gs_uint32)ah->getMBurstLength());
            if (slave_port->read(ah->getSData().getData(), 
                                  static_cast<gs_uint32>(ah->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                                  static_cast<gs_uint32>(ah->getMBurstLength()))) {
              ah->setSBurstLength(ah->getMBurstLength());
              GS_TRACE(name(), "react() slave_port->read() returned. Sending ResponseValid.");
              if (ph.getSimulationMode()==MODE_CT) { // play the CT protocol
                SC_REPORT_ERROR(name(), "CT mode not implemented.");
              }
              else {
                PORT::Response(ah, ph, SC_ZERO_TIME, ph.getSimulationMode());
                cmd=cmd_int;
                cmd_int=Generic_MCMD_IDLE;
              }
            }
            else {
              SC_REPORT_WARNING(name(), "react() slave_port->read() returned false. Sending ResponseValid with SError=Generic_Error_AccessDenied.");
              ah->setSError(GenericError::Generic_Error_AccessDenied);
              PORT::Response(ah, ph, SC_ZERO_TIME, ph.getSimulationMode());
            }
          }
          else { // write command
            GS_TRACE(name(), "react() got RequestValid with MCmd=Generic_MCMD_WR, MBurstLength=%d. Sending AckRequest.", (gs_uint32)ah->getMBurstLength());
          }
        }
        break;
        
      case GenericPhase::DataValid: // master sends data
        {
          #ifdef TIMED_SIM
          sc_assert(cmd==Generic_MCMD_WR);
          #endif

          GS_TRACE(name(), "react() got DataValid with MBurstLength=%d. Calling slave_port->write().", (gs_uint32)ah->getMBurstLength());
          if (slave_port->write(ah->getMData().getData(), 
                                static_cast<gs_uint32>(ah->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                                static_cast<gs_uint32>(ah->getMBurstLength()))) {
            ah->setSBurstLength(ah->getMBurstLength());              
            GS_TRACE(name(), "react() slave_port->write() returned. Sending AckData. Write transaction finished.");
            if (ph.getSimulationMode()==MODE_CT) { // play the CT protocol
              SC_REPORT_ERROR(name(), "CT mode not implemented.");
            }
            else {
              PORT::AckData(ah, ph, SC_ZERO_TIME, ph.getSimulationMode());
            }
          }
          else {
            SC_REPORT_WARNING(name(), "react() slave_port->write() returned false. Sending DataError with SError=Generic_Error_AccessDenied.");
            ah->setSError(GenericError::Generic_Error_AccessDenied);
            PORT::ErrorData(ah, ph, SC_ZERO_TIME, ph.getSimulationMode());
          }
          cmd = cmd_int;
          cmd_int = Generic_MCMD_IDLE;
        }
        break;
        
      case GenericPhase::ResponseAccepted: // master acks response
        {
          GS_TRACE(name(), "react() got ResponseAccepted. Read transaction finished.");
        }        
        break;
        
      default:
        {
          GS_TRACE(name(), "react() got unknown phase [%s]", ph.toString().c_str());
          SC_REPORT_WARNING(name(), "react() got triggered with unexpected phase. Ignoring.");
        }
      }
    }
  }

  private:
    sc_core::sc_event startReact;
    bool eventDetected;
    accessHandle ah_int;
    phase ph_int;
    MCmd cmd_int;

};

} // namspace tlm

#endif // __simplebusAPI_h__
