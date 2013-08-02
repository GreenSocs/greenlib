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

#include <stdarg.h>
#include <sstream>

namespace tlm{
/** 
 * Shared pointer type for GenericTransaction. 
 * Transaction containers always should be wrapped by a shared_ptr
 * in order to avoid memory leakage.
 */
typedef tlm::shared_ptr<GenericTransaction> GenericTransactionHandle;
typedef tlm::shared_ptr<GenericRouterAccess> GenericRouterAccessHandle;
typedef tlm::shared_ptr<GenericSlaveAccess> GenericSlaveAccessHandle;
typedef tlm::shared_ptr<GenericMasterAccess> GenericMasterAccessHandle;
typedef unevenpair<GenericTransactionHandle, GenericPhase> GS_ATOM;



//---------------------------------------------------------------------------
/**
 * A generic phase class. 
 * This is a set of phases and access functions 
 * to form the GreenBus generic protocol.
 */
//---------------------------------------------------------------------------
namespace gp{

class GenericPhase  // "Phases for the Generic Protocol";
{
  
public:
  
  /// the phases of the generic protocol
  enum {
    Idle = 0,
    RequestValid,RequestAccepted,RequestError,
    DataValid, DataAccepted, DataError,
    ResponseValid,ResponseAccepted, ResponseError,
    LAST_GENERIC_PHASE
  };
  
  gs_uint32 state;
  GenericSimulationModeType sim_mode;

  /// Number of data atoms sent in this transaction
  gs_uint32 BurstNumber;



  
  /**
   * Create phase with default state (Idle).
   */
  GenericPhase() 
    : state(Idle), sim_mode(MODE_BA), BurstNumber(0)
  {}

  GenericPhase(sc_event &_initiator_update_event, sc_event &_target_update_event) 
    : state(Idle), sim_mode(MODE_CT), BurstNumber(0)
  {}
                                                                                    

  /**
   * Create phase.
   * @param s Phase state
   */
  GenericPhase(gs_uint32 s)
    : state(s), sim_mode(MODE_BA), BurstNumber(0)
  {}

  /**
   * Create phase.
   * @param s Phase state
   * @param b Burst number
   */
  GenericPhase(gs_uint32 s, gs_uint32 b)
    : state(s), sim_mode(MODE_BA), BurstNumber(b)
  {}

  /// copy constructor
  GenericPhase(const GenericPhase &p) {
    *this = p; // use copy-operator
  }

  /// copy operator
  GenericPhase & operator=(const GenericPhase &p) {
    if (&p==this)
      return *this;
    state=p.state;
    sim_mode=p.sim_mode;
    BurstNumber=p.BurstNumber;
    return *this;
  }

  /// set the simulation mode for this phase
  inline void setSimulationMode(GenericSimulationModeType m) {
    sim_mode = m;
  }

  /// get the simulation mode for this phase
  inline GenericSimulationModeType getSimulationMode() {
    return sim_mode;
  }
  
  /// has a request atom been sent by the master?
  inline bool isRequestValid() { return state == RequestValid; }
  /// has a request atom been accepted by the slave?
  inline bool isRequestAccepted() { return state == RequestAccepted; }
  /// has the slave replied an error on a master's request atom?
  inline bool isRequestError() { return state == RequestError; }
  /// has a data atom been sent by the master?
  inline bool isDataValid() { return state == DataValid; }
  /// has a data atom been accepted by the slave?
  inline bool isDataAccepted() { return state == DataAccepted; }
  /// has the slave replied an error on a master's data atom?
  inline bool isDataError() { return state == DataError; }
  /// has a response atom been sent by the slave?
  inline bool isResponseValid() { return state == ResponseValid; }
  /// has a response atom been accepted by the master?
  inline bool isResponseAccepted() { return state == ResponseAccepted; }
  /// has the master replied an error on a slave's response atom?
  inline bool isResponseError() { return state == ResponseError; }
  
  /// get a string description of the current protocol phase
  std::string toString() { 
    if (isRequestValid()) return std::string("RequestValid");
    else if (isRequestAccepted()) return std::string("RequestAccepted");
    else if (isRequestError()) return std::string("RequestError");
    else if (isDataValid()) return std::string("DataValid");
    else if (isDataAccepted()) return std::string("DataAccepted");
    else if (isDataError()) return std::string("DataError");
    else if (isResponseValid()) return std::string("ResponseValid");
    else if (isResponseAccepted()) return std::string("ResponseAccepted");
    else if (isResponseError()) return std::string("ResponseError");
    else {
      std::stringstream ss;
      ss << "unknown phase (state=" << state 
         << ", simulation mode=" << sim_mode << ")";
      return ss.str();
    }
  }
};



#define REQUESTPHASE tlm::gp::GenericPhase::RequestValid, tlm::gp::GenericPhase::RequestAccepted, tlm::gp::GenericPhase::RequestError
#define DATAPHASE tlm::gp::GenericPhase::DataValid, tlm::gp::GenericPhase::DataAccepted, tlm::gp::GenericPhase::DataError
#define RESPONSEPHASE tlm::gp::GenericPhase::ResponseValid, tlm::gp::GenericPhase::ResponseAccepted, tlm::gp::GenericPhase::ResponseError

#define INITATTRIBUTE(name)                     \
  name.setName(#name)

// Don't use variadic macro, it is incompatible with old compilers like Visual C++ 7.1
#define ADDTOPHASE1(name, ARG1) \
  setPhases(name, false, ARG1 , 65563 )
#define ADDTOPHASE2(name, ARG1, ARG2) \
  setPhases(name, false, ARG1, ARG2 , 65563 )
#define ADDTOPHASE3(name, ARG1, ARG2, ARG3) \
  setPhases(name, false, ARG1, ARG2, ARG3 , 65563 )
#define ADDTOPHASE4(name, ARG1, ARG2, ARG3, ARG4) \
  setPhases(name, false, ARG1, ARG2, ARG3 , ARG4, 65563 )

// Don't use variadic macro, it is incompatible with old compilers like Visual C++ 7.1
#define ADDEXTTOPHASE1(name, ARG1) \
  setPhases(name, true, ARG1 , 65563 )
#define ADDEXTTOPHASE2(name, ARG1, ARG2) \
  setPhases(name, true, ARG1, ARG2 , 65563 )
#define ADDEXTTOPHASE3(name, ARG1, ARG2, ARG3) \
  setPhases(name, true, ARG1, ARG2, ARG3 , 65563 )
#define ADDEXTTOPHASE4(name, ARG1, ARG2, ARG3, ARG4) \
  setPhases(name, true, ARG1, ARG2, ARG3 , ARG4, 65563 )

//---------------------------------------------------------------------------
/**
 * A generic transaction class.
 * This is the GreenBus transaction container.
 * See GreenBus user manual for usage examples.
 * See GenericMasterAcess, GenericSlaveAccess, and GenericRouterAccess for
 * documentation of the attribute access methods.
 */
//---------------------------------------------------------------------------
class GenericTransaction
: protected virtual TransactionBase
{

TRANSACTION_MANDATORY_STUFF(gp::GenericMasterAccess, gp::GenericSlaveAccess, gp::GenericRouterAccess, gp::GenericTransaction);

#ifdef USE_DUST
  friend class scv_extensions<GenericTransaction>;
#endif
  
protected:
  // quarks ///////////////////////////////////////////////////////////////////
  MAddr mAddr;
  MID mID;

  MBurstLength mBurstLength, sBurstLength;    
  MSBytesValid msBytesValid; 
  MCmd mCmd;
  MData msData;
  //MDataWidth mDataWidth;  // depricated, see data_width parameter in tlm_port
  //SDataWidth sDataWidth;  // depricated, see data_width parameter in tlm_port
  SResp sResp;
  Error mError;
  Error sError;  
  TransID transID;


  // attribute iterators //////////////////////////////////////////////////////
  bool is_extended;
  std::string *extendedType;
  attribute_map *attribs;
  attribute_map *extended_attribs;

  // CT simulation mode ///////////////////////////////////////////////////////
  sc_event initiator_update_event, target_update_event;

  // quark access /////////////////////////////////////////////////////////////

  /// set master id
  inline void setMID(const MID& _mID) {mID=_mID;}
  /// set master command (see GenericMCmdType)
  inline void setMCmd(const MCmd& _mCmd){mCmd=_mCmd;}
  /// set target address
  inline void setMAddr(const MAddr& _mAddr){mAddr=_mAddr;}
  /// set master data (i.e. set the data pointer in the transaction container to the given GSDataType)
  inline void setMData(const GSDataType& _mData){msData.set(_mData);}   
  /// set master port data interface width
  //inline void setMDataWidth(const MDataWidth& _mDataWidth){mDataWidth=_mDataWidth;}  // depricated, see data_width parameter in tlm_port
  /// set slave port data interface width
  //inline void setSDataWidth(const SDataWidth& _sDataWidth){sDataWidth=_sDataWidth;}  // depricated, see data_width parameter in tlm_port

  /// set master burst length ('0' implies variable length burst, then you need to set MBurstLength to the overall transaction length with the last data/response atom)
  inline void setMBurstLength(const MBurstLength& _mBurstLength){  
    mBurstLength=_mBurstLength; 
    msBytesValid=_mBurstLength;
  }
  /// set slave burst length (deprecated, will be removed)
  //inline void setSBurstLength(const SBurstLength& _sBurstLength){  
  //  sBurstLength=_sBurstLength; 
  //  msBytesValid=_sBurstLength;
  //}

  /// set number of valid bytes with this atom
  inline void setMSBytesValid(const MSBytesValid& _msBytesValid){msBytesValid=_msBytesValid;}

  /// set response information
  inline void setSResp(const SResp& _sResp){ sResp = _sResp; }    

  /// set slave read data
  inline void setSData(MData& _sData) { 
    if (_sData.isPointer())
      msData.setPointer(_sData.getPointer());
    else
      msData.deepcopyFrom(const_cast<MData&>(_sData)); 
  }

  /// set transaction ID (only use if you want to overwrite default value, which is generated by the port) 
  inline void setTransID(const TransID& _transID) { transID = _transID; }
  
  /// get target address
  inline const MAddr& getMAddr() const {return mAddr; }
  /// get master id
  inline const MID& getMID() const {return mID;}
  /// get master cmd
  inline const MCmd& getMCmd() const {return mCmd; }

  /// get access to the transaction data (see GSDataType)
  inline const MData& getMData() const {return msData; }
  inline void getMData(const MData & _dst ) { msData.deepcopyTo(const_cast<MData&>(_dst)); }
  /// get master data bus width (# bytes that are transferred by the master in one cycle). This value is set by the initiator port.
  //inline const MDataWidth& getMDataWidth() const {return mDataWidth;}  // depricated, see data_width parameter in tlm_port
  /// get slave data bus width (# bytes that can be received by the slave in one cycle). This value is set by the target port.
  //inline const SDataWidth& getSDataWidth() const {return sDataWidth;}  // depricated, see data_width parameter in tlm_port
  /// get the master burst length
  inline const MBurstLength& getMBurstLength() const {return mBurstLength; }
  /// get slave burst length
  inline const SBurstLength& getSBurstLength() const {return sBurstLength; }
  /// get the number of valid data bytes in this atom
  inline const MSBytesValid& getMSBytesValid() const {return msBytesValid; }
  /// get the slave response (see GenericSRespType)
  inline const SResp& getSResp() const {return sResp; }
  /// get access to the transaction data (see GSDataType)
  inline MData& getSData() {return msData; }
  
  /// set the master error (see GenericError)
  inline void setMError(const Error& _mError) {mError=_mError;}
  /// get the master error (see GenericError)
  inline const Error getMError() const {return mError;}
  /// set the slave error (see GenericError)
  inline void setSError(const Error& _sError) {sError=_sError;}
  /// get the slave error (see GenericError)
  inline const Error getSError() const {return sError;}
  /// get the transaction ID (set by create_transaction)
  virtual const TransID& getTransID() const {return transID;}

public:
  // attribute iterators //////////////////////////////////////////////////////
  
  /// get an iterator over all standard attributes of this transaction container
  inline attribute_iterator getAttributeIterator(int phase){
    if (attribs==NULL)
      createAttributeIterator();
    return attribs->find(phase);
  }
  
  /// get an iterator pointing to the last attribute of this transaction container
  inline attribute_iterator lastAttribute(int phase) { 
    if (attribs==NULL)
      createAttributeIterator();
    if (attribs->find(phase)==attribs->end()) 
      return attribs->end() ;
    else 
      return attribs->upper_bound(phase);
  }
  
  /// get an iterator over the extended attributes of this transaction container
  inline attribute_iterator getExtendedAttributeIterator(int phase){
    if (extended_attribs==NULL)
      createExtendedAttributeIterator();
    return extended_attribs->find(phase);
  }
  
  /// get an iterator pointing to the last extended attribute of this transaction container
  inline attribute_iterator lastExtendedAttribute(int phase) {
    if (extended_attribs==NULL)
      createExtendedAttributeIterator();
    if (extended_attribs->find(phase)==extended_attribs->end()) 
      return extended_attribs->end() ;
    else 
      return extended_attribs->upper_bound(phase);
  }
  
  /// map an attribute to a set of phases (this function is used by the attribute iterators)
  inline void setPhases(AttributeRoot& attr, bool extended,...) {
    va_list ap;
    va_start(ap, extended);
    for (;;){
      int a=va_arg(ap, int);
      if (a==65563) break;
      if (extended)
        extended_attribs->insert(std::make_pair(a, &attr));
      else
        attribs->insert(std::make_pair(a, &attr));
    }
    va_end(ap);
  }
  
  /// does this transaction container contain extended attributes?
  inline bool isExtended() const{
    return is_extended;
  }
  
  /// get the extension type of this transaction container
  /**
   * This method is to be implemented in extended transaction containers.
   * It should create a string with the name of the extended transaction.
   */
  inline virtual std::string &getExtendedType() {
    if (extendedType==NULL) {
      extendedType = new std::string("N/A");
    }
    return *extendedType;
  }

  /// create the attribute iterator.
  /**
   * This method has to be called before using the attribute iterator
   * the first time. It creates a vector with all attributes of this 
   * transaction container.
   */
  void createAttributeIterator() {
    sc_assert(attribs==NULL);
    attribs=new attribute_map();
    
    INITATTRIBUTE(mCmd);
    ADDTOPHASE1(mCmd,REQUESTPHASE);
    INITATTRIBUTE(mAddr);
    ADDTOPHASE3(mAddr,REQUESTPHASE, DATAPHASE, RESPONSEPHASE);
    INITATTRIBUTE(msData);
    ADDTOPHASE2(msData,DATAPHASE, RESPONSEPHASE);
    //INITATTRIBUTE(mDataWidth);  // depricated, see data_width parameter in tlm_port
    //ADDTOPHASE1(mDataWidth, REQUESTPHASE);  // depricated, see data_width parameter in tlm_port
    //INITATTRIBUTE(sDataWidth);  // depricated, see data_width parameter in tlm_port
    //ADDTOPHASE1(sDataWidth, REQUESTPHASE);  // depricated, see data_width parameter in tlm_port
    INITATTRIBUTE(mBurstLength);
    ADDTOPHASE1(mBurstLength,REQUESTPHASE);
    INITATTRIBUTE(sResp);
    ADDTOPHASE1(sResp, RESPONSEPHASE);
    INITATTRIBUTE(mError);
    ADDTOPHASE3(mError, REQUESTPHASE, DATAPHASE, RESPONSEPHASE);
    INITATTRIBUTE(sError);
    ADDTOPHASE3(sError, REQUESTPHASE, DATAPHASE, RESPONSEPHASE);
    INITATTRIBUTE(mID);
    ADDTOPHASE3(mID, REQUESTPHASE, DATAPHASE, RESPONSEPHASE);
  }   

  /// create the extended attribute iterator.
  /**
   * This method has to be called before using the extended attribute
   * iterator the first time. It creates  a map with all extendeded
   * attributes of this transaction container.
   * Note: Extended attributes are declared by transaction containers
   *       that INHERIT from this class. They must implement this
   *       virtual method as well, and the implementation must call
   *       the createExtendedAtteributeIterator() method of the base
   *       class (e.g. a PLBTransaction would call 
   *       GenericTransaction::createExtendedAttributeIterator()
   *       at the beginning of its createExtendedAttributeIterator method,
   *       and a PLBExtendedTransaction which inherits from PLBTransaction
   *       would call PLBTransaction::createExtendedAttributeIterator()
   *       at the beginning of its createExtendedAttributeIterator method.
   *       This is to make sure that ALL extended attributes of all 
   *       extended transaction containers in the inheritence tree are added
   *       to the extended_attributes map.
   */
  virtual void createExtendedAttributeIterator() {
    sc_assert(extended_attribs==NULL);
    extended_attribs=new attribute_map();

    // override this method in extended transactions and init your extended
    // attributes here.
  }

  
  // construction /////////////////////////////////////////////////////////////

  /** 
   * Create a generic transaction. You should not use this constructor
   * directly. Always use master_port::createTransaction().
   */  
  GenericTransaction() 
    : mBurstLength(0),  // only init important flags
      sBurstLength(0),
      mError(0), 
      sError(0),
      is_extended(false), 
      extendedType(NULL),
      attribs(NULL),
      extended_attribs(NULL)
  {}

  /// copy constructor
  GenericTransaction(const GenericTransaction &t) {
    *this = t; // use copy-operator
  }
  
  void reset(){
    mBurstLength=0;
    sBurstLength=0;
    mError=0;
    sError=0;
  }
  
  /// destructor
  ~GenericTransaction() {
    if (attribs!=NULL) delete attribs;
    if (extended_attribs!=NULL) delete extended_attribs;
    if (extendedType!=NULL) delete extendedType;
  }

  /// copy operator
  GenericTransaction & operator=(const GenericTransaction &t) {
    if (&t==this)
      return *this;

    mAddr = t.mAddr;
    mBurstLength = t.mBurstLength; 
    sBurstLength = t.sBurstLength;
    mCmd = t.mCmd;
    msData.deepcopyFrom(t.msData);
    //mDataWidth = t.mDataWidth;  // depricated, see data_width parameter in tlm_port
    //sDataWidth = t.sDataWidth;  // depricated, see data_width parameter in tlm_port
    sResp = t.sResp;
    mError = t.mError;
    sError = t.sError; 
    mID = t.mID;  
    is_extended = t.is_extended;

    // TODO: check if quarks are missing
    
    return *this;
  }



  // get an access handle to this transaction /////////////////////////////////
  // ATTENTION: implemented in TAIL section
  /// Get the router access set to this transaction
  inline GenericRouterAccess &getRouterAccess();
  /// Get the master access set to this transaction
  inline GenericMasterAccess &getMasterAccess();
  /// Get the target access set to this transaction
  inline GenericSlaveAccess &getTargetAccess();

#define getSlaveAccess getTargetAccess




  // CT simulation helpers ////////////////////////////////////////////////////

  /// notify a CT slave of a quark update
  inline void notifyTargetUpdate() {target_update_event.notify();}
  /// notify a CT slave of a quark update
  inline void notifyTargetUpdate(const sc_time &t) {target_update_event.notify(t);}
  /// notify a CT slave of a quark update
  inline void notifyTargetUpdate(const double d, const sc_time_unit u) {target_update_event.notify(d,u);}
  /// get the initiator update event ( use in slave to trigger on master CT quark update notification)
  inline const sc_event& getInitiatorUpdateEvent() const {return initiator_update_event;}
  /// wait for an initiator update event (use in slave to wait for master CT quark update notification)
  inline void waitForInitiatorUpdate(const sc_time &timeout) const {wait(timeout, initiator_update_event);}


  /// notify a CT master of a quark update
  inline void notifyInitiatorUpdate() {initiator_update_event.notify();}
  /// notify a CT master of a quark update
  inline void notifyInitiatorUpdate(const sc_time &t) {initiator_update_event.notify(t);}
  /// notify a CT master of a quark update
  inline void notifyInitiatorUpdate(const double d, const sc_time_unit u) {initiator_update_event.notify(d,u);}
  /// get the target update event (use in master to trigger on slave CT quark update notification)
  inline const sc_event& getTargetUpdateEvent() const {return target_update_event;}
  /// wait for a target update event (use in master to wait for slave CT quark update notification)
  inline void waitForTargetUpdate(const sc_time &timeout) const {wait(timeout, target_update_event);}


};

//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the master.
 * A master can get all  attributes, but can set only master attributes.
 */
//---------------------------------------------------------------------------
class GenericMasterAccess 
: public virtual GenericTransaction
{
public:
  using GenericTransaction::setMAddr;
  using GenericTransaction::setMBurstLength;
  using GenericTransaction::setMSBytesValid;
  using GenericTransaction::setMCmd;
  using GenericTransaction::setMData;
  //using GenericTransaction::setMDataWidth;  // depricated, see data_width parameter in tlm_port
  using GenericTransaction::setMID;
  using GenericTransaction::setMError;
  using GenericTransaction::setTransID;

  using GenericTransaction::getMAddr;
  using GenericTransaction::getMBurstLength;
  //using GenericTransaction::getSBurstLength; // deprecated
  using GenericTransaction::getMSBytesValid;
  using GenericTransaction::getMCmd;
  using GenericTransaction::getMData;
  //using GenericTransaction::getMDataWidth;  // depricated, see data_width parameter in tlm_port
  //using GenericTransaction::getSDataWidth;  // depricated, see data_width parameter in tlm_port
  using GenericTransaction::getMID;
  using GenericTransaction::getSResp;
  using GenericTransaction::getSData;
  using GenericTransaction::getMError;
  using GenericTransaction::getSError;
  using GenericTransaction::getTransID;

};


//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the slave.
 * A slave can get all attributes, but can set only slave attributes.
 */
//---------------------------------------------------------------------------
class GenericSlaveAccess
: public virtual GenericTransaction
{
public:
  //using GenericTransaction::setSBurstLength; // deprecated
  using GenericTransaction::setSData;
  //using GenericTransaction::setSDataWidth;  // depricated, see data_width parameter in tlm_port
  using GenericTransaction::setMSBytesValid;
  using GenericTransaction::setSResp;
  using GenericTransaction::setSError;

  using GenericTransaction::getMAddr;
  using GenericTransaction::getMBurstLength;
  //using GenericTransaction::getSBurstLength; // deprecated
  using GenericTransaction::getMSBytesValid;
  using GenericTransaction::getMCmd;
  using GenericTransaction::getMData;
  //using GenericTransaction::getMDataWidth;  // depricated, see data_width parameter in tlm_port
  //using GenericTransaction::getSDataWidth;  // depricated, see data_width parameter in tlm_port
  using GenericTransaction::getMID;
  using GenericTransaction::getSResp;
  using GenericTransaction::getSData;
  using GenericTransaction::getMError;
  using GenericTransaction::getSError;
  using GenericTransaction::getTransID;
  
};


//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the router.
 * A router can only "see" the protocol attributes. It is not authorized
 * to access the transaction data.
 */
//---------------------------------------------------------------------------
class GenericRouterAccess 
: public virtual GenericTransaction
{
public:
  using GenericTransaction::getMID;
  using GenericTransaction::getMCmd;
  using GenericTransaction::getMAddr;
  using GenericTransaction::getMBurstLength;
  using GenericTransaction::getMSBytesValid;
  //using GenericTransaction::getSBurstLength; // deprecated
  //using GenericTransaction::getMDataWidth;  // depricated, see data_width parameter in tlm_port
  //using GenericTransaction::getSDataWidth;  // depricated, see data_width parameter in tlm_port
  using GenericTransaction::getMError;
  using GenericTransaction::getSError;
  
  //using GenericTransaction::setMDataWidth;  // depricated, see data_width parameter in tlm_port
  //using GenericTransaction::setSDataWidth;  // depricated, see data_width parameter in tlm_port
  using GenericTransaction::setMError;
  using GenericTransaction::setSError;
  using GenericTransaction::getTransID;
};

} //namespace gp

}  //end namespace tlm
