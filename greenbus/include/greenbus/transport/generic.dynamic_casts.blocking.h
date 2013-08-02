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

///////////////////////////////////////////////////////////////////////////////
/// \file generic.dynamic_casts.h
/// An implementation of the generic protocol that uses dynamic_casts.
///////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the master.
 * A master can get all  attributes, but can set only master attributes.
 */
//---------------------------------------------------------------------------
#include "generic.dynamic_casts.h"

template <class PORT>
class GenericBlockingAPIBase{
public:
  typedef std::pair<sc_event*, typename PORT::phase> WAITPAYLOAD;
  typedef std::map<unsigned long, typename GenericBlockingAPIBase<PORT>::WAITPAYLOAD> WAITMAP;
  typedef typename std::map<unsigned long, typename GenericBlockingAPIBase<PORT>::WAITPAYLOAD>::iterator WAITID;

  GenericBlockingAPIBase(){}
  virtual ~GenericBlockingAPIBase(){}
  
  std::map<unsigned long, WAITPAYLOAD > waiters;
};

//---------------------------------------------------------------------------
/**
 * This is the generic protocol API implementation.
 * There are two methods available to schedule an atom on the GreenBus (i.e. start a phase):
 * immediately, or delayed notification. Please note, that an 
 * immediate notification overwrites all prior immediate notifications 
 * that have been scheduled in the same delta cycle.
 */
//---------------------------------------------------------------------------
template <class PORT, class operation>
class BNotifications 
  : public Notifications<PORT, operation>
{
  GenericBlockingAPIBase<PORT>& api;
  
  typedef typename Notifications<PORT, operation>::transaction transaction;
  typedef typename Notifications<PORT, operation>::transactionHandle transactionHandle;
  typedef typename Notifications<PORT, operation>::accessHandle accessHandle;
  typedef typename Notifications<PORT, operation>::phase phase;
  
  using Notifications<PORT, operation>::port;

  
public:
  
  BNotifications(PORT &port_, GenericBlockingAPIBase<PORT>& api_) 
    : Notifications<PORT, operation>(port_), 
      api(api_) 
  {}
  
#define BLOCKFRAGMENT1(th, _p, m) \
  sc_event myEvent; \
  transactionHandle t = boost::dynamic_pointer_cast<transaction>(th); \
  operation p(_p); \
  p.setSimulationMode(m); \
  typename PORT::GS_ATOM tc(t,p); \
  typename GenericBlockingAPIBase<PORT>::WAITID waitID=api.waiters.insert( \
    std::pair<unsigned long, typename GenericBlockingAPIBase<PORT>::WAITPAYLOAD> \
    ((unsigned long)t.get(), typename GenericBlockingAPIBase<PORT>::WAITPAYLOAD(&myEvent, phase())) \
    ).first
  
#define BLOCKFRAGMENT2(out) \
  wait(*(waitID->second.first)); \
  out=waitID->second.second; \
  api.waiters.erase(waitID)
              
  void block(accessHandle th, phase _p, phase& out, GenericSimulationModeType m=MODE_BA) 
  {
    BLOCKFRAGMENT1(th, _p, m);
    port.out->notify(tc);
    BLOCKFRAGMENT2(out);
  }
  
  void block(accessHandle th, phase _p, phase& out, const sc_time &d, GenericSimulationModeType m=MODE_BA) 
  {
    BLOCKFRAGMENT1(th, _p, m);
    port.out->notify(tc,d);
    BLOCKFRAGMENT2(out);
  }
  
  void block(accessHandle th, phase _p, phase& out, double d, sc_time_unit u, GenericSimulationModeType m=MODE_BA) 
  {
    BLOCKFRAGMENT1(th, _p, m);
    port.out->notify(tc,d,u);
    BLOCKFRAGMENT2(out);
  }
  
  void block(accessHandle th, phase _p, phase& out, enumPeqDelay d, GenericSimulationModeType m=MODE_BA)
  {
    BLOCKFRAGMENT1(th, _p, m);
    port.out->notify(tc,d);
    BLOCKFRAGMENT2(out);
  }

#define BLOCKFRAGMENT3(th, m) \
  sc_event myEvent; \
  transactionHandle t = boost::dynamic_pointer_cast<transaction>(th); \
  operation p; \
  p.setSimulationMode(m); \
  typename PORT::GS_ATOM tc(t,p); \
  typename GenericBlockingAPIBase<PORT>::WAITID waitID=api.waiters.insert( \
    std::pair<unsigned long, typename GenericBlockingAPIBase<PORT>::WAITPAYLOAD> \
    ((unsigned long)t.get(), typename GenericBlockingAPIBase<PORT>::WAITPAYLOAD(&myEvent, phase())) \
    ).first
            
  void block(accessHandle th, phase& out, GenericSimulationModeType m=MODE_BA) 
  {
    BLOCKFRAGMENT3(th, m);
    port.out->notify(tc);
    BLOCKFRAGMENT2(out);
  }

  void block(accessHandle th, phase& out, const sc_time &d, GenericSimulationModeType m=MODE_BA) 
  {
    BLOCKFRAGMENT3(th, m);
    port.out->notify(tc,d);
    BLOCKFRAGMENT2(out);
  }
  
  void block(accessHandle th, phase& out, double d, sc_time_unit u, GenericSimulationModeType m=MODE_BA) 
  {
    BLOCKFRAGMENT3(th, m);
    port.out->notify(tc,d,u);
    BLOCKFRAGMENT2(out);
  }
  
  void block(accessHandle th, phase& out, enumPeqDelay d, GenericSimulationModeType m=MODE_BA)
  {
    BLOCKFRAGMENT3(th, m);
    port.out->notify(tc,d);
    BLOCKFRAGMENT2(out);
  }
  
};


//---------------------------------------------------------------------------
/**
 * The API for the generic protocol that can be accessed by the initiator port.
 */
//---------------------------------------------------------------------------
template <class PORT>
class GenericInitiatorBlockingAPI :
  public PORT,
  public payload_event_queue_output_if<typename PORT::GS_ATOM>,
  protected GenericBlockingAPIBase<PORT>
{
  
public:
  
  GenericInitiatorBlockingAPI ( sc_module_name port_name ) :
    PORT ( port_name ),
    Repass(*this, *this),
    Idle(*this, *this),
    Request(*this, *this),
    SendData(*this, *this),
    AckResponse(*this, *this),
    ErrorResponse(*this, *this)
  {
    PORT::peq.out_port(*this);
  }
  
  sc_port<payload_event_queue_output_if<typename PORT::GS_ATOM> > out_port;

  /// hold current phase 
  BNotifications<PORT, noChangePhase>                     Repass;
  /// master is idle, no active communication
  BNotifications<PORT, IdlePhase>                         Idle;
  /// master sends a request atom
  BNotifications<PORT, setRequestValid_BurstNumber0Phase> Request;
  /// master sends a data atom
  BNotifications<PORT, setDataValid_BurstUpdatePhase>     SendData;
  /// master acknowledges a slave response
  BNotifications<PORT, setResponseAcceptedPhase>          AckResponse;
  /// master signals an error on a slave response
  BNotifications<PORT, setResponseErrorPhase>             ErrorResponse;
  
  
  typedef typename PORT::transaction transaction;
  typedef typename PORT::accessHandle accessHandle;
  typedef typename PORT::phase phase;

  /**
   * PV transact method. It is used in PV mode 
   * to send a whole transaction container at once.
   * Please note that this method is blocking.
   */
  void Transact(accessHandle t)
  {
    PORT::b_out->b_transact(boost::dynamic_pointer_cast<transaction>(t)); 
    // cast from base class ACCESS to derived class TRANSACTION
  }
  
  void notify(typename PORT::GS_ATOM& tc){
    typename GenericBlockingAPIBase<PORT>::WAITID id=GenericBlockingAPIBase<PORT>::waiters.find((unsigned long)tc.first.get());
    //maybe we could check for a bool here if there is a waiter at all... or maybe check waiters.size()
    if (id!=GenericBlockingAPIBase<PORT>::waiters.end()){
      id->second.second=tc.second;
      id->second.first->notify(SC_ZERO_TIME);
    }
    else {
      out_port->notify(tc);
    }
  }
};

  
  //---------------------------------------------------------------------------
  /**
   * The API for the generic protocol that can be accessed by the target port.
   */
  //---------------------------------------------------------------------------
  template <class PORT>
  class GenericTargetBlockingAPI :
    public PORT,
    public payload_event_queue_output_if<typename PORT::GS_ATOM>,
    protected GenericBlockingAPIBase<PORT>
  {

  public:

    GenericTargetBlockingAPI ( sc_module_name port_name ) :
      PORT ( port_name ),
      Repass(*this, *this),
      Idle(*this, *this),
      AckRequest(*this, *this),
      ErrorRequest(*this, *this),
      AckData(*this, *this),
      ErrorData(*this, *this),
      Response(*this, *this)
    {
      PORT::peq.out_port(*this);
    }
  
    sc_port<payload_event_queue_output_if<typename PORT::GS_ATOM> > out_port;
    

    /// hold current phase (no change)
    BNotifications<PORT, noChangePhase>            Repass;
    /// slave is idle, no active communication
    BNotifications<PORT, IdlePhase>                Idle;
    /// slave acknowledges a request atom
    BNotifications<PORT, setRequestAcceptedPhase>  AckRequest;
    /// slave replies an error to a request atom
    BNotifications<PORT, setRequestErrorPhase>     ErrorRequest;
    /// slave acknowledges data atom
    BNotifications<PORT, setDataAcceptedPhase>     AckData;
    /// slave replies an error to a data atom
    BNotifications<PORT, setDataErrorPhase>        ErrorData;
    /// slave sends a response atom
    BNotifications<PORT, setResponseValidPhase>    Response;
    
    void notify(typename PORT::GS_ATOM& tc){
      typename GenericBlockingAPIBase<PORT>::WAITID id=GenericBlockingAPIBase<PORT>::waiters.find((unsigned long)tc.first.get());
      //maybe we could check for a bool here if there is a waiter at all... or maybe check waiters.size()
      if (id!=GenericBlockingAPIBase<PORT>::waiters.end()){
        id->second.second=tc.second;
        id->second.first->notify(SC_ZERO_TIME);
      }
      else
        out_port->notify(tc);
    }

  };


  //---------------------------------------------------------------------------
  /**
   * The default master initiator port.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef GenericInitiatorBlockingAPI<master_port<PCIeTransaction,PCIeMasterAccess,PCIePhase> > GenericMasterBlockingPort;
#else
  typedef GenericInitiatorBlockingAPI<master_port<GenericTransaction,GenericMasterAccess,GenericPhase> > GenericMasterBlockingPort;
# define GenericInitiatorBlockingPort GenericMasterBlockingPort
#endif

  //---------------------------------------------------------------------------
  /**
   * The default router initiator port.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef GenericInitiatorBlockingAPI<initiator_port<PCIeTransaction,PCIeRouterAccess,PCIePhase> > GenericRouterBlockingPort;
#else
  typedef GenericInitiatorBlockingAPI<initiator_port<GenericTransaction,GenericRouterAccess,GenericPhase> > GenericRouterBlockingPort;
#endif

  //---------------------------------------------------------------------------
  /**
   * The generic slave port.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef GenericTargetBlockingAPI<slave_port<PCIeTransaction,PCIeSlaveAccess,PCIePhase> > GenericSlaveBlockingPort;
#else
  typedef GenericTargetBlockingAPI<slave_port<GenericTransaction,GenericSlaveAccess,GenericPhase> > GenericSlaveBlockingPort;
# define GenericTargetBlockingPort GenericSlaveBlockingPort
#endif
