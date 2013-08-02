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


#ifndef __TLM_H__
#define __TLM_H__

// removed (call only PEQ is always used): #define USE_CALLONLY_PEQ // use call only PEQ (note that immediate notifications result in plain interface method calls) // Do NOT comment this out: GreenControl should use the CALL variant
#define USE_DELTA_LISTS // use tuning for delta-cycle payload events

//#define USE_STATIC_CASTS  // do not define here. Do a make STATIC=yes

// do not undef this when using SystemC2.1 and dynamic processes!
#define DONT_USE_BOOST_POOL // use GreenBus memory pool implementation

// tlm.h is included by GreenControl, possibly BEFORE include of GreenBus (GP.h etc):
//  be careful when adding includes here!
#include "greenbus/utils/gs_trace.h"
#include "greenbus/utils/gs_datatypes.h"
#include "greenbus/core/transactionBase.h"

#endif


