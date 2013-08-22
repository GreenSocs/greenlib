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

#ifndef __simplebusAPI_h__
#define __simplebusAPI_h__
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
#include "greenbus/transport/GP/GP.h"
#include <deque>

#ifdef GAV_ENABLED
#  include "greencontrol/config.h"
#  include "greencontrol/gav/plugin/StatCalc.h"
#  include "greencontrol/gcnf/apis/gs_param/gs_param_GenericTransactionCopy.h"
#endif

namespace tlm {

///////////////////////////////////////////////////////////////////////////////
/// The simplebus interface
///////////////////////////////////////////////////////////////////////////////
/**
 * This is a blocking interface for TLM communication.
 * The interface provides blocking methods for both
 * single-beat, and burst read and write transactions.
 */
class simplebus_if : virtual public sc_interface
{
public:
  /// Read data from a slave
  /**
   * Read data from a SimpleBus slave. The transaction
   * may be of arbitrary length.
   * The implementation of this method has to be blocking
   * and must not return until the data has been
   * completely received from the slave.
   * @param data Data array into which the data will be copied 
   *             that is received from the slave.
   * @param addr Target address of the slave.
   * @param length Burst length
   * @return true on success,
   *         false if no slave is present at the specified target address.
   */
  virtual bool read(std::vector<gs_uint8> &data, 
                    const gs_uint32 addr,
                    const gs_uint32 length) =0;

  /// Write data to a slave
  /**
   * Write data to a SimpleBus slave. The transaction
   * may be of arbitrary length.
   * The implementation of this method has to be blocking
   * and must not return until the data  has been
   * completely received by the slave.
   * @param data Data array to send to the slave.
   * @param addr Target address of the slave.
   * @param length Burstlength
   * @return true on susccess,
   *         false if no slave is present at the specified target address.
   */
  virtual bool write(const std::vector<gs_uint8> &data, 
                     const gs_uint32 addr, 
                     const gs_uint32 length) =0;
};



///////////////////////////////////////////////////////////////////////////////
/// The simplebus master port
///////////////////////////////////////////////////////////////////////////////
/**
 * This is the master port. It provides a read and a write method.
 * Both methods are blocking.
 * Use this port in a SimpleBus master to get access to a SimpleBus slave.
 *
 * Analysis abilities:
 * - Transaction overall latency:      
 *     Time a transaction needs between sending (in the master) 
 *     and completion (in the master).
 *     parameter: iport.transaction_overall_latency_result_0
 * - Transaction transport latency:
 *     Time the request needs to be sent by the master and acknowledged
 *     by the slave (phases Request and RequestAccepted). 
 */
class simplebusMasterPort
: public GenericMasterBlockingPort
{
public:
  typedef GenericMasterBlockingPort PORT;

  typedef PORT::accessHandle accessHandle;
  typedef PORT::phase phase;

protected:
  sc_event readDoneEvent;
  
  class notifyBinder : public payload_event_queue_output_if<PORT::GS_ATOM>{
  public:
    notifyBinder(simplebusMasterPort& owner_): owner(owner_){}
    void notify(PORT::GS_ATOM& tc){
      owner.react(tc);
    }
  private:
    simplebusMasterPort& owner;
  };
  
  notifyBinder myBinder;

#ifdef GAV_ENABLED
  // Analysis input parameter marking start timepoint of a transaction (used for overall and transport)
  gs::gs_param<sc_time> transaction_latency_start_timepoint;
  // Analysis input parameter marking end timepoint of a overall transaction
  gs::gs_param<sc_time> transaction_overall_latency_end_timepoint;
  // Analysis input parameter marking end timepoint of the request (and acked) phase of a transaction
  gs::gs_param<sc_time> transaction_transport_latency_end_timepoint;
  // StatCalc object for transaction overall latency
  gs::av::StatCalc<double> *tr_overall_latency;
  // StatCalc object for transaction transport latency
  gs::av::StatCalc<double> *tr_transport_latency;
#endif
  
public:
  /// simulation mode: 0=BA (default), 1=PV (use b_transact)
  gb_param<gs_uint32> simulation_mode;

  SC_HAS_PROCESS(simplebusMasterPort);

  /**
   * Create a simplebusMasterPort. Use configuration framework
   * to set parameters target_addr and simulation_mode.
   */
  simplebusMasterPort(sc_module_name _name) 
    : PORT(_name), myBinder(*this)
#ifdef GAV_ENABLED
      , transaction_latency_start_timepoint("transaction_latency_start_timepoint")
      , transaction_overall_latency_end_timepoint("transaction_overall_latency_end_timepoint")
      , transaction_transport_latency_end_timepoint("transaction_transport_latency_end_timepoint")
#endif
  {
#ifdef USE_DUST
    DUST_MASTER_PORT("SimpleBusMasterPort", "SimpleBus");
#endif
    
    GB_PARAM(simulation_mode, gs_uint32, 0); // default mode is BA
    
    PORT::out_port(myBinder);

#ifdef GAV_ENABLED
    // ** Transaction overall latency
    // Formula Calculator
    gs::av::Calculator<double> *tr_overall_latency_calc
      = new gs::av::Calculator<double>("transaction_overall_latency");
    tr_overall_latency_calc->calc("-", transaction_overall_latency_end_timepoint, transaction_latency_start_timepoint);
    // Trigger
    std::vector<gs::gs_param_base*> sensitive_params(1, &transaction_overall_latency_end_timepoint);
    gs::av::Trigger *tr = new gs::av::Trigger(sensitive_params);
    // StatCalc
    tr_overall_latency = new gs::av::StatCalc<double>("transaction_overall_latency_StatCalc", tr, tr_overall_latency_calc);
    //tr_overall_latency->deactivate(); // TODO: disable by default
    //cout << "transactions overall latency parameter name: " << tr_overall_latency->get_result_param()->getName() << endl;
    //gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, &transaction_latency_start_timepoint);
    //gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, &transaction_overall_latency_end_timepoint);
    //gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, tr_overall_latency->get_result_param());

    // ** Transaction transport latency
    // Formula Calculator
    gs::av::Calculator<double> *tr_transport_latency_calc
      = new gs::av::Calculator<double>("transaction_transport_latency");
    tr_transport_latency_calc->calc("-", transaction_transport_latency_end_timepoint, transaction_latency_start_timepoint);
    // Trigger
    std::vector<gs::gs_param_base*> sensitive_tr_params(1, &transaction_transport_latency_end_timepoint);
    gs::av::Trigger *trtr = new gs::av::Trigger(sensitive_tr_params);
    // StatCalc
    tr_transport_latency = new gs::av::StatCalc<double>("transaction_transport_latency_StatCalc", trtr, tr_transport_latency_calc);
    //tr_transport_latency->deactivate(); // TODO: disable by default
    //cout << "transactions transport latency parameter name: " << tr_transport_latency->get_result_param()->getName() << endl;
    //gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, &transaction_latency_start_timepoint);
    //gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, &transaction_transport_latency_end_timepoint);
    //gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, tr_transport_latency->get_result_param());
#endif
    
  }

  ~simplebusMasterPort() {
#ifdef GAV_ENABLED
    delete tr_overall_latency->get_calculator();
    gs::av::trigger_if* tr = tr_overall_latency->get_trigger();
    delete tr_overall_latency;
    delete tr;
    delete tr_transport_latency->get_calculator();
    tr = tr_transport_latency->get_trigger();
    delete tr_transport_latency;
    delete tr;
#endif
  }
  
  /**
   * The simplebus_if read method.
   */
  bool read(std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
    accessHandle ah = PORT::create_transaction();
    phase answer;
    
    MData mdata(data);
    ah->setMAddr(addr);
    ah->setMBurstLength(length);
    ah->setMCmd(Generic_MCMD_RD);
    ah->setMData(data); // slave copies its data into the master's vector

#ifdef GAV_ENABLED
    // Analysis: start transaction overall latency measurement
    transaction_latency_start_timepoint = sc_time_stamp();
#endif
    if (simulation_mode == 0) { // BA
      bool success=false;
      while (!success) {
        GS_TRACE(name(), "read() sending RequestValid with MAddr=0x%x, MBurstLength=%d", addr, length);
        PORT::Request.block(ah, answer, PEQ_YIELD, MODE_BA);
        if (answer.isRequestAccepted())
          success=true;
        else
          GS_TRACE(name(), "read() got unknown answer to RequestValid: phase=%s. Ignoring.", answer.toString().c_str());
      }
#ifdef GAV_ENABLED
      // Analysis: stop transaction transport latency measurement
      transaction_transport_latency_end_timepoint = sc_time_stamp();
#endif
      sc_core::wait(readDoneEvent); // wait for slave response
    }
    else { // PV
      GS_TRACE(name(), "read() doing PV read transaction with MAddr=0x%x, MBurstLength=%d", addr, length);
      PORT::Transact(ah);
    }
#ifdef GAV_ENABLED
    // Analysis: stop transaction overall latency measurement
    transaction_overall_latency_end_timepoint = sc_time_stamp();
#endif
    
    if (ah->getSError() != GenericError::Generic_Error_NoError) {
      SC_REPORT_WARNING(name(), "read() transaction was terminated by slave with an error.");
      return false;
    }
    return true;
  }

  /**
   * The simplebus_if write method.
   */
  bool write(const std::vector<gs_uint8> &data, const gs_uint32 addr, const gs_uint32 length) {
    accessHandle ah = PORT::create_transaction();
    phase answer;
    MData mdata(data);

    ah->setMAddr(addr);
    ah->setMBurstLength(length);
    ah->setMSBytesValid(length);
    ah->setMCmd(Generic_MCMD_WR);
    ah->setMData(mdata);

#ifdef GAV_ENABLED
    // Analysis: start transaction overall latency measurement
    transaction_latency_start_timepoint = sc_time_stamp();
#endif
    if (simulation_mode == 0) { // BA
      GS_TRACE(name(), "write() sending RequestValid with MAddr=0x%x, MBurstLength=%d", addr, length);
      bool success=false;
      while (!success) {
        PORT::Request.block(ah, answer, PEQ_YIELD, MODE_BA);
        if (answer.isRequestAccepted())
          success=true;
        else {
          GS_TRACE(name(), "write() got unknown answer to RequestValid: phase=%s. Ignoring", answer.toString().c_str());
        }
      }
#ifdef GAV_ENABLED
      // Analysis: stop transaction transport latency measurement
      transaction_transport_latency_end_timepoint = sc_time_stamp();
#endif
      PORT::SendData.block(ah, answer, answer, PEQ_YIELD, MODE_BA);
    }
    else { // PV
      GS_TRACE(name(), "write() doing PV write transaction with MAddr=0x%x, MBurstLength=%d", addr, length);
      PORT::Transact(ah);
    }
#ifdef GAV_ENABLED
    // Analysis: stop transaction overall latency measurement
    transaction_overall_latency_end_timepoint = sc_time_stamp();
#endif
    
    if (ah->getSError() != GenericError::Generic_Error_NoError) {
      SC_REPORT_WARNING(name(), "read() transaction was terminated by slave with an error.");
      return false;
    }

    return true;
  }


  /**
   * Play the generic protocol with GreenBus.
   */
  void react(PORT::GS_ATOM& tc) {
#ifdef USE_STATIC_CASTS
    accessHandle ah = _getMasterAccessHandle(tc);
    phase ph = _getPhase(tc);    
#else
    accessHandle ah = tc.first;
    phase ph = tc.second;    
#endif

    if (ph.isResponseValid()) {
      PORT::AckResponse(ah, ph, PEQ_YIELD, ph.getSimulationMode());
      GS_TRACE(name(), "react() got ResponseValid. Sending AckResponse.");
      if (ah->getMSBytesValid()==ah->getMBurstLength())
        readDoneEvent.notify();
    }
    else
    {
      GS_TRACE(name(), "react() got triggered by unknown phase=%s. Ignoring.", ph.toString().c_str());
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
/// The simplebus slave port
///////////////////////////////////////////////////////////////////////////////
/**
 * This is the slave port. Bind this port with a GreenBus router, and bind a
 * SimpleBus slave to this port.
 *
 * Analysis abilities (only if #ifdef GAV_ENABLED):
 * - Record transactions 
 *   (read and write, for blocking and non-blocking calls)
 *     Transaction recording can be switched on / off with a configurable
 *   parameter (record_transactions_enabled). Alternatively the recording can
 *   be enabled in the contructor call by default.
 *     Filtering to record transactions of only one master can
 *   be enabled by setting a configurable parameter to a master ID 
 *   (record_transaction_master_id_filter). If set to 0 the filter is disabled.
 *
 *     Note that this transaction recording is an example and insists not on well performing 
 *   and beautiful implementation.
 */
class simplebusSlavePort 
: public GenericSlavePort,
  public tlm_b_if<GenericTransactionHandle>,
  public payload_event_queue_output_if<GenericSlavePort::GS_ATOM>
{
public:
  typedef GenericSlavePort PORT;
  typedef PORT::accessHandle accessHandle;
  typedef PORT::phase phase;

protected:
  sc_event startThread;
  std::deque<PORT::GS_ATOM> m_tcs;

public:
  /// slave module socket
  sc_port<simplebus_if> slave_port;

  SC_HAS_PROCESS(simplebusSlavePort);

  /**
   * Create a SimpleBus slave port. Use configuration framework
   * to set address range (base_addr, high_addr) of the slave.
   */
  simplebusSlavePort(sc_module_name _name
#ifdef GAV_ENABLED
                     , bool record_transactions = false
#endif
                     ) 
    : PORT(_name), slave_port("slave_port")
#ifdef GAV_ENABLED
      , recorded_transaction("recorded_transaction")
      , record_transactions_enabled("record_transactions_enabled", record_transactions)
      , record_transaction_master_id_filter("record_transaction_master_id_filter", (long)0)
#endif
  {
#ifdef USE_DUST
    DUST_SLAVE_PORT("SimpleBusSlavePort", "SimpleBus");
#endif
      
    PORT::bind_b_if(*this);
    PORT::peq.out_port(*this);

    SC_THREAD(react); 
    
#ifdef GAV_ENABLED
    // ** Analysis: transaction recording
    //gs::av::GAV_Api::getApiInstance()->add_to_default_output(gs::av::STDOUT_OUTPUT_PLUGIN, recorded_transaction);
#endif
  }

  /**
   * The tlm_b_if PV transaction method
   */
  virtual void b_transact(transactionHandle th) {
    //sc_assert(th->getMBurstLength()>0);
    accessHandle ah = _getSlaveAccessHandle(th);

    if (ah->getMCmd() == Generic_MCMD_WR) { 
      if (!slave_port->write(ah->getMData().getData(), 
                             static_cast<gs_uint32>(ah->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                             static_cast<gs_uint32>(ah->getMSBytesValid()))) {
        ah->setSError(GenericError::Generic_Error_AccessDenied);
        SC_REPORT_WARNING(name(), "b_transact() slave_port->write() returned false. Setting SError=Generic_Error_AccessDenied.");
      }
#ifdef GAV_ENABLED
      // TODO: Analysis: record write transaction
      if (record_transactions_enabled) {
        if (record_transaction_master_id_filter == 0
          || record_transaction_master_id_filter == ah->getMID())
          recorded_transaction = *(ah.get());
      }
#endif
    }

    else if (ah->getMCmd() == Generic_MCMD_RD) {  
      if (slave_port->read(ah->getSData().getData(), 
                           static_cast<gs_uint32>(ah->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                           static_cast<gs_uint32>(ah->getMBurstLength()))) {
        ah->setMSBytesValid(static_cast<gs_uint32>(ah->getMBurstLength()));
      }
      else { // error
        ah->setSError(GenericError::Generic_Error_AccessDenied);
        SC_REPORT_WARNING(name(), "b_transact() slave_port->read() returned false. Setting SError=Generic_Error_AccessDenied.");
      }
#ifdef GAV_ENABLED
      // TODO: Analysis: record read transaction
      if (record_transactions_enabled) {
        if (record_transaction_master_id_filter == 0
            || record_transaction_master_id_filter == ah->getMID())
          recorded_transaction = *(ah.get());
      }      
#endif
    }

    else {
      SC_REPORT_WARNING(name(), "b_transact() got unknown MCmd. Ignoring. Setting SError=Generic_Error_AccessDenied.");
      ah->setSError(GenericError::Generic_Error_AccessDenied);
    }    
  }

  void notify(PORT::GS_ATOM& tc){
    m_tcs.push_back(tc);
    startThread.notify();
  }

  /**
   * Play the generic protocol with GreenBus.
   */
  void react() {

    MCmd cmd = Generic_MCMD_IDLE;

    while (1) {
      if (m_tcs.size()==0) sc_core::wait(startThread);
#ifdef USE_STATIC_CASTS
      accessHandle ah = _getSlaveAccessHandle(m_tcs.front());
      phase ph = _getPhase(m_tcs.front());    
#else
      accessHandle ah = m_tcs.front().first;
      phase ph = m_tcs.front().second;
#endif
      
      if (ah->getSError() == GenericError::Generic_Error_NoError && ah->getMError() == GenericError::Generic_Error_NoError) {
        switch(ph.state) {
        case GenericPhase::RequestValid: // master sends request
          {
            cmd = ah->getMCmd();
            if (cmd != Generic_MCMD_WR && cmd != Generic_MCMD_RD) {
              SC_REPORT_WARNING(name(), "react() got unknown MCmd in RequestValid phase. Ignoring. Sending Request Error with SError=Generic_Error_AccessDenied.");
              ah->setSError(GenericError::Generic_Error_AccessDenied);
              PORT::ErrorRequest(ah, ph, PEQ_YIELD, ph.getSimulationMode());
            }
            else {
              PORT::AckRequest(ah, ph, PEQ_YIELD, ph.getSimulationMode());
              if (cmd == Generic_MCMD_RD) { // read request
                GS_TRACE(name(), "react() got RequestValid with MCmd=Generic_MCMD_RD, MBurstLength=%d. Calling slave_port->read().", (gs_uint32)ah->getMBurstLength());
                if (slave_port->read(ah->getSData().getData(), 
                                     static_cast<gs_uint32>(ah->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                                     static_cast<gs_uint32>(ah->getMBurstLength()))) {
                  //ah->setSBurstLength(ah->getMBurstLength());
                  ah->setMSBytesValid(static_cast<gs_uint32>(ah->getMBurstLength()));
                  GS_TRACE(name(), "react() slave_port->read() returned. Sending ResponseValid.");
#ifdef GAV_ENABLED
                  // TODO: Analysis: record read transaction
                  if (record_transactions_enabled) {
                    if (record_transaction_master_id_filter == 0
                        || record_transaction_master_id_filter == ah->getMID())
                      recorded_transaction = *(ah.get());
                  }                    
#endif
                  if (ph.getSimulationMode()==MODE_CT) { // play the CT protocol
                    SC_REPORT_ERROR(name(), "CT mode not implemented.");
                  }
                  else {
                    PORT::Response(ah, ph, PEQ_YIELD, ph.getSimulationMode());
                  }
                }
                else {
                  SC_REPORT_WARNING(name(), "react() slave_port->read() returned false. Sending ResponseValid with SError=Generic_Error_AccessDenied.");
                  ah->setSError(GenericError::Generic_Error_AccessDenied);
                  ah->setMSBytesValid(static_cast<gs_uint32>(ah->getMBurstLength())); //make sure the master knows it's the first and last response phase
                  PORT::Response(ah, ph, PEQ_YIELD, ph.getSimulationMode());
                }
              }
              else { // write command
                GS_TRACE(name(), "react() got RequestValid with MCmd=Generic_MCMD_WR, MBurstLength=%d. Sending AckRequest.", (gs_uint32)ah->getMBurstLength());
              }
            }
          }
          break;
          
        case GenericPhase::DataValid: // master sends data
          {
            if (ah->getMSBytesValid() < ah->getMBurstLength()){
              GS_TRACE(name(), "react() accepts partial data valid phase");
              PORT::AckData(ah, ph, PEQ_YIELD, ph.getSimulationMode());
            }
            else{
              GS_TRACE(name(), "react() got DataValid with MBurstLength=%d. Calling slave_port->write().", (gs_uint32)ah->getMBurstLength());
              if (slave_port->write(ah->getMData().getData(), 
                                    static_cast<gs_uint32>(ah->getMAddr())-static_cast<gs_uint32>(base_addr.value), 
                                    static_cast<gs_uint32>(ah->getMBurstLength()))) {
                //ah->setSBurstLength(ah->getMBurstLength());
                GS_TRACE(name(), "react() slave_port->write() returned. Sending AckData. Write transaction finished.");
#ifdef GAV_ENABLED
                // TODO: Analysis: record write transaction
                if (record_transactions_enabled) {
                  if (record_transaction_master_id_filter == 0
                      || record_transaction_master_id_filter == ah->getMID())
                    recorded_transaction = *(ah.get());
                }                  
#endif
                if (ph.getSimulationMode()==MODE_CT) { // play the CT protocol
                  SC_REPORT_ERROR(name(), "CT mode not implemented.");
                }
                else {
                  PORT::AckData(ah, ph, PEQ_YIELD, ph.getSimulationMode());
                }
              }
              else {
                SC_REPORT_WARNING(name(), "react() slave_port->write() returned false. Sending DataError with SError=Generic_Error_AccessDenied.");
                ah->setSError(GenericError::Generic_Error_AccessDenied);
                PORT::ErrorData(ah, ph, PEQ_YIELD, ph.getSimulationMode());
              }
              cmd = Generic_MCMD_IDLE;
            }
          }
          break;
          
        case GenericPhase::ResponseAccepted: // master acks response
          {
            GS_TRACE(name(), "react() got ResponseAccepted. Read transaction finished.");
            cmd = Generic_MCMD_IDLE;
          }        
          break;
          
        default:
          {
            GS_TRACE(name(), "react() got unknown phase [%s]", ph.toString().c_str());
            SC_REPORT_WARNING(name(), "react() got triggered with unexpected phase. Ignoring.");
          }
        }
      }
      else
        SC_REPORT_WARNING(name(),"Found outstanding transaction container in queue, but had errors. Ignoring.");
      m_tcs.pop_front();      
    }
  }
    
#ifdef GAV_ENABLED
  // ** Analysis: transaction recording
  /// Transaction where the transaction to be recorded are assigned to
  gs::gs_param<gs::cnf::GenericTransactionCopy> recorded_transaction;
  /// If transaction recording is activated for this bus
  gs::gs_param<bool> record_transactions_enabled;
  /// Filter which master's transaction to record (record all if 0)
  gs::gs_param<long> record_transaction_master_id_filter;  // gs_handle
#endif
  
};

/*
class simplebusMasterPortForward: public basic_port_forwarder<simplebusMasterPort, simplebusSlavePort, GenericTransaction, GenericPhase> {
public:
  simplebusMasterPortForward(sc_module_name name): basic_port_forwarder<simplebusMasterPort, simplebusSlavePort, GenericTransaction, GenericPhase>(name){}
};


class simplebusSlavePortForward: public basic_port_forwarder<simplebusSlavePort, simplebusMasterPort, GenericTransaction, GenericPhase> {
public:
  simplebusSlavePortForward(sc_module_name name): basic_port_forwarder<simplebusSlavePort, simplebusMasterPort, GenericTransaction, GenericPhase>(name){}
};
*/

} // namspace tlm

#endif // __simplebusAPI_h__
