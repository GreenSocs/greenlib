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

// ChangeLog GreenSocs
// 2009-01-21 Christian Schroeder: changed namespace
// 2009-01-29 Christian Schroeder: renamings
// 2009-01-29 FURTHER CHANGES SEE SVN LOG!


#ifndef BIT_RANGE_DATA_H_
#define BIT_RANGE_DATA_H_

#include "greenreg/gr_externs.h"

#include "greenreg/sysc_ext/kernel/gr_notification_rule_container.h"

namespace gs {
namespace reg {

class bit_range;
class I_register_data;

////////////////////////////////////////////
/// bit_range_data
/// despite the name, this class does not store
/// the actual bit_range_data, instead it references
/// the owning register for data.  Most importantly this
/// protects the integrity of the data by removing
/// unnecessary synchronization while providing the
/// facade that this class stores real data.  This
/// also allows for multiple overlapping bit ranges.
/// @author 
/// @since 
////////////////////////////////////////////
class bit_range_data
{
public:

	////////////////////////////////////////////
	/// Constructor
	////////////////////////////////////////////
	bit_range_data();

	////////////////////////////////////////////
	/// post_init
	/// A secondary initialization procedure executed
	/// from the creating code block which passes
	/// in the register data for accessability and the
	/// parent bit range for traversal.  This cannot
	/// be done at construction time because the bit_range
	/// actually requires the data buffers to be passed
	/// in as parameters.  Thus creating a bit range
	/// becomes a multi-step process.
	///
	/// @param _register_data ? register data to reference
	/// @param _parent ? parent object for shared resources
	///
	/// @see bit_range_accessor::create
	////////////////////////////////////////////
	void post_init( I_register_data & _register_data, bit_range & _parent);

	////////////////////////////////////////////
	/// Destructor
	////////////////////////////////////////////
	virtual ~bit_range_data();

	////////////////////////////////////////////
	/// get
	///
	/// @return data in buffer
	////////////////////////////////////////////
	uint_gr_t get();

	////////////////////////////////////////////
	/// set
	///
	/// @param _data ? new data to set in buffer
	////////////////////////////////////////////
	void set( const uint_gr_t & _data);

	////////////////////////////////////////////
	/// put
	/// another common name for set in TLM terminology
	///
	/// @param _data ? new data to set in buffer
	///
	/// @see set
	////////////////////////////////////////////
	inline void put( const uint_gr_t & _data)
	{ set( _data); }

	////////////////////////////////////////////
	/// operator () for type uint_gr_t
	/// provides access casting for reading
	/// from "THIS" buffer (i.e. does not care if input or output)
	///
	/// @return copy of data in buffer
	////////////////////////////////////////////
	inline operator uint_gr_t ()
	{ return get(); }

	////////////////////////////////////////////
	/// operator = (uint_gr_t)
	/// provides assignment casting for writing
	/// to "THIS" buffer (i.e. does not care if input or output)
	///
	/// @param _val ? the value to be assigned to the buffer
	/// @return refernce to self
	////////////////////////////////////////////
	inline bit_range_data & operator = ( bool & _data)
	{
		set( _data);
		return( *this);
	}

	////////////////////////////////////////////
	/// get_write_rules
	/// returns the write rules for this data container (which could be input, output, or other)
	/// the rule container is not created until needed.
	///
	/// @return gr_notification_rule_container for USR_*BUF_WRITE rules
	///
	/// @see gr_notification_rule_container
	////////////////////////////////////////////
	inline gr_notification_rule_container & get_write_rules()
	{
		if( m_write_rules == NULL)
			m_write_rules = new gr_notification_rule_container();
		return( *m_write_rules);
	}

protected:
	friend class bit_range_accessor;

	/// reference to register data
	I_register_data * m_register_data;

	/// reference to the parent bit range
	bit_range * m_parent;

	/// reference to the dynamically created container for write rules
	gr_notification_rule_container * m_write_rules;
};

} // end namespace gs:reg
} // end namespace gs:reg

#endif /*BIT_RANGE_DATA_H_*/
