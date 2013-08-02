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

#ifndef __PCIe_h__
#define __PCIe_h__

/**
 * This is the main file for the PCI Express (PCIe) GreenBus model
 */

// //////////////////////// PCIe specific //////////////////////////// ///

// #define VERBOSE      // GreenBus Debug Output: define this in your model's global file
// #define PCIeDEBUG_ON // PCIe Debug Output: define this in your model's global file
// #define CHECK_RULES_ENABLE // Additional Check Rules: define this in your model's global file

#ifdef PCIeDEBUG_ON
    #define PCIeDEBUG(string) { cout << "@" << sc_time_stamp() << " /" << sc_delta_count() << " ("<<this->name() << "): " << string << endl << flush; }
    #define PCIeDEBUG2(name, msg, args...) { printf("@%s /%d (%s): ", sc_time_stamp().to_string().c_str(), (unsigned)sc_delta_count(), name); printf(msg, ##args); printf("\n"); } 
#else
    #define PCIeDEBUG(string)
    #define PCIeDEBUG2(name, msg, args...)
#endif

// enables some checks which makes things slower but safer
#ifdef CHECK_RULES_ENABLE
  #define CHECK_RULES(string) string
#else
  #define CHECK_RULES(string)
#endif


// //////////////// includes with general scheme ///////////////////// ///

#warning Info: Using PCIe Transaction.

#ifndef EXTENDED_TRANSACTION
# define EXTENDED_TRANSACTION PCIe
#endif

#define TLMCORE "greenbus/core/tlm.h"

// **** if static casts ****
#ifdef USE_STATIC_CASTS
#  define USE_PCIE_TRANSACTION

#  include "greenbus/core/tlm.h"
#  include "greenbus/transport/generic.static_casts.fwdD.h"

  namespace tlm {

    namespace PCIe {
      class PCIeTransaction;
      class PCIePhase;
      class PCIeMasterAccess;
      class PCIeSlaveAccess;
      class PCIeRouterAccess;
      #include "greenbus/transport/generic.static_casts.creator.h"
      typedef transactionCreator<PCIeMasterAccess, PCIeSlaveAccess, PCIeRouterAccess, PCIeTransaction> PCIeTransactionCreator;
    }

    typedef PCIe::PCIeTransaction GenericTransaction;
    typedef PCIe::PCIeSlaveAccess GenericSlaveAccess;
    typedef PCIe::PCIeMasterAccess GenericMasterAccess;
    typedef PCIe::PCIeRouterAccess GenericRouterAccess;
    typedef PCIe::PCIePhase GenericPhase;
    typedef PCIe::PCIeTransactionCreator GenericTransactionCreator;

  }


# include "greenbus/transport/generic.static_casts.head.h"

# include "PCIeDefs.h"
# include "PCIePhase.static_casts.h"
# include "PCIeTransaction.static_casts.h"
# include "PCIeAccessClasses.static_casts.h"
# include "PCIeTail.static_casts.h"

# include "greenbus/transport/generic.static_casts.tail.h"

# include "greenbus/transport/generic.static_casts.blocking.h"

// **** if dynamic casts ****
#else

#ifndef USE_PCIE_TRANSACTION
# warning You should define USE_PCIE_TRANSACTION when using dynamic casts!
#endif
#  define USE_PCIE_TRANSACTION

# include "greenbus/core/tlm.h"
# include "PCIeDefs.h"
# include "greenbus/transport/generic.h"
# include "PCIePhase.dynamic_casts.h"
# include "PCIeAccessClasses.dynamic_casts.h"
# include "PCIeTransaction.dynamic_casts.h"

// **** end static/dynamic casts ****
#endif 


namespace tlm {
namespace PCIe { 

#ifdef USE_STATIC_CASTS
  typedef tlm::shared_ptr<PCIeMasterAccess> PCIeMasterAccessHandle;
  typedef tlm::shared_ptr<PCIeSlaveAccess>  PCIeSlaveAccessHandle;
  typedef tlm::shared_ptr<PCIeRouterAccess> PCIeRouterAccessHandle;
  typedef tlm::shared_ptr<PCIeTransaction>  PCIeTransactionHandle;
#else
  typedef boost::shared_ptr<PCIeMasterAccess> PCIeMasterAccessHandle;
  typedef boost::shared_ptr<PCIeSlaveAccess>  PCIeSlaveAccessHandle;
  typedef boost::shared_ptr<PCIeRouterAccess> PCIeRouterAccessHandle;
  typedef boost::shared_ptr<PCIeTransaction>  PCIeTransactionHandle;
#endif
  typedef unevenpair<PCIeTransactionHandle,PCIePhase> PCIeTransactionContainer;

#ifdef USE_STATIC_CASTS
  /// PCIeBidirectianalPort is only bidirectional if it is instantiated with is_bidirectional=true
  typedef GenericInitiatorAPI<master_port<PCIeTransaction, PCIeMasterAccess, PCIePhase, PCIeTransactionCreator > > PCIeBidirectionalPort;
#else
  /// PCIeBidirectianalPort is only bidirectional if it is instantiated with is_bidirectional=true
  typedef GenericInitiatorAPI<master_port<PCIeTransaction, PCIeMasterAccess, PCIePhase, PCIeTransaction > > PCIeBidirectionalPort;
#endif
  
  typedef initiator_multi_port<PCIeTransaction,PCIeRouterAccess,PCIePhase>  PCIeRouterDevicePort;


} // end namespace PCIe
} // end namespace tlm


#endif
