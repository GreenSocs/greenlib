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

// globals

// ----------------------- User settings -----------------------

   #define AMOUNT_TRANSACTIONS 1000000  // How many transactions are created
   #define USE_RETURN_PATH         // if the return path should be used (if not, normal transactions are used)
   //#define ENABLE_PERF_TRACE       // if the output is displayed
   #define AMOUNT_PEQ_CONCATENATIONS 5 // how many PEQs the messages go through (minimum 1!)

// ----------------------- ----------------------- -----------------------

#ifdef ENABLE_PERF_TRACE
  #define PERF_TRACE(string)  { cout << "@" << sc_time_stamp() << " /" << sc_delta_count() << " ("<<name() << "): " << string << endl << flush; }
#else
  #define PERF_TRACE(string) 
#endif
