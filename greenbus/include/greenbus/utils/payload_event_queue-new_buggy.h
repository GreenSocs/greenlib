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

#include <map>
using std::multimap;
using std::pair;


namespace tlm {


  //---------------------------------------------------------------------------
  /**
   * Interface to the event queue with payload
   */
  //---------------------------------------------------------------------------
  template<typename PAYLOAD>
  class payload_event_queue_if : public virtual sc_interface
  {
  public:
   virtual void notify (PAYLOAD p) =0;
    virtual void notify (PAYLOAD p, double when, sc_time_unit base) =0;
    virtual void notify (PAYLOAD p, const sc_time& when) =0;
    virtual void cancel_all() =0;
    virtual PAYLOAD get_payload() =0;
    //virtual const bool fired() =0;
    virtual void wait(PAYLOAD) =0;
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

    SC_HAS_PROCESS( payload_event_queue );

    payload_event_queue();
    payload_event_queue( sc_module_name name_ );
    ~payload_event_queue();

    virtual void notify (PAYLOAD);
    virtual void notify (PAYLOAD, const sc_time& when);
    virtual void notify (PAYLOAD, double when, sc_time_unit base);
    virtual void cancel_all();

    virtual const sc_event& default_event() const;

//    virtual const bool fired();
    virtual bool advance();

    virtual void wait(PAYLOAD p);

    PAYLOAD get_payload();

  private:

    void fire_event();

  private:

    typedef pair<sc_time,PAYLOAD> payload_time;
    multimap< sc_time , PAYLOAD> m_ppq;

    sc_event m_e;   // default event
    sc_dt::uint64 m_delta;
    sc_time m_time;
  };


  //---------------------------------------------------------------------------
  //
  // Implementation (in the header to avoid use of export templates)
  //
  //---------------------------------------------------------------------------

  template<typename PAYLOAD>
  void payload_event_queue<PAYLOAD>::notify (PAYLOAD p, const sc_time& when)
  {
    m_ppq.insert( payload_time( when + sc_time_stamp() , p ) );
    m_e.notify(when); // note, this will only over-right the "newest" event.
  }
    
  template<typename PAYLOAD>
  void payload_event_queue<PAYLOAD>::notify (PAYLOAD p)
  {
    notify( p, SC_ZERO_TIME );
  }

  template<typename PAYLOAD>
  void payload_event_queue<PAYLOAD>::notify (PAYLOAD p, double when, sc_time_unit base )
  {
    notify( p, sc_time(when,base) );
  }

  template<typename PAYLOAD>
  const sc_event& payload_event_queue<PAYLOAD>::default_event() const
  {
    return m_e; 
  }

  template<typename PAYLOAD>
  bool payload_event_queue<PAYLOAD>::advance()
  {
    m_ppq.erase( m_ppq.begin() ); // eat top;

    if (m_ppq.size() > 0) {
      payload_time t=*(m_ppq.begin());
      if (t.first > sc_time_stamp()) {
        m_e.notify( t.first - sc_time_stamp() );
        return false;
      } else {
        assert(m_delta==sc_delta_count());
        m_e.cancel();
        return true;
      }
    } else {
      return false;
    }
    
  }
  
 template<typename PAYLOAD>
  PAYLOAD payload_event_queue<PAYLOAD>::get_payload()
  {
    if(m_time!=sc_time_stamp()) {//first "get_payload" call for this point of sim time
      m_time=sc_time_stamp();
      m_delta=sc_delta_count();
    }
    
    payload_time t=*(m_ppq.begin());                                   // get the current top
    while(t.first < sc_time_stamp())
    {		                                     // while it's an old one
      m_ppq.erase( m_ppq.begin() );              // eat top;
      t=*(m_ppq.begin());                                   // get the current top
      m_delta=sc_delta_count();
    }

    if (sc_delta_count() > m_delta ) {	// while we keep asking in the same delta - keep giving the same result
      m_ppq.erase( m_ppq.begin() );              // when the delta moves on, and we ask again, eat the top
      t=*(m_ppq.begin());                                   // get the current top
      m_delta=sc_delta_count();
    }

    //assert(t);
    assert( t.first==sc_time_stamp() );
    return t.second;
  }

//  template<typename PAYLOAD>
//  const bool payload_event_queue<PAYLOAD>::fired()
//  {
//    return m_delta == sc_delta_count();
//  }

  template<typename PAYLOAD>
  void payload_event_queue<PAYLOAD>::wait(PAYLOAD p)
  {
    //RG: do we need this?
    //sc_core::wait(default_event());
    //while (!p.same(m_payload)) {
    //  sc_core::wait(default_event());
    //}
  }


  static int 
  sc_time_compare( const void* p1, const void* p2 )
  {
    const sc_time* t1 = static_cast<const sc_time*>( p1 );
    const sc_time* t2 = static_cast<const sc_time*>( p2 );

    if( *t1 < *t2 ) {
      return 1;
    } else if( *t1 > *t2 ) {
      return -1;
    } else {
      return 0;
    }  
  }

  template<typename PAYLOAD>
  payload_event_queue<PAYLOAD>::payload_event_queue()
    : sc_module( sc_gen_unique_name( "payload_event_queue" ) ),
    m_delta(0),m_time(0-10,SC_NS)
    
  {
    SC_METHOD( fire_event );
    sensitive << m_e;
    dont_initialize();
    end_module();
  }

  template<typename PAYLOAD>
  payload_event_queue<PAYLOAD>::payload_event_queue( sc_module_name name_ )
    : sc_module( name_ ),
      m_delta(0),m_time(0-10,SC_NS)
  {
    SC_METHOD( fire_event );
    sensitive << m_e;
    dont_initialize();
    end_module();
  }

  template<typename PAYLOAD>
  payload_event_queue<PAYLOAD>::~payload_event_queue()
  {
    while (m_ppq.size() > 0) {
      m_ppq.erase( m_ppq.begin() );              // eat top;
    }
  }

  template<typename PAYLOAD>
  void payload_event_queue<PAYLOAD>::cancel_all()
  {
    while( m_ppq.size() > 0 )
      m_ppq.erase( m_ppq.begin() );              // eat top;
    m_e.cancel();
  }
  
  template<typename PAYLOAD>
  void payload_event_queue<PAYLOAD>::fire_event()
  {
    payload_time t=*(m_ppq.begin());                                   // get the current top
    while(t.first < sc_time_stamp())  //in case fire_event() triggers before any of the get_payload calls, we have to do the deletion of the old stuff here
    {		                                     // while it's an old one
      m_ppq.erase( m_ppq.begin() );              // eat top;
      t=*(m_ppq.begin());                                   // get the current top
    }
    // all thats left is to notify the next event
    if ( m_ppq.size() > 1 ) {
      m_e.notify( (*(++(m_ppq.begin()))).first - sc_time_stamp() );
    }
  }


} // namespace sc_core


#endif // PAYLOAD_EVENT_QUEUE_H
