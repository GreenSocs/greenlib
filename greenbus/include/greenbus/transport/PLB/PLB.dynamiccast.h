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

#ifndef __PLB_dynamic_h__
#define __PLB_dynamic_h__

#include "greenbus/core/attributes.h"
#include "greenbus/transport/generic.h"

/**
 *
 */

using namespace tlm;
#define PLBPHASES PLBPhase::SecondaryRequest, PLBPhase::WrPrim, PLBPhase::RdPrim

namespace PLB{ 

  GSGB_ATTRIBUTE (MPrio, gs_uint32); // "PLB Master priority" 

  namespace PLBPhase
  {
    enum {
      SecondaryRequest = GenericPhase::LAST_GENERIC_PHASE,
      WrPrim,
      RdPrim
    };
  };

}

namespace PLB {

  // the PLB additional phases
  class PLBMasterAccess : public virtual GenericMasterAccess {

  public :

    virtual void setMPrio(const MPrio&) =0;
    virtual const MPrio& getMPrio() const =0;

  };

  class PLBTargetAccess : public virtual GenericSlaveAccess {

  public :

    virtual const MPrio& getMPrio() const =0;

  };

  class PLBRouterAccess : public virtual tlm::GenericRouterAccess {

  public :
    virtual const MPrio& getMPrio() const =0;

  };

  class PLBTransaction :
    public virtual PLBMasterAccess,
    public virtual PLBTargetAccess,
    public virtual PLBRouterAccess,
    public virtual GenericTransaction 
  {

 private:
    MPrio mPrio;

  public:
    void setMPrio(const MPrio& _mPrio) {mPrio=_mPrio;};
    const MPrio& getMPrio() const {return mPrio;}

    PLBTransaction() 
      : GenericTransaction(), 
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


  };

  typedef boost::shared_ptr<PLBTransaction> PLBTransaction_P;
  typedef boost::shared_ptr<PLBTransaction> PLBTransactionHandle;


typedef GenericInitiatorAPI<master_port<GenericTransaction, PLBMasterAccess, GenericPhase, PLBTransaction > > PLBMasterPort;
typedef GenericTargetAPI<target_port<GenericTransaction, GenericSlaveAccess, GenericPhase> > PLBSlavePort;

}
#endif
