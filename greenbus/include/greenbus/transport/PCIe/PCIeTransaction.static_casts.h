// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
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

// included by PCIe.h


#ifndef __PCIeTransaction_STATIC_CASTS_h__
#define __PCIeTransaction_STATIC_CASTS_h__


namespace tlm {
namespace PCIe {

class PCIeMasterAccess;
class PCIeSlaveAccess;
class PCIeRouterAccess;

/// PCIe Transaction derived from gp::GenericTransaction
/**
 * Transaction for PCIe communication
 */
class PCIeTransaction
 : public virtual gp::GenericTransaction {

   TRANSACTION_MANDATORY_STUFF(PCIeMasterAccess,PCIeSlaveAccess,PCIeRouterAccess, PCIeTransaction);

private:

  MPCIeCmd mPCIeCmd;
  MTrafficClass mTrafficClass;
  MDigestExists mDigestExists;
  MECRCpoisoned mECRCpoisoned;
  // MBurstLength already exists, see generic.dynamic_casts.h
  // MAddr        already exists, see generic.dynamic_casts.h
  MIsIDbasedRouting mIsIDbasedRouting;
  MIsImplicitRouting mIsImplicitRouting;
  MBusNo  mBusNo; // target Bus Number
  MDevNo  mDevNo; // target Device Number
  MFuncNo mFuncNo;  // target Function Number // is a typedef of MConnID!!
  MRegNo  mRegNo;
  MBusNo  mRequBusNo; // Requester Bus Number
  MDevNo  mRequDevNo; // Requester Device Number
  MFuncNo mRequFuncNo;  // Requester Function Number // is a typedef of MConnID!!
  MByteEn mByteEn;
  // MID          already exists, see generic.dynamic_casts.h
  MTagID mTagID;
  MConfigAddr mConfigAddr;
  MMessageType mMessageType;
  MMessageCode mMessageCode;
  MComplStatus mComplStatus;
  MVC mVC;
  //MHdrFlowControl mHdrFlowControl;
  //MDataFlowControl mDataFlowControl;
  MDLLPtype mDLLPtype;
  MVendorID mVendorID;
  MVendorHeaderData mVendorHeaderData;
  
protected:
  inline void setMPCIeCmd(const MPCIeCmd& _mPCIeCmd) {
    mPCIeCmd=_mPCIeCmd;
    mCmd=*(  reinterpret_cast<const MCmd*>(  &mPCIeCmd  )  ); // TODO performance???
  }
  inline const MPCIeCmd& getMPCIeCmd() const {return mPCIeCmd;}
  inline void setMTrafficClass(const MTrafficClass& _mTrafficClass) {mTrafficClass=_mTrafficClass;}
  inline const MTrafficClass& getMTrafficClass() const {return mTrafficClass;}
  inline void setMDigestExists(const MDigestExists& _mDigestExists) {mDigestExists=_mDigestExists;}
  inline const MDigestExists& getMDigestExists() const {return mDigestExists;}
  inline void setMECRCpoisoned(const MECRCpoisoned& _mECRCpoisoned) {mECRCpoisoned=_mECRCpoisoned;}
  inline const MECRCpoisoned& getMECRCpoisoned() const {return mECRCpoisoned;}
  inline void setMIsIDbasedRouting(const MIsIDbasedRouting& _mIsIDbasedRouting) {mIsIDbasedRouting=_mIsIDbasedRouting;}
  inline const MIsIDbasedRouting& getMIsIDbasedRouting() const {return mIsIDbasedRouting;}
  inline void setMIsImplicitRouting(const MIsImplicitRouting& _mIsImplicitRouting) {mIsImplicitRouting=_mIsImplicitRouting;}
  inline const MIsImplicitRouting& getMIsImplicitRouting() const {return mIsImplicitRouting;}
  inline void setMBusNo(const MBusNo& _mBusNo) {mBusNo=_mBusNo;}
  inline const MBusNo& getMBusNo() const {return mBusNo;}
  inline void setMDevNo(const MDevNo& _mDevNo) {mDevNo=_mDevNo;}
  inline const MDevNo& getMDevNo() const {return mDevNo;}
  inline void setMFuncNo(const MFuncNo& _mFuncNo) {mFuncNo=_mFuncNo;}
  inline const MFuncNo& getMFuncNo() const {return mFuncNo;}
  inline void setMRegNo(const MRegNo& _mRegNo) {mRegNo=_mRegNo;}
  inline const MRegNo& getMRegNo() const {return mRegNo;}
  inline void setMRequBusNo(const MBusNo& _mRequBusNo) {mRequBusNo=_mRequBusNo;}
  inline const MBusNo& getMRequBusNo() const {return mRequBusNo;}
  inline void setMRequDevNo(const MDevNo& _mRequDevNo) {mRequDevNo=_mRequDevNo;}
  inline const MDevNo& getMRequDevNo() const {return mRequDevNo;}
  inline void setMRequFuncNo(const MFuncNo& _mRequFuncNo) {mRequFuncNo=_mRequFuncNo;}
  inline const MFuncNo& getMRequFuncNo() const {return mRequFuncNo;}
  inline void setMByteEn(const MByteEn& _mByteEn) {mByteEn=_mByteEn;}
  inline const MByteEn& getMByteEn() const {return mByteEn;}
  inline void setMTagID(const MTagID& _mTagID) {mTagID=_mTagID;}
  inline const MTagID& getMTagID() const {return mTagID;}
  inline void setMConfigAddr(const MConfigAddr& _mConfigAddr) {mConfigAddr=_mConfigAddr;}
  inline const MConfigAddr& getMConfigAddr() const {return mConfigAddr;}
  inline void setMMessageType(const MMessageType& _mMessageType) {mMessageType=_mMessageType;}
  inline const MMessageType& getMMessageType() const {return mMessageType;}
  inline void setMMessageCode(const MMessageCode& _mMessageCode) {mMessageCode=_mMessageCode;}
  inline const MMessageCode& getMMessageCode() const {return mMessageCode;}
  inline void setMComplStatus(const MComplStatus& _mComplStatus) {
    mComplStatus=_mComplStatus;
#ifdef USE_PCIE_TRANSACTION
    // See below for more actions concerning PCIe - Generic connections
    msBytesValid = mBurstLength;
#endif 
  }
  inline const MComplStatus& getMComplStatus() const {return mComplStatus;}
  inline void setMVC(const MVC& _mVC) {mVC=_mVC;}
  inline const MVC& getMVC() const {return mVC;}
  inline void setMDLLPtype(const MDLLPtype& _mDLLPtype) {mDLLPtype=_mDLLPtype;}
  inline const MDLLPtype& getMDLLPtype() const {return mDLLPtype;}
  inline void setMVendorID(const MVendorID& _mVendorID) {mVendorID=_mVendorID;}
  inline const MVendorID& getMVendorID() const {return mVendorID;}
  inline void setMVendorHeaderData(const MVendorHeaderData& _mVendorHeaderData) {mVendorHeaderData=_mVendorHeaderData;}
  inline const MVendorHeaderData& getMVendorHeaderData() const {return mVendorHeaderData;}

public:

  PCIeTransaction() 
    : gp::GenericTransaction(), 
      // TODO: only init important quarks (same as in reset)
      mPCIeCmd(NO_CMD), 
      mTrafficClass(0), 
      mDigestExists(0),
      mECRCpoisoned(0),
      mIsIDbasedRouting(false),
      mIsImplicitRouting(false),
      mBusNo(0),
      mDevNo(0),
      mFuncNo(0),
      mRegNo(0),
      mRequBusNo(256), // set to not allowed value: allows check in Access methods
      mRequDevNo(32),  // set to not allowed value: allows check in Access methods
      mRequFuncNo(0),
      mByteEn(0),
      mTagID(0),
      mConfigAddr(0),
      mMessageType(NO_MESSAGE),
      mMessageCode(NO_MESSAGE_CODE),
      mComplStatus(NO_COMPL_STATUS),
      mVC(0),
      mDLLPtype(0),
      mVendorID(0),
      mVendorHeaderData(0)
  {
    is_extended=true; // important: to indicate extended transaction
  }

  /// create the extended attribute iterator
  virtual void createExtendedAttributeIterator() {
    gp::GenericTransaction::createExtendedAttributeIterator(); // important: call base method

    // TODO!!
    
    /*INITATTRIBUTE(mPrio);
    ADDEXTTOPHASE4(mPrio, REQUESTPHASE, DATAPHASE, RESPONSEPHASE, PLBPHASES);
    ADDTOPHASE1(mCmd, PLBPhase::SecondaryRequest);
    ADDTOPHASE1(mAddr,PLBPHASES);    
    ADDTOPHASE1(msData,PLBPhase::WrPrim);    
    ADDTOPHASE1(mBurstLength,PLBPhase::SecondaryRequest);    
    ADDTOPHASE1(mID, PLBPHASES);*/
  }

  void reset() {
    gp::GenericTransaction::reset(); // call base method, too!!
    // TODO: reset only important quarks!
    mPCIeCmd = NO_CMD;
    mTrafficClass = 0; 
    mDigestExists = 0;
    mECRCpoisoned = 0;
    mIsIDbasedRouting = false;
    mIsImplicitRouting = false;
    mBusNo = 0;
    mDevNo = 0;
    mFuncNo = 0;
    mRegNo = 0;
    mRequBusNo = 256; // set to not allowed value: allows check in Access methods
    mRequDevNo = 32;  // set to not allowed value: allows check in Access methods
    mRequFuncNo = 0;
    mByteEn = 0;
    mTagID = 0;
    mConfigAddr = 0;
    mMessageType = NO_MESSAGE;
    mMessageCode = NO_MESSAGE_CODE;
    mComplStatus = NO_COMPL_STATUS;
    mVC = 0;
    mDLLPtype = 0;
    mVendorID = 0;
    mVendorHeaderData = 0;
  }
  
  /// get the extension type of this transaction container
  inline std::string &getExtendedType() {
    if (extendedType==NULL) {
      extendedType = new std::string("PCIe");
    }
    return *extendedType;
  }
  

protected:
  // //////////// Map GENERIC quark access to PCIe //////////////////////////////// //
  // works for Generic_MCMD_RD and Generic_MCMD_WR

  inline void setMCmd(const MCmd& _mCmd) {
    mCmd = _mCmd;
    // if CHECK_RULES: if not supported command: print warning and set PCIeCommand to NO_CMD
    CHECK_RULES( if (Generic_MCMD_RD != _mCmd && Generic_MCMD_WR != _mCmd) { PCIeDEBUG2("PCIeTransaction", "Usage of not supported command: %i", (gs_uint32)_mCmd); SC_REPORT_WARNING("PCIeTransaction", "Generic module uses not supported command mCmd!"); } )
    if (Generic_MCMD_RD == _mCmd || Generic_MCMD_WR == _mCmd) {
      mPCIeCmd = mCmd;
    } else mPCIeCmd = NO_CMD;
  }
  inline const MCmd& getMCmd() const {
    return reinterpret_cast<const MCmd&>( mPCIeCmd );
  }
  
  // Error handling
  inline void setMError(const Error& _mError) {
    mError=_mError;
    if (_mError > 0) mComplStatus = UnsupportedRequest; // TODO CompleterAbort is a harder error??
  }
  inline const Error getMError() const {
    if (mComplStatus > SuccessfulCompletion) // means: not SuccessfulCompletion and not NO_COMPL_STATUS
      return GenericError::Generic_Error_AccessDenied; // any error TODO
    return mError;
  }
  inline void setSError(const Error& _sError) {
    sError=_sError;
    if (_sError > 0) mComplStatus = UnsupportedRequest; // TODO CompleterAbort is a harder error??
  }
  inline const Error getSError() const {
    if (mComplStatus > SuccessfulCompletion) // means: not SuccessfulCompletion and not NO_COMPL_STATUS
      return GenericError::Generic_Error_AccessDenied; // any error TODO
    return sError;
  }
  
  // ** BEGIN Set Completion Status **
  inline void setMSBytesValid(const MSBytesValid& _msBytesValid) {
    // Wait for the last phase with setting completion
    if (_msBytesValid == mBurstLength)
      mComplStatus = SuccessfulCompletion;
    msBytesValid=_msBytesValid;
  }
  // set msBytesValid in PCIe setMComplStatus(...)
 
  // depricated in GreenBus: Use msBytesValid instead!!!
  // sBurstLength not used in PCIe, PCIe only uses mBurstLength
  inline void setSBurstLength(const SBurstLength& _sBurstLength) {
    sBurstLength=_sBurstLength; 
    mBurstLength=_sBurstLength; 
    msBytesValid=_sBurstLength;
    mComplStatus = SuccessfulCompletion;
  }

  // depricated in GreenBus: Use msBytesValid instead!!!
  inline const SBurstLength& getSBurstLength() const {
    return mBurstLength;
  }
  // ** END Set Completion Status **

public:

  /// Get the master access set to this transaction
   PCIeMasterAccess &getMasterAccess();/* {
    return reinterpret_cast<PCIeMasterAccess&>(*this); // TODO boost::static_pointer_cast<PCIeSlaveAccess>(*this) ??
  }*/

  /// Get the slave access set to this transaction
   PCIeSlaveAccess &getSlaveAccess();/* {
    return reinterpret_cast<PCIeSlaveAccess&>(*this); // TODO boost::static_pointer_cast<PCIeSlaveAccess>(*this) ??
  }*/

  /// Get the router access set to this transaction
   PCIeRouterAccess &getRouterAccess();/* {
    return reinterpret_cast<PCIeRouterAccess&>(*this); // TODO boost::static_pointer_cast<PCIeSlaveAccess>(*this) ??
  }*/

};


} // end namespace PCIe
} // end namespace tlm


#endif
