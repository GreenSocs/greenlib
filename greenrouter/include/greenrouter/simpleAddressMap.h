// LICENSETEXT
//
//   Copyright (C) 2007 - 2009 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//
//   Charles Wilson
//    XtremeEDA, Corporation
//    http://www.xtreme-eda.com
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

#ifndef __simpleAddressMap_h__
#define __simpleAddressMap_h__
#define CLASS_NAME SimpleAddressMap

#include "tlm.h"
#include "gsgpsocket/utils/gs_trace.h"
#include "gsgpsocket/transport/GSGPSlaveSocket.h"
#include "greenrouter/genericRouter_if.h"
#include <list>

/**
* Simple address map implementation for the generic protocol.
*/

namespace gs {
namespace gp {

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::dec;
using std::hex;
using std::ostream;

typedef sc_dt::uint64 Map_address_t;                  ///< address type
typedef unsigned int  Port_id_t;                      ///< port ID type

class Entry_t                                         ///< address map entry
{
 public:

 Map_address_t  lower_bound;                         ///< address lower bound
 Map_address_t  upper_bound;                         ///< address upper bound
 Port_id_t      port_id;                             ///< target port ID

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::Entry_t

   @brief constructor for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 Entry_t                                             ///< default constructor
 ( void
 )
 : lower_bound ( 0 )                                 ///< address lower bound
 , upper_bound ( 0 )                                 ///< address upper bound
 , port_id     ( 0 )                                 ///< port ID
 {
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::Entry_t

   @brief explicit constructor for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 Entry_t                                             ///< copy constructor
 ( const Map_address_t  lower                        ///< address lower bound
 , const Map_address_t  upper                        ///< address upper bound
 , const Port_id_t      id                           ///< target port ID
 )
 {
   // initialize entry based on passed values
   lower_bound = lower;
   upper_bound = upper;
   port_id     = id;
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::Entry_t

   @brief copy constructor for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 Entry_t                                             ///< copy constructor
 ( const Entry_t & entry                             ///< existing address map entry
 )
 {
   // initialize entry based on passed value
   lower_bound = entry.lower_bound;
   upper_bound = entry.upper_bound;
   port_id     = entry.port_id;
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::~Entry_t

   @brief destructor for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 ~Entry_t                                            ///< destructor
 ( void
 )
 {
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::operator=

   @brief assignment operator for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 Entry_t &                                           ///< new address map
 operator=                                           ///< assignment operator
 ( const Entry_t & entry                             ///< new address map entry to assign
 )
 {
   // assign entry
   this->lower_bound = entry.lower_bound;
   this->upper_bound = entry.upper_bound;
   this->port_id     = entry.port_id;

   return *this;
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::operator==

   @brief equality operator for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 int                                                 ///< 0 - false / 1 - true
 operator==                                          ///< equality operator
 ( const Entry_t & entry                             ///< address map entry to compare
 )
 const
 {
   int equality ( 1 );                               ///< entry equality return value

   // check for equality
   if (    ( this->lower_bound != entry.lower_bound  )
       ||  ( this->upper_bound != entry.upper_bound  )
       ||  ( this->port_id     != entry.port_id      ) )
   {
     equality = 0;
   }

   return equality;
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::operator<

   @brief less than operator for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 int                                                 ///< 0 - false / 1 - true
 operator<                                           ///< less than operator
 ( const Entry_t & entry                             ///< address map entry to compare
 )
 const
 {
   int less_than ( 0 );                              ///< entry less than return value

   // is the left's upper bound less than the right's lower
   if ( this->upper_bound < entry.lower_bound )
   {
     less_than = 1;
   }

   return less_than;
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::is_in

   @brief address is in 3-tuple address map entry's range

 -------------------------------------------------------------------------------------------- */    

 bool                                                ///< true / false
 is_in                                               ///< address in entry's range
 ( const Map_address_t & address                     ///< address to validate
 )
 const
 {
   bool in_range ( true );                           ///< entry less than return value

//    cout << "testing " << address << " against " << *this << endl;

   // is the address in the entries range
   if (    ( address < this->lower_bound )
       ||  ( address > this->upper_bound ) )
   {
     in_range = false;
   }

   return in_range;
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t:: merge_with

   @brief merge the current entry with the given address range 

   return true if succeed (address range are overlapping) else return false;
   start_address should be smaller then the end_address

 -------------------------------------------------------------------------------------------- */    

 bool                                                ///< true / false
 merge_with                                     
 ( Map_address_t start_address,Map_address_t end_address)
 {
   // is the address in the entries range
   if (    ( end_address < this->lower_bound )  ||  ( start_address > this->upper_bound ) )
   {
	// The passed range is not overlapping with the given entry
	// thus return false
	return false;
   }
   else if (    ( start_address >= this->lower_bound )  &&  ( end_address <= this->upper_bound ) )
   {
	// nothing to do, passed address range is completely inside the given range
	return true;
   }
   else
   {
       if( start_address < this->lower_bound )
		this->lower_bound = start_address;

       if( end_address > this->upper_bound )
		this->upper_bound = end_address;
       
       return true;
   }
 }

 /** -------------------------------------------------------------------------------------------
   @func Entry_t:: remove_range

   @brief remove the passed address range from the gien entry

   return true if succeed (that is address range are partially or not overlapping) 
   else return false (that is passed address range lies strictly with in the entry)
   start_address should be smaller then the end_address

 -------------------------------------------------------------------------------------------- */    

 bool                                                ///< true / false
 remove_range                  
 ( Map_address_t start_address,Map_address_t end_address)
 {
   // is the address in the entries range
   if (    ( end_address < this->lower_bound )  ||  ( start_address > this->upper_bound ) )
   {
	// nothing to do in this case 
	return true;
   }
   else if (    ( start_address >= this->lower_bound )  &&  ( end_address <= this->upper_bound ) )
   {
	// given addresse range can not be removed as it is completely lie inside the entry
	return false;
   }
   else
   {
       if( start_address <= this->lower_bound && end_address <= this->upper_bound)
		this->lower_bound = end_address +1;

       if( start_address >= this->lower_bound && end_address >= this->upper_bound )
		this->upper_bound = start_address - 1;

	if ( this->lower_bound > this->upper_bound )
		this->lower_bound = this->upper_bound = 0;
       
       return true;
   }
 }

 private:

 /** -------------------------------------------------------------------------------------------
   @func Entry_t::operator<<

   @brief stream insertion operator for 3-tuple address map entry

 -------------------------------------------------------------------------------------------- */    

 friend
 ostream &                                           ///< output stream
 operator<<                                          ///< stream insertion operator
 ( ostream &       output_stream                     ///< current stream
 , const Entry_t & entry                             ///< address map entry to stream
 )
 {
   // output [ lower, upper ] on port
   output_stream << "[ 0x" << hex << entry.lower_bound
                 << ", 0x" << hex << entry.upper_bound
                 << " ] on " << dec << entry.port_id;

   return output_stream;
 }
};

// types

typedef std::list < Entry_t > Map_t;                  ///< address map type
typedef Map_t::iterator       Map_iterator_t;         ///< address map iterator type

// ***********************************************************************************************
// ***********************************************************************************************
// ***********************************************************************************************

template < typename TRAITS, unsigned int PORTMAX = 255>
class SimpleAddressMap
{
 
 protected:

 Map_t           m_address_map;                        ///< address map ( list )

 private:

 typedef typename TRAITS::tlm_payload_type payload_type;
 // class constants

 const Port_id_t m_max_port;                           ///< maximum port ID
 const char *    m_routine_name;                       ///< routine name

 // class member variables

 Map_address_t   m_return_address;                     ///< generic return address

 public:

 SimpleAddressMap
 ( void
 )
 : m_max_port      ( PORTMAX             )             ///< 255 port maximum
 , m_routine_name  ( "SimpleAddressMap"  )             ///< routine name
 {
 }

 virtual ~SimpleAddressMap( void )
 {
 }



 //--------------------------------------------------------------------------

 template < typename SOCKET >
 void
 generateMap
 ( SOCKET & socket
 )
 {
   GS_DUMP_N ( m_routine_name, "Generating address map:" );
   GS_DUMP_N ( m_routine_name, "Number of target connections: " << socket.size () );

   for ( Port_id_t port_id = 0; port_id < socket.size (); port_id++ )
   {
     // get target_port
     Port_id_t tmp;

     gs::socket::bindability_base < TRAITS > * other_side    ( socket.get_other_side ( port_id, tmp ) );
     gs::socket::GreenSocketAddress_base *     other_gp      ( dynamic_cast<gs::socket::GreenSocketAddress_base*>(other_side) );
     GenericRouterBase < TRAITS, PORTMAX, SimpleAddressMap<TRAITS, PORTMAX> > *            other_router  ( NULL );
     string                                    other_name    ( "generic OSCI socket" );

     if ( other_side )
     {
       other_router  = dynamic_cast < GenericRouterBase < TRAITS, PORTMAX, SimpleAddressMap<TRAITS, PORTMAX>  > * > ( other_side->get_parent () );
       other_name    = other_side->get_name ();
     }

     Map_address_t address_low;                        ///< address lower bound
     Map_address_t address_high;                       ///< address upper bound
     bool          mapped        ( false );            ///< port mapped flag

     if ( other_router )
     {
       other_router->do_eoe ();

       address_low   = other_router->getAddressMap ().get_min ();
       address_high  = other_router->getAddressMap ().get_max ();

       insert ( address_low, address_high, port_id );
     }
     else if (other_gp)
     {
        address_low = other_gp->base_addr;
        address_high = other_gp->high_addr;

        /*
         * We don't want to MAP devices with null sized area.
         */
        if (other_gp->base_addr < other_gp->high_addr)
        {
            insert(address_low, address_high, port_id);
        }

#ifdef GS_SOCKET_ADDRESS_ARRAY
        /*
         * This loop will add all the other address range exists in the
         * greensocket, if there is only one address range num_address_range
         * should be 0.
         */
        for (int i = 0; i < other_gp->num_address_range; i++)
        {
            address_low = other_gp->parr_base_addr[i].getValue();
            address_high = other_gp->parr_high_addr[i].getValue();

            /*
             * We don't want to MAP devices with null sized area.
             */
            if (address_low < address_high)
            {
                insert(address_low, address_high, port_id);
            }
        }
#endif
     }
     else
     {
       // verify port mapping
       for ( Map_iterator_t i = m_address_map.begin (); i != m_address_map.end (); i++ )
       {
         if ( port_id == i->port_id )
         {
           mapped = true;

           break;
         }
       }      

       if ( ! mapped )
       {
         cerr  << "WARNING: Target ( "
               << other_name
               << " ) on port "
               << port_id
               << " of "
               << socket.name ()
               << " is not derived from GSGPSlaveSocket_base, so the addresses cannot be determined"
                  " automatically. Use router_instance.assign_address ( lower, upper, "
               << port_id
               << ")"
               << endl;
       }
     }

     // get address parameters
     if ( mapped )
     {
       for ( Map_iterator_t i = m_address_map.begin (); i != m_address_map.end (); i++ )
       {
         if ( port_id == i->port_id )
         {
           GS_DUMP_N ( m_routine_name
                     , "Target ( " << other_name
                                   << " ) connected to port "
                                   << port_id
                                   << " range: "
                                   << "[ 0x" << hex << i->lower_bound
                                   << ", 0x" << hex << i->upper_bound
                                   << " ]"
                                   << dec );
         }
       }
     }
   }
 }

 //--------------------------------------------------------------------------
 /**
  * Check for overlapping address ranges
  */
 //--------------------------------------------------------------------------
 void
 checkSanity
 ( void
 )
 {
   cout << "Address check successful." << endl;
 }

 //--------------------------------------------------------------------------
 // Define GS_VERBOSE_DUMPMAP if not defined in the compilation command line
 #if ( ! defined ( GS_VERBOSE_DUMPMAP ) )
   #if ( defined ( GS_VERBOSE ) )
     #define GS_VERBOSE_DUMPMAP 1
   #else /* GS_VERBOSE */
     #define GS_VERBOSE_DUMPMAP 0
   #endif  /*GS_VERBOSE  */
 #endif  /* GS_VERBOSE_DUMPMAP */

 /** --------------------------------------------------------------------------
  * dumpMap
  * @param verbose Only show the map if value is true. Default to true only when macro GS_VERBOSE is defined.
 -------------------------------------------------------------------------- */
 void
 dumpMap
 ( bool verbose = GS_VERBOSE_DUMPMAP
 )
 {
   if ( verbose )
   {
     cout << m_routine_name << " address map:" << endl;

     for ( Map_iterator_t i = m_address_map.begin (); i != m_address_map.end (); i++ )
     {
       cout << *i << endl;
     }
   }
 }

 /** --------------------------------------------------------------------------
  @func decode

  @param decode_address address to decode

  @return target port number

  @note on error, m_max_port is returned, success is false..

 -------------------------------------------------------------------------- */

 virtual std::vector<Port_id_t>&                                             ///< port ID
 decode                                                ///< decode
 //( Map_address_t decode_address                        ///< address to decode
 ( payload_type& txn,                        ///< address to decode
   bool &success,
 gs::socket::config<TRAITS>* conf           /// conf of the socket from which the trnx is coming. 
                                            /// This value is ignored here, and used when address map is extension based
 , unsigned int from                /// portId of the socket from which the trnx is coming
 )
 {
   static std::vector<Port_id_t> targetId(1);
   Map_address_t decode_address = txn.get_address(); 
   Port_id_t port_id ( m_max_port  );

   success = false;

   for ( Map_iterator_t i = m_address_map.begin (); i != m_address_map.end (); i++ )
   {
     if ( i->is_in ( decode_address ) )
     {
       success = true;
       port_id = i->port_id;

       break;
     }
   }

   targetId[0] = port_id;
   return targetId;
 }

 //--------------------------------------------------------------------------
 const Map_address_t &
 get_max
 ( void
 )
 {
   m_return_address = 0;

   if ( m_address_map.size () )
   {
     m_return_address = ( m_address_map.end () )->upper_bound;
   }
   else
   {
     SC_REPORT_ERROR ( m_routine_name, "get_max() called on empty map" );
   }

   return m_return_address;
 }

 //--------------------------------------------------------------------------
 const Map_address_t &
 get_min
 ( void
 )
 {
   if ( m_address_map.size () )
   {
     m_return_address = ( m_address_map.begin () )->lower_bound;
   }
   else
   {
     SC_REPORT_ERROR ( m_routine_name, "get_min () called on empty map" );
   }

   return m_return_address;
 }

 //--------------------------------------------------------------------------
 // returns the maximum port associated with this simple_address_map
 Port_id_t get_max_port(void)
 {
   return m_max_port;
 }



 /** --------------------------------------------------------------------------
  * Insert an address range into the address map
 -------------------------------------------------------------------------- */

 void
 insert
 ( Map_address_t  address_low                        ///< address lower bound
 , Map_address_t  address_high                       ///< address upper bound
 , Port_id_t      port_id                            ///< port ID
 )
 {
   if ( address_low > address_high )
   {
     SC_REPORT_ERROR ( m_routine_name, "address lower bound is greater than upper bound" );
   }
   else if ( port_id >= m_max_port )
   {
     SC_REPORT_ERROR ( m_routine_name, "target connection maximum exceeded" );
   }

   // add the entry
   bool    success ( true                                );
   Entry_t entry   ( address_low, address_high, port_id  );

   // check for range overlap
   for ( Map_iterator_t i = m_address_map.begin (); i != m_address_map.end (); i++ )
   {
     if ( ( i->is_in ( address_low ) ) || ( i->is_in ( address_high ) ) )
     {
       cout << "mapping failed: " << entry << " overlaps " << *i << endl;

       SC_REPORT_ERROR ( m_routine_name, "address range already mapped" );

       success = false;

       break;
     }
   }

   if ( success )
   {
     cout << __FUNCTION__ << " adding entry " << entry << endl;

     // add the entry
     m_address_map.push_back ( entry );

     // sort the map
     m_address_map.sort ();
   }    
 }
 /** --------------------------------------------------------------------------
  * returns the default address map 
 -------------------------------------------------------------------------- */
 const Map_t & get_address_map() const 
 {
    return m_address_map;
 }
};

}
}

#undef CLASS_NAME
#endif  /* __simpleAddressMap_h__ */
