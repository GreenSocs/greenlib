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

#ifdef USE_PCIE_TRANSACTION
  namespace PCIe {
    class PCIeTransaction;
    class PCIePhase;
    class PCIeMasterAccess;
    class PCIeSlaveAccess;
    class PCIeRouterAccess;
  }
  using namespace tlm::PCIe;
#endif

//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the master.
 * A master can get all  attributes, but can set only master attributes.
 */
//---------------------------------------------------------------------------
class GenericMasterAccess : public TransactionBase {
  
public :
  
  /// set target address
  virtual void setMAddr(const MAddr&) =0;
  /// set master burst length ('0' implies variable length burst, then you have to check SBurstLength in slave reply)
  virtual void setMBurstLength(const MBurstLength&) =0;
  /// set number of valid bytes in this response
  virtual void setMSBytesValid(const MSBytesValid&) =0;
  /// set master command (see GenericMCmdType)
  virtual void setMCmd(const MCmd&) =0;
  /// set master data (i.e. set the data pointer in the transaction container to the given GSDataType)
  virtual void setMData(const MSData&) =0;
  /// set master data width (# bytes that are transferred by the master in one cycle)
  //virtual void setMDataWidth(const MDataWidth&) =0;  // depricated, see data_width parameter in tlm_port
  /// set master id
  virtual void setMID(const MID&) =0;
  /// set the master error (see GenericError)
  virtual void setMError(const Error& _mError) =0;
  /// set the transaction ID (should not be set by the user)
  virtual void setTransID(const TransID& _transID) =0;
  
  /// get target address
  virtual const MAddr& getMAddr() const =0;
  /// get the master burst length
  virtual const MBurstLength& getMBurstLength() const =0;
  /// get slave burst length
  virtual const SBurstLength& getSBurstLength() const =0;
  /// get number of valid bytes
  virtual const MSBytesValid& getMSBytesValid() const =0;
  /// get master cmd
  virtual const MCmd& getMCmd() const =0;
  /// get access to the transaction data (see GSDataType)
  virtual const MSData& getMData() const =0;
  /// get master data bus width (# bytes that are transferred by the master in one cycle). This value is set by the initiator port.
  //virtual const MDataWidth& getMDataWidth() const =0;  // depricated, see data_width parameter in tlm_port
  /// get slave data bus width (# bytes that can be received by the slave in one cycle). This value is set by the target port.
  //virtual const SDataWidth& getSDataWidth() const =0;  // depricated, see data_width parameter in tlm_port
  /// get master id
  virtual const MID& getMID() const =0;
  /// get the slave response (see GenericSRespType)
  virtual const SResp& getSResp() const =0;
  /// get access to the transaction data (see GSDataType)
  virtual MSData& getSData() =0;
  /// get the master error (see GenericError)
  virtual const Error getMError() const =0;
  /// get the slave error (see GenericError)
  virtual const Error getSError() const =0;  
  /// get the transaction ID (set by create_transaction)
  virtual const TransID& getTransID() const =0;


  // TODO: move these functions into the phase. To this end, make the phase a shared_ptr which lives throughout the whole transactions.

  /// notify the target immediately of a phase update (used for CT simulation)
  virtual void notifyTargetUpdate() =0;
  /// notify the target of a phase update (used for CT simulation)
  virtual void notifyTargetUpdate(const sc_time &t) =0;
  /// notify the target of a phase update (used for CT simulation)
  virtual void notifyTargetUpdate(const double d, const sc_time_unit u) =0;

  /// get the initiator update event
  virtual const sc_event& getInitiatorUpdateEvent() const =0;
  /// wait for initiator update event to be fired (may only be invoked from a sc_thread)
  virtual void waitForInitiatorUpdate(const sc_time &timeout) const =0;
  
};


//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the slave.
 * A slave can get all attributes, but can set only slave
 * attributes.
 */
//---------------------------------------------------------------------------
class GenericSlaveAccess : public TransactionBase {
  
public :
  
  /// set slave burst length
  virtual void setSBurstLength(const SBurstLength&) =0;
  /// set number of valid bytes in this response
  virtual void setMSBytesValid(const MSBytesValid&) =0;
  /// copy data into the transaction (the data array resides in the master)
  virtual void setSData(MSData&) =0;
  /// set slave data width (# bytes that can be received by the slave in one cycle)
  //virtual void setSDataWidth(const SDataWidth&) =0;  // depricated, see data_width parameter in tlm_port
  /// set slave response type (see GenericSRespType)
  virtual void setSResp(const SResp&) =0;
  /// set slave error (see GenericError)
  virtual void setSError(const Error& _sError) =0;
  
  /// get target address of this transaction
  virtual const MAddr& getMAddr() const =0;
  /// get master burst length
  virtual const MBurstLength& getMBurstLength() const =0;
  /// get slave burst length
  virtual const SBurstLength& getSBurstLength() const =0;
  /// get number of valid bytes
  virtual const MSBytesValid& getMSBytesValid() const =0;
  /// get master command (see GenericMCmdType)
  virtual const MCmd& getMCmd() const =0;
  /// get a reference to the transaction data (which resides in master port)
  virtual const MSData& getMData() const =0;
  /// get a local copy of the transaction data (see GSDataType)
  virtual void getMData(const MSData & _dst ) =0;
  // get access to the slave data. Use this function with care. It gives write access to the data vector that resides in the master. Using push_back(), you can append data directly to the data vector)
  virtual MSData& getSData() =0;
  /// get master data bus width (# bytes that are transferred by the master in one cycle). This value is set by the initiator port.
  //virtual const MDataWidth& getMDataWidth() const =0;  // depricated, see data_width parameter in tlm_port
  /// get slave data bus width (# bytes that can be received by the slave in one cycle). This value is set by the target port.
  //virtual const SDataWidth& getSDataWidth() const =0;  // depricated, see data_width parameter in tlm_port
  /// get master id
  virtual const MID& getMID() const =0;
  /// get master error (see GenericError)
  virtual const Error getMError() const=0;
  /// get slave error (see GenericError)
  virtual const Error getSError() const=0;
  /// get the transaction ID (set by create_transaction)
  virtual const TransID& getTransID() const =0;



  // TODO: move these functions into the phase. To this end, make the phase a shared_ptr which lives throughout the whole transactions.

  /// notify the initiator immediately of a phase update (used for CT simulation)
  virtual void notifyInitiatorUpdate() =0;
  /// notify the initiator of a phase update (used for CT simulation)
  virtual void notifyInitiatorUpdate(const sc_time &t) =0;
  /// notify the initiator of a phase update (used for CT simulation)
  virtual void notifyInitiatorUpdate(const double d, const sc_time_unit u) =0;

  /// get the target update event
  virtual const sc_event& getTargetUpdateEvent() const =0;
  /// wait for target update event to be fired (may on ly be invoked from a sc_thread)
  virtual void waitForTargetUpdate(const sc_time &timeout) const =0;

};

//---------------------------------------------------------------------------
/**
 * Interface to access the generic transaction through the router.
 * A router can only "see" the protocol attributes. It is not authorized
 * to access the transaction data.
 */
//---------------------------------------------------------------------------
class GenericRouterAccess : public TransactionBase {
  
public :
  
  /// get master id
  virtual const MID& getMID() const =0;
  /// get master command (see GenericMCmdType)
  virtual const MCmd& getMCmd() const =0;
  /// get target address of the transaction
  virtual const MAddr& getMAddr() const =0;
  /// get master burst length
  virtual const MBurstLength& getMBurstLength() const =0;
  /// get slave burst length
  virtual const SBurstLength& getSBurstLength() const =0;
  /// get master data bus width (# bytes that are transferred by the master in one cycle). This value is set by the initiator port.
  //virtual const MDataWidth& getMDataWidth() const =0;  // depricated, see data_width parameter in tlm_port
  /// get slave data bus width (# bytes that can be received by the slave in one cycle). This value is set by the target port.
  //virtual const SDataWidth& getSDataWidth() const =0;  // depricated, see data_width parameter in tlm_port
  /// get number of valid bytes
  virtual const MSBytesValid& getMSBytesValid() const =0;
  /// get master error
  virtual const Error getMError() const=0;
  /// get slave error
  virtual const Error getSError() const=0;
  /// get the transaction ID (set by create_transaction)
  virtual const TransID& getTransID() const =0;

  
  /// set master data bus width (# bytes that are transferred by the master in one cycle). This value is set by the initiator port, but can be overridden by the router.
  //virtual void setMDataWidth(const MDataWidth&) =0;  // depricated, see data_width parameter in tlm_port
  /// set slave data bus width (# bytes that can be received by the slave in one cycle). This value usually is set by the target port, but can be overridden by the router.
  //virtual void setSDataWidth(const SDataWidth&) =0;  // depricated, see data_width parameter in tlm_port
  /// set master error
  virtual void setMError(const Error& _mError)=0;
  /// set slave error
  virtual void setSError(const Error& _sError)=0;
};




//---------------------------------------------------------------------------
/**
 * A generic phase class. 
 * This is a set of phases and access functions 
 * to form the GreenBus generic protocol.
 */
//---------------------------------------------------------------------------
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



#define REQUESTPHASE GenericPhase::RequestValid, GenericPhase::RequestAccepted, GenericPhase::RequestError
#define DATAPHASE GenericPhase::DataValid, GenericPhase::DataAccepted, GenericPhase::DataError
#define RESPONSEPHASE GenericPhase::ResponseValid, GenericPhase::ResponseAccepted, GenericPhase::ResponseError

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

#ifdef USE_PCIE_TRANSACTION
} // end namespace tlm
# include "greenbus/transport/PCIe/PCIePhase.dynamic_casts.h"
namespace tlm {
#endif

//---------------------------------------------------------------------------
/**
 * A generic transaction class.
 * This is the GreenBus transaction container.
 * See GreenBus user manual for usage examples.
 * See GenericMasterAcess, GenericSlaveAccess, and GenericRouterAccess for
 * documentation of the attribute access methods.
 */
//---------------------------------------------------------------------------
class GenericTransaction :
  public virtual GenericMasterAccess,
  public virtual GenericSlaveAccess,
  public virtual GenericRouterAccess
{
  
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
  MSData msData;
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
  
  
public:

  // quark access /////////////////////////////////////////////////////////////

  inline void setMID(const MID& _mID) {mID=_mID;}
  inline void setMCmd(const MCmd& _mCmd){mCmd=_mCmd;}
  inline void setMAddr(const MAddr& _mAddr){mAddr=_mAddr;}
  inline void setMData(const MSData& _mData){msData.set(_mData);}
  //inline void setMDataWidth(const MDataWidth& _mDataWidth){mDataWidth=_mDataWidth;}  // depricated, see data_width parameter in tlm_port
  //inline void setSDataWidth(const SDataWidth& _sDataWidth){sDataWidth=_sDataWidth;}  // depricated, see data_width parameter in tlm_port
  
  inline void setMBurstLength(const MBurstLength& _mBurstLength) {  
    mBurstLength=_mBurstLength; 
    msBytesValid=_mBurstLength;
  }
  inline void setSBurstLength(const SBurstLength& _sBurstLength){  
    sBurstLength=_sBurstLength; 
    msBytesValid=_sBurstLength;
  }
  
  inline void setMSBytesValid(const MSBytesValid& _msBytesValid){msBytesValid=_msBytesValid;}
  
  inline void setSResp(const SResp& _sResp){ sResp = _sResp; }
  
  void setSData(MSData& _sData) { 
    if (_sData.isPointer())
      msData.setPointer(_sData.getPointer());
    else
      msData.deepcopyFrom(const_cast<MSData&>(_sData)); 
  }
  
  inline void setTransID(const TransID& _transID) { transID = _transID; }
  
  // todo sData for "slices"

  inline const MAddr& getMAddr() const {return mAddr; }
  inline const MID& getMID() const {return mID;}

  inline const MCmd& getMCmd() const {return mCmd; }
  inline const MSData& getMData() const {return msData; }
  inline void getMData(const MSData & _dst ) { msData.deepcopyTo(const_cast<MSData&>(_dst)); }
  // TODO: mData for "slices"
  
  //inline const MDataWidth& getMDataWidth() const {return mDataWidth;}  // depricated, see data_width parameter in tlm_port
  //inline const SDataWidth& getSDataWidth() const {return sDataWidth;}  // depricated, see data_width parameter in tlm_port
  
  inline const MBurstLength& getMBurstLength() const {return mBurstLength; }
  inline const SBurstLength& getSBurstLength() const {return sBurstLength; }
  inline const MSBytesValid& getMSBytesValid() const {return msBytesValid; }
  inline const SResp& getSResp() const {return sResp; }
  inline MData& getSData() {return msData; }
  
  inline void setMError(const Error& _mError) {mError=_mError;}
  inline const Error getMError() const {return mError;}
  inline void setSError(const Error& _sError) {sError=_sError;}
  inline const Error getSError() const {return sError;}
  
  inline const TransID& getTransID() const {return transID;}


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
  inline bool isExtended(){
    return is_extended;
  }
  
  /// get the extension type of this transaction container
  /**
   * This method is to be implemented in extended transaction containers.
   * It should create a string with the name of the extended transaction.
   */
  inline 
  virtual std::string &getExtendedType() {
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

  
  // CT simulation functions //////////////////////////////////////////////////

  /// notify a CT slave of a quark update
  inline void notifyTargetUpdate() {target_update_event.notify();}
  /// notify a CT slave of a quark update
  inline void notifyTargetUpdate(const sc_time &t) {target_update_event.notify(t);}
  /// notify a CT slave of a quark update
  inline void notifyTargetUpdate(const double d, const sc_time_unit u) {target_update_event.notify(d,u);}
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
  /// get the initiator update event ( use in slave to trigger on master CT quark update notification)
  inline const sc_event& getInitiatorUpdateEvent() const {return initiator_update_event;}
  /// wait for an initiator update event (use in slave to wait for master CT quark update notification)
  inline void waitForInitiatorUpdate(const sc_time &timeout) const {wait(timeout, initiator_update_event);}


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
    msBytesValid = t.msBytesValid;
    mCmd = t.mCmd;
    msData.deepcopyFrom(t.msData);
    //mDataWidth = t.mDataWidth;  // depricated, see data_width parameter in tlm_port
    //sDataWidth = t.sDataWidth;  // depricated, see data_width parameter in tlm_port
    sResp = t.sResp;
    mError = t.mError;
    sError = t.sError; 
    mID = t.mID;  
    is_extended = t.is_extended;
    
    return *this;
  }


  /// Get the router access set to this transaction
  GenericRouterAccess &getRouterAccess() {
    return dynamic_cast<GenericRouterAccess&>(*this);
  }

  /// Get the master access set to this transaction
  GenericMasterAccess &getMasterAccess() {
    return dynamic_cast<GenericMasterAccess&>(*this);
  }

  /// Get the target access set to this transaction
  GenericSlaveAccess &getTargetAccess() {
    return dynamic_cast<GenericSlaveAccess&>(*this);
  }
};



/** 
 * Shared pointer type for GenericTransaction. 
 * Transaction containers always should be wrapped by a shared_ptr
 * in order to avoid memory leakage.
 */
#ifdef USE_PCIE_TRANSACTION
  typedef boost::shared_ptr<PCIeTransaction> GenericTransactionHandle;
#else
  typedef boost::shared_ptr<GenericTransaction> GenericTransactionHandle;
#endif


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
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc);
    //GS_DELTA_WARNING;
  }
  
  void operator()(accessHandle th, phase _p, const sc_time &d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
  
  void operator()(accessHandle th, phase _p, double d, sc_time_unit u, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d, u);
  }

  void operator()(accessHandle th, phase _p, enumPeqDelay d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p(_p);
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
  
  void operator()(accessHandle th, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc);
    //GS_DELTA_WARNING;
  }
  
  void operator()(accessHandle th , const sc_time &d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
  
  void operator()(accessHandle th , double d, sc_time_unit u, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d, u);
  }

  void operator()(accessHandle th , enumPeqDelay d, GenericSimulationModeType m=MODE_BA)
  {
    transactionHandle t = boost::dynamic_pointer_cast<transaction>(th);
    operation p;
    p.setSimulationMode(m);
    typename PORT::GS_ATOM tc(t,p);
    port.out->notify(tc, d);
  }
  
};

// Functor to be used with Notifications
struct  noChangePhase : public PHASE
{
  noChangePhase( const GenericPhase & p) :PHASE(p.state,p.BurstNumber){ };
};

// Functor to be used with Notifications
struct IdlePhase : public PHASE
{
  IdlePhase( const GenericPhase& p) :PHASE(GenericPhase::Idle){};
  IdlePhase( ) :PHASE(GenericPhase::Idle){};
};

// Functor to be used with Notifications
struct setRequestValid_BurstNumber0Phase : public PHASE
{
  setRequestValid_BurstNumber0Phase( const GenericPhase & p) : PHASE(GenericPhase::RequestValid,0) { };
  setRequestValid_BurstNumber0Phase() : PHASE(GenericPhase::RequestValid,0) { };
};

// Functor to be used with Notifications
struct setRequestAcceptedPhase : public PHASE
{
  setRequestAcceptedPhase(const GenericPhase & p) : PHASE(GenericPhase::RequestAccepted)      {      };
  setRequestAcceptedPhase() : PHASE(GenericPhase::RequestAccepted)      {      };
};

// Functor to be used with Notifications
struct setRequestErrorPhase : public PHASE
{
  setRequestErrorPhase(const GenericPhase & p) : PHASE(GenericPhase::RequestError)      {      };
  setRequestErrorPhase() : PHASE(GenericPhase::RequestError)      {      };
};

// Functor to be used with Notifications
struct setDataValid_BurstUpdatePhase : public PHASE
{
  setDataValid_BurstUpdatePhase(const GenericPhase & p) 
  : PHASE(GenericPhase::DataValid, p.BurstNumber+1)
  {};
};

// Functor to be used with Notifications
struct setDataAcceptedPhase : public PHASE
{
  setDataAcceptedPhase(const GenericPhase & p) : PHASE(GenericPhase::DataAccepted,p.BurstNumber)      {      };
};

// Functor to be used with Notifications
struct setDataErrorPhase : public PHASE
{
  setDataErrorPhase(const GenericPhase & p) : PHASE(GenericPhase::DataError,p.BurstNumber)      {      };
};
    
// Functor to be used with Notifications
struct setResponseValidPhase : public PHASE
{
  setResponseValidPhase(const GenericPhase & p) : PHASE(GenericPhase::ResponseValid)      {      };
  setResponseValidPhase() : PHASE(GenericPhase::ResponseValid)      {      };
};


// Functor to be used with Notifications
struct setResponseAcceptedPhase : public PHASE
{
  setResponseAcceptedPhase(const GenericPhase & p) : PHASE(GenericPhase::ResponseAccepted)      {      };
  setResponseAcceptedPhase() : PHASE(GenericPhase::ResponseAccepted)      {      };
};

// Functor to be used with Notifications
struct setResponseErrorPhase : public PHASE
{
  setResponseErrorPhase(const GenericPhase & p) : PHASE(GenericPhase::ResponseError)      {      };
  setResponseErrorPhase() : PHASE(GenericPhase::ResponseError)      {      };
};




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
  void Transact(accessHandle t)
  {
    PORT::b_out->b_transact(boost::dynamic_pointer_cast<transaction>(t)); 
    // cast from base class ACCESS to derived class TRANSACTION
  }
  
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
   * The default master initiator port.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef GenericInitiatorAPI<master_port<PCIeTransaction,PCIeMasterAccess,PCIePhase> > GenericMasterPort;
#else
  typedef GenericInitiatorAPI<master_port<GenericTransaction,GenericMasterAccess,GenericPhase> > GenericMasterPort;
#endif
#define GenericInitiatorPort GenericMasterPort


  //---------------------------------------------------------------------------
  /**
   * The default router initiator port.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef GenericInitiatorAPI<initiator_port<PCIeTransaction,PCIeRouterAccess,PCIePhase> > GenericRouterPort;
#else
  typedef GenericInitiatorAPI<initiator_port<GenericTransaction,GenericRouterAccess,GenericPhase> > GenericRouterPort;
#endif


  //---------------------------------------------------------------------------
  /**
   * The generic slave port.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef GenericTargetAPI<slave_port<PCIeTransaction,PCIeSlaveAccess,PCIePhase> > GenericSlavePort;
#else
  typedef GenericTargetAPI<slave_port<GenericTransaction,GenericSlaveAccess,GenericPhase> > GenericSlavePort;
#endif
#define GenericTargetPort GenericSlavePort
  
  //---------------------------------------------------------------------------
  /**
   * Generic router multi-ports.
   */
  //---------------------------------------------------------------------------
#ifdef USE_PCIE_TRANSACTION
  typedef target_multi_port<PCIeTransaction,PCIeSlaveAccess,PCIePhase> GenericRouterTargetPort;
  typedef initiator_multi_port<PCIeTransaction,PCIeRouterAccess,PCIePhase>  GenericRouterInitiatorPort;
#else
  typedef target_multi_port<GenericTransaction,GenericSlaveAccess,GenericPhase> GenericRouterTargetPort;
  typedef initiator_multi_port<GenericTransaction,GenericRouterAccess,GenericPhase>  GenericRouterInitiatorPort;
#endif


/**
 * Helper function to get access to the transaction from an access handle.
 */
template <class T>
inline GenericTransactionHandle _getTransactionAccess(const T &a) {
  return boost::dynamic_pointer_cast<TRANSACTION>(a);
}


// TODO: build like static casts
#ifdef USE_PCIE_TRANSACTION

/**
 * Helper function to get master access handle from a GS_ATOM 
 */
inline boost::shared_ptr<PCIeMasterAccess> _getMasterAccessHandle(const GS_ATOM &tc) {
  return boost::dynamic_pointer_cast<PCIeMasterAccess>(tc.first);
}
  
/**
 * Helper function to get slave access handle from a GS_ATOM 
 */
inline boost::shared_ptr<PCIeSlaveAccess> _getSlaveAccessHandle(const GS_ATOM &tc) {
  return boost::dynamic_pointer_cast<PCIeSlaveAccess>(tc.first);
}

/**
 * Helper function to get router access handle from a GS_ATOM 
 */
inline boost::shared_ptr<PCIeRouterAccess> _getRouterAccessHandle(const GS_ATOM &tc) {
  return boost::dynamic_pointer_cast<PCIeRouterAccess>(tc.first);
}

/**
 * Helper function to get master access handle from a GS_ATOM 
 */
inline boost::shared_ptr<PCIeMasterAccess> _getMasterAccessHandle(const GenericTransactionHandle &tc) {
  return boost::dynamic_pointer_cast<PCIeMasterAccess>(tc);
}
  
/**
 * Helper function to get slave access handle from a GS_ATOM 
 */
inline boost::shared_ptr<PCIeSlaveAccess> _getSlaveAccessHandle(const GenericTransactionHandle &tc) {
  return boost::dynamic_pointer_cast<PCIeSlaveAccess>(tc);
}

/**
 * Helper function to get router access handle from a GS_ATOM 
 */
inline boost::shared_ptr<PCIeRouterAccess> _getRouterAccessHandle(const GenericTransactionHandle &tc) {
  return boost::dynamic_pointer_cast<PCIeRouterAccess>(tc);
}

#else

/**
 * Helper function to get master access handle from a GS_ATOM 
 */
inline boost::shared_ptr<MASTERACCESS> _getMasterAccessHandle(const GS_ATOM &tc) {
  return tc.first;
}
  
/**
 * Helper function to get slave access handle from a GS_ATOM 
 */
inline boost::shared_ptr<TARGETACCESS> _getSlaveAccessHandle(const GS_ATOM &tc) {
  return tc.first;
}

/**
 * Helper function to get router access handle from a GS_ATOM 
 */
inline boost::shared_ptr<ROUTERACCESS> _getRouterAccessHandle(const GS_ATOM &tc) {
  return tc.first;
}

/**
 * Helper function to get master access handle from a GS_ATOM 
 */
inline boost::shared_ptr<MASTERACCESS> _getMasterAccessHandle(const GenericTransactionHandle &tc) {
  return tc;
}
  
/**
 * Helper function to get slave access handle from a GS_ATOM 
 */
inline boost::shared_ptr<TARGETACCESS> _getSlaveAccessHandle(const GenericTransactionHandle &tc) {
  return tc;
}

/**
 * Helper function to get router access handle from a GS_ATOM 
 */
inline boost::shared_ptr<ROUTERACCESS> _getRouterAccessHandle(const GenericTransactionHandle &tc) {
  return tc;
}

#endif

/**
 * Helper function to get the phase from a GS_ATOM
 */
inline PHASE _getPhase(const GS_ATOM &tc) { return tc.second; }
