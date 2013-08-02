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


#ifndef __GC_TLM_PORT_H__
#define __GC_TLM_PORT_H__

#include <systemc.h>
#include "greenbus/utils/gc_payload_event_queue.h"
#ifdef USE_DUST
#include "greenbus/utils/gs_dust_port.h"
#endif


namespace tlm {

//forward declaration
  template <class PV, class PVT>
  class call_tlm_multi_port;

  //---------------------------------------------------------------------------
  /**
   * Base for all TLM ports. The communication uses events for both
   * sending and receiving.
   */
  //---------------------------------------------------------------------------
  template <class PV, class PVT>
  class call_tlm_port
  : public sc_module
  {

  public:

    typedef PV b_if_type;
    typedef PVT if_type;
    typedef call_payload_event_queue_if<if_type> peq_if_type;
    
#ifdef USE_DUST
    gs_dust_port<b_if_type> b_out;
    sc_export<b_if_type> b_in;

    gs_dust_port<peq_if_type> out;
    sc_export<peq_if_type> in;
#else
    sc_port<b_if_type> b_out;
    sc_export<b_if_type> b_in;

    sc_port<peq_if_type> out;
    sc_export<peq_if_type> in;
#endif

    call_payload_event_queue<if_type> peq;

    SC_HAS_PROCESS(call_tlm_port);

    call_tlm_port( sc_module_name port_name ) :
      sc_module(port_name),
      b_out(sc_gen_unique_name("b_out")),
      b_in(sc_gen_unique_name("b_in")),
      out(sc_gen_unique_name("out")),
      in(sc_gen_unique_name("in"))
    {
      in(peq);
    }

    virtual ~call_tlm_port() {
    }

    void operator() (call_tlm_port<b_if_type,if_type>& other) {
      b_out(other.b_in);
      other.b_out(b_in);
      out(other.in);
      other.out(in);
    }

    void bind_b_if(b_if_type& other) {
      b_in(other);
    }

    void operator() (call_tlm_multi_port<b_if_type,if_type>& other) {
      b_out(other.b_in);
      other.b_bind_out_port(b_in);
      out(other.in);
      other.bind_out_port(in);
    }

    /*void wait() {
      sc_core::wait(in->default_event());
      }*/

    void wait(if_type payload) {
      in->wait(payload);
    }

    //if_type get_payload() {
    //  return in->get_payload();
    //}

    //const sc_event& default_event() {
    //  return in->default_event();
    //}

//    const bool fired() {
//      return in->fired();
//    }
    
  };



  //---------------------------------------------------------------------------
  /**
   * A TLM multi port.
   */
  //---------------------------------------------------------------------------
  template <class PV, class PVT>
  class call_tlm_multi_port
  : public sc_module
  {

  public:

    typedef PVT if_type;
    typedef PV b_if_type;
    typedef call_payload_event_queue_if<if_type> peq_if_type;

#ifdef USE_DUST
    gs_dust_port<peq_if_type, 0> out;
    sc_export<peq_if_type> in;

    gs_dust_port<b_if_type, 0> b_out;
    sc_export<b_if_type> b_in;
#else
    sc_port<peq_if_type, 0> out;
    sc_export<peq_if_type> in;

    sc_port<b_if_type, 0> b_out;
    sc_export<b_if_type> b_in;
#endif

    std::vector< sc_export<b_if_type>* > connected_b_in_ports;

    call_payload_event_queue<if_type> peq;
    std::vector< sc_export<peq_if_type>* > connected_in_ports;
    
    call_tlm_multi_port( sc_module_name port_name ) :
      sc_module(port_name),
      out(sc_gen_unique_name("out")),
      in(sc_gen_unique_name("in")),
      b_out(sc_gen_unique_name("b_out")),
      b_in(sc_gen_unique_name("b_in"))
    {
      in(peq);
    }

    virtual ~call_tlm_multi_port() {
    }

    void operator() (call_tlm_port<b_if_type,if_type>& other) {
      b_bind_out_port(other.b_in);
      other.b_out(b_in);
      bind_out_port(other.in);
      other.out(in);
    }

    void bind_b_if(b_if_type& other) {
      b_in(other);
    }

    void operator() (call_tlm_multi_port<b_if_type,if_type>& other) {
      //SC_REPORT_FATAL(sc_core::SC_ID_NOT_IMPLEMENTED_, "multi-to-multi-port binding not supported (yet).");
      b_bind_out_port(other.b_in);
      other.b_bind_out_port(b_in);
      bind_out_port(other.in);
      other.bind_out_port(in);
    }

    void bind_out_port(sc_export<peq_if_type>& in){
      for (unsigned int i=0; i<connected_in_ports.size(); i++){
        if(connected_in_ports[i]==(&in)){
          cout<<"Double binding of PVT ports between "<<this->name()<<" and "<<dynamic_cast<sc_object*>(&in)->get_parent()->name()<<endl<<flush;
          return;
        }
      }
      connected_in_ports.push_back(&in);
      out(in);
    }
    
    void b_bind_out_port(sc_export<b_if_type>& b_in){
      for (unsigned int i=0; i<connected_b_in_ports.size(); i++){
        if(connected_b_in_ports[i]==(&b_in)){
          cout<<"Double binding of PV ports between "<<this->name()<<" and "<<dynamic_cast<sc_object*>(&b_in)->get_parent()->name()<<endl<<flush;
          return;
        }
      }
      connected_b_in_ports.push_back(&b_in);
      b_out(b_in);
    }

    //void wait() {
    //  sc_core::wait(in->default_event());
    //}

    void wait(if_type payload) {
      in->wait(payload);
    }

    //if_type get_payload() {
    //  return in->get_payload();
    //}

    //const sc_event& default_event() {
    //  return in->default_event();
    //}

    const bool fired() {
      return in->fired();
    }

  };


} // end of namespace tlm

#endif
