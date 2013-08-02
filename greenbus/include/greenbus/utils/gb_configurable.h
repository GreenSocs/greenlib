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

#ifndef __gb_configurable_h__
#define __gb_configurable_h__

#include <systemc>

#include <string>
#include <ostream>
#include <map>

#include "greenbus/utils/gb_param_root.h"

namespace tlm {

//-----------------------------------------------------------------------------
// gb_configurable
//-----------------------------------------------------------------------------

typedef std::map<std::string, gb_param_root*> param_map;
typedef std::map<std::string, gb_param_root*>::iterator param_iterator;

/**
 * SC_MODULEs may inherit this class to provide a configuration interface
 * to the outside world.
 */
class gb_configurable {
public:

  gb_configurable() {}
  virtual ~gb_configurable() {} // force to be polymorphic

  void addParam(const std::string name, gb_param_root *param) {
    params.insert(param_map::value_type(name, param));
    //std::cout << "GB_CONFIGURABLE: Added param [" << name << "] to param map of [" << (dynamic_cast<sc_core::sc_object*>(this))->name() << "] (map size now " << params.size() << ")" << std::endl;
  }

  bool setParam(const std::string &name, const std::string &value) {
    param_iterator pos;
    pos = params.find(name);
    if (pos == params.end()) {
      char ch[256];
      sprintf(ch, "Set param [%s] of [%s] to value=[%s] failed! Param doesn't exist.", name.c_str(), (dynamic_cast<sc_core::sc_object*>(this))->name(), value.c_str());
      SC_REPORT_WARNING((dynamic_cast<sc_core::sc_object*>(this))->name(), ch);
      return false;
    }
    //std::cout << "GB_CONFIGURABLE: Set param [" << name << "] of [" << (dynamic_cast<sc_core::sc_object*>(this))->name() << "] to value=[" << value.c_str() << "]" << std::endl;

    gb_param_root *r = pos->second;
    r->set(value);
    return true;
  }

  /**
   * Get the value of a parameter.
   * @param name Parameter name.
   * @return This method returns a string that contains the value of the requested parameter.
   *         Use the operator>>(stringstream&,..) to assign it to a variable of the parameter's type. Example:
   *           MAddr a;
   *           stringstream s(myModule.getParam("address"));
   *           s >> a;
   */
  const std::string getParam(const std::string &name) {
    param_iterator pos;
    pos = params.find(name);
    if (pos == params.end())
      return NULL;
    //std::cout << "GB_CONFIGURABLE: Get param [" << name << "] of [" << (dynamic_cast<sc_core::sc_object*>(this))->name() << "] called (value=[" << pos->second->get().c_str() << "]" << std::endl;
    return pos->second->get();
  }

protected:  
  param_map params;
};


} // end of namespace tlm


// have to include gb_param.h here to avoid circular dependencies which cannot be resolved by the compiler
#include "gb_param.h"


#endif
