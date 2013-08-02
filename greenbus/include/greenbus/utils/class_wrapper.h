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

/*******************************************************************************
**
** $Log:  $
**
**
*******************************************************************************/

#ifndef __CLASS_WRAPPER_H__
#define __CLASS_WRAPPER_H__

#ifdef USE_DUST
#include "scv.h"
#endif


namespace tlm {


//---------------------------------------------------------------------------
/**
 * Wrapper so basic C++ types (POD types) turns into classes while
 * maintaining all the original operations (but not the increment-like ones)
 */
//---------------------------------------------------------------------------
template <typename T>
struct class_wrapper {
#ifdef USE_DUST
  friend class scv_extensions<class_wrapper<T> >;
#endif
  T value;
  operator const T&() const {return value;}
  //operator T&() {return value;}
  T& operator =(const T& v) {return value = v;}
  class_wrapper<T>() {}
  class_wrapper<T>(const T& v) {value = v;}  
};


// stream operators

template <typename T>
inline
std::istream& operator >> (std::istream& is, class_wrapper<T> obj) {
  is >> obj.value;
  return is;
}

template <typename T>
inline
std::ostream& operator << (std::ostream& os, const class_wrapper<T> obj) {
  os << obj.value;
  return os;
}



} // end of namespace tlm

#endif
