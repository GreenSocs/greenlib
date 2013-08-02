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

#ifndef __PLBScheduler_h__
#define __PLBScheduler_h__

#include <systemc.h>
#include "greenbus/transport/genericRouter.h"
#include "greenbus/transport/genericScheduler_if.h"
#include <set>
#include "greenbus/core/tlm.h"

#ifndef USE_STATIC_CASTS
#include "greenbus/transport/generic.h"
#endif

#include "greenbus/utils/gs_trace.h"
#include "greenbus/utils/gb_configurable.h"

using namespace tlm;

#ifndef USE_STATIC_CASTS
#include "greenbus/transport/PLB/PLB.dynamiccast.h"
#endif

struct unevenpair_with_prio_and_time {
  public:
  GS_ATOM m_tp;
  sc_time m_enqueue_time;
  unsigned int m_prio;
  
  unevenpair_with_prio_and_time(GS_ATOM tp, sc_time enqueue_time, unsigned int prio) 
    : m_tp(tp), 
      m_enqueue_time(enqueue_time), 
      m_prio(prio)
  {}
};

class PLBCmpScheme {

public:
  PLBCmpScheme(std::map<unsigned long, unsigned int>* masterMap_)
  : m_pMasterMap(masterMap_) 
  {}

  PLBCmpScheme()
  : m_pMasterMap(NULL) 
  {}

  bool operator()(const unevenpair_with_prio_and_time& x,
                  const unevenpair_with_prio_and_time& y){
    bool temp(false);
    if(x.m_prio == y.m_prio)
      //temp=(*m_pMasterMap)[x.m_tp.first->getMID()]<(*m_pMasterMap)[y.m_tp.first->getMID()];
      temp=(*m_pMasterMap)[_getRouterAccessHandle(x.m_tp)->getMID()]<(*m_pMasterMap)[_getRouterAccessHandle(y.m_tp)->getMID()];
    else
      temp = x.m_prio > y.m_prio;
    return temp;
  }

private:
  std::map<unsigned long, unsigned int>* m_pMasterMap;

};


//----------------------------------------------------------------------------

class dynamicPriorityScheduler 
: public GenericScheduler_if, // <GenericTransaction, GenericPhase>,
#ifdef DEBUG
  public gb_configurable,
#endif
  public sc_module
{
public:
  dynamicPriorityScheduler(sc_module_name name_, sc_time &t)
    : sc_module(name_),
      m_isPendingWasCalled(false), 
      m_clkPeriod(t)
  {    
    init();
  }

  dynamicPriorityScheduler(sc_module_name name_, double time, sc_time_unit base) 
    : sc_module(name_),
      m_isPendingWasCalled(false),
      m_clkPeriod(time,base)  	
  {
    init();
  }

  typedef std::multiset<unevenpair_with_prio_and_time, PLBCmpScheme > transactionSet;

  virtual void enqueue(GS_ATOM& t) {
#ifdef USE_STATIC_CASTS
  #ifndef USE_PLB_TRANSACTION
    m_queue.insert(unevenpair_with_prio_and_time(t, sc_time_stamp(), 2));
  #else
    m_queue.insert(unevenpair_with_prio_and_time(t, sc_time_stamp(), t.first->getRouterAccess().getMPrio()));
  #endif
#else
    PLB::PLBTransactionHandle test=boost::dynamic_pointer_cast<PLB::PLBTransaction>((t.first));
    if (test) {
      m_queue.insert(unevenpair_with_prio_and_time(t, sc_time_stamp(), test->getMPrio()));
    }
    else {
      m_queue.insert(unevenpair_with_prio_and_time(t, sc_time_stamp(), 2));
    }
#endif

#ifdef VERBOSE
    GenericRouterAccess& t_ = t.first->getRouterAccess();
    GS_TRACE(name(), "Queuing a RequestValid atom from master id=0x%x", (unsigned int)t_.getMID());
    GS_TRACE(name(), "Queue size now is %d", (int)m_queue.size());
#endif

#ifdef DEBUG
    queueSize = m_queue.size();
#endif

  }


  virtual GS_ATOM& dequeue(bool remove) {
    if(!m_isPendingWasCalled){
      SC_REPORT_ERROR(name(), "Dequeue was called without preceeding isPending(). This is not allowed.");
      m_last = GS_ATOM();
      return m_last;
    }
    if (m_queue.size()==0) {
      SC_REPORT_ERROR(name(), "Dequeue was called on empty queue.");
      m_last = GS_ATOM();
      return m_last;
    }

    m_last = (m_pos->m_tp);
    
    if (remove){
      m_queue.erase(m_pos);
      m_isPendingWasCalled=false;
      
#ifdef VERBOSE
      GenericRouterAccess& t_ = m_last.first->getRouterAccess();
      GS_TRACE(name(), "Popping a request from master id=0x%x", (unsigned int)t_.getMID());
      GS_TRACE(name(), "Queue size now is %d", (int)m_queue.size());
#endif
      
#ifdef DEBUG
    queueSize = m_queue.size();
#endif
    
    }
    
    return m_last;
  }

  
  virtual bool isPending() {
    m_isPendingWasCalled=true;
    for(m_pos=m_queue.begin(); m_pos!=m_queue.end();++m_pos){
      if(m_pos->m_enqueue_time+m_clkPeriod<=sc_time_stamp()){
        return true;
      }
    }
    return false;
  }

  virtual bool isEmpty() {
    return m_queue.size()==0;
  }

  /**
   * Store the master map of the router to get access to the master connection ordering
   */
  virtual void setMasterMap(std::map<unsigned long, unsigned int> *masterMap_) {
    m_cmp = PLBCmpScheme(masterMap_);
    m_queue = transactionSet(m_cmp);
  }


protected:
  bool m_isPendingWasCalled;
  transactionSet m_queue;
  transactionSet::iterator m_pos;
  PLBCmpScheme m_cmp;
  GS_ATOM m_last;
  sc_time m_clkPeriod;
  
private:
    void init() {
      GS_TRACE(name(), "I am a PLB scheduler queue.");
      
      // DUST structure analysis
#ifdef USE_DUST
      DUST_SCHEDULER("DynamicPriorityScheduler");
#endif

      // queue debugging
#ifdef DEBUG
      GB_PARAM(queueSize, gs_uint32, 0);
#endif
    } 

public:
#ifdef DEBUG
  gb_param<unsigned> queueSize;
#endif
     
};
  


#endif
