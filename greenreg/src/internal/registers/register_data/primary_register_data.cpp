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
// 2008-11-25 Christian Schroeder: comment
// 2009-01-21 Christian Schroeder: changed namespace
// 2009-01-29 Christian Schroeder: renamings
// 2009-01-29 FURTHER CHANGES SEE SVN LOG!
//


#include "greenreg/internal/registers/register_data/primary_register_data.h"

#include <boost/config.hpp>
#include <systemc>

#include "greenreg/internal/registers/register/register_container.h"
#include "greenreg/internal/registers/bit_range/bit_range_accessor.h"

#include "greenreg/sysc_ext/kernel/gr_notification_rule_container.h"

namespace gs {
namespace reg {

primary_register_data::primary_register_data( sc_core::sc_module_name _name,
	I_register & _register, register_container & _register_container,
  uint_gr_t _offset, uint_gr_t _data, uint_gr_t _write_mask, uint_gr_t _lock_mask)
: I_register_data( _register, _write_mask, _lock_mask)
,sc_core::sc_module( _name)
,m_data( _register_container.m_register_input_store, 
		_register_container.get_addressing_mode() == ALIGNED_ADDRESS ? _offset : _offset * 4, 
		_register.get_width() / 8)
{

//	int reg;
	//unsigned int offset; // warning: unused variable
	//unsigned int size; // warning: unused variable

	if( _register_container.get_addressing_mode() == INDEXED_ADDRESS)
	{
		 _register_container.get_next_register_index();
	}

//	std::stringstream ss;
//	ss << std::hex << "reg: " << reg << " offset: " << offset << " size: " << size << "\n";
//	std::cout << ss.str();

	m_data = _data;
}

primary_register_data::~primary_register_data()
{
}

void primary_register_data::set( uint_gr_t _data, bool _warn_write_mask)
{
	uint_gr_t old_data = m_data;
  /*uint_gr_t masked_value = (_data & ~m_write_mask);
  if (_warn_write_mask) {
    if (masked_value != (m_data & ~m_write_mask)) { // warn if write different than current value to write protected area
      GR_REPORT_FORCE_WARNING("write_protected/unequal_current", "User write to write protected pri_reg_dat area/bits detected (written with bits different from current ones)! Will be ignored for the protected bits.");
    }
    if (masked_value != 0) { // warn if write to write protected area with values different from zero
      GR_REPORT_FORCE_WARNING("write_protected/unequal_zero", "User write to write protected pri_reg_dat area/bits detected (written with bits different from zero)! Will be ignored for the protected bits.");
    }
  }*/
  // original: m_data = _data & m_write_mask; // TODO: why was this not _data = ( _data & m_write_mask ) | ( old_data & ~m_write_mask ); ???
  //_data = ( _data & m_write_mask ) | ( old_data & ~m_write_mask ); // here should not be any write mask applied!
	m_data = _data;
	if( m_write_rules != NULL)
		m_write_rules->process_active_rules( old_data, _data);
	m_parent_register->br.process_obuf_write_rules( old_data, _data);
}

void primary_register_data::on_received_write_request( uint_gr_t _value, transaction_type* _transaction, bool _delayed)
{
  uint_gr_t old_value = m_parent_register->i.get();
  /*uint_gr_t masked_value = (_value & ~m_write_mask);
  if (masked_value != (m_data & ~m_write_mask)) { // warn if write different than current value to write protected area
    GR_REPORT_FORCE_WARNING("write_protected/unequal_current", "Bus write to write protected pri_reg_dat area/bits detected (written with bits different from current ones)! Will be ignored for the protected bits.");
  }
  if (masked_value != 0) { // warn if write to write protected area with values different from zero
    GR_REPORT_FORCE_WARNING("write_protected/unequal_zero", "Bus write to write protected pri_reg_dat area/bits detected (written with bits different from zero)! Will be ignored for the protected bits.");
  }*/
  uint_gr_t new_value;
	new_value = (( _value & m_write_mask ) | ( old_value & ~m_write_mask ));
  
  // nothing or wait or next_trigger, dependent on switch and current context
#if SYSTEMC_API == 210
  switch (sc_core::sc_get_curr_process_kind()) {
#else
  switch (sc_core::sc_get_current_process_handle().proc_kind()) {
#endif
    case sc_core::SC_METHOD_PROC_:
      // if reg uses events and this is being called by an SC_METHOD, suspend notification of pre rules!
      if (m_parent_register->get_events_enabled() || _delayed) {
        GR_FORCE_WARNING("ATTENTION: Pre read notification rules will be skipped because you are calling from within an SC_METHOD! Either disable events for notification rules of this register or use an SC_THREAD to be able to process a wait here!");
      } else {
        m_parent_register->get_pre_write_rules().process_active_rules( old_value, new_value, _transaction, _delayed);
        m_parent_register->br.process_tlm_pre_write_rules( old_value, new_value, _transaction, _delayed);
      }
      break;
    case sc_core::SC_THREAD_PROC_:
    case sc_core::SC_CTHREAD_PROC_:
      m_parent_register->get_pre_write_rules().process_active_rules( old_value, new_value, _transaction, _delayed);
      m_parent_register->br.process_tlm_pre_write_rules( old_value, new_value, _transaction, _delayed);
      if (m_parent_register->get_events_enabled() || _delayed) {
        wait( sc_core::SC_ZERO_TIME);
      }
      break;
    default:
      if (m_parent_register->get_events_enabled() || _delayed) {
        assert(false && "unexpected process handle"); exit(666);
      }
  }
    
  m_parent_register->i.set( _value);

  m_parent_register->get_post_write_rules().process_active_rules( old_value, _value, _transaction, _delayed);
  m_parent_register->br.process_tlm_post_write_rules( old_value, _value, _transaction, _delayed);
  
  // potentially call wait, dependent on switch and current context
  if (m_parent_register->get_events_enabled() || _delayed) {
#if SYSTEMC_API == 210
    switch (sc_core::sc_get_curr_process_kind()) {
#else
    switch (sc_core::sc_get_current_process_handle().proc_kind()) {
#endif
      case sc_core::SC_METHOD_PROC_:
        break;
      case sc_core::SC_THREAD_PROC_:
      case sc_core::SC_CTHREAD_PROC_:
        wait( sc_core::SC_ZERO_TIME);
        break;
      default:
        assert(false && "unexpected process handle"); exit(666);
    }
  }
}


uint_gr_t primary_register_data::on_received_read_request(transaction_type* _transaction, bool _delayed)
{
	
	uint_gr_t old_value = m_data;

  // call pre rules and potentially wait, dependent on switch and current context
#if SYSTEMC_API == 210
  switch (sc_core::sc_get_curr_process_kind()) {
#else
  switch (sc_core::sc_get_current_process_handle().proc_kind()) {
#endif
    case sc_core::SC_METHOD_PROC_:
      // if reg uses events and this is being called by an SC_METHOD, suspend notification of pre rules!
      if (m_parent_register->get_events_enabled() || _delayed) {
        GR_FORCE_WARNING("ATTENTION: Pre read notification rules will be skipped because you are calling from within an SC_METHOD! Either disable events for notification rules of this register or use an SC_THREAD to be able to process a wait here!");
      } else {
        m_parent_register->get_pre_read_rules().process_active_rules( old_value, old_value, _transaction, _delayed);
        m_parent_register->br.process_tlm_pre_read_rules( old_value, old_value, _transaction, _delayed);
      }
      break;
    case sc_core::SC_THREAD_PROC_:
    case sc_core::SC_CTHREAD_PROC_:
      m_parent_register->get_pre_read_rules().process_active_rules( old_value, old_value, _transaction, _delayed);
      m_parent_register->br.process_tlm_pre_read_rules( old_value, old_value, _transaction, _delayed);
      if (m_parent_register->get_events_enabled() || _delayed) {
        wait( sc_core::SC_ZERO_TIME);
      }
      break;
    default:
      if (m_parent_register->get_events_enabled() || _delayed) {
        assert(false && "unexpected process handle"); exit(666);
      }
  }
  
	uint_gr_t return_value = m_data;

  m_parent_register->get_post_read_rules().process_active_rules( old_value, m_data, _transaction, _delayed);
  m_parent_register->br.process_tlm_post_read_rules( old_value, m_data, _transaction, _delayed);

  // potentially call wait, dependent on switch and current context
  if (m_parent_register->get_events_enabled() || _delayed) {
#if SYSTEMC_API == 210
    switch (sc_core::sc_get_curr_process_kind()) {
#else
        switch (sc_core::sc_get_current_process_handle().proc_kind()) {
#endif
      case sc_core::SC_METHOD_PROC_:
        break;
      case sc_core::SC_THREAD_PROC_:
      case sc_core::SC_CTHREAD_PROC_:
        wait( sc_core::SC_ZERO_TIME);
        break;
      default:
        assert(false && "unexpected process handle"); exit(666);
    }
  }
  
	return( return_value);
}

} // end namespace gs:reg
} // end namespace gs:reg
