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


#ifndef __BASICPORTS_H__
#define __BASICPORTS_H__

#include "greenbus/core/tlm_b_if.h"
#include "greenbus/core/tlm_port.h"
#include "greenbus/utils/memoryutils.h"
#include "greenbus/utils/unevenpair.h"

#ifdef USE_STATIC_CASTS
#define PTRTYPE tlm::shared_ptr
#else
#define PTRTYPE boost::shared_ptr
#endif

                                                       
namespace tlm {


#if 0 // does not work with DUST
template <class GenericTransactionHandle>
class tlm_master_b_transact_error_t
: public tlm_b_if<GenericTransactionHandle>
{
public:
  void b_transact( GenericTransactionHandle ) {
    SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Tried to access blocking interface from a master as a target" );
  }
  virtual ~tlm_master_b_transact_error_t() {}
};
#endif


/**
 * Create a unique number (there may be better ways to implement this...)
 */
class UniqueNumber {
public:
  static inline gs_uint64 get() 
  {
    static gs_uint64 unique_number = 0;
    return unique_number++;
  }
};


//---------------------------------------------------------------------------
/**
 * Base for target ports.
 */
//---------------------------------------------------------------------------
template <class TRANSACTION, class ACCESS, class PHASE>
class target_port 
: public tlm_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                  unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
{
protected:
  using tlm_port<tlm_b_if<PTRTYPE<TRANSACTION> >, unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
        ::gb_configurable::addParam;
  
 public:
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef PTRTYPE<ACCESS> ACCESS_P;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;
  
  typedef TRANSACTION       transaction;  
  typedef GenericTransactionHandle     transactionHandle;
  typedef ACCESS_P          accessHandle;
  typedef PHASE             phase;

  typedef tlm_port<tlm_b_if<GenericTransactionHandle>, GS_ATOM> base_type;
  
  target_port( sc_module_name port_name ) :
    base_type(port_name)
  {
#ifdef USE_DUST
    DUST_SLAVE_PORT("BasicTargetPort", "GSTLM");
#endif
  }

};




/**
 * The generic slave port.
 */
template <class TRANSACTION, class ACCESS, class PHASE>
class slave_port :
public target_port<TRANSACTION, ACCESS, PHASE>
{
public:
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef PTRTYPE<ACCESS> ACCESS_P;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;
  
  typedef TRANSACTION       transaction;
  typedef GenericTransactionHandle     transactionHandle;
  typedef ACCESS_P          accessHandle;
  typedef PHASE             phase;

  typedef target_port<TRANSACTION,ACCESS,PHASE> base_type;
  
  slave_port( sc_module_name port_name ) 
    : base_type(port_name)
  { 
  }
  
  ~slave_port(){
  }

};

//---------------------------------------------------------------------------
/**
 * Base for target ports, which is able to accessed by multiple initiators.
 */
//---------------------------------------------------------------------------

template <class TRANSACTION, class ACCESS, class PHASE> 
class target_multi_port 
: public tlm_multi_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                        unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
{
protected:
  using tlm_multi_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                        unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
        ::gb_configurable::addParam;
  
public:
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef PTRTYPE<ACCESS> ACCESS_P;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;

  typedef TRANSACTION      							transaction;  
  typedef GenericTransactionHandle   	  transactionHandle;
  typedef ACCESS_P          						accessHandle;
  typedef PHASE            							phase;

  typedef tlm_multi_port<tlm_b_if<GenericTransactionHandle>, GS_ATOM> base_type;
  
  target_multi_port( sc_module_name port_name ) 
    : base_type(port_name)
  {
#ifdef USE_DUST
    DUST_SLAVE_PORT("BasicTargetMultiPort", "GSTLM");
#endif
  }

};


//---------------------------------------------------------------------------
/**
 * Base for initiator ports, who owns the phases.
 */
//---------------------------------------------------------------------------
template <class TRANSACTION, class ACCESS, class PHASE>
class initiator_port 
: public tlm_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                  unevenpair<PTRTYPE<TRANSACTION>, PHASE> >,
  public tlm_b_if<PTRTYPE<TRANSACTION> >
{
protected:
  using tlm_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                  unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
        ::gb_configurable::addParam;
  
public:
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef PTRTYPE<ACCESS> ACCESS_P;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;

  typedef tlm_port<tlm_b_if<GenericTransactionHandle>, GS_ATOM> base_type;

  typedef TRANSACTION       					 transaction;  
  typedef GenericTransactionHandle     transactionHandle;
  typedef ACCESS_P          					 accessHandle;
  typedef PHASE             					 phase;
  
  /// if initiator should be able to receive transactions (as a slave) set is_bidirectional true
  initiator_port( sc_module_name port_name, bool is_bidirectional = false ) :
    base_type(port_name)
  {
#ifdef USE_DUST
    DUST_MASTER_PORT("BasicInitiatorPort", "GSTLM");
#endif

    // bind tlm_b_if
    //base_type::bind_b_if(tlm_master_b_transact_error); // does not work with DUST, because reference between b_out and b_in cannot be resolved
    if (!is_bidirectional) base_type::bind_b_if(*this);
  }

  
private:
  //removed static qualifier because if every init_port is bound to the same (static) transact_error class
  //bindings to a target_multi_port (router) will fail, since the kernel
  //does not allow binding the same interface multiple times to the same port
  //tlm_master_b_transact_error_t<GenericTransactionHandle> tlm_master_b_transact_error; // does not work with DUST

  void b_transact( GenericTransactionHandle ) {
    SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Tried to access blocking interface from a master as a target" );
  }

  
};


//---------------------------------------------------------------------------
/**
 * Base for initiator ports, who owns the phases and is able to handle mutiple targets
 */
//---------------------------------------------------------------------------
template <class TRANSACTION, class ACCESS, class PHASE>
class initiator_multi_port 
: public tlm_multi_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                        unevenpair<PTRTYPE<TRANSACTION>, PHASE> >,
  public tlm_b_if<PTRTYPE<TRANSACTION> >
{
protected:
  using tlm_multi_port<tlm_b_if<PTRTYPE<TRANSACTION> >, 
                        unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
        ::gb_configurable::addParam;
  
public:
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef PTRTYPE<ACCESS> ACCESS_P;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;

  typedef TRANSACTION       					 transaction;  
  typedef GenericTransactionHandle     transactionHandle;
  typedef ACCESS_P          					 accessHandle;
  typedef PHASE             					 phase;

  typedef tlm_multi_port<tlm_b_if<GenericTransactionHandle>, GS_ATOM > base_type;
  
  /// if initiator should be able to receive transactions (as a slave) set is_bidirectional true
  initiator_multi_port( sc_module_name port_name, bool is_bidirectional = false ) 
    : base_type(port_name)
  {
#ifdef USE_DUST
    DUST_MASTER_PORT("BasicInitiatorMultiPort", "GSTLM");
#endif

    //base_type::bind_b_if(tlm_master_b_transact_error);
    if (!is_bidirectional) base_type::bind_b_if(*this);
  }
  
  
private:
  //removed static qualifier because if every init_port is bound to the same (static) transact_error class
  //bindings to a target_multi_port (router) will fail, since the kernel
  //does not allow binding the same interface multiple times to the same port
  //tlm_master_b_transact_error_t<GenericTransactionHandle> tlm_master_b_transact_error;
  
  void b_transact( GenericTransactionHandle ) {
    SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Tried to access blocking interface from a master as a target" );
  }
  
};



//---------------------------------------------------------------------------
/**
 * Specialized initiator port for the master port, who owns the transactions.
 */
//---------------------------------------------------------------------------
template <class TRANSACTION, class ACCESS, class PHASE,
          class CREATED_TRANSACTION = TRANSACTION>
class master_port :
public initiator_port<TRANSACTION, ACCESS, PHASE>
{
public:
  typedef PTRTYPE<CREATED_TRANSACTION> CREATED_TRANSACTION_P;
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef PTRTYPE<ACCESS> ACCESS_P;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;
  
  typedef TRANSACTION       					 transaction;
  typedef GenericTransactionHandle     transactionHandle;
  typedef ACCESS_P          					 accessHandle;
  typedef PHASE             					 phase;
  typedef CREATED_TRANSACTION_P 			 created_transaction_handle;
  typedef CREATED_TRANSACTION   			 created_transaction;  

  typedef initiator_port<TRANSACTION,ACCESS,PHASE> base_type;
  
  /// if master (initiator_port) should be able to receive transactions (as a slave) set is_bidirectional true
  master_port( sc_module_name port_name, bool is_bidirectional = false ) 
    : base_type(port_name, is_bidirectional)
  { 
    // use in-port pointer address as unique master ID (this is important to make genericRouter work)
    MasterPortNumber = (gs_handle) &this->in;
#ifdef USE_DUST
    this->out.setMasterID(MasterPortNumber);
#endif
  }

  /**
   * Create a transaction with a unique master ID.
   */
  accessHandle create_transaction() {
#ifndef DONT_USE_POOL 
    created_transaction_handle tmp=TransactionFactory.create();
#else // for benchmark purposes only!
    created_transaction_handle tmp = created_transaction_handle(new CREATED_TRANSACTION());
#endif

#ifdef USE_STATIC_CASTS
    //PTRTYPE<void> im = boost::static_pointer_cast<void>(tmp); // simulate reinterpret_cast
    //accessHandle ah = boost::static_pointer_cast<ACCESS>(im);
    accessHandle ah = tmp;
#else
    accessHandle ah = boost::dynamic_pointer_cast<ACCESS>(tmp); // cannot use _getMasterAccessHandle because ther the needed return type is unknown!
#endif
    ah->setMID(MasterPortNumber);
    ah->setMAddr(base_type::target_addr);
    //ah->setMDataWidth(base_type::data_width); // depricated, see data_width parameter in tlm_port
    //ah->setSDataWidth(base_type::data_width); // default: MDataWidth=SDataWidth (SDataWidth may be overridden by the bus implementation)    
    
    ah->setTransID(UniqueNumber::get());
    return ah;
  }
  
  gs_handle get_master_port_number() {
    return MasterPortNumber;
  }
 
  
private:
  
  ObjectFactory<created_transaction_handle> TransactionFactory;
  gs_handle MasterPortNumber;
  
};



// TODO: anpassen an neue GenericTransaction-Klassenhierarchie
template <class FORWARDS, class CONNECTS_TO, class TRANSACTION, class PHASE>
class basic_port_forwarder 
: public tlm_port_forwarder_base<tlm_b_if<PTRTYPE<TRANSACTION> >,
                                 unevenpair<PTRTYPE<TRANSACTION>, PHASE> >
{
public:  
  typedef PTRTYPE<TRANSACTION> GenericTransactionHandle;
  typedef unevenpair<GenericTransactionHandle,PHASE> GS_ATOM;

  basic_port_forwarder(sc_module_name name) 
    : tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>(name)
  {
#ifdef USE_DUST
    DUST_MASTER_PORT("BasicPortForwarder", "GSTLM");
#endif
  }

  //here we override the forward method to decide whether the port should be forwarded or bound
  virtual void forward(tlm_port<tlm_b_if<GenericTransactionHandle>,GS_ATOM>& port_)
  {
    FORWARDS* pfw= dynamic_cast<FORWARDS*>(&port_);
    CONNECTS_TO* pct= dynamic_cast<CONNECTS_TO*>(&port_);
    if (pfw) {
      std::cout<<sc_module::name()<<" : forwarding the port named "<<port_.name()<<std::endl<<std::flush;
      tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::forward(port_);
    }
    if (pct) {
      std::cout<<sc_module::name()<<" : binding the port named "<<port_.name()<<std::endl<<std::flush;
      tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::bind(port_);      
    }    
  }

  //here we override the forward method to decide whether the port should be forwarded or bound
  virtual void forward(tlm_multi_port<tlm_b_if<GenericTransactionHandle>,GS_ATOM>& port_)
  { 
    FORWARDS* pfw= dynamic_cast<FORWARDS*>(&port_);
    CONNECTS_TO* pct= dynamic_cast<CONNECTS_TO*>(&port_);
    if (pfw) {
      tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::forward(port_);
    }
    if (pct) {
      tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::bind(port_);      
    }
  }

  //connect to target
  void operator () ( CONNECTS_TO& port_ )
  { 
    std::cout<<sc_module::name()<<" : binding to the port named "<<port_.name()<<std::endl<<std::flush;  
    tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::bind(port_);
  }

  //connect to target forward
  void operator () ( basic_port_forwarder<CONNECTS_TO, FORWARDS, TRANSACTION, PHASE>& port_ )
  { 
    std::cout<<sc_module::name()<<" : binding to the fw port named "<<port_.name()<<std::endl<<std::flush;
    tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::bind(port_);
  }
  
  //forward another forward
  void operator () ( basic_port_forwarder<FORWARDS, CONNECTS_TO, TRANSACTION, PHASE>& port_ )
  { 
    std::cout<<sc_module::name()<<" : forwarding to the fw port named "<<port_.name()<<std::endl<<std::flush;  
    tlm_port_forwarder_base<tlm_b_if<GenericTransactionHandle>,GS_ATOM>::forward(port_);
  }

};

} // end of namespace tlm

#undef PTRTYPE

#endif
