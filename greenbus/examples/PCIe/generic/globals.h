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

// 
// Global settings for simulation
//

#ifndef __globals_h__
#define __globals_h__


// if you want to use static casts:
#define USE_STATIC_CASTS
// if you want to use dynamic casts:
//#define EXTENDED_TRANSACTION PCIe  // Uses PCIe transaction
//#define USE_PCIE_TRANSACTION  // Enables typedefs Generic = PCIe

// PCIe Debug Output
#define PCIeDEBUG_ON

// Additional Check Rules
#define CHECK_RULES_ENABLE

// GreenBus Debug Output
//#define VERBOSE

// for ram
//#define DEMO_VERBOSE

#ifdef DEMO_VERBOSE
# define DEMO_TRACE(name, msg, args...) { printf("@%s /%d (%s): ", sc_time_stamp().to_string().c_str(), (unsigned)sc_delta_count(), name); printf(msg, ##args); printf("\n"); } 
#else
# define DEMO_TRACE(name, msg, args...)
#endif


#endif