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

#include "../include/greenscript/gsp_sc.h"
#include "../include/greenscript/gsp_sc_helper.h"
#include "../include/greenscript/gs_sc_api_detection.h"

#include <iostream>
#include <vector>
#include <utility> //define pair
#include <string>
#include <map>
using namespace std;
using namespace sc_core;


// Next line defines a fix to avoid the gcc warning: deprecated conversion from string constant to 'char*'
// because Python functions should take 'const char*', but currently are taking only 'char*'
#define FIX_FOR_OLD_PYTHON const_cast<char*>


//////////////////////////////////////////////////
// Map GS to SC objects

static map<PyObject*,sc_event*> event_map;
static map<PyObject*,const sc_event*> event_const_map;
static map<PyObject*,sc_signal<int>*> signal_map;
static map<PyObject*,sc_fifo<int>*> fifo_map;
gs::msg::WriteIf *gsp_sc_writeif::current_writeif;
const sc_event *current_event;


//////////////////////////////////////////////////
// Event

void gsp_sc_set_current_event(const sc_event *e)
{
  current_event = e;
}

void gsp_sc_event_bind(PyObject* e, PyObject* obj, event_type type)
{
  if (!obj) {
    // create a new event (can be notified)
    event_map[e] = new sc_event();
  } else {
    // bind to an existing event (which is constant, cannot be notified)
    switch(type) {
    case EVENT_CHANGED:
      event_const_map[e] = &signal_map[obj]->value_changed_event();
      break;
    case EVENT_READ:
      event_const_map[e] = &fifo_map[obj]->data_read_event();
      break;
    case EVENT_WRITE:
      event_const_map[e] = &fifo_map[obj]->data_written_event();
      break;
    case EVENT_WRITEIF:
      {
        PyObject *r =
          PyObject_CallMethod(obj, "set_writeif_context", NULL);
        if(r == NULL) {
          // should never happen:  bug in GreenScript
          PyErr_Print();
          exit(-1);
        }
        Py_DECREF(r);
        PyThreadState *_save = PyEval_SaveThread();
        const sc_core::sc_event *tmp =
          &gsp_sc_writeif::current_writeif->data_read_event();
        PyEval_AcquireThread(_save);
        event_const_map[e] = tmp;
      }
      break;
    case EVENT_CURRENT_EVENT:
      event_const_map[e] = current_event;
      break;
    }
  }
}

bool gsp_sc_event_notify(PyObject* e)
{
  map<PyObject*,sc_event*>:: iterator i = event_map.find(e);
  if(i == event_map.end()) return false;
  i->second->notify();
  return true;
}

bool gsp_sc_event_notify(PyObject* e, double time, sc_core::sc_time_unit tu)
{
  map<PyObject*,sc_event*>:: iterator i = event_map.find(e);
  if(i == event_map.end()) return false;
  i->second->notify(time, tu);
  return true;
}

void gsp_sc_event_remove(PyObject* e)
{
  // called by the Python destructor of a gs.event() object
  map<PyObject*,sc_event*>:: iterator i = event_map.find(e);
  if(i != event_map.end()) {
    delete i->second;
    event_map.erase(i);
  }
  map<PyObject*,const sc_event*>:: iterator j = event_const_map.find(e);
  if(j != event_const_map.end()) {
    event_const_map.erase(j);
  }
}


//////////////////////////////////////////////////
// Wrapped Signals

bool gsp_sc_signal_bind(PyObject* s, char* name)
{
  sc_signal<int> *s_ptr = dynamic_cast<sc_signal<int>*>(findInSimContextByName(name));
  if(s_ptr == 0) return false;
  signal_map[s] = s_ptr;
  return true;
}

int gsp_sc_signal_read(PyObject* s)
{
  return signal_map[s]->read();
}

void gsp_sc_signal_write(PyObject* s, int val)
{
  signal_map[s]->write(val);
}


//////////////////////////////////////////////////
// Wrapped FIFOs

bool gsp_sc_fifo_bind(PyObject* f, char* name)
{
  sc_fifo<int> * f_ptr = dynamic_cast<sc_fifo<int>*>(findInSimContextByName(name));
  if(f_ptr == 0) return false;
  fifo_map[f] = f_ptr;
  return true;
}

int gsp_sc_fifo_read(PyObject* f)
{
  return fifo_map[f]->read();
}

std::pair<bool,int> gsp_sc_fifo_nb_read(PyObject* f)
{
  int val;
  bool ret = fifo_map[f]->nb_read(val);
  return std::make_pair(ret,val);
}

void gsp_sc_fifo_write(PyObject* f, int val)
{
  fifo_map[f]->write(val);
}

bool gsp_sc_fifo_nb_write(PyObject* f, int val)
{
  return fifo_map[f]->nb_write(val);
}

int gsp_sc_fifo_num_available(PyObject* f)
{
  return fifo_map[f]->num_available();
}

int gsp_sc_fifo_num_free(PyObject* f)
{
  return fifo_map[f]->num_free();
}


//////////////////////////////////////////////////
// Initialization code. Set up Python threads

class kernel_thread_cl {
  public:
    kernel_thread_cl() {
      // make sure Python threads are enabled, then establish a global
      // variable for the initially active thread
      PyEval_InitThreads();
      kernel_thread = PyThreadState_Get();
    };
    operator PyThreadState*() {return kernel_thread;};
    operator PyInterpreterState*() {return kernel_thread->interp;};
  private:
    PyThreadState *kernel_thread;
};
// initialization code is the constructor for this global variable
kernel_thread_cl kernel_thread;



//////////////////////////////////////////////////
// Wrapper function for all callbacks from C++ to Python during
// the simulation

template<class T>
T callback_to_user_py(PyObject *cb, T postproc(PyObject*,PyObject*,bool*)) {
  // Python interpreter can not track stack through C++, so all such
  // callbacks are in a new Python thread.
  PyThreadState *callback_state = PyThreadState_New(kernel_thread);
  PyEval_AcquireThread(callback_state);
  PyObject *pResult  = PyObject_CallObject(cb, NULL);

  if (pResult == NULL) {
    if(PyErr_Occurred() != NULL) {
      PyErr_Print();
      PyEval_ReleaseThread(callback_state);
      PyThreadState_Clear(callback_state);
      PyThreadState_Delete(callback_state);
      exit(-1);
    }
    //Unlucky case if PyErr_Occurred() returned NULL
    std::cout << "ERROR: Executing a python callback. No more info, sorry."
              << std::endl;
    PyEval_ReleaseThread(callback_state);
    PyThreadState_Clear(callback_state);
    PyThreadState_Delete(callback_state);
    exit(-1);
  }
  // Successful completion of callback
  // User-supplied post-processing, which typically decrements ref count of
  // callback and converts the result to the desired type
  // postprocessing has to be done before thread-state is given up and destroyed
  bool err_hint = false;
  T rv = postproc(cb, pResult, &err_hint);
  if(err_hint && PyErr_Occurred()) {
    PyErr_Print();
    PyEval_ReleaseThread(callback_state);
    PyThreadState_Clear(callback_state);
    PyThreadState_Delete(callback_state);
    exit(-1);
  }
  Py_DECREF(pResult);
  PyEval_ReleaseThread(callback_state);
  PyThreadState_Clear(callback_state);
  PyThreadState_Delete(callback_state);
  return rv;
}

bool decref_callback(PyObject *cb, PyObject *r, bool *eh) {
  Py_DECREF(cb);
  return true;
}


//////////////////////////////////////////////////
// Primitive Channel Proxy

void gsp_sc_prim_channel_proxy::update() {
  while(!awaiting_update.empty()) {
    PyObject *cb = awaiting_update.front();
    callback_to_user_py<bool>(cb, &decref_callback);
    awaiting_update.pop();
  }
}

void gsp_sc_prim_channel_proxy::push(PyObject *cb) {
  if(awaiting_update.empty()) request_update();
  awaiting_update.push(cb);
  Py_INCREF(cb);
}

gsp_sc_prim_channel_proxy::~gsp_sc_prim_channel_proxy() {
  while(!awaiting_update.empty()) {
    PyObject *cb = awaiting_update.front();
    awaiting_update.pop();
    Py_DECREF(cb);
  }
}


//////////////////////////////////////////////////
// Function: gsp_spawn

void run_spawned_thread(PyObject* runnable) {
  callback_to_user_py<bool>(runnable, &decref_callback);
}

PyObject *gsp_sc_spawn(PyObject *runnable, PyObject *name)
{
  // called either from some GreenScript process or from the original
  // Python interpreter thread before gsp_start.  In either case it is
  // non-blocking and there is a Python thread state, so we don't need
  // to do anything with thread states or interpreter locks
  if(!PyCallable_Check(runnable)) {
      PyErr_SetString(PyExc_TypeError, "parameter must be callable");
      return 0;
  }

  char *csn = PyString_AsString(name);
  if(csn == NULL) return 0;  // TypeError is raised already

  // Ref the new call-back: will be freed when the thread function returns
  Py_INCREF(runnable);

  sc_spawn(sc_bind(run_spawned_thread, runnable), sc_gen_unique_name(csn));
  Py_RETURN_NONE;
}


//////////////////////////////////////////////////
// Function: gsp_sc_start

void gsp_sc_start()
{
  // must be in the kernel thread on entry.  Exit it so we
  // know that no thread is active on entry to any other function
  // called by the kernel
  PyEval_ReleaseThread(kernel_thread);
  sc_start();
  // re-acquire thread before returning control to Python
  PyEval_AcquireThread(kernel_thread);
}

void gsp_sc_start(double time, sc_core::sc_time_unit tu)
{
  // must be in the kernel thread on entry.  Exit it so we
  // know that no thread is active on entry to any other function
  // called by the kernel
  PyEval_ReleaseThread(kernel_thread);
  sc_start(sc_time(time, tu));
  // re-acquire thread before returning control to Python
  PyEval_AcquireThread(kernel_thread);
}


//////////////////////////////////////////////////
// Function: gsp_sc_stop

void gsp_sc_stop()
{
  sc_stop();
}


//////////////////////////////////////////////////
// Function: gsp_sc_wait
void gsp_sc_wait(double time, sc_core::sc_time_unit tu)
{
  // relinquish thread lock and set current thread to NULL
  PyThreadState *_save = PyEval_SaveThread();
  sc_core::wait(time, tu);
  // now re-establish the previous thread
  PyEval_AcquireThread(_save);
}

void gsp_sc_wait(PyObject* e)
{
  // relinquish thread lock and set current thread to NULL
  PyThreadState *_save = PyEval_SaveThread();
  if (event_const_map[e]) sc_core::wait(*event_const_map[e]);
  else sc_core::wait(*event_map[e]);
  // now re-establish the previous thread
  PyEval_AcquireThread(_save);
}


//////////////////////////////////////////////////
// Function: gsp_sc_simulation_time

double gsp_sc_simulation_time(sc_core::sc_time_unit tu)
{
  return sc_time_stamp().to_seconds() / sc_time(1,tu).to_seconds();
}


//////////////////////////////////////////////////
// Function: gsp_sc_delta_count

double gsp_sc_delta_count()
{
  return sc_delta_count();
}


//////////////////////////////////////////////////
// Function: gsp_sc_get_curr_process_handle

long gsp_sc_get_curr_process_handle()
{
#if SYSTEMC_API == 210
  return (long) sc_get_curr_process_handle();
#elif SYSTEMC_API == 220 || SYSTEMC_API == 230
  return (long) sc_get_curr_simcontext()->get_curr_proc_info()->process_handle;
#else
#error Unknown SystemC API to call for sc_get_current_process_handle
#endif
}


//////////////////////////////////////////////////
// Function: gsp_sc_is_running
bool gsp_sc_is_running() {
#if SYSTEMC_API == 210
  return sc_get_curr_simcontext()->is_running();
#elif SYSTEMC_API == 220 || SYSTEMC_API == 230
  return sc_is_running();
#else
#error Unknown SystemC API to call for sc_is_running
#endif
}


//////////////////////////////////////////////////
// Function: gsp_sc_print_sc_splash

namespace sc_core {extern void pln();}
void gsp_sc_print_sc_splash()
{
  pln();
}


#ifdef USE_GREENCONTROL

//////////////////////////////////////////////////
// Parameters (using GreenConfig)
//////////////////////////////////////////////////

// functions for direct database access //

bool gsp_sc_param_exists(std::string name) {
  gs::cnf::cnf_api *configAPI = gs::cnf::GCnf_Api::getApiInstance(NULL);
  return configAPI->existsParam(name);
}

std::string gsp_sc_read_param(std::string name) {
  gs::cnf::cnf_api *configAPI = gs::cnf::GCnf_Api::getApiInstance(NULL);
  return configAPI->getValue(name);
}

void gsp_sc_write_param(std::string name, std::string value) {
  gs::cnf::cnf_api *configAPI = gs::cnf::GCnf_Api::getApiInstance(NULL);
  configAPI->setInitValue(name, value);
}

gsp_sc_param_list::gsp_sc_param_list(std::string name) {
  gs::cnf::cnf_api *configAPI = gs::cnf::GCnf_Api::getApiInstance(NULL);
  all_params = configAPI->getParamList(name);
}

int gsp_sc_param_list::length() {
  return all_params.size();
}

std::string gsp_sc_param_list::read(int i) {
  return all_params[i];
}


// functions for parameter object creation //

// globals used to pass C++ pointers from Python to constructors
static gsp_sc_array_param *array_mother = 0;
static gsp_sc_ext_array_param *ext_array_mother = 0;
static gsp_sc_param *current_param = 0;


gsp_sc_param::gsp_sc_param(std::string name, std::string default_val) {
  // constructor for a standalone param or a param child of an extended array.
  // if a child, the mother is found in a global variable
  if(ext_array_mother != 0) {
    gs::gs_param_array *mother = ext_array_mother->getParam();
    m_param = new gs::gs_param<std::string>(name, default_val, mother);
    m_has_mother = true;
  } else {
    m_param = new gs::gs_param<std::string>(name, default_val);
    m_has_mother = false;
  }
  m_is_array_element = false;
  ext_array_mother = 0;
  array_mother = 0;
}

gsp_sc_param::gsp_sc_param(unsigned index, std::string default_val) {
  // constructor for a param child of a simple array.  mother is in a global.
  // makes this into a std::string.  in this case all
  // get/set go through the mother, default val is used only if the mother
  // notes that the size from the dB is smaller than the index, and then
  // the default val should also be applied to the dB.
  assert(array_mother != 0);
  gs::gs_param<std::string *> *mother = array_mother->getParam();
  if(mother->size() <= index) mother->resize(index+1);
  if(array_mother->db_size() <= index) (*mother)[index] = default_val;
  m_is_array_element = true;
  m_has_mother = false;
  m_mother = mother;
  m_index_in_array = index;
  ext_array_mother = 0;
  array_mother = 0;
}

gsp_sc_param::~gsp_sc_param() {
  if((!m_is_array_element) && (!m_has_mother)) delete m_param;
}

std::string gsp_sc_param::get_value() {
  if(m_is_array_element) return (*m_mother)[m_index_in_array];
  return m_param->getValue();
}

void gsp_sc_param::set_value(std::string val) {
  if(m_is_array_element) (*m_mother)[m_index_in_array] = val;
  else m_param->setValue(val);
}

gs::gs_param<std::string> *gsp_sc_param::getParam() {
  if(m_is_array_element) return 0;
  return m_param;
}

void gsp_sc_param::set_param_as_current() {
  current_param = this;
}


gsp_sc_array_param::gsp_sc_array_param(std::string name) {
  // constructor for a standalone array param or an array param child
  // of an extended array.  if a child, the mother is in a global variable
  if(ext_array_mother != 0) {
    gs::gs_param_array *mother = ext_array_mother->getParam();
    m_param = new gs::gs_param<std::string *>(name, mother);
    m_has_mother = true;
  } else {
    m_param = new gs::gs_param<std::string *>(name);
    m_has_mother = false;
  }
  m_length_from_db = m_param->size();
  ext_array_mother = 0;
  array_mother = 0;
}

gsp_sc_array_param::gsp_sc_array_param(unsigned index) {
  // constructor for a child of a simple array and will fail
  // with the current version of GreenConfig.  mother is found in a global
  assert(false);
  ext_array_mother = 0;
  array_mother = 0;
}

gsp_sc_array_param::~gsp_sc_array_param() {
  if(!m_has_mother) delete m_param;
}

unsigned gsp_sc_array_param::get_length() {
  return m_param->size();
}

gs::gs_param<std::string *> *gsp_sc_array_param::getParam() {
  return m_param;
}

unsigned gsp_sc_array_param::db_size() {
  return m_length_from_db;
}

void gsp_sc_array_param::set_length(unsigned length) {
  m_param->resize(length);
}

void gsp_sc_array_param::set_mother() {
  // function to call before the construction of a child param
  array_mother = this;
}


gsp_sc_ext_array_param::gsp_sc_ext_array_param(std::string name) {
  // constructor for a standalone param or a param child of an extended array.
  // if a child, the mother is found in a global variable
  if(ext_array_mother != 0) {
    gs::gs_param_array *mother = ext_array_mother->getParam();
    m_param = new gs::gs_param_array(name, mother);
    m_has_mother = true;
  } else {
    m_param = new gs::gs_param_array(name);
    m_has_mother = false;
  }
  ext_array_mother = 0;
  array_mother = 0;
}

gsp_sc_ext_array_param::gsp_sc_ext_array_param(unsigned index) {
  // constructorfor a child of a simple array and will fail
  // with the current version of GreenConfig.  mother is found in a global
  // function to call before the construction of a child param
  assert(false);
  ext_array_mother = 0;
  array_mother = 0;
}

gsp_sc_ext_array_param::~gsp_sc_ext_array_param() {
  if(!m_has_mother) delete m_param;
}

gs::gs_param_array *gsp_sc_ext_array_param::getParam() {
  return m_param;
}

void gsp_sc_ext_array_param::set_mother() {
  ext_array_mother = this;
}

#endif


//////////////////////////////////////////////////
// Analysis (using GreenConfig)
//////////////////////////////////////////////////

#ifdef USE_GREENCONTROL

gsp_sc_gav_output::gsp_sc_gav_output(gs::av::OutputPluginType type, std::string name)
{
  configAPI = gs::cnf::GCnf_Api::getApiInstance(NULL);
  analysisAPI = gs::av::GAV_Api::getApiInstance(NULL);
  op = (*analysisAPI).create_OutputPlugin(type,name);
}

bool gsp_sc_gav_output::add(std::string param_name)
{
  gs::cnf::gs_param_base* param_p = (*configAPI).getPar(param_name);
  if (!param_p) return false;
  op->observe(*param_p);
  return true;
}

void gsp_sc_gav_output::add()
{
  if(current_param != 0) op->observe(*(current_param->getParam()));
  current_param = 0;
}

void gsp_sc_gav_output::add_all()
{
  op->observe_all(*configAPI);
}

bool gsp_sc_gav_output::remove(std::string param_name)
{
  gs::cnf::gs_param_base* param_p = (*configAPI).getPar(param_name);
  if (!param_p) return false;
  op->remove(*param_p);
  return true;
}

void gsp_sc_gav_output::remove()
{
  if(current_param != 0) op->remove(*(current_param->getParam()));
  current_param = 0;
}

void gsp_sc_gav_output::pause()
{
  op->pause();
}

void gsp_sc_gav_output::pause(double time, sc_core::sc_time_unit tu)
{
  op->pause(time, tu);
}

void gsp_sc_gav_output::pause(PyObject* e)
{
  if (event_const_map[e])
    op->pause(const_cast<sc_core::sc_event&>(*event_const_map[e]));
  else
    op->pause(*event_map[e]);
}

void gsp_sc_gav_output::resume()
{
  op->resume();
}

#endif // USE_GREENCONTROL


//////////////////////////////////////////////////
// Function: gsp_sc_init

#ifdef USE_GREENCONTROL
// Global pointers to the system-wide GreenControl objects
gs::ctr::GC_Core *core;
gs::cnf::ConfigDatabase *configDb;
gs::cnf::ConfigPlugin *configPlugin;
gs::av::GAV_Plugin *analysisPlugin;
gs::cnf::ConfigFile_Tool *configreader;
gs::cnf::LuaFile_Tool *luareader;
#endif


void gsp_sc_init()
{
#ifdef USE_GREENCONTROL
  // Find the system-wide objects (defined in gsp_sc_helper.h)
  find_or_create_new(core, "ControlCore");
  find_or_create_new(configDb, "ConfigDatabase");
  find_or_create_new(configPlugin, "ConfigPlugin", configDb);
  find_or_create_new(analysisPlugin, "AnalysisPlugin");
#endif

  gsp_sc_print_sc_splash();
}


//////////////////////////////////////////////////
// Function: gsp_sc_config_file

#ifdef USE_GREENCONTROL
void gsp_sc_config_file(char* filename)
{
  // Find a filereader object
  find_or_create_new(configreader, "configreader");
  // Load new config
  configreader->config(filename);
}
#endif


//////////////cout////////////////////////////////////
// Function: gsp_sc_config_lua

#ifdef USE_GREENCONTROL
void gsp_sc_config_lua(char* filename)
{
  // Find a luareader object
  find_or_create_new(luareader, "luareader");
  // Load new config
  luareader->config(filename);
}
#endif


//////////////////////////////////////////////////
// GreenMessage
//////////////////////////////////////////////////

// use this define to enable garbage-collection testing
// #define GC_CHECK_OUTPUT

// Message class methods

PyObject *gsp_sc_msg_base::__getitem__(PyObject *k) {
  std::string k_str;
  if(!Py_to_str(k, k_str)) return 0;
  if(contains(k_str)) {
    // contains() also sets current_message if it returns true
    if(current_message->has_value()) {
      // user has requested a leaf parameter of a message - return it by value
      return str_to_Py(*current_message);
      // have incremented the Py ref count and returned a new ref
    } else {
      // user has requested a hierarchical message in a message
      // current_message is already set by contains()
      current_mother = this;
      return PyObject_CallObject(m_msg_ref_class, NULL);
      // new reference: I leave it alone to return a new reference or
      // an error
    }
  } else {
    // key is not present; raise KeyError
    PyErr_SetString(PyExc_KeyError, "Message key not found");
    return 0;
  }
}

PyObject *gsp_sc_msg_base::__setitem__(PyObject *k, PyObject *v) {
  std::string k_str;
  if(!Py_to_str(k, k_str)) return 0;
  gs::msg::Message *msg_to_change;
  bool added_an_element = false;
  if(contains(k_str)) {
    // existing message entry being replaced
    // contains() has set up current_message
    if((!current_message->has_value()) && (*m_ref_count > 1)) {
      // not safe to replace a hierarchical entry if references
      // to this message exist
      PyErr_SetString(PyExc_KeyError,
        "Not safe to modify message with existing refs");
      return 0;
    }
    // changing an existing entry
    msg_to_change = current_message;
  } else {
    // creating a new entry
    msg_to_change = &((*m_msg)[k_str]);
    added_an_element = true;
  }
  // special case for booleans
  if(PyBool_Check(v)) {
    *msg_to_change = PyObject_IsTrue(v);
    Py_RETURN_NONE;
  }
  // special case for hierarchical gsp_sc_msg
  PyObject *retval = PyObject_CallMethod(v, "set_gs_msg_context", NULL);
  if(retval == NULL) {
    PyErr_Clear();
  } else {
    Py_DECREF(retval);
    // msg copied from from global variable
    *msg_to_change = *current_message;
    Py_RETURN_NONE;
  }
  // default: convert to string
  std::string v_str;
  if(Py_to_str(v, v_str)) {
    *msg_to_change = v_str;
    Py_RETURN_NONE;
  }
  if(added_an_element) m_msg->erase(k_str);
  return 0;
}

PyObject *gsp_sc_msg_base::__delitem__(PyObject *k) {
  std::string k_str;
  if(!Py_to_str(k, k_str)) return 0;
  if(contains(k_str)) {
    // existing message entry being replaced
    // contains() has set up current_message
    if((!current_message->has_value()) && (*m_ref_count > 1)) {
      // not safe to replace a hierarchical entry if references
      // to this message exist
      PyErr_SetString(PyExc_KeyError,
        "Not safe to modify message with existing refs");
      return 0;
    }
    // OK to delete (could have saved the iterator from contains()
    // but this seems excessive: delete not expected to be frequent
    m_msg->erase(k_str);
  } else {
    // key not present
    PyErr_SetString(PyExc_KeyError,"Message key not found");
    return 0;
  }
  Py_RETURN_NONE;
}

unsigned gsp_sc_msg_base::__len__() {
  return (*m_msg).size();
}

PyObject *gsp_sc_msg_base::__contains__(PyObject *k) {
  std::string k_str;
  if(!Py_to_str(k, k_str)) return 0;
  if(contains(k_str)) Py_RETURN_TRUE;
  // also sets up current_messge to be the content, if possible
  Py_RETURN_FALSE;
}

template<PyObject *ELEMENT(
  gs::msg::Message::children_map::iterator, gsp_sc_msg_base *)>
PyObject *gsp_sc_msg_base::build_list() {
  // does not flatten the message - only one level of the hierarchy
  PyObject *result_list = PyList_New(0);
  for(gs::msg::Message::children_map::iterator item = m_msg->begin();
    item != m_msg->end(); item++) {
    PyObject *element = ELEMENT(item, this);
    if(element == NULL) {
      Py_DECREF(result_list);
      return NULL;
    }
    PyList_Append(result_list, element);
    Py_DECREF(element);
  }
  return result_list;
}

PyObject *key_elem(
  gs::msg::Message::children_map::iterator i, gsp_sc_msg_base *m) {
  return PyString_FromString(i->first);
}

PyObject *gsp_sc_msg_base::keys() {
  return build_list<&key_elem>();
}

PyObject *value_elem(
  gs::msg::Message::children_map::iterator i, gsp_sc_msg_base *m) {
  return m->__getitem__(m->str_to_Py(i->first));
}

PyObject *gsp_sc_msg_base::values() {
  return build_list<&value_elem>();
}

PyObject *item_elem(
  gs::msg::Message::children_map::iterator i, gsp_sc_msg_base *m) {
  PyObject *value = value_elem(i,m);
  if(value == NULL) return NULL;
  PyObject *key = key_elem(i,m);
  PyObject *item = PyTuple_Pack(2, key, value);
  Py_DECREF(key);
  Py_DECREF(value);
  return item;
}

PyObject *gsp_sc_msg_base::items() {
  return build_list<&item_elem>();
}

PyObject *gsp_sc_msg_base::copy() {
  // returns a msg copied from "this"
  current_const_message = this->m_msg;
  return PyObject_CallObject(m_msg_class, NULL);
}

PyObject *gsp_sc_msg_base::update(PyObject *nargs) {
  // check nargs is a dict, then iterate, adding each element
  if(nargs != 0) {
    if (!PyDict_Check(nargs)) {
      PyErr_SetString(PyExc_TypeError, "Message update called with a non-dict");
      return 0;
    }
    // Iterate the dict
    PyObject* keys = PyDict_Keys(nargs);
    PyObject* values = PyDict_Values(nargs);
    for (int i=0; i < PyList_Size(keys); i++) {
      __setitem__(PyList_GetItem(keys,i), PyList_GetItem(values,i));
    }
    Py_DECREF(values);
    Py_DECREF(keys);
  }
  Py_RETURN_NONE;
}

PyObject *gsp_sc_msg_base::force_string() {
  // returns a msg reference to "this", with m_force_string set
  current_mother = this;
  current_message = this->m_msg;
  current_force_string = true;
  return PyObject_CallObject(gsp_sc_msg_base::m_msg_ref_class, NULL);
}

unsigned gsp_sc_msg_base::length_as_array() {
  return m_msg->length_as_array();
}

gsp_sc_msg_base *gsp_sc_msg_base::current_mother = 0;

void gsp_sc_msg_base::set_gs_msg_context() {
  // set a global which allows a message-ref to find its mother
  current_message = m_msg;
}

bool gsp_sc_msg_base::contains(std::string k_str) {
  // does not support hierarchical names with '.' spacing
  current_message = m_msg->find(k_str);
  return (current_message != 0);
}

// Convert string to Py for a normal message (call Py eval())
PyObject *gsp_sc_msg_base::str_to_Py(const std::string &s) {
  PyObject *pResult = NULL;
  if(!m_force_string) {
    PyObject *g = PyDict_New();
    PyObject *l = PyDict_New();
    pResult = PyRun_String(s.c_str(), Py_eval_input, g, l);
    Py_DECREF(l);
    Py_DECREF(g);
  }
  if(pResult == NULL) {
    // Recover from error, return the string
    PyErr_Clear();
    pResult = PyString_FromString(s.c_str());
  }
  return pResult;  // always returns a new reference
}

bool gsp_sc_msg_base::Py_to_str(PyObject *po, std::string &s) {
  PyObject *obj_PyString = PyObject_Str(po);
  if(obj_PyString == NULL) {
    PyErr_SetString(PyExc_TypeError,
      "failed to convert key or value to string");
    return false;
  }
  s = PyString_AsString(obj_PyString);
  Py_DECREF(obj_PyString);
  return true;
}

gsp_sc_msg_base::~gsp_sc_msg_base() {
  #ifdef GC_CHECK_OUTPUT
  std::cout << "gc-check-dtor: " << this << " : " << m_msg << " : " << *m_ref_count << " : " << m_mother_msg << std::endl;
  #endif
  (*m_ref_count)--;
  if(0 == *m_ref_count) {
    #ifdef GC_CHECK_OUTPUT
    std::cout << "gc-check-dtor-M: " << this << " : " << m_msg << " : " << *m_ref_count << " : " << m_mother_msg << std::endl;
    #endif
    delete m_ref_count;
    delete m_mother_msg;
  }
}

// Message Constructors
gsp_sc_msg::gsp_sc_msg(PyObject *args) {
  m_msg = new gs::msg::Message;
  m_mother_msg = m_msg;
  m_ref_count = new unsigned;
  *m_ref_count = 1;
  m_force_string = false;

  #ifdef GC_CHECK_OUTPUT
  std::cout << "gc-check-ctor: " << this << " : " << m_msg << " : " << *m_ref_count << " : " << m_mother_msg << std::endl;
  std::cout << "gc-check-ctor-M: " << this << " : " << m_msg << " : " << *m_ref_count << " : " << m_mother_msg << std::endl;
  #endif

  if(current_const_message != 0) {
    // this is a message from a writeif_base or copy()
    *m_msg = *current_const_message;
    current_const_message = 0;
    return;
  }

  // if we get to here, msg was created from Python
  if(PySequence_Check(args)) {
    for(unsigned i=0; ; i++) {
      PyObject *item = PySequence_GetItem(args, i);
      if(item == NULL) break;
      PyObject *index = PyInt_FromLong(i);
      __setitem__(index, item);
      Py_DECREF(item);
      Py_DECREF(index);
    }
  } else {
    // if not a sequence, try to see if it is a dict
    update(args);
  }
}


bool gsp_sc_msg_base::current_force_string = false;

gsp_sc_msg_ref::gsp_sc_msg_ref() {
  // this is a message reference
  // its message is part of another message
  // information is set up in global variables
  m_msg = current_message;
  // the mother message has a reference count
  m_mother_msg = current_mother->m_mother_msg;
  // the root reference count is also a pointer, in case
  // the mother gsp_sc_msg is deleted before the last gsp_sc_msg_ref
  m_ref_count = current_mother->m_ref_count;
  // now acquire the root message
  (*m_ref_count)++;
  m_force_string =
    current_force_string || (current_mother->m_force_string);
  current_force_string = false;
  #ifdef GC_CHECK_OUTPUT
  std::cout << "gc-check-ctor-R: " << this << " : " << m_msg << " : " << *m_ref_count << " : " << m_mother_msg << std::endl;
  #endif
}

// global variables
PyObject *gsp_sc_msg_base::m_msg_ref_class = 0;
PyObject *gsp_sc_msg_base::m_msg_class = 0;

void gsp_sc_init_msg_classes(
  PyObject *msg_class, PyObject *msg_ref_class) {
  // function to make the constructor for the Python wrapper classes
  // for gsp_sc_msg[_ref] visible to the C++
  gsp_sc_msg::m_msg_class = msg_class;
  gsp_sc_msg::m_msg_ref_class = msg_ref_class;
}


// WriteIf
gsp_sc_writeif::gsp_sc_writeif()
{
  m_writeif = 0;
}

bool gsp_sc_writeif::invalid() const
{
  return(m_writeif == 0);
}

gsp_sc_writeif::gsp_sc_writeif(gs::msg::WriteIf& writeif)
{
  m_writeif = &writeif;
}

bool
gsp_sc_writeif::write(const gsp_sc_msg_base *msg)
{
  // relinquish thread lock and set current thread to NULL
  PyThreadState *_save = PyEval_SaveThread();

  m_writeif->write(*(msg->m_msg));

  // now re-establish the previous thread
  PyEval_AcquireThread(_save);

  return true;
}

bool
gsp_sc_writeif::nb_write(const gsp_sc_msg_base *msg)
{
  // relinquish thread lock and set current thread to NULL
  PyThreadState *_save = PyEval_SaveThread();

  bool b = m_writeif->nb_write(*(msg->m_msg));

  // now re-establish the previous thread
  PyEval_AcquireThread(_save);

  return b;
}

int
gsp_sc_writeif::num_free() const
{
  // relinquish thread lock and set current thread to NULL
  PyThreadState *_save = PyEval_SaveThread();

  int nf = m_writeif->num_free();

  // now re-establish the previous thread
  PyEval_AcquireThread(_save);

  return nf;
}

void gsp_sc_writeif::set_writeif_context()
{
  current_writeif = m_writeif;
}


// WriteIf base
gsp_sc_writeif_base::gsp_sc_writeif_base(char *name, PyObject *self):
  gs::msg::receiver_base(name), py_self(self) {}

gsp_sc_writeif_base::~gsp_sc_writeif_base() {}

PyObject *gsp_sc_writeif_base::get_method(const char *name) const {
  PyObject *cb = PyObject_GetAttrString(py_self, FIX_FOR_OLD_PYTHON(name));
  if(cb == NULL) {
    std::cout << "ERROR:  writeif_base interface function not available\n";
    exit(-1);
  }
  return cb;
}


void gsp_sc_writeif_base::write(const gs::msg::Message &msg) {
  gsp_sc_msg_base::current_const_message = &msg;
  callback_to_user_py<bool>(get_method("write_callback"), &decref_callback);
}

// static variable
const gs::msg::Message *gsp_sc_msg_base::current_const_message = 0;
gs::msg::Message *gsp_sc_msg_base::current_message = 0;


bool bool_postproc(PyObject *cb, PyObject *result, bool *err_hint) {
  Py_DECREF(cb);
  int iv = PyObject_IsTrue(result);
  if(iv == -1) *err_hint = true;
  return (1 == iv);
}

bool gsp_sc_writeif_base::nb_write(const gs::msg::Message& msg) {
  gsp_sc_msg_base::current_const_message = &msg;
  return callback_to_user_py<bool>(get_method("nb_write_callback"),
    &bool_postproc);
}


int int_postproc(PyObject *cb, PyObject *result, bool *err_hint) {
  Py_DECREF(cb);
  long lv = PyInt_AsLong(result);
  if(lv == -1) *err_hint = true;
  return int(lv);
}

int gsp_sc_writeif_base::num_free() const {
  return callback_to_user_py<int>(get_method("num_free_callback"),
    &int_postproc);
}


sc_core::sc_event *event_postproc(
  PyObject *cb, PyObject *result, bool *err_hint) {
  Py_DECREF(cb);
  map<PyObject*,sc_core::sc_event*>::iterator te = event_map.find(result);
  if(te == event_map.end()) {
    *err_hint = true;
    PyErr_SetString(PyExc_TypeError,
      "data_read_event() did not return a gs.event");
    return 0;
  }
  return te->second;
}

const sc_core::sc_event &gsp_sc_writeif_base::data_read_event() const {
  return *callback_to_user_py<sc_core::sc_event *>(
    get_method("data_read_event_callback"), &event_postproc);
  // if the postprocessor returns 0, callback_to_user_py will have exited
}



// findReceiver
gsp_sc_writeif
gsp_sc_findReceiver(std::string name)
{
  try {
    return gs::msg::findReceiver(name);
  }

  catch(gs::msg::invalid_receiver ir) {
    return gsp_sc_writeif();
  }
}


