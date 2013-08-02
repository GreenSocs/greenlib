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

#ifndef __ship_slaveport_h__
#define __ship_slaveport_h__

#include "ship_serializable_if.h"
#include "ship_datatypes.h"


namespace tlm {



//---------------------------------------------------------------------------
/**
 * This is the SHIP slave port.
 * Use base_addr and high_addr parameter to configure the  address range if
 * connected to a router.
 * Use mode parameter to configure operation mode of this port (default is SHIP_MODE_PV).
 */
//---------------------------------------------------------------------------
template <class T>
class shipSlaveAPI
: public GenericSlaveBlockingPort,
  public tlm_b_if<GenericTransactionHandle>
{     
public:
  typedef GenericSlaveBlockingPort PORT;
  typedef typename PORT::transaction transaction;
  typedef typename PORT::transactionHandle transactionHandle;
  typedef typename PORT::accessHandle accessHandle;
  typedef typename PORT::phase phase;
  
protected:
    class notifyBinder : public payload_event_queue_output_if<PORT::GS_ATOM>{
public:
      notifyBinder(shipSlaveAPI& owner_) : owner(owner_){}
      void notify(PORT::GS_ATOM& tc){
        owner.react(tc);
      }
private:
      shipSlaveAPI& owner;
    };
  
  notifyBinder mBinder;
  
public:  

  // configurable parameters
  gb_param<gs_uint32> mode;
  gb_param<gs_uint32> clk_period;


  SC_HAS_PROCESS(shipSlaveAPI);


  /**
   * Create a SHIP slave port. Use configuration framework
   * to set address raneg (base_addr, high_addr) and operation mode (mode).
   */
  shipSlaveAPI ( sc_module_name port_name ) : 
    PORT(port_name),
    mBinder(*this),
    mReplyObj(NULL),
    mVector(NULL)
  {
    // DUST structure analysis
#ifdef USE_DUST    
    DUST_SLAVE_PORT("ShipSlaveAPI", "SHIP");
#endif
    
    PORT::bind_b_if(*this); 
    PORT::out_port(mBinder);

    mCmd.cmd = SHIP_NONE;
    mCmd.burstlength = 0;

    mVector = new std::vector<gs_uint8>();

    GB_PARAM(mode, gs_uint32, SHIP_MODE_BA_C); // default is PV
    GB_PARAM(clk_period, gs_uint32, 10); // default clock period is 10ns (not used in PV mode)
  }

  
  /// Receive a SHIP object from a master 
  /**
   * The recv method copies the last received
   * SHIP object into a user SHIP object.
   * @param obj A reference to a SHIP object. The received
   *            object will be copied into this object.
   */
  void recv(T& obj) {
    if (mCmd.cmd != SHIP_SEND) {
      SC_REPORT_ERROR(name(), "recv() called, but not in RECV state");
      return;
    }

    if (mObj != NULL) { // PV mode
      // we received a pointer to the master's SHIP object, 
      // so do a direct copy
      obj = *mObj; // mObj has already been set by b_transact
      mTransactEvent.notify(); // notify b_transact
    }

    else { // PVT mode
      GSDataType data;
      data.set(mah->getMData());

      if (data.isPointer()) { // master uses BA performance mode
        obj = *(static_cast<T*>(data.getPointer()));
      }

      else { // received serialized SHIP object
        obj.deserialize(data); // deserialize SHIP object from received byte array        
      }
      
      PORT::AckData(mah, mph, SC_ZERO_TIME, mph.getSimulationMode());
    }

    mCmd.cmd = SHIP_NONE; // idle again    

    GS_TRACE(name(), "recv() deliverd a SHIP object.");
  }
  

  ~shipSlaveAPI() {
    if (mVector != NULL)
      delete mVector;
  }


  /// Send a SHIP object to a master in answer to a request
  /**
   * The reply method sends a SHIP object to a master 
   * in answer to a SHIP request.
   * @param obj The object to send to the master.
   */
  void reply(T& obj) {
    sc_assert(mRequestHandle != NULL);
    mCmd.burstlength = obj.getSerialLength();

    unsigned int objbytes = obj.getSerialLength()*8;

    if (mode == SHIP_MODE_PV) { // PV bypass mode 
      // create local copy of reply data, so that the master can access the data safely until the next request() invocation
      if (mReplyObj != NULL)
        delete mReplyObj;
      mReplyObj = new T(obj);

      mRequestHandle->setMSBytesValid(objbytes); 
      GSDataType data;
      data.setPointer(mReplyObj);
      mRequestHandle->setSData(data);
      //GS_TRACE(name(), "reply() set data pointer to local SHIP object in transaction container.");
      mCmd.cmd = SHIP_NONE;
      mTransactEvent.notify(); // notify b_transact      
    }

    else { // PVT transaction
      // map SHIP object onto transaction container
      if (mph.getSimulationMode()==MODE_BA && mode==SHIP_MODE_BA_P) { // BA performance mode
        mah->getSData().setPointer(static_cast<void*>(&obj));
      }
      else { // use serialization
        mVector->empty();
        mah->getSData().setData(*mVector); // TODO: should be mah->setSData(*mVector), but that does a deepcopy, which is not necessary here
        obj.serialize(mah->getSData());
      }
      
      mah->setMSBytesValid(objbytes);
        
      // response phase
      GS_TRACE(name(), "reply() sends response with data size %d.", objbytes);
      
      PORT::Response.block(mah, mph); // TODO: does not support "partial" AckResponse
      
      if (mph.isResponseAccepted()) {
        GS_TRACE(name(), "reply() response has been accepted by the master.", objbytes);
      } else {
        char ch[1024];
        sprintf(ch, "Slave response was not acknowledged by the master - got phase=%s (%d) - an error occured.", mph.toString().c_str(), mph.state);
        SC_REPORT_ERROR(name(), ch);
      }

      mCmd.cmd = SHIP_NONE;
    }
  }

  
  /// Wait for commands from a master
  /**
   * For each SHIP slave port, a SC_THREAD has to be implemented
   * which handles commands from the SHIP master connected
   * to this slave port. This can be performed by calling the
   * waitEvent method, which does not return unless a SHIP command
   * has been received. 
   * 
   * A typical implementation would look like this:
   * <pre>
   * ship_command comm;
   * while(1) {
   *   comm = slave_port->waitEvent();
   *   switch(comm.cmd) {
   *     case SHIP_SEND:
   *       // receive data using slave_port->recv(...)
   *       break;
   *     case SHIP_REQUEST:
   *       // send data using slave_port->reply(...)
   *       break;
   *     default:
   *       cout << "Unknown ship_command: " << comm.cmd << endl;
   *   }
   * }
   * </pre>
   * 
   * @return The return value is a ship_command-structure.
   * @see ship_command
   */
  ship_command waitEvent() {
    if (mCmd.cmd != SHIP_NONE) // return immediately if we are inside a transaction
      return mCmd;
    else {  
      do {  // wait for something to happen        
        sc_core::wait(mEvent); 
      } while(mCmd.cmd == SHIP_NONE);
      return mCmd;
    }
  }

  
  
  
protected:
  
  /**
   * Play the PVT protocol with the master.
   */
  virtual void react(GS_ATOM &tc) {
    mah = _getSlaveAccessHandle(tc);
    mph = _getPhase(tc);
    PVTProcess();
  }

  void PVTProcess() {
    switch (mph.state) {
    case GenericPhase::RequestValid: // master sends request
      {
        if (mCmd.cmd != SHIP_NONE) {
          SC_REPORT_ERROR(name(), "react() got request, but is already handling another transaction.");
        }
        else {
          PORT::AckRequest(mah, mph, SC_ZERO_TIME, mph.getSimulationMode()); // ack master request and simulation mode
          
          if (mah->getMCmd() == Generic_MCMD_WR) {
            mCmd.burstlength = mah->getMBurstLength();
            GS_TRACE(name(), "react() accepted write request, burstlength=%d. Now waiting for master data.", mCmd.burstlength);
            mObj = NULL; // indicate PVT mode transaction
          }
          else if (mah->getMCmd() == Generic_MCMD_RD) {
            GS_TRACE(name(), "react() accepted read request. Now waiting for slave to call reply().");
            mCmd.cmd = SHIP_REQUEST;
            mCmd.burstlength = 0;
            mObj = NULL; // indicate PVT mode transaction
            mRequestHandle = mah;
            mEvent.notify();
          }
          else {
            SC_REPORT_WARNING(name(), "react() got unknown request. Ignoring.");
          }
        }
      }
      break;

    case GenericPhase::DataAccepted: // master acknowledges data (should this happen??)
      {
        SC_REPORT_ERROR(name(), "react() got DataAccepted. This should not happen!?");
      }
      break;

    case GenericPhase::DataValid: // master sends data
      {
        if (mah->getMSBytesValid()<mah->getMBurstLength()) {
	        GS_TRACE(name(), "react() got some bytes of data. There is more to come...");
  	      PORT::AckData(mah, mph, SC_ZERO_TIME, mph.getSimulationMode());
        }
        else {
	        GS_TRACE(name(), "react() got %d bytes of data. Now waiting for slave to call recv().", (int)mah->getMSBytesValid());	
  	      // notify waitEvent() method
          mCmd.cmd = SHIP_SEND;
    	    mEvent.notify();
        }
      }
      break;


    case GenericPhase::ResponseAccepted: // master acknowledges response
      {
        GS_TRACE(name(), "react() got ResponseAccepted. SHIP transaction was finished OK.");
      }
      break;

    default:
      {
        GS_TRACE(name(), "react() got unknown phase [%s]", mph.toString().c_str());
        SC_REPORT_WARNING(name(), "react() got triggered with unexpected phase. Ignoring.");
      }
    }
  }



  /**
   * The tlm_b_if PV transaction implementation
   */
  virtual void b_transact(transactionHandle th) {
    sc_assert(mCmd.cmd == SHIP_NONE);

    accessHandle ah = _getSlaveAccessHandle(th);

    if (ah->getMCmd() == Generic_MCMD_WR) { // master sends data
      mObj = static_cast<T*>(ah->getMData().getPointer());      
      mCmd.cmd = SHIP_SEND;
      mCmd.burstlength = mObj->getSerialLength();
      
      GS_TRACE(name(), "b_transact() PV-received a SHIP object of size %d. Now waiting for slave to call recv().", (gs_uint32)ah->getMBurstLength());

      // notify waitEvent method
      mEvent.notify();

      // wait for finalization of this request by the slave
      sc_core::wait(mTransactEvent);
    }

    else if (ah->getMCmd() == Generic_MCMD_RD) { // master requests data
      mCmd.cmd = SHIP_REQUEST;
      mCmd.burstlength = ah->getMBurstLength(); // should be 0

      // save transaction container for reply method
      mRequestHandle = _getSlaveAccessHandle(th);

      GS_TRACE(name(), "b_transact() PV-received a SHIP request. Now waiting for slave to call request().");

      // notify waitEvent method
      mEvent.notify();

      // wait for finalization of this request by the slave
      sc_core::wait(mTransactEvent);
    }
  }


  void operator() (tlm_port_forwarder_base<b_if_type,if_type>& other) {
    PORT::operator()(other);
  }

  
protected:
  /// The current ship object
  T *mObj, *mReplyObj;
  /// State change event
  sc_event mEvent, mTransactEvent, mAckResponseEvent;
  /// The current state
  ship_command mCmd;
  /// Handle to the current transaction
  accessHandle mRequestHandle;
  /// Serialization buffer
  std::vector<gs_uint8> *mVector;
  /// The current transcation container
  accessHandle mah;
  /// The current phase
  phase mph;
};


} // namespace tlm


#endif
