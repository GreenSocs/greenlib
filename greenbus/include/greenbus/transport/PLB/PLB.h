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

#ifndef __PLB_h__
#define __PLB_h__

#warning Info: Using PLB Transaction.

#ifndef EXTENDED_TRANSACTION
# define EXTENDED_TRANSACTION PLB
#endif

#ifdef USE_STATIC_CASTS
#define USE_PLB_TRANSACTION

#include "greenbus/core/tlm.h"
#include "greenbus/transport/generic.static_casts.fwdD.h"

namespace PLB{
  class PLBMasterAccess;
  class PLBSlaveAccess;
  class PLBRouterAccess;
  class PLBTransaction;
  #include "greenbus/transport/generic.static_casts.creator.h"
  typedef transactionCreator<PLBMasterAccess, PLBSlaveAccess, PLBRouterAccess, PLBTransaction> PLBTransactionCreator;
}
  
namespace tlm{

typedef PLB::PLBTransaction GenericTransaction;
typedef PLB::PLBSlaveAccess GenericSlaveAccess;
typedef PLB::PLBMasterAccess GenericMasterAccess;
typedef PLB::PLBRouterAccess GenericRouterAccess;
typedef tlm::gp::GenericPhase GenericPhase;
typedef PLB::PLBTransactionCreator GenericTransactionCreator;
}

#include "greenbus/transport/generic.static_casts.head.h"

#include "greenbus/transport/PLB/PLBTransaction.h"

#include "greenbus/transport/generic.static_casts.tail.h"

#include "greenbus/transport/generic.static_casts.blocking.h"

#else
#include "greenbus/transport/generic.h"
#include "greenbus/transport/PLB/PLB.dynamiccast.h"
#endif

#endif
