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

#ifndef _GB_PARAM_ROOT_H_
#define _GB_PARAM_ROOT_H_

namespace tlm {

/**
 * Base class for all gb_param template specialisations.
 */
class gb_param_root {
public:
  gb_param_root() : name("N/A") {}
  gb_param_root(const char *n) : name(n) {}
  virtual ~gb_param_root() {} // force to be polymorphic
  
  /** 
   * sets the value of this parameter.
   * @param str the new value represented as a string.
   */
  virtual void set(const std::string &str) =0;
  
  /**
   * gets the value of this parameter.
   * @return the value of this parameter represented as a string.
   */
  virtual const std::string get() =0;


  /**
   * Get the name of this parameter.
   * @return Name of the parameter.
   */
  const std::string &getName() { 
    return name; 
  }
  
  
  std::string name;
  
};

} // namespace tlm

#endif /* _GB_PARAM_ROOT_H_ */