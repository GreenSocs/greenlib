/*
Copyright (c) 2008, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// ChangeLog GreenSocs
//
// 2008-12-03 Christian Schroeder: 
//             renamed function gr_event::add_static gr_event::add_callback
// 2009-01-21 Christian Schroeder: changed namespace
// 2009-01-27 Christian Schroeder: changed bahavior to call callback and 
//             ignore m_timed setting as soon as a callback is registered;
//             deprecated notify_delayed
// 2009-01-29 Christian Schroeder: renamings
// 2009-01-29 FURTHER CHANGES SEE SVN LOG!
//


#include "greenreg/sysc_ext/kernel/gr_event.h"
#include "greenreg/sysc_ext/utils/gr_report.h"
#include "greencontrol/core/gc_globals.h" // for DEPRECATED_WARNING


namespace gs {
namespace reg {

bool gr_event::legacy_warned = false;
  
void gr_event::add_callback( gs::reg_utils::callback_base & _callback)
{
  gs::reg_utils::void_callback *cb = dynamic_cast<gs::reg_utils::void_callback*>(&_callback);
  assert(cb != NULL && "Must be called with void callback without parameters!");
  // create local callback to wrapper which wraps the 2-param-calls to this 0-param-call
  callback_2_0_wrapper *wr = new callback_2_0_wrapper(cb);
  m_callback_2_0_wrappers.push_back(wr);
  add_payload_callback(*(wr->m_cb));
}
  
void gr_event::add_payload_callback( gs::reg_utils::callback_base & _callback) {
  callback_type *cb = dynamic_cast<callback_type*>(&_callback);
  assert(cb != NULL && "Must be called with void callback with two parameters (transaction, time)!");
  m_callback_registered = true;
	m_callbacks.insert( cb);
}


gr_event::gr_event()
  : m_timed(GR_DEFAULT_EVENT_BEHAVIOR) // Change in gr_settings.h, If you change this default manually, also change the default m_events_enabled in I_event_switch.h which mus be consistent!!
  , m_callback_registered( false)
  , m_delayed(false)
  , m_delayer(NULL)
{
}

gr_event::~gr_event()
{
  // delete the wrappers
  std::vector<callback_2_0_wrapper*>::iterator it;
  for (it = m_callback_2_0_wrappers.begin(); it != m_callback_2_0_wrappers.end(); it++) {
    delete (*it); (*it) = NULL;
  }
  m_callback_2_0_wrappers.clear();

  delete m_delayer;
}

//sc_core::sc_event& gr_event::operator()()
//{
//	return m_event;
//}

void gr_event::notify_as_callback()
{
  transaction_type* tr = NULL;
  sc_core::sc_time ti(sc_core::SC_ZERO_TIME);

  callback_storage_type::iterator sit;
  for( sit = m_callbacks.begin(); sit!= m_callbacks.end(); sit++)
  {
    callback_type * cb = *sit;
    cb->execute(tr, ti);
  }
}
  
void gr_event::notify_as_callback(transaction_type* const &_transaction)
{
  transaction_type* tr = _transaction;
  
	callback_storage_type::iterator sit;
	for( sit = m_callbacks.begin(); sit!= m_callbacks.end(); sit++)
	{
		callback_type * cb = *sit;
    cb->execute(tr, m_delay);
	}
}

void gr_event::notify_as_callback_been_delayed(tlm::tlm_generic_payload* const &_tlm_transaction)
{
  assert (m_ext_supp_base != NULL && "Needs to be set when enqueuing to peq - where the transaction is acquired.");
  
  // Rebuild temporary GSGP transaction
  transaction_type tr(m_ext_supp_base, _tlm_transaction);
  transaction_type* tr_ptr = &tr;

  callback_storage_type::iterator sit;
  for( sit = m_callbacks.begin(); sit!= m_callbacks.end(); sit++)
  {
    callback_type * cb = *sit;
    cb->execute(tr_ptr, m_delay);
  }
  // came out of delayer, release transaction
  if (_tlm_transaction != NULL) {
    _tlm_transaction->release();
  }
}
  

void gr_event::enable_timing()
{
	m_timed = true;
}

void gr_event::disable_timing()
{
	m_timed = false;
}

bool gr_event::is_timing_enabled()
{
	return m_timed;
}

void gr_event::cancel()
{
	sc_core::sc_event::cancel();
}

void gr_event::notify()
{
	if(m_timed && !m_callback_registered) {
    legacy_event_switch_warning();
		sc_core::sc_event::notify();
	} else
		notify_as_callback();
}

void gr_event::notify( const sc_core::sc_time& _time)
{
	if(m_timed && !m_callback_registered) {
    legacy_event_switch_warning();
		sc_core::sc_event::notify( _time);
  } else {
    if (_time != sc_core::SC_ZERO_TIME)
      GR_WARNING("A gr_event will callback immediately although the event is scheduled for later");
		notify_as_callback();
  }
}

void gr_event::notify( double _time, sc_core::sc_time_unit _unit)
{
	if(m_timed && !m_callback_registered) {
    legacy_event_switch_warning();
		sc_core::sc_event::notify( _time, _unit);
	} else {
    if (_time != 0)
      GR_WARNING("A gr_event will callback immediately although the event is scheduled for later");
		notify_as_callback();
  }
}

void gr_event::notify( transaction_type* _transaction) {
	if(m_timed && !m_callback_registered) {
    //GR_INFO("Transaction will not be forwarded to notified function because no callback registered!");
    legacy_event_switch_warning();
    sc_core::sc_event::notify();
	} else {
    notify_as_callback(_transaction);
  }
}

void gr_event::notify_with_delay( transaction_type* _transaction) {
  if(m_timed && !m_callback_registered) {
    //GR_INFO("Transaction will not be forwarded to notified function because no callback registered!");
    legacy_event_switch_warning();
    sc_core::sc_event::notify(m_delay);
  } else {
    if (m_delayed) {
      // enqueue to delayer, acquire transaction
      tlm::tlm_generic_payload *tlm_tr = NULL;
      if (_transaction != NULL) {
        tlm_tr = _transaction->get_tlm_transaction();
        tlm_tr->acquire();
        m_ext_supp_base = _transaction->get_extension_support_base();
      }
      if (m_delayer == NULL) { // create the delayer, not during construction because user dont_initialize() would direct to wrong process handle!!
        m_delayer = new func_functor<tlm::tlm_generic_payload*>();
        m_delayer->set_function(this, &gr_event::notify_as_callback_been_delayed);
      }
      (*m_delayer)(tlm_tr, m_delay);
    } else {
      notify_as_callback(_transaction);
    }
  }
}
  
  
void gr_event::notify_delayed()
{
  DEPRECATED_WARNING("gr_event", "DEPRECATED: The function notify_delayed is deprecated!");
	if(m_timed && !m_callback_registered) {
    legacy_event_switch_warning();
		sc_core::sc_event::notify_delayed();
	} else
		notify_as_callback();
}

void gr_event::notify_delayed( const sc_core::sc_time& _time)
{
  DEPRECATED_WARNING("gr_event", "DEPRECATED: The function notify_delayed is deprecated!");
	if(m_timed && !m_callback_registered) {
    legacy_event_switch_warning();
		sc_core::sc_event::notify_delayed( _time);
	} else {
    if (_time != sc_core::SC_ZERO_TIME)
      GR_WARNING("A gr_event will callback immediately although the event is scheduled for later");
		notify_as_callback();
  }
}

void gr_event::notify_delayed( double _time, sc_core::sc_time_unit _unit)
{
  DEPRECATED_WARNING("gr_event", "DEPRECATED: The function notify_delayed is deprecated!");
	if(m_timed && !m_callback_registered) {
    legacy_event_switch_warning();
		sc_core::sc_event::notify_delayed( _time, _unit);
	} else {
    if (_time != 0)
      GR_WARNING("A gr_event will callback immediately although the event is scheduled for later");
		notify_as_callback();
  }
}

void gr_event::set_delay(const sc_core::sc_time delay) {
  m_delay = delay;
  m_delayed = true;
}

const sc_core::sc_time& gr_event::get_delay() const {
  if (!m_delayed) {
    GR_FORCE_WARNING("Warning: You get a not specified delay");
  }
  return m_delay;
}
  
  
} // end namespace gs:reg
} // end namespace gs:reg
