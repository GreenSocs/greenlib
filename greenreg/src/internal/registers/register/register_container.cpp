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
// 2008-12-08 Puneet Arora: introduced deviceName in the error mesg. when invalid offset is provided while bus_read/bus_write.
// 2009-01-21 Christian Schroeder: changed namespace
// 2009-01-29 Christian Schroeder: renamings
// 2009-01-29 FURTHER CHANGES SEE SVN LOG!
//

//#define EXCLUDE_FROM_BUILD
#ifndef EXCLUDE_FROM_BUILD
#undef EXCLUDE_FROM_BUILD

#include "greenreg/internal/registers/register/register_container.h"
#include "greenreg/sysc_ext/utils/gr_report.h"

#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include "greenreg/utils/stl_ext/string_utils.h"

namespace gs {
namespace reg {

register_container::register_container( sc_core::sc_module_name _name, register_container_addressing_mode_e _address_mode, unsigned int _dword_size)
: sc_core::sc_module( _name),
m_register_input_store( "register_bank", _dword_size),
m_addressing_mode( _address_mode),
m_next_register_index( -1)
{
}

register_container::~register_container()
{
}

void register_container::gr_dump( gr_dump_format_e _format, ::std::ostream & _stream, unsigned int _tab_level)
{
	std::map< uint_gr_t, I_register *>::const_iterator cmit;

	if( m_contents.size() > 0)
	{
		_stream << "\n" << gs::reg_utils::string::tabs( _tab_level).c_str() << "############\n"
				<< gs::reg_utils::string::tabs( _tab_level).c_str() << "# registers\n"
				<< gs::reg_utils::string::tabs( _tab_level).c_str() << "############\n\n";

		_stream << gs::reg_utils::string::tabs( _tab_level).c_str() << "# Specifies binding register container, DO NOT MODIFY!!!"
				<< gs::reg_utils::string::tabs( _tab_level).c_str() << "rc[" << basename() << "] = " << basename() << "\n";

		_tab_level++;

		for( cmit = m_contents.begin(); cmit != m_contents.end(); cmit++)
		{
			I_register * reg_p = cmit->second;
			if( reg_p != NULL)
				reg_p->gr_dump( _format, _stream, _tab_level);
		}

		_tab_level--;

	}
}

bool register_container::bus_read( unsigned int& _data, unsigned int _address, unsigned int _byte_enable, transaction_type* _transaction, bool _delayed)
{
	primary_register_data * primary = NULL;
	
	if( _byte_enable != 0)
	{
		// using m_active_register is a hack, need a better solution...
		if( this->is_valid_key(_address) )
		{
			
			m_active_register = &((*this)[_address]);
			primary = m_active_register->get_primary_register_data();
			_data = primary->on_received_read_request(_transaction, _delayed);
			m_active_register = NULL;
			
		} else {

			std::stringstream ss;
			ss << std::hex;
			ss << "WARNING: invalid register offset: 0x" << _address << " in " << name();
			ss << ", BUS: unknown, RD (forced), D: 0x" << _data;
			_data = 0xffffffff;
			ss << ", returned default data: 0x" << _data;
			GR_REPORT_FORCE_WARNING( "wrong_register_access", ss.str().c_str());

		}
		return( true);
	}
	return( false);
}

bool register_container::bus_write( unsigned int _data, unsigned int _address, unsigned int _byte_enable, transaction_type* _transaction, bool _delayed)
{
	primary_register_data * primary = NULL;

	if( _byte_enable != 0)
	{
		if( this->is_valid_key(_address) )
		{
			
			m_active_register = &((*this)[_address]);
			primary = m_active_register->get_primary_register_data();
			primary->on_received_write_request(_data, _transaction, _delayed);
			m_active_register = NULL;
			
		} else {

			std::stringstream ss;
			ss << std::hex;
			ss << "WARNING: invalid register offset: 0x" << _address << " in " << name();
			ss << ", BUS: unknown, WR (ignored), D: 0x" << _data;
			GR_REPORT_FORCE_WARNING("wrong_register_access", ss.str().c_str());

		}
		
		return( true);
	}
	return( false);
}

register_container_addressing_mode_e register_container::get_addressing_mode()
{
	return( m_addressing_mode);
}

int register_container::get_next_register_index()
{
  if (m_addressing_mode == INDEXED_ADDRESS)
  {
    if (((m_next_register_index + 1) >= 0) &&
  (((unsigned int)(m_next_register_index + 1)) < m_register_input_store.size()))	
    {
      m_next_register_index++;
      return m_next_register_index;
    }
    else
    {
      std::stringstream ss;
      ss << "Attempting to add more registers than allocated in indexing mode to container: "
      << basename() << "\n";
      GR_ERROR( ss.str().c_str());
    }
  }
  return -1;
}

} // end namespace gs:reg
} // end namespace gs:reg

#endif // EXCLUDE_FROM_BUILD
#undef EXCLUDE_FROM_BUILD
