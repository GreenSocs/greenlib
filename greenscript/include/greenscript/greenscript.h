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

// This file is to be included when embedding GreenScript.  It defines
// a SystemC module class, which provides a GreenScript interpreter.


#ifndef GREENSCRIPT_H
#define GREENSCRIPT_H


// Use newer boost (fix for a buggy SystemC 2.1 boost)
#include <boost/config.hpp>

// SystemC library
#include <systemc>


// forward declaration
struct _object;
typedef struct _object PyObject;


namespace gs { namespace script {


class GreenScriptModule: public sc_core::sc_module {
  public:
    GreenScriptModule(
      sc_core::sc_module_name name,
      const char* script_filename = 0);

    virtual ~GreenScriptModule();

    void load(const char* script_filename);
    void run(const char* statement);
    void add_to_pythonpath(const char* path);

    void end_of_elaboration();
    void start_of_simulation();
    void end_of_simulation();

  private:
    bool private_load(const char *fullname);
    void establish_interpreter_name();
    void run_gs_member(const char* name);

    bool initialised;
    PyObject *my_namespace, *gs_module, *sys_path, *name_py;
};


} } // namespace gs::script

// multiple inclusion protection
#endif


