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
/// \file generic.static_casts.h
/// An implementation of the generic protocol that uses static_casts.
/// Gives up to 50% speed up over dynamic_cast version.
///////////////////////////////////////////////////////////////////////////////

namespace tlm{ //reopen tlm namespace

inline gp::GenericRouterAccess& gp::GenericTransaction::getRouterAccess() {
  return *(this->getSuperClassPtr());
}

/// Get the master access set to this transaction
inline gp::GenericMasterAccess& gp::GenericTransaction::getMasterAccess() {
  return *(this->getSuperClassPtr());//reinterpret_cast<GenericMasterAccess&>(*this);
}

/// Get the target access set to this transaction
inline gp::GenericSlaveAccess& gp::GenericTransaction::getTargetAccess() {
  return *(this->getSuperClassPtr());//reinterpret_cast<GenericSlaveAccess&>(*this);
}

/**
 * Helper function to get access to the transaction from an access handle.
 */
template <class T>
inline GenericTransactionHandle _getTransactionAccess(const T &a) {
  //tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(a); // simulate reinterpret_cast
  return a;
}

/**
 * Helper function to get master access handle from a GS_ATOM 
 */
inline tlm::shared_ptr<GenericMasterAccess> _getMasterAccessHandle(const GS_ATOM &tc) {
  //tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(tc.first); // simulate reinterpret_cast
  return tlm::static_access_down_cast<GenericMasterAccess>(tc.first);//tlm::static_pointer_cast<GenericMasterAccess>(im);
}
  
/**
 * Helper function to get slave access handle from a GS_ATOM 
 */
inline tlm::shared_ptr<GenericSlaveAccess> _getSlaveAccessHandle(const GS_ATOM &tc) {
  //tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(tc.first); // simulate reinterpret_cast
  //return tlm::static_pointer_cast<GenericSlaveAccess>(im);
  return tlm::static_access_down_cast<GenericSlaveAccess>(tc.first);
}

/**
 * Helper function to get router access handle from a GS_ATOM 
 */
inline tlm::shared_ptr<GenericRouterAccess> _getRouterAccessHandle(const GS_ATOM &tc) {
  //tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(tc.first); // simulate reinterpret_cast
  //return tlm::static_pointer_cast<GenericRouterAccess>(im);
  return tlm::static_access_down_cast<GenericRouterAccess>(tc.first);
}

/**
 * Helper function to get the phase from a GS_ATOM
 */
inline GenericPhase _getPhase(const GS_ATOM &tc) { return tc.second; }

/**
 * Helper function to get master access handle from a GenericTransaction handle (to be used in b_transact)
 */
inline tlm::shared_ptr<GenericMasterAccess> _getMasterAccessHandle(const GenericTransactionHandle &t) {
  //tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(t); // simulate reinterpret_cast
  //return tlm::static_pointer_cast<GenericMasterAccess>(im);
  return tlm::static_access_down_cast<GenericMasterAccess>(t);
}
  
/**
 * Helper function to get slave access handle from a GenericTransaction handle (to be used in b_transact)
 */
inline tlm::shared_ptr<GenericSlaveAccess> _getSlaveAccessHandle(const GenericTransactionHandle &t) {
 // tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(t); // simulate reinterpret_cast
  //return tlm::static_pointer_cast<GenericSlaveAccess>(im);
  return tlm::static_access_down_cast<GenericSlaveAccess>(t);
}

/**
 * Helper function to get router access handle from a GenericTransaction handle (to be used in b_transact)
 */
inline tlm::shared_ptr<GenericRouterAccess> _getRouterAccessHandle(const GenericTransactionHandle &t) {
  //tlm::shared_ptr<void> im = tlm::static_pointer_cast<void>(t); // simulate reinterpret_cast
  //return tlm::static_pointer_cast<GenericRouterAccess>(im);
  return tlm::static_access_down_cast<GenericRouterAccess>(t);
}







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
class Notifications
{
protected:
  PORT& port;
  
public:
  
  Notifications(PORT &port_) : port(port_) {}
  
  typedef typename PORT::transaction transaction;
  typedef typename PORT::transactionHandle transactionHandle;
  typedef typename PORT::accessHandle accessHandle;
  typedef typename PORT::phase phase;
  
  void operator()(accessHandle th, phase _p, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc);
  }
  
  void operator()(accessHandle th, phase _p, const sc_time &d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc,d);
  }
  
  void operator()(accessHandle th, phase _p, double d, sc_time_unit u, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc,d,u);
  }

  void operator()(accessHandle th, phase _p, enumPeqDelay d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
  
 
  void operator()(accessHandle th, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc);
  }
  
  void operator()(accessHandle th , const sc_time &d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
  
  void operator()(accessHandle th , double d, sc_time_unit u, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc,d,u);
  }
 
  void operator()(accessHandle th , enumPeqDelay d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = _getTransactionAccess(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
 
 
};


// Functor to be used with Notifications
struct  noChangePhase : public GenericPhase
{
  noChangePhase( const GenericPhase & p) :GenericPhase(p.state,p.BurstNumber){ };
};

// Functor to be used with Notifications
struct IdlePhase : public GenericPhase
{
  IdlePhase( const GenericPhase& p) :GenericPhase(tlm::gp::GenericPhase::Idle){};
  IdlePhase( ) :GenericPhase(tlm::gp::GenericPhase::Idle){};
};

// Functor to be used with Notifications
struct setRequestValid_BurstNumber0Phase : public GenericPhase
{
  setRequestValid_BurstNumber0Phase( const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::RequestValid,0) { };
  setRequestValid_BurstNumber0Phase() : GenericPhase(tlm::gp::GenericPhase::RequestValid,0) { };
};

// Functor to be used with Notifications
struct setRequestAcceptedPhase : public GenericPhase
{
  setRequestAcceptedPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::RequestAccepted)      {      };
  setRequestAcceptedPhase() : GenericPhase(tlm::gp::GenericPhase::RequestAccepted)      {      };
};

// Functor to be used with Notifications
struct setRequestErrorPhase : public GenericPhase
{
  setRequestErrorPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::RequestError)      {      };
  setRequestErrorPhase() : GenericPhase(tlm::gp::GenericPhase::RequestError)      {      };
};

// Functor to be used with Notifications
struct setDataValid_BurstUpdatePhase : public GenericPhase
{
  setDataValid_BurstUpdatePhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::DataValid, p.BurstNumber+1)      {      };
};

// Functor to be used with Notifications
struct setDataAcceptedPhase : public GenericPhase
{
  setDataAcceptedPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::DataAccepted,p.BurstNumber)      {      };
};

// Functor to be used with Notifications
struct setDataErrorPhase : public GenericPhase
{
  setDataErrorPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::DataError,p.BurstNumber)      {      };
};
    
// Functor to be used with Notifications
struct setResponseValidPhase : public GenericPhase
{
  setResponseValidPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::ResponseValid)      {      };
  setResponseValidPhase() : GenericPhase(tlm::gp::GenericPhase::ResponseValid)      {      };
};


// Functor to be used with Notifications
struct setResponseAcceptedPhase : public GenericPhase
{
  setResponseAcceptedPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::ResponseAccepted)      {      };
  setResponseAcceptedPhase() : GenericPhase(tlm::gp::GenericPhase::ResponseAccepted)      {      };
};

// Functor to be used with Notifications
struct setResponseErrorPhase : public GenericPhase
{
  setResponseErrorPhase(const GenericPhase & p) : GenericPhase(tlm::gp::GenericPhase::ResponseError)      {      };
  setResponseErrorPhase() : GenericPhase(tlm::gp::GenericPhase::ResponseError)      {      };
};



//---------------------------------------------------------------------------
/**
 * The API for the generic protocol that can be accessed by the target port.
 */
//---------------------------------------------------------------------------
template <class PORT>
class GenericTargetAPI :
  public PORT
{ 
public:

  GenericTargetAPI ( sc_module_name port_name ) :
    PORT ( port_name ),
    Repass(*this),
    Idle(*this),
    AckRequest(*this),
    ErrorRequest(*this),
    AckData(*this),
    ErrorData(*this),
    Response(*this)
  {}
  
  
  /// hold current phase (no change)
  Notifications<PORT, noChangePhase>            Repass;
  /// slave is idle, no active communication
  Notifications<PORT, IdlePhase>                Idle;
  /// slave acknowledges a request atom
  Notifications<PORT, setRequestAcceptedPhase>  AckRequest;
  /// slave replies an error to a request atom
  Notifications<PORT, setRequestErrorPhase>     ErrorRequest;
  /// slave acknowledges data atom
  Notifications<PORT, setDataAcceptedPhase>     AckData;
  /// slave replies an error to a data atom
  Notifications<PORT, setDataErrorPhase>        ErrorData;
  /// slave sends a response atom
  Notifications<PORT, setResponseValidPhase>    Response;
};


//---------------------------------------------------------------------------
/**
 * The generic slave port.
 */
//---------------------------------------------------------------------------
typedef GenericTargetAPI<slave_port<GenericTransaction,GenericSlaveAccess,GenericPhase> > GenericSlavePort;
# define GenericTargetPort GenericSlavePort


//---------------------------------------------------------------------------
/**
 * The API for the generic protocol that can be accessed by the initiator port.
 */
//---------------------------------------------------------------------------
template <class PORT>
class GenericInitiatorAPI :
  public PORT
{
  
public:
  typedef typename PORT::transactionHandle transactionHandle;
  
  /// if initiator should be able to receive transactions (as a slave) set is_bidirectional true
  GenericInitiatorAPI ( sc_module_name port_name, bool is_bidirectional = false ) :
    PORT ( port_name, is_bidirectional ),
    Repass(*this),
    Idle(*this),
    Request(*this),
    SendData(*this),
    AckResponse(*this),
    ErrorResponse(*this)
  {}

  /// hold current phase 
  Notifications<PORT, noChangePhase>                     Repass;
  /// master is idle, no active communication
  Notifications<PORT, IdlePhase>                         Idle;
  /// master sends a request atom
  Notifications<PORT, setRequestValid_BurstNumber0Phase> Request;
  /// master sends a data atom
  Notifications<PORT, setDataValid_BurstUpdatePhase>     SendData;
  /// master acknowledges a slave response
  Notifications<PORT, setResponseAcceptedPhase>          AckResponse;
  /// master signals an error on a slave response
  Notifications<PORT, setResponseErrorPhase>             ErrorResponse;
  
  
  typedef typename PORT::transaction transaction;
  typedef typename PORT::accessHandle accessHandle;
  typedef typename PORT::phase phase;

  /**
   * PV transact method. It is used in PV mode 
   * to send a whole transaction container at once.
   * Please note that this method is blocking.
   */
  inline void Transact(transactionHandle t) {
    PORT::b_out->b_transact(t); 
  }

  /// PV Transact method for access handle
  inline void Transact(accessHandle a) {
    Transact(_getTransactionAccess(a));
  }
};

  
//---------------------------------------------------------------------------
/**
 * The generic master initiator port.
 */
//---------------------------------------------------------------------------
typedef GenericInitiatorAPI<master_port<GenericTransaction,GenericMasterAccess,GenericPhase, GenericTransactionCreator> > GenericMasterPort;
#define GenericInitiatorPort GenericMasterPort



//---------------------------------------------------------------------------
/**
 * The generic router target multi-port.
 */
//---------------------------------------------------------------------------
typedef target_multi_port<GenericTransaction,GenericSlaveAccess,GenericPhase> GenericRouterTargetPort;


//---------------------------------------------------------------------------
/**
 * The generic router initiator multi-port.
 */
//---------------------------------------------------------------------------
typedef initiator_multi_port<GenericTransaction,GenericRouterAccess,GenericPhase>  GenericRouterInitiatorPort;

} //end namespace tlm
