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

#ifndef __roundRobinScheduler_h__
#define __roundRobinScheduler_h__

#include <systemc.h>
#include "greenbus/transport/genericRouter.h"
#include "greenbus/transport/genericScheduler_if.h"
#include <set>
#include "greenbus/core/tlm.h"
#include "greenbus/utils/gb_configurable.h"

using namespace tlm;


//--------------------------------------------------------------------------
/**
 * This is the comparator used for inserting new requests into the queue.
 */
//--------------------------------------------------------------------------
template <class T>
class fixedPrioritySchedulerCmp
{
public:
  fixedPrioritySchedulerCmp() {}

  fixedPrioritySchedulerCmp(std::map<unsigned long, unsigned int> *masterMap) :
    m_pMasterMap(masterMap)
  {}
    
  bool operator()(const T& x, const T& y)
  {
    GenericRouterAccess& t1 = x.first->getRouterAccess();
    GenericRouterAccess& t2 = y.first->getRouterAccess();

    
    //    cout<<"t1.get_mID(): "<<t1.get_mID()<<endl;
    //    cout<<"t2.get_mID(): "<<t2.get_mID()<<endl;
    //    cout<<"mastermap[t1.get_mID()]: "<< (*m_pMasterMap)[t1.get_mID()]<<endl;
    //    cout<<"mastermap[t2.get_mID()]: "<< (*m_pMasterMap)[t2.get_mID()]<<endl;
    
    // sorting criterium is the master connection ordering
    return (*m_pMasterMap)[t1.getMID()]<(*m_pMasterMap)[t2.getMID()];
  }

protected:
  std::map<unsigned long, unsigned int> * m_pMasterMap;  
};
                 

//--------------------------------------------------------------------------
/**
 * A fixed priority queue. Priorities are assigned in descending order, i.e.
 * the first masterport connected to the router is assigned the highest priority.
 *
 * The queue provides a gb_param queueSize with which to observe the current queue size.
 * Enable observation feature by #define QUEUE_DEBUG
 */
//--------------------------------------------------------------------------

class fixedPriorityScheduler 
: public GenericScheduler_if,
#ifdef QUEUE_DEBUG
  public gb_configurable,
#endif
  public sc_module
{
public:
  typedef fixedPrioritySchedulerCmp<GS_ATOM > transactionCmp;
  typedef std::multiset<GS_ATOM, transactionCmp> transactionSet;

  SC_HAS_PROCESS(fixedPriorityScheduler);
  /**
   * Default constructor.
   */
  fixedPriorityScheduler(sc_module_name name_)
    : sc_module(name_)
  { 
    init();
  }

  /**
   * This constructor is for compatibility reasons only.
   */
  fixedPriorityScheduler(sc_module_name name_, sc_time &t) 
    : sc_module(name_)
  {
    init();
  }


  virtual void enqueue(GS_ATOM& t) {
    m_queue.insert(t);

#ifdef VERBOSE
    GenericRouterAccess &t_ = t.first->getRouterAccess();
    GS_TRACE(name(), "Queuing a RequestValid atom from master id=0x%x", (unsigned int)t_.getMID());
    GS_TRACE(name(), "Queue size now is %d", (int)m_queue.size());
#endif

#ifdef QUEUE_DEBUG
    //queueSize = m_queue.size();
    if (m_queue.size()==1)
      time_queue_filled = sc_time_stamp();
#endif

  }


  virtual GS_ATOM& dequeue(bool remove) {
    if (m_queue.size()==0) {
      SC_REPORT_ERROR(name(), "Dequeue was called on empty queue.");
      m_last = GS_ATOM();
      return m_last;
    }
    m_pos = m_queue.begin();    
    m_last = (*m_pos);
    if (remove)
      m_queue.erase(m_pos);

#ifdef VERBOSE
    GenericRouterAccess& t_ = m_last.first->getRouterAccess();
    GS_TRACE(name(), "Popping a request from master id=0x%x", (unsigned int)t_.getMID());
    GS_TRACE(name(), "Queue size now is %d", (int)m_queue.size());
#endif

#ifdef QUEUE_DEBUG
    //queueSize = m_queue.size();
    if (m_queue.size()==0) // measure time when queue is in use
      time_queue_full += sc_time_stamp()-time_queue_filled;
#endif

    return m_last;
  }

  
  virtual bool isPending() {
    return m_queue.size()>0;
  }

  virtual bool isEmpty() {
    return m_queue.size()==0;
  }

  /**
   * Store the master map of the router to get access to the master connection ordering
   */
  virtual void setMasterMap(std::map<unsigned long, unsigned int> *masterMap_) {
    GS_TRACE(name(), "Master map has been set");
    m_cmp = transactionCmp(masterMap_);
    m_queue = transactionSet(m_cmp);
  }

#ifdef QUEUE_DEBUG
  virtual void end_of_simulation() {
    cout << name() << ": Queue was in use " << time_queue_full << " time. This is a queue workload of " 
         << time_queue_full/sc_time_stamp()*100 << "%" << endl;
  }
#endif
    


protected:
  transactionSet m_queue;
  transactionSet::iterator m_pos;
  transactionCmp m_cmp;
  GS_ATOM m_last;
  
private:
  void init() {
    GS_TRACE(name(), "I am a fixed-priority scheduler.");

#ifdef QUEUE_DEBUG
    GB_PARAM(queueSize, gs_uint32, 0);
    time_queue_filled=SC_ZERO_TIME;
    time_queue_full=SC_ZERO_TIME;
#endif
    
    // DUST structure analysis
#ifdef USE_DUST
    DUST_SCHEDULER("FixedPriorityScheduler");
#endif
  }

public:
#ifdef QUEUE_DEBUG
  gb_param<unsigned> queueSize;
  sc_time time_queue_filled, time_queue_full;
#endif
  
};
  


#endif
