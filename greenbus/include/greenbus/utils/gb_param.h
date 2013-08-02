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

#ifndef _GB_PARAM_H_
#define _GB_PARAM_H_

#include "greenbus/utils/gb_param_root.h"
#include "greenbus/utils/gb_configurable.h"
#include "greenbus/utils/gs_datatypes.h"


//-----------------------------------------------------------------------------
// gb_param
//-----------------------------------------------------------------------------

namespace tlm {

/**
 * A class wrapper for runtime configurable SC_MODULE parameters.
 * May only be used in classes that inherit from gb_configurable.
 * All gb_param parameters must be initialized by the GB_PARAM macro.
 * You can write your own template specialization for this class
 * to support user data types.
 */
template <typename T>
class gb_param : public gb_param_root
{
public:
  T value;

  gb_param() {}

  gb_param(const char *n) : gb_param_root(n) {}

  gb_param(const char *n, T v) : gb_param_root(n), value(v) {}
  
  gb_param(const T& v) : value(v) {}
  
  virtual ~gb_param() {}

  /**
   * Get the value of this parameter.
   */
  operator const T& () const { 
    return value; 
    // TODO: could inform monitor callback function here
  }

  /**
   * Set the value of this parameter.
   */
  T& operator = (const T& v) { 
    return value = v; 
    // TODO: could inform monitor callback function here
  }

  /**
   * Set the value of this parameter with a string.
   * The default implementation uses stringstream to convert the string input to
   * the parameter type. This works with most of the POD and STL data types.
   * However, for special user data types, a template specialisation is necessary.
   * @param str The new value for this parameter, represented as a string.
   */
  void set(const std::string &str) {
    std::istringstream is(str);
    is >> value;
    //std::cout << "GB_PARAM: Param [" << getName() << "] set method called. Value now " << value << std::endl;
    // TODO: could inform callback function here
  }
  

  /**
   * Get the value of this parameter as a string.
   * The default implementation uses stringstream to convert the parameter
   * type into a string representation. This works with most of the POD and STL 
   * data atypes. However, for special user data types, a template specialisation
   * is necessary.
   * @return String representation of the current value of this parameter.
   */
  const std::string get() {
    std::ostringstream os;
    os << value;
    //std::cout << "GB_PARAM: Param " << getName() << " string get method called. Value is " << os.str() << std::endl;
    return os.str();
    // TODO: could inform callback function here
  }

};




//-----------------------------------------------------------------------------
// GB_PARAM macro
//-----------------------------------------------------------------------------

/**
 * Use this macro inside SC_MODULE constructor to register a gb_param
 * with the configuration framework. The elipsis corresponds to an
 * optional value for the parameter.
 */
#define GB_PARAM(name,type,...)                         \
  name = gb_param<type>(#name,##__VA_ARGS__);           \
  addParam(name.getName(), &name)

} // end namespace tlm

#endif /* _GB_PARAM_H_ */
