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

#ifndef __phase_trace_h__
#define __phase_trace_h__
#include "greenbus/core/tlm.h"
#include "greenbus/transport/generic.h"
#include "greenbus/transport/genericRouter.h"
#include "systemc.h"
#include <map>
#include <sstream>

class transactionTrace{
public:
  transactionTrace(const char* name, std::map<unsigned int, std::string>& phases, sc_trace_file* tf): m_name(name){
    std::map<unsigned int, std::string>::iterator i;
    for (i=phases.begin(); i!=phases.end(); i++){
      traces[i->first]=(new sc_signal<bool>);
      std::string newname(name);
      std::string::size_type pos=newname.find('.', 0);
      while(pos!=std::string::npos){
        newname.replace(pos, 1, 1, '_');
        pos=newname.find('.', pos+1);
      }
      std::stringstream s;
      s<<newname<<"_"<<i->first<<i->second;
      sc_trace(tf, *traces[i->first], s.str().c_str());
    }
  }
  
  void set(unsigned int phase, unsigned int mid){
    *traces[phase]=mid+1;
  }

  void reset(unsigned int phase, unsigned int mid){
    *traces[phase]=0;
  }
    
private:
  std::map<unsigned int, sc_signal<bool>* > traces;
  const char * m_name;
};

class pendingTrace{
public:
  pendingTrace(bool tNi, unsigned int rank, unsigned int phase, bool sNr): m_tNi(tNi), m_sNr(sNr), m_rank(rank), m_phase(phase){}
  
  bool m_tNi, m_sNr;
  unsigned int m_rank, m_phase;
};

class phase_trace : public sc_module {
typedef tlm::unevenpair<GenericTransactionHandle,GenericPhase> pair_type;

public:
  SC_HAS_PROCESS(phase_trace);
  phase_trace(sc_module_name filename, sc_port<GenericRouter_if>& router_port, 
                std::map<unsigned int, std::string>& phases, std::map<unsigned int, bool>& i_aligns, std::map<unsigned int, bool>& t_aligns)
   :sc_module(filename), m_isSlvAligned(t_aligns), m_isMstAligned(i_aligns), m_router_port(router_port)
   {
    m_tf=sc_create_vcd_trace_file(name());
    for (unsigned int i=0; i<router_port->getTargetPort()->connected_in_ports.size(); i++){
#ifdef SYSTEMC21V1
      sc_module* p_mod=dynamic_cast<sc_module*>(router_port->getTargetPort()->connected_in_ports[i]->get_parent());
#else
      sc_module* p_mod=dynamic_cast<sc_module*>(router_port->getTargetPort()->connected_in_ports[i]->get_parent_object());
#endif
      if (p_mod){
        m_masters[((unsigned int)(router_port->getTargetPort()->connected_in_ports[i]))]=new transactionTrace(p_mod->name(), phases, m_tf);
      }
      else{
        SC_REPORT_ERROR("phase_trace", "Detected a port that is not owned by a module.");
      }
    }
    
    if (i_aligns.size()){
      std::map<unsigned int, std::string> tmp;
      std::map<unsigned int, bool>::iterator it;
      for (it=i_aligns.begin(); it!= i_aligns.end(); it++){
        tmp[it->first]=phases[it->first];
      }
      m_mstAligned=new transactionTrace("Master_phases_aligned", tmp, m_tf);
    }
    
    for (unsigned int i=0; i<router_port->getInitPort()->connected_in_ports.size(); i++) {
#ifdef SYSTEMC21V1
      sc_module* p_mod=dynamic_cast<sc_module*>(router_port->getInitPort()->connected_in_ports[i]->get_parent());
#else
      sc_module* p_mod=dynamic_cast<sc_module*>(router_port->getInitPort()->connected_in_ports[i]->get_parent_object());
#endif
      if (p_mod){
        m_slaves[i]=new transactionTrace(p_mod->name(), phases, m_tf);
      }
      else{
        SC_REPORT_ERROR("phase_trace", "Detected a port that is not owned by a module.");
      }
    }

    if (t_aligns.size()){
      std::map<unsigned int, std::string> tmp;
      std::map<unsigned int, bool>::iterator it;
      for (it=t_aligns.begin(); it!= t_aligns.end(); it++){
        tmp[it->first]=phases[it->first];
      }
      m_slvAligned=new transactionTrace("Slave_phases_aligned", tmp, m_tf);
    }    
    
    SC_METHOD(do_update);
    sensitive<<m_update;
    dont_initialize();
  }
  
  ~phase_trace(){
    sc_close_vcd_trace_file(m_tf);
    cout<<"file closed"<<endl<<flush;
  }
  
  #define MSTALIGN_S(name, rank) \
    if (m_isMstAligned[name]) m_mstAligned->set(name, rank)

  #define SLVALIGN_S(name, rank) \
    if (m_isSlvAligned[name]) m_slvAligned->set(name, rank)

  #define MSTALIGN_R(name, rank) \
    if (m_isMstAligned[name]) m_mstAligned->reset(name, rank)

  #define SLVALIGN_R(name, rank) \
    if (m_isSlvAligned[name]) m_slvAligned->reset(name, rank)
  
  void iSetTrace(pair_type& trans){
    m_masters[trans.first->getMID()]->set(trans.second.state, trans.first->getMID());
    MSTALIGN_S(trans.second.state, trans.first->getMID());
  }

  void iResetTrace(pair_type& trans){
    m_masters[trans.first->getMID()]->reset(trans.second.state, trans.first->getMID());
    MSTALIGN_R(trans.second.state, trans.first->getMID());
  }

  void tSetTrace(pair_type& trans){
    m_slaves[m_router_port->decodeAddress(trans.first->getMAddr())]->set(trans.second.state, m_router_port->decodeAddress(trans.first->getMAddr()));
    SLVALIGN_S(trans.second.state, m_router_port->decodeAddress(trans.first->getMAddr()));
  }

  void tResetTrace(pair_type& trans){
    m_slaves[m_router_port->decodeAddress(trans.first->getMAddr())]->reset(trans.second.state, m_router_port->decodeAddress(trans.first->getMAddr()));
    SLVALIGN_R(trans.second.state, m_router_port->decodeAddress(trans.first->getMAddr()));
  }

  void iSetTrace(pair_type& trans, unsigned int phase){
    m_masters[trans.first->getMID()]->set(phase, trans.first->getMID());
    MSTALIGN_S(phase, trans.first->getMID());
  }

  void iResetTrace(pair_type& trans, unsigned int phase){
    m_masters[trans.first->getMID()]->reset(phase, trans.first->getMID());
    MSTALIGN_R(phase, trans.first->getMID());
  }

  void tSetTrace(pair_type& trans, unsigned int phase){
    m_slaves[m_router_port->decodeAddress(trans.first->getMAddr())]->set(phase, m_router_port->decodeAddress(trans.first->getMAddr()));
    SLVALIGN_S(phase, m_router_port->decodeAddress(trans.first->getMAddr()));
  }

  void tResetTrace(pair_type& trans, unsigned int phase){
    m_slaves[m_router_port->decodeAddress(trans.first->getMAddr())]->reset(phase,m_router_port->decodeAddress(trans.first->getMAddr()));
    SLVALIGN_R(phase, m_router_port->decodeAddress(trans.first->getMAddr()));
  }

  void iSetTrace(pair_type& trans, sc_time time){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(false, trans.first->getMID(), trans.second.state, true)));
    m_update.notify(time);
  }

  void iResetTrace(pair_type& trans, sc_time time){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(false, trans.first->getMID(), trans.second.state, false)));
    m_update.notify(time);
  }

  void tSetTrace(pair_type& trans, sc_time time){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(true, m_router_port->decodeAddress(trans.first->getMAddr()), trans.second.state, true)));
    m_update.notify(time);
  }

  void tResetTrace(pair_type& trans, sc_time time){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(true, m_router_port->decodeAddress(trans.first->getMAddr()), trans.second.state, false)));
    m_update.notify(time);
  }

  void iSetTrace(pair_type& trans, sc_time time, unsigned int phase){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(false, trans.first->getMID(), phase, true)));
    m_update.notify(time);
  }

  void iResetTrace(pair_type& trans, sc_time time, unsigned int phase){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(false, trans.first->getMID(), phase, false)));
    m_update.notify(time);
  }

  void tSetTrace(pair_type& trans, sc_time time, unsigned int phase){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(true, m_router_port->decodeAddress(trans.first->getMAddr()), phase, true)));
    m_update.notify(time);
  }

  void tResetTrace(pair_type& trans, sc_time time, unsigned int phase){
    m_pendingTraces.insert(std::make_pair(sc_time_stamp()+time, pendingTrace(true, m_router_port->decodeAddress(trans.first->getMAddr()), phase, false)));
    m_update.notify(time);
  }
  
  void do_update(){
    std::multimap<sc_time, pendingTrace>::iterator it;
    
    for (it=m_pendingTraces.find(sc_time_stamp()); it!=m_pendingTraces.end() && it->first==sc_time_stamp(); it++){
      if (it->second.m_sNr){
        if (it->second.m_tNi) {
          m_slaves[it->second.m_rank]->set(it->second.m_phase, it->second.m_rank);
          SLVALIGN_S(it->second.m_phase, it->second.m_rank);
        }
        else {
          m_masters[it->second.m_rank]->set(it->second.m_phase, it->second.m_rank);
          MSTALIGN_S(it->second.m_phase, it->second.m_rank);
        }
      }
      else{
        if (it->second.m_tNi) {
          m_slaves[it->second.m_rank]->reset(it->second.m_phase, it->second.m_rank);
          SLVALIGN_R(it->second.m_phase, it->second.m_rank);
        }
        else {
          m_masters[it->second.m_rank]->reset(it->second.m_phase, it->second.m_rank);
          MSTALIGN_R(it->second.m_phase, it->second.m_rank);          
        }
      }
      m_pendingTraces.erase(it);      
    }
  }

private:
  sc_trace_file* m_tf;
  std::map<unsigned int, transactionTrace*> m_masters;
  std::map<unsigned int, transactionTrace*> m_slaves;
  std::map<unsigned int, bool >m_isSlvAligned;
  std::map<unsigned int, bool >m_isMstAligned;
  transactionTrace* m_mstAligned;
  transactionTrace* m_slvAligned;
  std::multimap<sc_time, pendingTrace> m_pendingTraces;
  sc_event_queue m_update;
  sc_port<GenericRouter_if>& m_router_port;
  
};
#endif
