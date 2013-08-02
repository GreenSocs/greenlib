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

#ifndef __genericScheduler_if_h__
#define __genericScheduler_if_h__

#include "greenbus/transport/genericRouter.h"
#include <set>
#include <map>

using namespace tlm;

//--------------------------------------------------------------------------
/**
 * The generic scheduler interface
 */
//--------------------------------------------------------------------------
class GenericScheduler_if 
: public sc_interface
{
public:

  virtual void enqueue(GS_ATOM& t) =0;
  virtual GS_ATOM& dequeue(bool remove=true) =0;
  virtual bool isPending() =0; //differs from isempyt because there could be something inside that is not "valid" yet
  virtual bool isEmpty() =0;
  virtual void setMasterMap(std::map<unsigned long, unsigned int> *masterMap_) =0;

};
  
#endif
