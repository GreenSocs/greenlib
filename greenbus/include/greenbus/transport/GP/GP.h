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

#ifndef __INCL_MACRO_DEFS__
#define __INCL_MACRO_DEFS__

#define __INCLSTART__ greenbus/transport
#define __INCLEND__ h

#define __MACRO_STRINGYFY__(S) __STRINGIFY__(S)
#define __STRINGIFY__(S) #S

#endif

#ifdef EXTENDED_TRANSACTION
#include __MACRO_STRINGYFY__(__INCLSTART__/EXTENDED_TRANSACTION/EXTENDED_TRANSACTION.__INCLEND__)

#define __GP_h__
#endif

#ifndef __GP_h__
#define __GP_h__

#ifdef USE_STATIC_CASTS

#include "greenbus/core/tlm.h"
#include "greenbus/transport/generic.static_casts.fwdD.h"

namespace tlm{

typedef tlm::gp::GenericTransaction GenericTransaction;
typedef tlm::gp::GenericSlaveAccess GenericSlaveAccess;
typedef tlm::gp::GenericMasterAccess GenericMasterAccess;
typedef tlm::gp::GenericRouterAccess GenericRouterAccess;
typedef tlm::gp::GenericPhase GenericPhase;
typedef tlm::gp::GenericTransactionCreator GenericTransactionCreator;

}

#include "greenbus/transport/generic.static_casts.head.h"

#include "greenbus/transport/generic.static_casts.tail.h"

#include "greenbus/transport/generic.static_casts.blocking.h"

#else
#include "greenbus/transport/generic.h"
#endif

#endif
