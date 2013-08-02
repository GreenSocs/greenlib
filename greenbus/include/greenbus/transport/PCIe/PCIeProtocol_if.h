// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf, Robert Guenzel
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

#ifndef __PCIeProtocol_if_h__
#define __PCIeProtocol_if_h__

#include "greenbus/core/tlm.h"
//#include "greenbus/transport/GP/GP.h"

namespace tlm {
namespace PCIe {

//--------------------------------------------------------------------------
/**
 * The PCIe protocol interface.
 */
//--------------------------------------------------------------------------

class PCIeProtocol_if 
: public sc_interface 
{  
  
public:

  virtual bool registerMasterAccess(PCIeTransactionContainer& transaction) =0;
  virtual bool processMasterAccess() =0;
  virtual bool registerSlaveAccess(PCIeTransactionContainer& transaction) =0;
  virtual bool processSlaveAccess() =0;

  virtual void assignProcessMasterAccessSensitivity(sc_process_b* pMethod) =0;
  virtual void assignProcessSlaveAccessSensitivity(sc_process_b* pMethod) =0;


};
 
} // end namespace PCIe
} // end namespace tlm

#endif
