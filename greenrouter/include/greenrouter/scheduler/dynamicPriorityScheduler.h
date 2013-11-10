// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
//     GreenSocs Ltd
// 
// 
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT

#ifndef __dynamicPriorityScheduler_h__
#define __dynamicPriorityScheduler_h__

#include <set>
#include "tlm.h"
#include "greenrouter/genericScheduler_if.h"
#include "gsgpsocket/utils/gs_trace.h"
#include "gsgpsocket/extensions/priority_extension.h"
#include "greensocket/generic/green_socket_extension_support_base.h"

namespace gs{
namespace gp{

template<typename TRAITS>
struct unevenpair_with_prio_and_time_and_sock_id {
  public:
  typename GenericScheduler_if<TRAITS>::pair_type m_tp;
  sc_core::sc_time m_enqueue_time;
  unsigned int m_prio;
  unsigned int m_sock_id;
  
  unevenpair_with_prio_and_time_and_sock_id(const typename GenericScheduler_if<TRAITS>::pair_type& tp
                               , const sc_core::sc_time& enqueue_time
                               , unsigned int prio
                               , unsigned int sock_id
                               ) 
    : m_tp(tp), 
      m_enqueue_time(enqueue_time), 
      m_prio(prio),
      m_sock_id(sock_id)
  {}
};

template <typename TRAITS>
class dynamicPrioCmpScheme
{

public:
  dynamicPrioCmpScheme()
  {}

  bool operator()(const unevenpair_with_prio_and_time_and_sock_id<TRAITS>& x,
                  const unevenpair_with_prio_and_time_and_sock_id<TRAITS>& y){
    bool temp(false);
    if(x.m_prio == y.m_prio)
      temp=x.m_sock_id<y.m_sock_id;
    else
      temp = x.m_prio > y.m_prio;
    return temp;
  }

};


//----------------------------------------------------------------------------
template <typename TRAITS>
class dynamicPriorityScheduler_b 
: public GenericScheduler_if<TRAITS>,
  public sc_core::sc_module,
  protected gs::socket::extension_support_base<TRAITS>
{
  std::string m_type;

  virtual void bound_to(const std::string&, gs::socket::bindability_base<TRAITS>*, unsigned int){}
  virtual const std::string& get_type_string(){return m_type;}
  //virtual gs::socket::bindability_base<TRAITS>* get_other_side(unsigned int,unsigned int&){return NULL;}
  //virtual unsigned int get_num_bindings(){return 0;}

public:
  sc_core::sc_export<GenericScheduler_if<TRAITS> > protocol_scheduler_target;

  dynamicPriorityScheduler_b(sc_core::sc_module_name name_, const sc_core::sc_time &t)
    : sc_core::sc_module(name_),
      protocol_scheduler_target("protocol_scheduler_target"),
      m_isPendingWasCalled(false), 
      m_clkPeriod(t)
  {    
    init();
    protocol_scheduler_target(*this);
  }

  dynamicPriorityScheduler_b(sc_core::sc_module_name name_, double time, sc_core::sc_time_unit base) 
    : sc_core::sc_module(name_),
      protocol_scheduler_target("protocol_scheduler_target"),
      m_isPendingWasCalled(false),
      m_clkPeriod(time,base)  	
  {
    init();
    protocol_scheduler_target(*this);
  }

  typedef std::multiset<unevenpair_with_prio_and_time_and_sock_id<TRAITS>, dynamicPrioCmpScheme<TRAITS> > transactionSet;
  typedef typename GenericScheduler_if<TRAITS>::pair_type pair_type;

  virtual void enqueue(typename TRAITS::tlm_payload_type* txn, const typename TRAITS::tlm_phase_type& ph, unsigned int mid){
    gs::priority* prio;
    if (gs::socket::extension_support_base<TRAITS>::get_extension(prio, *txn))
      m_queue.insert(unevenpair_with_prio_and_time_and_sock_id<TRAITS>(pair_type(txn,ph), sc_core::sc_time_stamp(), prio->value, mid));
    else
      m_queue.insert(unevenpair_with_prio_and_time_and_sock_id<TRAITS>(pair_type(txn,ph), sc_core::sc_time_stamp(), 2, mid));
    
    GS_DUMP("Queuing a RequestValid txn from master id="<<mid);
    GS_DUMP("Queue size now is "<<(int)m_queue.size());
  }


  virtual pair_type& dequeue(bool remove=true){
    if(!m_isPendingWasCalled){
      SC_REPORT_ERROR(name(), "Dequeue was called without preceeding isPending(). This is not allowed.");
      m_last = pair_type();
      return m_last;
    }
    if (m_queue.size()==0) {
      SC_REPORT_ERROR(name(), "Dequeue was called on empty queue.");
      m_last = pair_type();
      return m_last;
    }

    m_last = (m_pos->m_tp);
    
    if (remove){      
      GS_DUMP("Popping a request from master id="<<m_pos->m_sock_id);
      m_queue.erase(m_pos);
      GS_DUMP("Queue size now is "<<(int)m_queue.size());
      m_isPendingWasCalled=false;      
    }
    
    return m_last;
  }

  
  virtual bool isPending() {
    m_isPendingWasCalled=true;
    for(m_pos=m_queue.begin(); m_pos!=m_queue.end();++m_pos){
      if(m_pos->m_enqueue_time+m_clkPeriod<=sc_core::sc_time_stamp()){
        return true;
      }
    }
    return false;
  }

  virtual bool isEmpty() {
    return m_queue.size()==0;
  }

protected:
  bool m_isPendingWasCalled;
  transactionSet m_queue;
  typename transactionSet::iterator m_pos;
  pair_type m_last;
  sc_core::sc_time m_clkPeriod;
  
private:
    void init() {
      GS_DUMP("I am a dynamic priority scheduler queue.");
      gs::socket::config<TRAITS> conf;
      conf.template use_mandatory_extension<gs::priority>();
    } 

};

class dynamicPriorityScheduler : public dynamicPriorityScheduler_b<tlm::tlm_base_protocol_types>{
public:
  dynamicPriorityScheduler(const char* name, double time, sc_core::sc_time_unit base)
    : dynamicPriorityScheduler_b<tlm::tlm_base_protocol_types>(name, time, base)
  { 
  }

  dynamicPriorityScheduler(const char* name, const sc_core::sc_time &t) 
    : dynamicPriorityScheduler_b<tlm::tlm_base_protocol_types>(name, t)
  {
  }  
};


}
}

#endif
