// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Manuel Geffken, Wolfgang Klingauf, Robert Guenzel
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

#ifndef _GS_DUST_PORT_H_
#define _GS_DUST_PORT_H_

#include "systemc.h"
#include "scv.h"
#include <string>
#include <sstream>
#include "greenbus/utils/gs_datatypes.h"
#include "dust/xml_log/static_model/model_builder.h"

namespace sc_core {

// DUST id prefix
#define DUST_ID_PREFIX_STRING "id"


// SCV stream name _reserved_ for port call transaction recording
#define DUST_PORT_STREAM_NAME "port_stream"
// SCV stream kind for port call transaction recording
#define DUST_PORT_STREAM_KIND "transactor"

// SCV generator name _reserved_ for port call transaction recording
#define DUST_PORT_GEN_NAME "call_gen"
// SCV begin attribute name for port call transaction recording
#define DUST_PORT_CALL_BEGIN_NAME "attrib1"
// SCV end attribute name for port call transaction recording
#define DUST_PORT_CALL_END_NAME "attrib2"


/**
 * Helper class to make sure that DUST is only initialized once.
 */
class dust_model_dumper {
public:
  dust_model_dumper() {}
  virtual ~dust_model_dumper() {}
  void init_model_builder() {
    static bool dust_static_model_has_already_been_dumped=false;
    if (!dust_static_model_has_already_been_dumped) {
      dust_static_model_has_already_been_dumped=true;
      // TODO: pruefen, ob DUST im Server-Modus gestartet werden muss
      model_builder();
    }
  }
};

extern void sc_warn_port_constructor();

/**
 * Template class which can be used to replace the original class sc_port to
 * enable transaction recording and later visualisation of SystemC port calls.
 * 
 * This class works just in the same way as its base class sc_port.
 */
template <class IF, int N = 1, sc_port_policy P=SC_ONE_OR_MORE_BOUND>
class gs_dust_port
: public sc_port_b<IF>,
  public virtual dust_model_dumper // we want exactly one dust_model_dumper instance
{
    
public:
  // typdefs
  
  typedef sc_port_b<IF> base_type;
  typedef gs_dust_port<IF,N> this_type;
  
  // constructors (analog to base class)
#ifdef DUST_RECORD_TRANSACTIONS    
  gs_dust_port() 
  : base_type( N, P ), 
    // instantiate tr. stream with stream name and stream kind
    port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
    // instantiate tr. generator with name, stream, begin attribute name
    call_gen(DUST_PORT_GEN_NAME, port_stream,
             DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
    mb(NULL)
  {}
    
  explicit gs_dust_port( const char* name_ ) // may not be used as a converting constructor
    : base_type( name_, N, P ),
    // instantiate tr. stream with stream name and stream kind
    port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
    // instantiate tr. generator with name, stream, begin attribute name
    call_gen(DUST_PORT_GEN_NAME, port_stream,
             DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
    mb(NULL)
  {}
  
    
  explicit gs_dust_port( IF& interface_ ) // may not be used as a converting constructor
  : base_type( N, P ), 
    port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
    call_gen(DUST_PORT_GEN_NAME, port_stream,
             DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
    mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( interface_ ); }
      
  gs_dust_port( const char* name_, IF& interface_ ) // may not be used as a converting constructor
  : base_type( name_, N, P ), 
    port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
    call_gen(DUST_PORT_GEN_NAME, port_stream,
             DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
    mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( interface_ ); }
      
    
  explicit gs_dust_port( base_type& parent_ ) // may not be used as a converting constructor
  : base_type( N, P ), 
    port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
    call_gen(DUST_PORT_GEN_NAME, port_stream,
             DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
    mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
  
  
  gs_dust_port( const char* name_, base_type& parent_ )
    : base_type( name_, N, P ), 
      port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
      call_gen(DUST_PORT_GEN_NAME, port_stream,
               DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
  
  
  gs_dust_port( this_type& parent_ )
    : base_type( N, P ), 
      port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
      call_gen(DUST_PORT_GEN_NAME, port_stream,
               DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
  
  
  gs_dust_port( const char* name_, this_type& parent_ )
    : base_type( name_, N, P ), 
      port_stream(DUST_PORT_STREAM_NAME, DUST_PORT_STREAM_KIND),
      call_gen(DUST_PORT_GEN_NAME, port_stream,
               DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }

#else
  gs_dust_port() 
    : base_type( N, P ),
      mb(NULL)
  {}
  
  explicit gs_dust_port( const char* name_ )
    : base_type( name_, N, P ), 
      mb(NULL)
  {}
  
  explicit gs_dust_port( IF& interface_ )
    : base_type( N, P ),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( interface_ ); }
  
  gs_dust_port( const char* name_, IF& interface_ )
    : base_type( name_, N, P ),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( interface_ ); }
  
  
  explicit gs_dust_port( base_type& parent_ )
    : base_type( N, P ),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
  
  
  gs_dust_port( const char* name_, base_type& parent_ )
    : base_type( name_, N, P ),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
  
  
  gs_dust_port( this_type& parent_ )
    : base_type( N, P ),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
  
  
  gs_dust_port( const char* name_, this_type& parent_ )
    : base_type( name_, N, P ),
      mb(NULL)
  { sc_warn_port_constructor(); base_type::bind( parent_ ); }
#endif
  
  virtual ~gs_dust_port() {
    if (mb != NULL)
      delete mb;
  }
  
  virtual const char* kind() const
  { return "gs_dust_port"; }    
  
  /// overwrite base class operator for transaction recording
  inline IF* operator -> ()
  {
    IF *ret = base_type::operator -> ();    
    record_tr_attribute(ret);    
    return ret;
  }
  

  /// overwrite base class operator for transaction recording
  inline const IF* operator -> () const
  {
    IF *ret = base_type::operator -> ();    
    record_tr_attribute(ret);    
    return ret;
  }  
  
  /// overwrite base class operator for transaction recording
  inline IF* operator [] (int index_)
  {
    IF *ret = base_type::operator [] (index_);    
    record_tr_attribute(ret);    
    return ret;
  }
  
  /// overwrite base class operator for transaction recording
  inline const IF* operator [] (int index_) const
  {
    IF *ret = base_type::operator [] (index_);    
    record_tr_attribute(ret);    
    return ret;
  }  
  
  /**
   * Set the master ID of this port. Used for transaction recording.
   */
  inline void setMasterID(tlm::gs_handle _id) 
  {
    id = _id;
  }  
    
private:
  // disabled
  gs_dust_port( const this_type& );
  this_type& operator = ( const this_type& );
  
protected:
  
    /**
     * This function performs the recording of the port call via SCV.
     * The recorded data includes the caller and the invoked channel/module.
     * 
     * @param IF the interface bound to this port
     * 
     */
    inline void record_tr_attribute(IF* interface) {
#ifdef DUST_RECORD_TRANSACTIONS
      if (interface != NULL) {      
        stringstream *stream = new stringstream();
        // module?
        if (dynamic_cast<sc_module *>(interface)) {
          sc_module *p = dynamic_cast<sc_module *>(interface);        
          *stream << DUST_ID_PREFIX_STRING << p;
        }
        // prim channel?
        else if (dynamic_cast<sc_prim_channel *>(interface)) {
          sc_prim_channel *p = dynamic_cast<sc_prim_channel *>(interface);
          *stream << DUST_ID_PREFIX_STRING << p;
        }
        
        // record invoked interface
        scv_tr_handle h = call_gen.begin_transaction(stream->str());
        
        h.record_attribute("master_id", id);
        
        stream->str("");
        stream->clear();
        
        *stream << DUST_ID_PREFIX_STRING << dynamic_cast<sc_port_base *>(this);
        
        // record port (sc_port_base) belonging to the caller
        call_gen.end_transaction(h, stream->str());
        
        delete stream;
      }
#endif
    }
  

  
  /**
   * The start_of_simulation method is called by the SystemC kernel
   * after end of elaboration, just before the simulation of the model
   * is going to start. 
   * We use this method here to initialize the DUST model_builder.
   * To prevent multiple initializations, a static init counter is used.
   */
  void start_of_simulation() {
    dust_model_dumper::init_model_builder();
  }

  tlm::gs_handle id;

    
private:
#ifdef DUST_RECORD_TRANSACTIONS
  /* for SCV transaction recording */
  scv_tr_stream port_stream;
  scv_tr_generator<std::string, std::string> call_gen;
#endif

  model_builder *mb;
  
  /* 
   * Note:
   * Common _static_ SCV stream and generator for port call recording
   * with the aim to reduce the amount of data in transaction model. The
   * concept does not work so far due to missing SCV database when static
   * members are instantiated. Therefore no SCV database is assigned to
   * the common SCV stream therefore SCV transaction recording does
   * not work.
   */
  //static scv_tr_stream port_stream2;
  //static scv_tr_generator <std::string, std::string> call_gen2;
  
};
// static members (see note above)
//template <class IF, int N> scv_tr_stream 
//    gs_dust_port<IF, N>::port_stream2(DUST_PORT_STREAM_NAME,
//    DUST_PORT_STREAM_KIND);
//template <class IF, int N> scv_tr_generator<std::string, std::string>
//    gs_dust_port<IF, N>::call_gen2(DUST_PORT_GEN_NAME,
//    port_stream2,
//   DUST_PORT_CALL_BEGIN_NAME, DUST_PORT_CALL_END_NAME);

} // namespace sc_core

#endif //_GS_DUST_PORT_H_

