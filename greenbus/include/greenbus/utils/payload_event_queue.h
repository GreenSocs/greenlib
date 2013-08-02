/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2005 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 2.4 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

 Modified from
  payload_event_queue.h -- Event Queue Facility Definitions

  Original Author: Ulli Holtmann, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Marcus Bartholomeu, GreenSocs, 2006-05
  Description of Modification: starting from the original sc_event_queue.cpp,
                               a payload was associated to each event trigger.

 *****************************************************************************/

// DONOTRELICENSE

#ifndef PAYLOAD_EVENT_QUEUE_H
#define PAYLOAD_EVENT_QUEUE_H

#include "sysc/communication/sc_interface.h"
#include "sysc/kernel/sc_module.h"
#include "sysc/kernel/sc_event.h"

#ifdef USE_DUST
# include "greenbus/utils/gs_dust_port.h"
#endif

#include <map>
using std::multimap;
using std::pair;


namespace tlm {

template <typename PAYLOAD>
class timeorderedlist
{
public:
  struct element
  {
    struct element  *next;
    PAYLOAD p;
    sc_time t;
    unsigned long long d;
    element(PAYLOAD p, sc_time t, unsigned long long d): p(p),t(t),d(d) {}
  };
  
  element *empties;
  element *list;
  unsigned int size;
    
  timeorderedlist() 
    : empties(NULL),
      list(NULL),
      size(0)
  {
  }
  
  ~timeorderedlist() {
    while(size) {
      delete_top();
    }

    while(empties){
      struct element *e=empties->next;
      delete empties;
      empties=e;
    }

  }
  
  void insert(PAYLOAD p, sc_time t) {
    if (!empties) {
      empties=new struct element(p,t, 0);
      empties->next=NULL;
    }
    
    struct element *e=empties;
    empties=empties->next;
    e->p=p;
    e->t=t;
    e->d=sc_delta_count();
    
    struct element * ancestor=NULL;
    struct element * iterator=list;
    while (iterator && iterator->t<=t){
      ancestor=iterator;
      iterator=iterator->next;
    }
    if (ancestor==NULL){
      e->next=list;
      list=e;
    }
    else {
      e->next=iterator;
      ancestor->next=e;
    }
    size++;
  }
  
  void delete_top(){
    if (list) {
      struct element *e=list;
      list=list->next;
      e->next=empties;
      empties=e;
      size--;
    }
  }
  
  unsigned int getSize()
  {
    return size;
  }
  
  PAYLOAD &top()
  {
    return list->p;
  }
  sc_time top_time()
  {
    return list->t;
  }

  unsigned long long& top_delta()
  {
    return list->d;
  }

  sc_time next_time()
  {
    return list->next->t;
  }
  
};
//private:
//timeorderedlist *empties;
//timeorderedlist *m_ppq;




//---------------------------------------------------------------------------
/**
 * Interface to the event queue with payload
 */
//---------------------------------------------------------------------------

//RG: wouldn't it be faster if the notifys took references?

  template<typename PAYLOAD>
  class payload_event_queue_if : public virtual sc_interface
  {
  public:
    virtual void notify (PAYLOAD p) =0;
    virtual void notify (PAYLOAD p, double when, sc_time_unit base) =0;
    virtual void notify (PAYLOAD p, const sc_time& when) =0;
    virtual void cancel_all() =0;
    virtual PAYLOAD &get_payload() =0;
    //virtual const bool fired() =0;
    virtual void wait(PAYLOAD) =0;
  };

  template<typename PAYLOAD>
  class payload_event_queue_output_if : public virtual sc_interface
  {
  public:
    virtual void notify (PAYLOAD& p) =0;
  };

  template<typename PAYLOAD>
  class peq_outport_binder : public sc_module, public payload_event_queue_output_if<PAYLOAD>
  {
  public:
    SC_CTOR(peq_outport_binder){}
    virtual void notify (PAYLOAD& p) {SC_REPORT_ERROR(name(),"This should not have happend!!!!");}
  };
  

//---------------------------------------------------------------------------
/**
 * An event queue that can contain any number of pending
 * notifications. Each notification have an associate payload.
 */
//---------------------------------------------------------------------------
template<typename PAYLOAD>
class payload_event_queue: 
  public payload_event_queue_if<PAYLOAD>,
  public sc_module
{
public:

#ifndef USE_DUST
  sc_port<payload_event_queue_output_if<PAYLOAD>,0> out_port;
#else
  gs_dust_port<payload_event_queue_output_if<PAYLOAD>,0> out_port;
#endif
  
  SC_HAS_PROCESS( payload_event_queue );
  
  payload_event_queue();
  payload_event_queue( sc_module_name name_ );
  virtual ~payload_event_queue();
  
  virtual void notify (PAYLOAD);
  virtual void notify (PAYLOAD, const sc_time& when);
  virtual void notify (PAYLOAD, double when, sc_time_unit base);
  virtual void cancel_all();
  
  virtual const sc_event& default_event() const;
  
  //virtual const bool fired();
  
  virtual void wait(PAYLOAD p);
  
  virtual PAYLOAD &get_payload();
  
  virtual void end_of_elaboration();
  
private:
  
  void advance();
  void fire_event(); //used for event based interface
  void fec();  //used for direct call based interface
  peq_outport_binder<PAYLOAD> dummy_bind;
  bool callMode;
  
private:
  
  timeorderedlist<PAYLOAD> m_ppq;
  
  sc_event m_e;   // default event
  sc_dt::uint64 m_delta;
  sc_time m_time;
  
private:
#ifdef DUST_RECORD_TRANSACTIONS
  /* for DUST SCV transaction recording */
  scv_tr_stream peq_stream;
  scv_tr_generator<std::string, std::string> peq_gen;
#endif
};


//---------------------------------------------------------------------------
//
// Implementation (in the header to avoid use of export templates)
//
//---------------------------------------------------------------------------

template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::notify (PAYLOAD p, const sc_time& when)
{
  m_ppq.insert(p,  when + sc_time_stamp() );
  m_e.notify(when); // note, this will only over-right the "newest" event.
}

template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::notify (PAYLOAD p)
{
  if (callMode){
    out_port[1]->notify(p);
  }
  else
  {
    //    notify( p, SC_ZERO_TIME ); // this is the original implementation, which adds one delta
    
    // TODO: this is Mark's no-delta patch, which might results in a problem for UserAPIs, if
    //       master and slave ports are directly connected:
    //       After ACKRequest has been received, the second get_payload() call of the slave 
    //       (after the master's SendData) delivers the wrong payload (i.e. the payload
    //       of the request phase, and not the payload of the data phase)! The reason is,
    //       that both Request and SendData are called during the same delta cycle by the master.
    //       To work around this "problem", a UserAPIs either can use notify(PAYLOAD, SC_ZERO_TIME)
    //       or has to call wait(SC_ZERO_TIME).
    m_ppq.insert(p, sc_time_stamp() ); // TODO: problem: get_payload() always delivers first element in queue
    m_e.notify(); // note, this will only over-right the "newest" event.
  }
}

template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::notify (PAYLOAD p, double when, sc_time_unit base )
{
  notify( p, sc_time(when,base) );
}

//TODO: make sure this is only called when NOT in call mode
// either call assert(!callMode) which adds runtime overhead
// or this method could just call a function pointer which gets set according to 
// the chosen mode during _end_of_elab. This would also insert a runtimeoverhead (jummp)
// but this should be less then an assert.
template<typename PAYLOAD>
const sc_event& payload_event_queue<PAYLOAD>::default_event() const
{
  return m_e; 
}

/// advance PEQ to now
template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::advance()
{
  sc_time now=sc_time_stamp();
  sc_time top=m_ppq.top_time();
  uint64 delta=sc_delta_count();
  
  if(m_time!=now) { //first "advance" call for this point of sim time
    m_time=now;
    m_delta=sc_delta_count();
  }
  
  while(top < now) { // while it's an old one
    m_ppq.delete_top();
    top=m_ppq.top_time();
    m_delta=delta;
  }
  
  if (delta > m_delta ) {  // if being asked in the same timepoint, but in a later delta, advance to next element
    m_ppq.delete_top();
    top=m_ppq.top_time();
    m_delta=delta;
  }
}

//TODO: make sure this is only called when NOT in call mode
// either call assert(!callMode) which adds runtime overhead
// or this method could just call a function pointer which gets set according to 
// the chosen mode during end_of_elab. This would also insert a runtimeoverhead (jummp)
// but this should be less then an assert.
template<typename PAYLOAD>
PAYLOAD &payload_event_queue<PAYLOAD>::get_payload()
{
  advance();
  return m_ppq.top();
}

//  template<typename PAYLOAD>
//  const bool payload_event_queue<PAYLOAD>::fired()
//  {
//    return m_delta == sc_delta_count();
//  }

//TODO: make sure this is only called when NOT in call mode
// either call assert(!callMode) which adds runtime overhead
// or this method could just call a function pointer which gets set according to 
// the chosen mode during end_of_elab. This would also insert a runtimeoverhead (jummp)
// but this should be less then an assert.
template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::wait(PAYLOAD p)
{
  sc_core::wait(default_event());
  while (p != get_payload()) {
    sc_core::wait(default_event());
  }
}

#ifdef DUST_RECORD_TRANSACTIONS
# define DUST_PEQ_STREAM_NAME "peq_stream"
# define DUST_PEQ_STREAM_KIND "transactor"
# define DUST_PEQ_GEN_NAME "peq_gen"
# define DUST_PEQ_CALL_BEGIN_NAME "begin"
# define DUST_PEQ_CALL_END_NAME "end"
# define DUST_PEQ_GS_ATOM_NAME "GS_ATOM"
# define DUST_PEQ_PHASE_NAME "phase"
# define DUST_PEQ_DATA_PREFIX "data_"
# define DUST_RECORD_TRANSACTION_DATA_WIDTH 64
#endif

template<typename PAYLOAD>
payload_event_queue<PAYLOAD>::payload_event_queue()
  : sc_module( sc_gen_unique_name( "payload_event_queue" ) ),
    dummy_bind("dummy_bind"), m_delta(0),m_time(0-10,SC_NS)
#ifdef DUST_RECORD_TRANSACTIONS
  ,peq_stream(DUST_PEQ_STREAM_NAME, DUST_PEQ_STREAM_KIND),
    peq_gen(DUST_PEQ_GEN_NAME, peq_stream,
            DUST_PEQ_CALL_BEGIN_NAME, DUST_PEQ_CALL_END_NAME)
#endif
{
  out_port(dummy_bind); //bind once to prevent unbound exception
  end_module();
}

template<typename PAYLOAD>
payload_event_queue<PAYLOAD>::payload_event_queue( sc_module_name name_ )
  : sc_module( name_ ),
    dummy_bind("dummy_bind"), m_delta(0),m_time(0-10,SC_NS)
#ifdef DUST_RECORD_TRANSACTIONS
   ,peq_stream(DUST_PEQ_STREAM_NAME, DUST_PEQ_STREAM_KIND),
    peq_gen(DUST_PEQ_GEN_NAME, peq_stream,
            DUST_PEQ_CALL_BEGIN_NAME, DUST_PEQ_CALL_END_NAME)
#endif
{
  out_port(dummy_bind); //bind once to prevent unbound exception
  end_module();
}

template<typename PAYLOAD>
payload_event_queue<PAYLOAD>::~payload_event_queue()
{
}

template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::end_of_elaboration(){
  if (out_port.size()==1){ //only dummy binding
    callMode=false;
    SC_METHOD( fire_event );
    sensitive << m_e;
    dont_initialize();
  }
  else {
      callMode=true;  
      //cout<<name()<<": Outport bound, so choosing 'call' version"<<endl<<flush;
      //cout<<name()<<": bound to "<<out_port.size()<<" target(s)."<<endl<<flush;
      SC_METHOD( fec );
      sensitive << m_e;
      dont_initialize();
  }
}

//TODO: determine rank of dummy_bind at start of simulation becaue there may be implementations where it's not at rank 0

template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::cancel_all()
{
  while( m_ppq.getSize() > 0 )
    m_ppq.delete_top();
  m_e.cancel();
}


template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::fire_event()
{
#ifdef DUST_RECORD_TRANSACTIONS
  // DUST transaction recording
  PAYLOAD p = m_ppq.top();
  // todo: put some sense into the begin and end strings
  scv_tr_handle h = peq_gen.begin_transaction(std::string(DUST_PEQ_CALL_BEGIN_NAME));
  // record phase 
  h.record_attribute(DUST_PEQ_PHASE_NAME, p.second);
  // record transaction container
  h.record_attribute(DUST_PEQ_GS_ATOM_NAME, *(p.first));
  
#ifdef DUST_ALSO_RECORD_DATA
  // record transaction data if in data phase
  if (p.second.isDataValid() || p.second.isResponseValid()) {    
    MData d;
    d.set((*(p.first)).getMData());
    char text[200];
    char data[DUST_RECORD_TRANSACTION_DATA_WIDTH];
    memset(data, 0, DUST_RECORD_TRANSACTION_DATA_WIDTH);
    if (!d.isPointer()) {
      for (unsigned int i=0; i<d.getSize(); i+=DUST_RECORD_TRANSACTION_DATA_WIDTH) {
        sprintf(text, "data_%d", i);
        memcpy(data, &(d.getData()[0])+i, i+DUST_RECORD_TRANSACTION_DATA_WIDTH>d.getSize() ? d.getSize()-i : DUST_RECORD_TRANSACTION_DATA_WIDTH);
        h.record_attribute(text, data);
      }
    } 
    else {
      h.record_attribute("data_is_pointer", (gs_uint32)d.getPointer());
    }
  }
#endif
  
  peq_gen.end_transaction(h, std::string(DUST_PEQ_CALL_END_NAME));
#endif
  
  // all thats left is to notify the next event
  advance();
  sc_time now=sc_time_stamp();  
  if ( m_ppq.getSize() > 1 ) { // TODO: wenn vorheriges PAYLOAD im selben Timepoint (nur älterer Delta) ist, ist es hier noch da!!!
    m_e.notify( m_ppq.next_time() - now) ;
  }
}


//TODO: call into all connected 'targets' (there could be more than one when using an external monitor)
template<typename PAYLOAD>
void payload_event_queue<PAYLOAD>::fec(){
  assert(m_ppq.getSize()); //there has to be something inside
  sc_time now=sc_time_stamp();
  sc_time top=m_ppq.top_time();
  unsigned long long delta=sc_delta_count();
  unsigned long long top_delta=m_ppq.top_delta();
  
  assert(top==now); //we must have waken up at the top's time
  
  while(m_ppq.getSize() && top==now && delta!=top_delta) { // push all active ones into target
    out_port[1]->notify(m_ppq.top()); //binding zero is dummy bind
    m_ppq.delete_top();
    if (m_ppq.getSize()) {
      top_delta=m_ppq.top_delta();
      top=m_ppq.top_time();
    }
  }
  if ( m_ppq.getSize()) {
    m_e.notify( top - now) ;
  }
  
}

} // namespace tlm


#endif // PAYLOAD_EVENT_QUEUE_H
