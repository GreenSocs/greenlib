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

#ifndef __cpuAPI_h__
#define __cpuAPI_h__

#include "systemc.h"
#include "greenbus/utils/gs_datatypes.h"
#include "greenbus/utils/gs_trace.h"
#include "greenbus/transport/generic.h"
#include "greenbus/api/simplebusAPI.h"

namespace tlm {


///////////////////////////////////////////////////////////////////////////////
/// The simplebus master port
///////////////////////////////////////////////////////////////////////////////
/**
 * This is the master port. It provides a read and a write method.
 * Both methods are blocking.
 * Use this port in a SimpleBus master to get access to a SimpleBus slave.
 */
class cpuMasterPort 
: public GenericMasterPort
{
public:
  typedef GenericMasterPort PORT;

  typedef PORT::accessHandle accessHandle;
  typedef PORT::phase phase;
  bool dump;
protected:
  sc_event readDoneEvent;
  std::vector<gs_uint8> xferData;

public:
  /// simulation mode: 0=BA (default), 1=PV (use b_transact)
  gb_param<gs_uint32> simulation_mode;
  gs_uint32 mhz;

  SC_HAS_PROCESS(cpuMasterPort);

  /**
   * Create a cpuMasterPort. Use configuration framework
   * to set parameters target_addr and simulation_mode.
   */
  cpuMasterPort(sc_module_name _name, gs_uint32 mhz) 
    : PORT(_name), mhz(mhz)
  {
#ifdef USE_DUST
    DUST_MASTER_PORT("cpuMasterPort", "SimpleBus");
#endif

    SC_METHOD(react);
    sensitive << PORT::default_event();
    dont_initialize();
    
    GB_PARAM(simulation_mode, gs_uint32, 0); // default mode is BA
    dump=false;
    xferData.resize(4);
  }

  /**
   * The simplebus_if read method.
   */

  #define CLPERIOD 1000/mhz, SC_NS

  bool read(std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
    if (length>4) assert(length%4==0);
    else {
      assert(length!=3);
      if (length==2) assert(length%2==0);
    }
    if (simulation_mode == 0) { // BA
      #ifdef TIMED_SIM
      sc_core::wait(6*CLPERIOD);
      #endif
      if (length<4){
        accessHandle ah = PORT::create_transaction();
        ah->setMAddr(addr);
        ah->setMBurstLength(length);
        ah->setMCmd(Generic_MCMD_RD);
        ah->setMData(xferData); // slave copies its data into the master's vector
        bool success=false;
        while (!success) {
          GS_TRACE(name(), "read() sending RequestValid with MAddr=0x%x, MBurstLength=%d", addr, length);
          PORT::Request.block(ah, SC_ZERO_TIME, MODE_BA);
          phase ph = PORT::get_phase();
          if (ph.isRequestAccepted())
            success=true;
          else
            GS_TRACE(name(), "read() got unknown answer to RequestValid: phase=%s. Ignoring.", ph.toString().c_str());
        }
        sc_core::wait(readDoneEvent); // wait for slave response
        if (ah->getSError() != GenericError::Generic_Error_NoError) {
          SC_REPORT_WARNING(name(), "read() transaction was terminated by slave with an error.");
          return false;
        }
        memcpy(&data[0], &xferData[0], length);
        #ifdef TIMED_SIM
        sc_core::wait(32*CLPERIOD);
        #endif
      }
      else{
        for (int i=0; i<length; i+=4){
          accessHandle ah = PORT::create_transaction();
          ah->setMAddr(addr+i);
          ah->setMBurstLength(4);
          ah->setMCmd(Generic_MCMD_RD);
          ah->setMData(xferData); // slave copies its data into the master's vector
      
          bool success=false;
          while (!success) {
            GS_TRACE(name(), "read() sending RequestValid with MAddr=0x%x, MBurstLength=%d", addr+i, 4);
            PORT::Request.block(ah, SC_ZERO_TIME, MODE_BA);
            phase ph = PORT::get_phase();
            if (ph.isRequestAccepted())
              success=true;
            else
              GS_TRACE(name(), "read() got unknown answer to RequestValid: phase=%s. Ignoring.", ph.toString().c_str());
          }
          sc_core::wait(readDoneEvent); // wait for slave response
          if (ah->getSError() != GenericError::Generic_Error_NoError) {
            SC_REPORT_WARNING(name(), "read() transaction was terminated by slave with an error.");
            return false;
          }
          memcpy(&data[i], &xferData[0], 4);
          #ifdef TIMED_SIM
          sc_core::wait(32*CLPERIOD);
          #endif
        }
      }
    }
    else { // PV
      GS_TRACE(name(), "read() doing PV read transaction with MAddr=0x%x, MBurstLength=%d", addr, length);
      accessHandle ah = PORT::create_transaction();
      ah->setMAddr(addr);
      ah->setMBurstLength(length);
      ah->setMCmd(Generic_MCMD_RD);
      ah->setMData(data); // slave copies its data into the master's vector
      PORT::Transact(ah);
    }
    return true;
  }

#define DUMP(msg) if (dump) cout<<sc_time_stamp()<<" "<<name()<<" "<<msg<<endl<<flush;

  /**
   * The simplebus_if write method.
   */
  bool write(const std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
    if (length>4) assert(length%4==0);
    else {
      assert(length!=3);
      if (length==2) assert(length%2==0);
    }
    if (simulation_mode == 0) { // BA
      #ifdef TIMED_SIM
      sc_core::wait(6*CLPERIOD);
      #endif
      if (length<4){
        accessHandle ah = PORT::create_transaction();
        memcpy(&xferData[0], &data[0], length);
        #ifdef TIMED_SIM
        sc_core::wait(7*CLPERIOD);
        #endif
        ah->setMAddr(addr);
        ah->setMBurstLength(length);
        ah->setMCmd(Generic_MCMD_WR);
        ah->setMData(xferData);
  
        GS_TRACE(name(), "write() sending RequestValid with MAddr=0x%x, MBurstLength=%d", addr, length);
        bool success=false;
        phase ph;
        while (!success) {
          PORT::Request.block(ah, SC_ZERO_TIME, MODE_BA);
          ph = PORT::get_phase();
          if (ph.isRequestAccepted())
            success=true;
          else {
            GS_TRACE(name(), "write() got unknown answer to RequestValid: phase=%s. Ignoring", ph.toString().c_str());
          }
        }
        PORT::SendData.block(ah, ph, SC_ZERO_TIME, MODE_BA);
        if (ah->getSError() != GenericError::Generic_Error_NoError) {
          SC_REPORT_WARNING(name(), "read() transaction was terminated by slave with an error.");
          return false;
        }
        #ifdef TIMED_SIM
        sc_core::wait(3*CLPERIOD);
        #endif
      }
      else{
        for (int i=0; i<length; i+=4){
          accessHandle ah = PORT::create_transaction();
          memcpy(&xferData[0], &data[i], 4);
          #ifdef TIMED_SIM
          sc_core::wait(7*CLPERIOD);
          #endif
          ah->setMAddr(addr+i);
          ah->setMBurstLength(4);
          ah->setMCmd(Generic_MCMD_WR);
          ah->setMData(xferData);
    
          GS_TRACE(name(), "write() sending RequestValid with MAddr=0x%x, MBurstLength=%d", addr+i, 4);
          bool success=false;
          phase ph;
          while (!success) {
            PORT::Request.block(ah, SC_ZERO_TIME, MODE_BA);
            ph = PORT::get_phase();
            if (ph.isRequestAccepted())
              success=true;
            else {
              GS_TRACE(name(), "write() got unknown answer to RequestValid: phase=%s. Ignoring", ph.toString().c_str());
            }
          }
          PORT::SendData.block(ah, ph, SC_ZERO_TIME, MODE_BA);
          if (ah->getSError() != GenericError::Generic_Error_NoError) {
            SC_REPORT_WARNING(name(), "read() transaction was terminated by slave with an error.");
            return false;
          }
          #ifdef TIMED_SIM
          sc_core::wait(3*CLPERIOD);
          #endif
        }
      }
    }
    else { // PV
      GS_TRACE(name(), "write() doing PV write transaction with MAddr=0x%x, MBurstLength=%d", addr, length);
      accessHandle ah = PORT::create_transaction();
      ah->setMAddr(addr);
      ah->setMBurstLength(length);
      ah->setMCmd(Generic_MCMD_WR);
      ah->setMData(data);
      PORT::Transact(ah);
    }
    return true;
  }


  /**
   * Play the generic protocol with GreenBus.
   */
  void react() {
    accessHandle ah = PORT::get_transaction();    
    phase ph = PORT::get_phase();    

    if (ph.isResponseValid()) {
      PORT::AckResponse(ah, ph, SC_ZERO_TIME, ph.getSimulationMode());
      GS_TRACE(name(), "react() got ResponseValid. Sending AckResponse.");
      readDoneEvent.notify();
    }   
    else if (ph.isRequestAccepted() || ph.isDataAccepted()) {} 
    else
    {
      GS_TRACE(name(), "react() got triggered by unknown phase=%s. Ignoring.", ph.toString().c_str());
    }
  }
};

} // namspace tlm

#endif // __simplebusAPI_h__
