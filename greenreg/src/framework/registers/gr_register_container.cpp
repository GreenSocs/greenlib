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
// 2008-12-01 Christian Schroeder: 
// 2008-12-08 Puneet Arora: replaced deviceName+basename() with name() in the error condition in create_register
// 2009-01-21 Christian Schroeder: changed namespace
// 2009-01-29 Christian Schroeder: renamings
// 2009-01-29 FURTHER CHANGES SEE SVN LOG!
//


#include "greenreg/framework/registers/gr_register_container.h"

#include "greenreg/framework/registers/gr_register_container_reference_container.h"

#include "greenreg/framework/registers/gr_register_sharedio.h"
#include "greenreg/framework/registers/gr_register_splitio.h"

#include "greenreg/framework/devices/gr_device.h"

namespace gs {
namespace reg {

gr_register_container::gr_register_container( sc_core::sc_module_name _name, register_container_addressing_mode_e _address_mode, unsigned int _dword_size, device & _owner)
: register_container( _name, _address_mode, _dword_size)
{
	_owner.rc.add_key_data_pair( (const char *)_name, *this);
}

gr_register_container::~gr_register_container()
{

}

void gr_register_container::create_register( std::string _name, std::string _description,
	uint_gr_t _offset, unsigned int _type,
	uint_gr_t _data, uint_gr_t _input_mask,
	uint_gr_t _width, uint_gr_t _lock_mask)
{
	// MARCUS: Check for out-of-bounds
	if( get_addressing_mode() == INDEXED_ADDRESS &&
		_offset * 4 >= m_register_input_store.size())
	{
		std::stringstream ss;
		ss << "Attempting to add the register '" << _name
                   << "' with offset " << std::showbase << std::hex  <<_offset << std::dec
                   << " and width " << _width
                   << " that goes out-of-bounds in the indexed-addressing container '" << basename() 
                   << "' of size " << m_register_input_store.size() << ".\n";
		GR_FATAL( ss.str().c_str());
	}
	else if( get_addressing_mode() == ALIGNED_ADDRESS &&
		_offset*8 + _width > m_register_input_store.size()*8)
	{
		std::stringstream ss;
		ss << "Attempting to add the register '" << _name
                   << "' with offset " << std::showbase << std::hex  <<_offset << std::dec
                   << " and width " << _width
                   << " that goes out-of-bounds in the aligned-addressing container '" << basename() 
                   << "' of size " << m_register_input_store.size() << ".\n";
		GR_FATAL( ss.str().c_str());
	}

	if( (_type & SINGLE_IO) > 0)
	{
    new gr_register_sharedio(*this, _name, _description, _offset, _data,
                             _input_mask, _type, _width, _lock_mask);
	}
	else if( (_type & SPLIT_IO) > 0)
	{
    new gr_register_splitio(*this, _name, _description, _offset, _data,
                            _input_mask, _type, _width, _lock_mask);
	}
	else {
	  std::stringstream ss;
	  ss << "ERROR: Invalid Register Type is specified. Failed to create register " << _name << " in " << name() << "\n";
	  GR_ERROR( ss.str().c_str());
	}
}

void gr_register_container::create_register_block( std::string _description,
	uint_gr_t _offset_start, uint_gr_t _offset_end, unsigned int _type,
	uint_gr_t _data, uint_gr_t _input_mask,
	uint_gr_t _width, uint_gr_t _lock_mask)
{
	uint_gr_t offset = _offset_start;

	while( offset < _offset_end)
	{
		std::stringstream ss;
		ss << "reg_0x" << std::hex << offset;
		create_register( ss.str(), _description, offset, _type, _data, _input_mask, _width, _lock_mask);
		
		if( this->get_addressing_mode() == gs::reg::ALIGNED_ADDRESS)
		{
			offset += _width/8;
		} else if( this->get_addressing_mode() == gs::reg::INDEXED_ADDRESS)
		{
			offset += 1;
		}
	}
	
}

} // end namespace gs:reg
} // end namespace gs:reg
