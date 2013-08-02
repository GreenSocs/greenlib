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

#ifndef USE_PLB_TRANSACTION
  #error Define USE_PLB_TRANSACTION has to be set, when using the PLB Protocol (with static casts)
  #define __PLB_static_h__
#endif

#ifndef __PLB_static_h__
#define __PLB_static_h__

#include "greenbus/core/attributes.h"
#include "greenbus/api/basicPorts.h"

/**
 *
 */

#define PLBPHASES PLBPhase::SecondaryRequest, PLBPhase::WrPrim, PLBPhase::RdPrim

namespace PLB{ 

  GSGB_ATTRIBUTE (MPrio, tlm::gs_uint32); // "PLB Master priority" 

  namespace PLBPhase
  {
    enum {
      SecondaryRequest = tlm::gp::GenericPhase::LAST_GENERIC_PHASE,
      WrPrim,
      RdPrim
    };
  };

}

namespace PLB {

  class PLBTransaction :
    public virtual tlm::gp::GenericTransaction
  {

  TRANSACTION_MANDATORY_STUFF(PLBMasterAccess,PLBSlaveAccess,PLBRouterAccess, PLBTransaction);

 private:
    MPrio mPrio;

  protected:
    inline void setMPrio(const MPrio& _mPrio) {mPrio=_mPrio;};
    inline const MPrio& getMPrio() const {return mPrio;}
  public:
    PLBTransaction() 
      : tlm::gp::GenericTransaction(), 
        mPrio(1)
    {
      is_extended=true; // important: to indicate extended transaction
    }

    /// create the extended attribute iterator
    virtual void createExtendedAttributeIterator() {
      GenericTransaction::createExtendedAttributeIterator(); // important: call base method

      INITATTRIBUTE(mPrio);
      ADDEXTTOPHASE4(mPrio, REQUESTPHASE, DATAPHASE, RESPONSEPHASE, PLBPHASES);
      ADDTOPHASE1(mCmd, PLBPhase::SecondaryRequest);
      ADDTOPHASE1(mAddr,PLBPHASES);    
      ADDTOPHASE1(msData,PLBPhase::WrPrim);    
      ADDTOPHASE1(mBurstLength,PLBPhase::SecondaryRequest);    
      ADDTOPHASE1(mID, PLBPHASES);
    }

    /// get the extension type of this transaction container
    inline 
    virtual std::string &getExtendedType() {
      if (extendedType==NULL) {
        extendedType = new std::string("PLB");
      }
      return *extendedType;
    }

    /// Get the router access set to this transaction
    inline PLBRouterAccess &getRouterAccess(); 

    /// Get the master access set to this transaction
    inline PLBMasterAccess &getMasterAccess();

    /// Get the target access set to this transaction
    inline PLBSlaveAccess &getSlaveAccess();


  };

  typedef tlm::shared_ptr<PLBTransaction> PLBTransaction_P;

  class PLBMasterAccess : 
    public virtual tlm::gp::GenericMasterAccess,
    public virtual PLBTransaction {

  public :
    using PLBTransaction::setMPrio;
    using PLBTransaction::getMPrio;

  };

  class PLBSlaveAccess : 
    public virtual tlm::gp::GenericSlaveAccess,
    public virtual PLBTransaction {

  public :
    using PLBTransaction::getMPrio;

  };

  class PLBRouterAccess : 
    public virtual tlm::gp::GenericRouterAccess,
    public virtual PLBTransaction {

  public :
    using PLBTransaction::getMPrio;
    
  };


/// Get the router access set to this transaction
inline PLBRouterAccess& PLBTransaction::getRouterAccess() {
  return *(this->getSuperClassPtr());
}

/// Get the master access set to this transaction
inline PLBMasterAccess& PLBTransaction::getMasterAccess() {
  return *(this->getSuperClassPtr());
}

/// Get the target access set to this transaction
inline PLBSlaveAccess& PLBTransaction::getSlaveAccess() {
  return *(this->getSuperClassPtr());
}


typedef tlm::GenericInitiatorAPI<tlm::master_port<PLBTransaction, PLBMasterAccess, tlm::GenericPhase, PLBTransactionCreator> > PLBMasterPort;
typedef tlm::GenericTargetAPI<tlm::slave_port<PLBTransaction, PLBSlaveAccess, tlm::GenericPhase> > PLBSlavePort;

}
#endif
