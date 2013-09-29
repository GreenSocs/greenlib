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

// This file is to be included when embedding GreenScript


#include <Python.h>

#include "../include/greenscript/greenscript.h"


////////// Includes for GreenControl //////////

// Fix for a buggy SystemC 2.1 boost (use a newer tokenizer)
#include <boost/tokenizer.hpp>

// SystemC library
#include <systemc>

///////////////////////////////////////////////


#include <string>
#include <stdlib.h>
#include <stdio.h>


// Next line defines a fix to avoid the gcc warning: deprecated conversion from string constant to 'char*'
// because Python functions should take 'const char*', but currently are taking only 'char*'
#define FIX_FOR_OLD_PYTHON const_cast<char*>


namespace gs { namespace script {


// Code for creating a Python virtual machine.  Only one VM is created and it is
// torn down when the final GreenScriptModule is deleted.
namespace GreenScriptController {
  static unsigned _nr_subscribers = 0;
  static PyThreadState *_save;
  void block_threads() {
    PyEval_RestoreThread(_save);
  }
  void unblock_threads() {
    _save = PyEval_SaveThread();
  }
  void subscribe() {
    if(_nr_subscribers==0) {
      // Initialize Python without signal handlers
      Py_InitializeEx(0);
      PyEval_InitThreads();
      unblock_threads();
    }
    _nr_subscribers++;
  }
  void unsubscribe() {
    _nr_subscribers--;
    if(_nr_subscribers==0) {
      block_threads();
      Py_Finalize();
    }
  }
}


// two functions to allow OS-independent operation of load()

std::string path_separator() {
  #ifdef MS_WINDOWS
    return std::string("\\"};
  #else
    return std::string("/");
  #endif
}

bool is_simple_filename(const char *path) {
  #ifdef MS_WINDOWS
    // relative to current directory
    if(*path == '.') return false;
    // absolute
    if((*path >= 'a') && (*path <= 'z') && (path[1] == ':')) return false;
    if((*path >= 'A') && (*path <= 'Z') && (path[1] == ':')) return false;
    if(*path == '\\') return false;
    return true;
  #else  // UNIX-style filesystem paths
    // relative to current directory
    if(*path == '.') return false;
    // absolute
    if(*path == '/') return false;
    return true;
  #endif
}


#define greenscript_test_result(pob_ptr) \
  if(pob_ptr == NULL) {                  \
      PyErr_Print();                     \
      initialised = false;               \
      GreenScriptController::unblock_threads();       \
      return;                            \
    }


// constructor
GreenScriptModule::GreenScriptModule(
  sc_core::sc_module_name name_p, const char* script_filename) :
  sc_core::sc_module(name_p), initialised(true),
  my_namespace(NULL), gs_module(NULL), sys_path(NULL), name_py(NULL)
{
  // set up interpreter and gs module and context object
  GreenScriptController::subscribe();
  GreenScriptController::block_threads();

  // get a globals() dict for this GreenScriptModule
  PyObject* main_module = PyImport_AddModule( FIX_FOR_OLD_PYTHON("__main__"));  // borrowed ref
  greenscript_test_result(main_module);
  my_namespace = PyModule_GetDict(main_module);  // borrowed ref
  my_namespace = PyDict_Copy(my_namespace);  // new ref

  // make sure there's a reference to the gs module available
  gs_module = // new ref
    PyImport_ImportModuleEx( FIX_FOR_OLD_PYTHON("gs"), my_namespace, my_namespace, NULL);
  greenscript_test_result(gs_module);

  // get a ref to sys.path
  // note that we do this once only.  if sys.path is ever assigned to a
  // new object, subsequent load()s/add-to-path()s will use the original
  // but imports will use the new
  PyObject *sys = // new ref
    PyImport_ImportModuleEx( FIX_FOR_OLD_PYTHON("sys"), my_namespace, my_namespace, NULL);
  greenscript_test_result(sys);
  Py_XDECREF(sys);
  sys_path = PyObject_GetAttrString(sys, FIX_FOR_OLD_PYTHON("path"));  // new ref
  greenscript_test_result(sys_path);

  // if we get to here, we consider ourselves initialised
  // note that:
  // - the namespace has no name, so is impossible to access from
  //   another GreenScriptModule
  // - gs and sys have been imported, but are not added to the
  //   namespace

  // tell the Python code it is embedded
  PyObject_SetAttrString(gs_module, FIX_FOR_OLD_PYTHON("__standalone__"), Py_False);

  // tell the Python code its interpreter name
  name_py = PyString_FromString(name());  // new ref
  establish_interpreter_name();

  GreenScriptController::unblock_threads();

  add_to_pythonpath(".");

  // run a script if one has been requested
  if ((script_filename!=0) && (*script_filename!=0)) {
    load(script_filename);
  }
}

#undef greenscript_test_result


// desctructor
GreenScriptModule::~GreenScriptModule() {
  Py_XDECREF(my_namespace);
  my_namespace = NULL;
  Py_XDECREF(gs_module);
  gs_module = NULL;
  Py_XDECREF(sys_path);
  sys_path = NULL;
  Py_XDECREF(name_py);
  name_py = NULL;
  GreenScriptController::unsubscribe();
}


// public load function - runs a Python file in a module-specific
// namespace.  searches for the file in the PYTHONPATH unless it
// is an absolute pathname or starts wth a '.'
void GreenScriptModule::load(const char* script_filename) {
  if(!initialised) return;
  GreenScriptController::block_threads();
  // Load the script, trying the python path first, then the CWD
  // Go direct to the CWD if an absolute or relative path is given
  // rather than a simple filename
  if(is_simple_filename(script_filename)) {
    int path_size = PyList_Size(sys_path);
    for(int i=0; i<path_size; i++) {
      PyObject *path_py = PyList_GetItem(sys_path, i);  // borrowed ref
      std::string s(PyString_AsString(path_py));
      s += path_separator() + std::string(script_filename);
      if(private_load(s.c_str())) {
        GreenScriptController::unblock_threads();
        return;
      }
    }
  }
  if(!private_load(script_filename)) {
    std::string s(name());
    s += std::string(" could not find ") + std::string(script_filename);
    perror(s.c_str());
  }
  GreenScriptController::unblock_threads();
}


// public run function - runs a Python command in a module-specific
// namespace
void GreenScriptModule::run(const char* statement) {
  if(!initialised) return;
  GreenScriptController::block_threads();

  establish_interpreter_name();

  // run the command
  PyObject *ret = PyRun_String(
    statement, Py_single_input, my_namespace, my_namespace);
  if(ret == NULL) PyErr_Print();
  Py_XDECREF(ret);

  GreenScriptController::unblock_threads();
}


// public function to add a path to the load/import search path
// actually it is not guaranteed to affect the import search path
// but will do so unless someone is doing crazy stuff with sys.path
void GreenScriptModule::add_to_pythonpath(const char* path) {
  if(!initialised) return;
  GreenScriptController::block_threads();
  PyObject *path_py = PyString_FromString(path);  // new ref
  if(PyList_Insert(sys_path, 0, path_py) < 0) {
    PyErr_Print();
  }
  Py_XDECREF(path_py);
  GreenScriptController::unblock_threads();
}


// private function used by load()
bool GreenScriptModule::private_load(const char *fullname) {
  FILE *script = fopen(fullname,"r");
  if(!script) return false;

  establish_interpreter_name();

  // run the script
  PyObject *ret = PyRun_File(
    script, fullname, Py_file_input, my_namespace, my_namespace);
  if(ret == NULL) PyErr_Print();
  Py_XDECREF(ret);

  // finish
  fclose(script);
  return true;
}


// private function to set up interpreter name in the greenscript
// namespace
void GreenScriptModule::establish_interpreter_name() {
  PyObject_SetAttrString(gs_module, FIX_FOR_OLD_PYTHON("__interpreter_name__"), name_py);
}


// callbacks

// private run function - runs a Python function from the gs module
void GreenScriptModule::run_gs_member(const char* name) {
  if(!initialised) return;
  GreenScriptController::block_threads();

  establish_interpreter_name();

  // get the callable Python object
  PyObject *member =
    PyObject_GetAttrString(gs_module, FIX_FOR_OLD_PYTHON(name));  // new ref

  // run the command
  if(member == NULL) {
    PyErr_Print();
  } else {
    PyObject *ret = PyObject_CallObject(member, NULL);
    if(ret == NULL) PyErr_Print();
    Py_XDECREF(ret);
    Py_XDECREF(member);
  }

  GreenScriptController::unblock_threads();
}


void GreenScriptModule::end_of_elaboration() {
  run_gs_member("end_of_elaboration");
}

void GreenScriptModule::start_of_simulation() {
  run_gs_member("start_of_simulation");
}

void GreenScriptModule::end_of_simulation() {
  run_gs_member("end_of_simulation");
}


} }  // namespace gs::script


