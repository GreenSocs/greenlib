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

#ifndef GSP_SC_H
#define GSP_SC_H

#include <Python.h>

#ifdef USE_GREENCONTROL
// Fix for a buggy SystemC 2.1 boost (use a newer tokenizer)
#include <boost/tokenizer.hpp>
#endif // USE_GREENCONTROL

#include <systemc>
#include <vector>
#include <queue>
#include <utility>  //for std::pair


//////////////////////////////////////////////////
// Constants

enum event_type {EVENT_CHANGED, EVENT_READ, EVENT_WRITE,
  EVENT_WRITEIF, EVENT_CURRENT_EVENT};
#ifdef SWIG
namespace sc_core {
  enum sc_time_unit {SC_FS, SC_PS, SC_NS, SC_US, SC_MS, SC_SEC};
}
#endif


//////////////////////////////////////////////////
// Event

void gsp_sc_set_current_event(const sc_core::sc_event *e);
void gsp_sc_event_bind(PyObject* e, PyObject* obj=0, event_type type=EVENT_CHANGED);
bool gsp_sc_event_notify(PyObject* e);
bool gsp_sc_event_notify(PyObject* e, double time, sc_core::sc_time_unit tu=sc_core::SC_NS);
void gsp_sc_event_remove(PyObject* e);


//////////////////////////////////////////////////
// Wrapped Signals

bool gsp_sc_signal_bind(PyObject* s, char* name);
int gsp_sc_signal_read(PyObject* s);
void gsp_sc_signal_write(PyObject* s, int val);


//////////////////////////////////////////////////
// Wrapped FIFOs

bool gsp_sc_fifo_bind(PyObject* f, char* name);
int gsp_sc_fifo_read(PyObject* f);
std::pair<bool,int> gsp_sc_fifo_nb_read(PyObject* f);
int gsp_sc_fifo_num_available(PyObject* f);
void gsp_sc_fifo_write(PyObject* f, int val);
bool gsp_sc_fifo_nb_write(PyObject* f, int val);
int gsp_sc_fifo_num_free(PyObject* f);



//////////////////////////////////////////////////
// Primitive Channel Proxy
//
//   There is just one primitive channel instance in the SystemC world
//   and all GreenScript dynamic channels use it as a proxy.

class gsp_sc_prim_channel_proxy : public sc_core::sc_prim_channel {
public:
  virtual void update();
  void push(PyObject *cb);
  ~gsp_sc_prim_channel_proxy();
private:
  std::queue<PyObject *> awaiting_update;
};


//////////////////////////////////////////////////
// GreenConfig

#ifdef USE_GREENCONTROL

// GreenControl API
#include "greencontrol/config.h"
// API Tool to read configuration file(s)
#include "greencontrol/gcnf/apis/configFileApi/configfile_tool.h"

// API Tool to read lua configuration file(s)
#include "greencontrol/gcnf/apis/luaFileApi/luafile_tool.h"


// functions for direct database access //

bool gsp_sc_param_exists(std::string name);
std::string gsp_sc_read_param(std::string name);
void gsp_sc_write_param(std::string name, std::string value);

class gsp_sc_param_list {
public:
  gsp_sc_param_list(std::string name);
  int length();
  std::string read(int i);
private:
  std::vector<std::string> all_params;
};


// functions for parameter object creation //

class gsp_sc_param {
public:
  // constructor for a standalone param or a param child of an extended array.
  // if a child, the mother is found in a global variable
  gsp_sc_param(std::string name, std::string default_val);
  // constructor for a param child of a simple array.  mother is in a global
  // makes this into a std::string.  in this case all
  // get/set go through the mother, and the default val is used if the mother
  // notes that the size from the dB is smaller than the index; in this case
  // the default val should also be applied to the dB.
  gsp_sc_param(unsigned index, std::string default_val);
  ~gsp_sc_param();
  std::string get_value();
  void set_value(std::string val);
  void set_param_as_current();
  gs::gs_param<std::string> *getParam();
private:
  bool m_is_array_element;
  gs::gs_param<std::string> *m_param;
  unsigned m_index_in_array;
  gs::gs_param<std::string *> *m_mother;
  bool m_has_mother;
};

class gsp_sc_array_param {
// user may supply default-vector which gives actual size and default values
// but this is dealt with in Python
public:
  // constructor for a standalone param or a param child of an extended array.
  // if a child, the mother is found in a global variable
  gsp_sc_array_param(std::string name);
  // the following constructor is for a child of a simple array and will fail
  // with the current version of GreenConfig.  mother is found in a global
  gsp_sc_array_param(unsigned index);
  ~gsp_sc_array_param();
  unsigned get_length();
  unsigned db_size();
  void set_length(unsigned length);
  // function to call before the construction of a child param
  void set_mother();
  gs::gs_param<std::string *> *getParam();
private:
  gs::gs_param<std::string *> *m_param;
  unsigned m_length_from_db;
  bool m_has_mother;
};

class gsp_sc_ext_array_param {
// user may supply a default-dict which gives actual structure and default values
// but this is dealt with in Python
public:
  // constructor for a standalone param or a param child of an extended array.
  // if a child, the mother is found in a global variable
  gsp_sc_ext_array_param(std::string name);
  // the following constructor is for a child of a simple array and will fail
  // with the current version of GreenConfig.  mother is found in a global
  gsp_sc_ext_array_param(unsigned index);
  virtual ~gsp_sc_ext_array_param();
  // function to call before the construction of a child param
  void set_mother();
  gs::gs_param_array *getParam();
private:
  gs::gs_param_array *m_param;
  bool m_has_mother;
};

#endif

#ifdef USE_OLD_GSP_SC_PARAM
struct gsp_sc_param_module
{
public:
  gs::cnf::gs_param<std::string>* param_factory(char* name);
  gs::cnf::gs_param<std::string*>* array_param_factory(char* name);
  gs::cnf::gs_param<std::string*>* array_param_factory(char* name, unsigned size);
private:
  std::vector<gs::cnf::gs_param<std::string>*> param_vec;
  std::vector<gs::cnf::gs_param<std::string*>*> array_param_vec;
};

gsp_sc_param_module* get_gsp_module();

class gsp_sc_param
{
public:
  gsp_sc_param(char* name);
  std::string get_value();
  void set_value(const std::string& value);
  gs::cnf::gs_param<std::string>& get_param();
protected:
  gs::cnf::gs_param<std::string>* v;
  gs::cnf::gs_param_base* v1;
  bool external;
};

class gsp_sc_array_param
{
public:
  gsp_sc_array_param(char* name);
  gsp_sc_array_param(char* name, unsigned size);
  std::string get_value(unsigned index);
  void set_value(const std::string& value, unsigned index);
  unsigned size();
  void resize(unsigned new_size);
  gs::cnf::gs_param<std::string*>& get_param();
protected:
  gs::cnf::gs_param<std::string*>* v;
  gs::cnf::gs_param_base* v1;
  bool external;
};

#endif // USE_GREENCONTROL


//////////////////////////////////////////////////
// GreenAV

#ifdef USE_GREENCONTROL

// TODO: change order of #includes in GreenAV plugins so this next include is no more necessary
#include "greencontrol/gav/plugin/gav_plugin.h"

// Importing known plugins
#include "greencontrol/gav/plugin/File_OutputPlugin.h"
#include "greencontrol/gav/plugin/Stdout_OutputPlugin.h"
#include "greencontrol/gav/plugin/CSV_OutputPlugin.h"
#ifdef USE_SCV
#include "greencontrol/gav/plugin/SCV_OutputPlugin.h"
#endif // USE_SCV

// Declaring known plugins to SWIG
namespace gs {
  namespace av {

    typedef unsigned OutputPluginType;

    extern const OutputPluginType NULL_OUT;
    extern const OutputPluginType DEFAULT_OUT;
    extern const OutputPluginType TXT_FILE_OUT;
    extern const OutputPluginType STDOUT_OUT;
    extern const OutputPluginType CSV_FILE_OUT;
#ifdef USE_SCV
    extern const OutputPluginType SCV_STREAM_OUT;
#endif // USE_SCV

  }
}

class gsp_sc_gav_output
{
public:
  //TODO: if name is null, return default output for a type
  gsp_sc_gav_output(gs::av::OutputPluginType type, std::string name);

  bool add(std::string param_name);
  void add();
  void add_all();

  bool remove(std::string param_name);
  void remove();

  void pause();
  void pause(double time, sc_core::sc_time_unit tu=sc_core::SC_NS);
  void pause(PyObject* e);

  void resume();

protected:
  gs::cnf::cnf_api* configAPI;
  boost::shared_ptr<gs::av::GAV_Api> analysisAPI;
  gs::av::OutputPlugin_if* op;
};

#endif // USE_GREENCONTROL


//////////////////////////////////////////////////
// Functions

void gsp_sc_start();
void gsp_sc_start(double time, sc_core::sc_time_unit tu=sc_core::SC_NS);

void gsp_sc_stop();

void gsp_sc_wait(double time, sc_core::sc_time_unit tu=sc_core::SC_NS);
void gsp_sc_wait(PyObject* e);

double gsp_sc_simulation_time(sc_core::sc_time_unit tu=sc_core::SC_NS);
double gsp_sc_delta_count();

PyObject *gsp_sc_spawn(PyObject *runnable, PyObject *name);

long gsp_sc_get_curr_process_handle();

bool gsp_sc_is_running();

void gsp_sc_print_sc_splash();

void gsp_sc_init();

#ifdef USE_GREENCONTROL
void gsp_sc_config_file(char* filename);
void gsp_sc_config_lua(char* filename);
#endif


//////////////////////////////////////////////////
// GreenMessage

#include "greenmessage/greenmessage.hpp"

class gsp_sc_msgfifo;
class gsp_sc_msg_ref;

// Message class
class gsp_sc_msg_base
{
public:
  virtual ~gsp_sc_msg_base();

  // some methods return PyObject* rather than void to convince
  // SWIG to propagate Python exceptions
  PyObject *__getitem__(PyObject *k);
  PyObject *__setitem__(PyObject *k, PyObject *v);
  PyObject *__delitem__(PyObject *k);

  unsigned __len__();
  PyObject *__contains__(PyObject *k);

  PyObject *items();
  PyObject *keys();
  PyObject *values();

  PyObject *copy();
  PyObject *update(PyObject *nargs);

  PyObject *force_string();
  unsigned length_as_array();

  // function to give us a pointer to a C++ object from PyObject*
  void set_gs_msg_context();
  PyObject *str_to_Py(const std::string &s);

protected:
  static gs::msg::Message *current_message;
  static const gs::msg::Message *current_const_message;
  static gsp_sc_msg_base *current_mother;
  static bool current_force_string;
  static PyObject *m_msg_class;
  static PyObject *m_msg_ref_class;

  bool contains(std::string k_str);
  bool Py_to_str(PyObject *po, std::string &s);

  template<PyObject *ELEMENT(
    gs::msg::Message::children_map::iterator, gsp_sc_msg_base *)>
    PyObject *build_list();

  // friend classes the can access the m_msg directly
  friend class gsp_sc_writeif;
  friend class gsp_sc_writeif_base;
  friend class gsp_sc_msg_ref;
  friend void gsp_sc_init_msg_classes(PyObject *, PyObject *);

  // GreenMessage object.  this is always created new() in a gsp_sc_msg
  // and always a reference to an external object in a gsp_sc_string_msg
  gs::msg::Message *m_msg;
  gs::msg::Message *m_mother_msg;
  unsigned *m_ref_count;
  bool m_force_string;
};

// Class for presenting sub-messages (that is, messages that do not
// own their own content but just point to a part of higher-level
// [sub-]message
class gsp_sc_msg_ref : public gsp_sc_msg_base
{
public:
  gsp_sc_msg_ref();
};

// Normal messages
class gsp_sc_msg : public gsp_sc_msg_base
{
public:
  gsp_sc_msg(PyObject* args=0);
//  gsp_sc_msg(const gs::msg::Message &msg);
//  gsp_sc_msg(const gsp_sc_msg &msg);
};

// function to make the constructors for the Python wrapper classes
// for gsp_sc_msg(_ref) visible to the C++.
// must be called on import of gsp_sc (by gs.py).
// needed because we may return different sorts of PyObject from
// __getitem__() and others.  also gets SWIG out of the garbage
// collection
void gsp_sc_init_msg_classes(
  PyObject *msg_class, PyObject *msg_ref_class);


// WriteIf
// this class is used when Python wants to send a message to C++.  The
// Python code gets a reference to an object of this class and can
// write messages into it.
class gsp_sc_writeif {
public:
  gsp_sc_writeif(gs::msg::WriteIf& writeif);
  gsp_sc_writeif();
  bool write(const gsp_sc_msg_base *msg);
  bool nb_write(const gsp_sc_msg_base *msg);
  int num_free() const;
  bool invalid() const;
  void set_writeif_context();
protected:
  gs::msg::WriteIf* m_writeif;
  static gs::msg::WriteIf *current_writeif;
  friend void gsp_sc_event_bind(PyObject* e, PyObject* o, event_type t);
};


// a similar class is needed for when Python wants to receive messages from
// C++ without using a normal sc_fifo<>.  In this case the constructor
// is called from Python, with a name provided by Python for inclusion
// in the name registry.
class gsp_sc_writeif_base : public gs::msg::receiver_base {
public:
  // "self" is a Python object containing the necessary callback functions
  gsp_sc_writeif_base(char *name, PyObject *self);
  ~gsp_sc_writeif_base();
  void write(const gs::msg::Message &msg);
  bool nb_write(const gs::msg::Message& msg);
  int num_free() const;
  const sc_core::sc_event &data_read_event() const;
private:
  PyObject *get_method(const char *name) const;
  PyObject *py_self;
};


gsp_sc_writeif
gsp_sc_findReceiver(std::string name);


#endif //GSP_SC_H
