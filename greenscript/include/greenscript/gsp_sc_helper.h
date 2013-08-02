// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Marcus Bartholomeu
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

#ifndef GSP_SC_HELPER_H
#define GSP_SC_HELPER_H

#include <systemc>
#include "gs_sc_api_detection.h"


//////////////////////////////////////////////////
// Function: Find a SystemC object in the hierarchy by it's name

sc_core::sc_object *
findInSimContextByName(const char* name, sc_core::sc_object *node = 0) {

  // Base of the recursion
  if (node && strcmp(node->name(),name) == 0) return node;

  // Get the childs of the given node
  const std::vector<sc_core::sc_object *> *childs_stack;
  if (node) {
    // get child vector
#if SYSTEMC_API == 210
    sc_core::sc_module* node_as_module = dynamic_cast< sc_core::sc_module* >(node);
    if (node_as_module) childs_stack = &(node_as_module->get_child_objects());
    else return 0;
#else
    childs_stack = &node->get_child_objects();
#endif
  } else {
    // get child vector of sim context
#if SYSTEMC_API == 210
    sc_core::sc_simcontext *sim; // deprecated with SystemC-2.2
    sim = sc_core::sc_get_curr_simcontext(); // deprecated with SystemC-2.2
    childs_stack = &(sim->get_child_objects());
#else
    childs_stack = &sc_core::sc_get_top_level_objects();
#endif
  }
  // go through childs
  for (unsigned int i = 0; i < childs_stack->size(); i++) {
    sc_core::sc_object *chnode = childs_stack->at(i);
    sc_core::sc_object *found = findInSimContextByName(name,chnode);
    if (found) return found;
  }
  return 0;
}


//////////////////////////////////////////////////
// Function: Get system-wide SystemC objects

template<class T>
inline void find_or_create_new(T*& obj, const char* name) {
  obj = (T*) findInSimContextByName(name);
  if (!obj) obj = new T(name);
}
template<class T, class ARG1>
inline void find_or_create_new(T*& obj, const char* name, ARG1*& arg1) {
  obj = (T*) findInSimContextByName(name);
  if (!obj) obj = new T(name, arg1);
}


#endif //GSP_SC_HELPER_H
