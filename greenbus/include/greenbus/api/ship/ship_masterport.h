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

#ifndef __ship_masterport_h__
#define __ship_masterport_h__

#include "ship_serializable_if.h"
#include "ship_datatypes.h"

namespace tlm {


//---------------------------------------------------------------------------
/**
 * This is the SHIP master port.
 * Use target_addr parameter to configure the target address if
 * connected to a router.
 * Use mode parameter to configure operation mode of this SHIP port.
 *
 * SHIP is an asynchronous communication protocol. Send places
 * a data packet onto the channel and returns immediately if the
 * channel is free. Otherwise, send blocks until the channel becomes
 * free again. The slave can get an event from the shipSlaveAPI
 * to listen for master activity. 
 */
//---------------------------------------------------------------------------
template <class T>
class shipMasterAPI
  : public GenericMasterBlockingPort
{  
public:
  typedef GenericMasterBlockingPort PORT;

  typedef typename PORT::transaction transaction;
  typedef typename PORT::transactionHandle transactionHandle;
  typedef typename PORT::accessHandle accessHandle;
  typedef typename PORT::phase phase;
  
protected:
  class notifyBinder : public payload_event_queue_output_if<PORT::GS_ATOM>{
public:
    notifyBinder(shipMasterAPI& owner_) : owner(owner_){}
    void notify(PORT::GS_ATOM& tc){
      owner.react(tc);
    }
private:
    shipMasterAPI& owner;
  };
  
  notifyBinder mBinder;
  
public:
  /// the operation mode of this SHIP port
  gb_param<gs_uint32> mode;
  /// the clock period of this SHIP port
  gb_param<gs_uint32> clk_period;


  SC_HAS_PROCESS(shipMasterAPI);

  /**
   * Create a SHIP master port. Use configuration framework
   * to set parameters target_addr and mode.
   */
  shipMasterAPI ( sc_module_name port_name ) 
    : PORT(port_name),
      mBinder(*this),
      mVector(NULL)
  {
    // DUST structure analysis
#ifdef USE_DUST
    DUST_MASTER_PORT("ShipMasterAPI", "SHIP");
#endif
        
    // bind PEQ output port
    PORT::out_port(mBinder);

    mCmd.cmd = SHIP_NONE;

    mVector = new std::vector<gs_uint8>();

    GB_PARAM(mode, gs_uint32, SHIP_MODE_BA_C); // default mode is PV
    GB_PARAM(clk_period, gs_uint32, 10); // default clock period is 10ns (not used in PV mode)
  }

 
  /// Send a SHIP object to a slave
  /**
   * The send method sends a SHIP object to a SHIP slave.
   * Communication using this method is blocking, thus 
   * it will not return until the SHIP object has been 
   * completely delivered to the slave.
   *
   * @param obj The object to be send.
   */
  void send( T& obj ) {
    sc_assert(mCmd.cmd == SHIP_NONE); // multi-threading not supported
    
    mCmd.cmd = SHIP_SEND;
    
    mah = PORT::create_transaction();
    
    GSDataType data(*mVector);

    unsigned int objbytes = obj.getSerialLength();
    
    mCmd.burstlength = objbytes; 

    mah->setMAddr(target_addr);
    mah->setMBurstLength(objbytes); 
    mah->setMSBytesValid(objbytes);
    mah->setMCmd(Generic_MCMD_WR);

    if (mode==SHIP_MODE_PV) { // PV bypass mode 
      data.setPointer(static_cast<void*>(&obj));
      mah->setMData(data); // pass SHIP object by pointer
      GS_TRACE(name(), "send() calls PV Transact.");
      PORT::Transact(mah);
    }

    else { // PVT mode
      // fill transaction container with transaction data
      if (mode==SHIP_MODE_BA_P) { // BA performance mode
        data.setPointer(static_cast<void*>(&obj)); 
        mah->setMData(data); // pass SHIP object by pointer
      }
      else { // BA or CT mode
        obj.serialize(data); // serialize SHIP object
        mah->setMData(data); // pass SHIP object by value
      }
      
      // request phase
      bool success=false;
      while(!success) {
        GS_TRACE(name(), "send() starts write request with burstlength=%d.", objbytes);
        PORT::Request.block(mah, mph);
        
        if (mph.state == GenericPhase::RequestAccepted) {
          GS_TRACE(name(), "send() write request has been accepted by the slave.");
          success = true;
        } else {
          char ch[1024];
          sprintf(ch, "A write request was not acknowledged by the slave - got phase=%s (%d) - maybe a timeout occured. Sending request again.", mph.toString().c_str(), mph.state);
          SC_REPORT_WARNING(name(), ch);
        }
      }
      
      // data phase 
      PORT::SendData.block(mah, mph, mph); 

      if (mph.state == GenericPhase::DataAccepted) {
        GS_TRACE(name(), "send() write data has been accepted by the slave.");
      } else {
        char ch[1024];
        sprintf(ch, "Master data was not acknowledged by the slave - got phase=%s (%d) - a severe error occured.", mph.toString().c_str(), mph.state);
        SC_REPORT_ERROR(name(), ch);
      }
      
    }
    mCmd.cmd = SHIP_NONE;
    GS_TRACE(name(), "send returns.");
  }

  ~shipMasterAPI() {
    if (mVector != NULL)
      delete mVector;
  }
  

  /// Request a SHIP object from a slave
  /**
   * The request method requests a SHIP object from a SHIP slave.
   * Communication using this method is blocking, thus the request
   * method will not return unless the SHIP object
   * has been completely received.
   * @param obj A reference to an SHIP object, into which 
   *            the requested data will be copied.
   */
  void request( T& obj ) {
		sc_assert(mCmd.cmd == SHIP_NONE); // multi-threading not supported
    
    mCmd.cmd = SHIP_REQUEST;

    accessHandle mah = PORT::create_transaction();

    mah->setMCmd(Generic_MCMD_RD);
    mah->setMAddr(target_addr);
    mah->setMBurstLength(obj.getSerialLength());

    mCmd.burstlength = obj.getSerialLength();

    if (mode==SHIP_MODE_PV) { // PV bypass mode 
      GS_TRACE(name(), "request() calls PV b_transact.");
      PORT::Transact(mah);
      
      // now lets look what we have received
      GSDataType data;
      data.set(mah->getMData());
      sc_assert(data.isPointer());
      T *robj = static_cast<T*>(data.getPointer()); 
      obj = *robj; // copy received SHIP object to master
    }

    else { // PVT modes
      bool success=false;
      while(!success) {
        GS_TRACE(name(), "request() starts request phase.");
        PORT::Request.block(mah, mph);
        
        if (mph.state == GenericPhase::RequestAccepted) {
          GS_TRACE(name(), "request() read request has been accepted by the slave, burstlength=%d.", mCmd.burstlength);
          success = true;
        } else {
          char ch[1024];
          sprintf(ch, "A read request was not acknowledged by the slave - got phase=%s (%d) - maybe a timeout occured. Sending request again.", mph.toString().c_str(), mph.state);
          SC_REPORT_WARNING(name(), ch);
        }
      }

      bool done=false;
      GSDataType data;

      while(!done) {
	      // now wait for slave response (will be received by react method)
  	    sc_core::wait(mResponseEvent);
        PORT::AckResponse(mah, mph, SC_ZERO_TIME, mph.getSimulationMode());         
        if (mah->getMSBytesValid()==mah->getMBurstLength())
          done = true;
      }

   	  // check the received data
      data.set(mah->getMData());

 	    if (data.isPointer()) { // slave uses BA performance mode and sent just a pointer
        T *robj = static_cast<T*>(data.getPointer()); 
      	 obj = *robj; // copy received SHIP object to master
        mah->setMBurstLength(obj.getSerialLength()); // ack complete reception
      }

	    else { // slave sent a serialized SHIP object
  	    obj.deserialize(data); // deserialize received data
    	}            
    }
      
    GS_TRACE(name(), "request() received slave response OK.");
    
    mCmd.cmd = SHIP_NONE;
  }


  /// Wrapper for bind operator
  void operator() (tlm_port<b_if_type,if_type>& other) {
    PORT::operator()(other);
  }
  
  /// Wrapper for bind operator
  void operator() (tlm_multi_port<b_if_type,if_type>& other) {
    PORT::operator()(other);
  }
  
  void operator() (tlm_port_forwarder_base<b_if_type,if_type>& other) {
    PORT::operator()(other);
  }


protected:
  /**
   * Play the PVT protocol with the slave
   */
  virtual void react(GS_ATOM &tc) {
    mah = _getMasterAccessHandle(tc);
    mph = _getPhase(tc);
    PVTProcess();
  }

  void PVTProcess() { 
    switch (mph.state) {
    case GenericPhase::ResponseValid: // slave sends response
      {
        GS_TRACE(name(), "react() received slave response (%d bytes).", (int)mah->getMSBytesValid()-mCmd.burstlength);        
        mCmd.burstlength = mah->getMSBytesValid();
        // notify request() method
        mResponseEvent.notify();
      }
      break;

    default:
      break; // do nothing
    }
  }


protected:
  /// The current SHIP command
  ship_command mCmd;
  /// Internal notification
  sc_event mResponseEvent;
  /// Serialization buffer
  std::vector<gs_uint8> *mVector;
  /// The current transcation container
  accessHandle mah;
  /// The current phase
  phase mph;
};

} // namespace tlm

#endif
