// LICENSETEXT
//
//   Copyright (C) 2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//
//   Stephan Beal (stephan@s11n.net)
//
//   Modified by :
//
//   Marcus Bartholomeu
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

#ifndef s11n_LEX_T_HPP_INCLUDED
#define s11n_LEX_T_HPP_INCLUDED 1
////////////////////////////////////////////////////////////////////////
// lex_t.hpp: the lext::lex_t class
////////////////////////////////////////////////////////////////////////


#include <string>
#include <sstream>
#include <map>

/**
   The lext namespace houses a little class for
   lexically casting strings and other types 
   to and from each other.


   This software is released into the Public Domain by it's author,
   stephan beal (stephan@s11n.net).


Change history:

31 Dec 2004:
- Changed the impl of istream>>() again. Now uses a horrible
  hack but works across many more use cases.

28 Dec 2004:
- Changed the impl of istream>>() to use getc() instead of getline(),
  so newlines are preserved.


25 Nov 2004:
- Minor doc updates.
- Changed multiple-include guard to allow inclusion of this file twice
  for purposes of registering lex_t with s11n. Before, this header
  must have been included AFTER including s11n to pick up the
  registration. Now including this header after including s11n is safe
  if it has previously been included (and thus didn't pick up s11n
  registration).


2 Oct 2004:

- Accomodated recent changes in libs11n.


22 Aug 2004:

- Added ambiguity-buster overloads for operator==() for (const char *)
  and std::string.


20 Aug 2004:

- Added lex_t::empty()
- Moved lex_t::operator==() to a free function.
- Added lex_t::operator=(const char *) (see API notes for why).


17 Aug 2004:

- Initial release.
- After-relase:
- Added more docs.
- Added std::string and (const char *) overloads, to avoid some ambiguities.



16 Aug 2004:
- Zen Coding Session.

*/
namespace lext {

        /**
           The functions in the Private namespace should not be used
           by client code.
        */
        namespace Private
        {


                /**
                   Lexically casts str to a value_type, returning
                   errorVal if the conversion fails.

                   TODO: implement the following suggestion from  
                   Kai Unger <kai.unger@hacon.de> (21 Sept 2004):

                   When the cast is done, you should check if there
                   are unread characters left. For example, casting
                   "1.2this_definitly_is_not_a_number" to double will
                   not result in returning the error value, because
                   conversion of "1.2" to 1.2d succeeds and the rest
                   of the string is ignored.
                */
                template <typename value_type>
                value_type from_string( const std::string & str, const value_type & errorVal ) throw()
                {
                        std::istringstream is( str );
                        if ( !is )
                                return errorVal;
                        value_type foo = value_type();
                        if ( is >> foo )
                                return foo;
                        return errorVal;
                }

                /**
                   Returns a string representation of the given
                   object, which must be ostreamble.
                */
                template <typename value_type>
                std::string to_string( const value_type & obj ) throw()
                {
                        std::ostringstream os;
                        os << std::fixed;
                        os << obj;
                        return os.str();
                }

                /**
                   Convenience/efficiency overload.
                */
                inline std::string from_string( const std::string & str, const std::string & errorVal ) throw()
                {
                        return str;
                }

                /**
                   Convenience/efficiency overload.
                */
                inline std::string from_string( const char *str, const char *errorVal ) throw()
                {
                        return str;
                }

                /**
                   Convenience/efficiency overload.
                */
                inline std::string to_string( const char *obj ) throw()
                {
                        return obj;
                }

                /**
                   Convenience/efficiency overload.
                */
                inline std::string to_string( const std::string & obj ) throw()
                {
                        return obj;
                }


        } // end Private namespace



        /**
           lex_t provides a really convenient way to lexically cast
           strings and other streamable types to/from each other.

           All parameterized types used by this type must be:

           - i/o streamable. The operators must complement each other.

           - Assignable.

           - Default Constructable.

           This type is fairly light-weight, with only one std::string
           data member, so it should copy quickly and implicitely use
           std::string's CoW and reference counting features. Adding
           reference counting to this class would be of no benefit,
           and would probably hurt performance, considering that
           std::string's are optimized in these ways, and this type
           is simply a proxy for a std::string.

           For some uses the lex_t type can replace the requirement
           for returning a proxy type from a type's operator[](), as
           discussed in Scott Meyers' <em>More Effective C++</em>, Item
           30. This class originally was such a proxy, and then
           evolved into a generic solution for POD-based types, which
           inherently also covers most i/ostreamable types. It is less
           efficient than specialized proxies for, e.g. (char &), but
           it is also extremely easy to use, as shown here:


<pre>
        lext::lext_t lex = 17;
        int bogo = lex;
        ulong bogol = bogo * static_cast<long>(lex);
        lex = "bogus string";

        typedef std::map<lex_t,lex_t> LMap;
        LMap map;
        
        map[4] = "one";
        map["one"] = 4;
        map[123] = "eat this";
        map['x'] = "marks the spot";
        map["fred"] = 94.3 * static_cast<double>( map["one"] );
        map["fred"] = 10 * static_cast<double>( map["fred"] );
        map["123"] = "this was re-set";
        int myint = map["one"];

</pre>

        Finally, Perl-ish type flexibility in C++. :)

        It gets better: if we're using s11n, we can now save and load
        these objects at will:

<pre>
        s11nlite::save( map, "somefile.s11n" );
        ...

        LMap * map = s11nlite::load_serializable<LMap>( "somefile.s11n" );
</pre>

	If s11n is detected by this header it will automatically
        register a serialization proxy with s11n. See the
	end of this header file for the exact registration code.

        */
        struct lex_t
        {
                /**
                   Constructs an empty object. Calling <code>cast_to<T>()</code>
                   on an un-populated lex_t object will return T().
                */
                lex_t() throw(){}

                ~lex_t() throw() {}

                /**
                   Lexically casts f to a string.
                */
                template <typename FromT>
                explicit lex_t( const FromT & f ) throw()
                {
                        this->m_data = Private::to_string( f );
                }

                /**
                   Efficiency overload.
                */
                lex_t( const std::string & f ) throw() : m_data(f)
                {
                }
                /**
                   See operator=(const char *) for a note about why
                    this exists.
                 */
                lex_t( const char * str ) throw() : m_data(str?str:"")
                {
                }


                /**
                   Copies rhs's data to this object.
                */
                lex_t( const lex_t & rhs ) throw() : m_data(rhs.m_data)
                {
                }

                /**
                 Returns (this-&lt;str() &lt; rhs.str()).
                */
                inline bool operator<( const lex_t & rhs ) const
                {
                        return this->str() < rhs.str();
                }

                /**
                 Returns (this-&lt;str() > rhs.str()).
                */
                inline bool operator>( const lex_t & rhs ) const
                {
                        return this->str() > rhs.str();
                }


                /**
                   Copies rhs's data and returns this object.
                */
                inline lex_t & operator=( const lex_t & rhs ) throw()
                {
                        if( &rhs != this ) this->m_data = rhs.m_data;
                        return *this;
                }

                /**
                   This overload exists to keep the compiler/linker
                   from generating a new instantiation of this function
                   for each differently-lengthed (const char *)
                   which is assigned to a lex_t.
                */
                inline lex_t & operator=( const char * rhs ) throw()
                {
                        this->m_data = rhs ? rhs : "";
                        return *this;
                }


                /**
                   lexically casts str() to a ToType, returning
                   dflt if the cast fails.

                   When calling this function you may need to use the
                   following syntax to avoid compile errors:

                   Foo foo = lex.template cast_to<Foo>();

                   (It's weird, i know, and the first time i saw it,
                   finding the solution to took me days. (Thank you,
                   Nicolai Josuttis!))

                   However, in normal usage you won't need to use this
                   function, as the generic type conversion operator
                   does the exact same thing:

<pre>
lex_t lex = 17;
int foo = lex;
</pre>
                */
                template <typename ToType>
                ToType cast_to( const ToType & dflt = ToType() ) const throw()
                {
                        return Private::from_string( this->m_data, dflt );
                }


                /**
                   i used to LOVE C++... After writing this function i
                   WORSHIP C++. The grace with which C++ handles this
                   is pure magic, my friends.

                   16.8.2004 ----- stephan
                */
                template <typename ToType>
                inline operator ToType() const throw()
                {
                        return this->template cast_to<ToType>();
                }

                /**
                   Overload to avoid ambiguity in some cases.
                */
                inline operator std::string () const throw()
                {
                        return this->str();
                }
                /**
                   Returns the same as str().
                */
                operator std::string & () throw() { return  this->m_data; }

                /**
                   Returns the same as str().
                */
                operator const std::string & () const throw() { return  this->m_data; }


                /**
                   Overload to avoid ambiguity in some cases.
                   Useful for mixing C and C++ APIs:

<pre>
lex_t arg = "USER";
lex_t user = ::getenv(arg);
</pre>
                */
                inline operator const char * () const throw()
                {
                        return this->str().c_str();
                }



                /**
                   Sets this object's value and returns this object.
                */
                template <typename ToType>
                inline lex_t & operator=( const ToType & f ) throw()
                {
                        this->m_data = Private::to_string( f );
                        return *this;
                }

                /**
                   Returns a reference to this object's raw string
                   data.
                */
                inline std::string & str() throw() { return  this->m_data; }


                /**
                   Returns a const reference to this object's raw
                   string data.
                */
                inline const std::string & str() const throw() { return  this->m_data; }

                /**
                   Returns true if this object contains no data, else false.
                 */
                inline bool empty() const { return this->m_data.empty(); }


        private:
                std::string m_data;
        };

        /**
           Copies a.str() to os.
        */
        inline std::ostream & operator<<( std::ostream & os, const lex_t & a )
        {
                return os << a.str();
        }

        /**
           Reads from the input stream, appending to a.str() until the stream
           gives up. If the implementation of this function seems "wrong"
           to you, please read the justification in this paper:

           http://s11n.net/papers/lexically_casting.html

	   i believe there to be no satisfying, truly generic
	   implementation for this function :(.
        */
        inline std::istream & operator>>( std::istream & is, lex_t & a )
	{
		/****
		     WTF doesn't this do anything?!?!?!?!?

		     int c;
		     while( is.get(c).good() ) { 
		     	a.str() += c;
		     }
		****/
		/*****
  		while( std::getline( is, a.str() ).good() );
 		// ^^^ eeek! strips newlines!
		*****/
		/*****
  		is >> a.str(); // depends on skipws.
		*****/
		/*****
 		std::getline( is, a.str(), '\v' ); // UGLY, EVIL hack!
		      The \v char ("virtical tab") is an ugly hack: it is simply a char from
		      the ascii chart which never shows up in text. At least, i hope it
		      doesn't. AFIK, \v was historically used on old line printers and some
		      ancient terminals, but i've never seen it actually used. Unicode maps
		      0-255 to the same as ascii, so this shouldn't be a problem for
		      unicode.
		*****/
		// Many thanks to Marc Duerner:
		std::getline( is, a.str(), static_cast<std::istream::char_type>(std::istream::traits_type::eof()) );
                return is;
        }



        /**
           Casts lhs to a T object and returns true only if that
           object compares as equal to rhs.
        */
        template <typename T>
        inline bool operator==( const lex_t & lhs, const T & rhs )
        {
                return lhs.template cast_to<T>() == rhs;
        }

        /** Returns lhs.str() == rhs.str(). */
        inline bool operator==( const lex_t & lhs, const lex_t & rhs )
        {
                return lhs.str() == rhs.str();
        }

        /**
           Avoid an ambiguity...

           If rhs == 0 then this function returns true if
           lhs.empty().
        */
        inline bool operator==( const lex_t & lhs, const char * rhs )
        {
                if( ! rhs ) return lhs.empty();
                return lhs.str() == std::string(rhs);
        }

        /**
           Avoid an ambiguity...
        */
        inline bool operator==( const lex_t & lhs, const std::string & rhs )
        {
                return lhs.str() == rhs;
        }

        /* special: cast to bool means some value exist and is not zero/false */
        template<>
        inline bool lex_t::cast_to(const bool&) const throw()
        {
          return !this->m_data.empty() &&
            ( this->m_data.size()!=1 || this->m_data[0]!='0' );
                 
        }


} // namespace lext


#endif // s11n_LEX_T_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////
#ifdef s11n_S11N_INCLUDED // <--- that macro was added in s11n 0.9.7
#  ifndef s11n_LEX_T_REGISTERED_WITH_S11N
#    define s11n_LEX_T_REGISTERED_WITH_S11N 1
  // We have s11n! Let's use it!
  // Plug in a proxy for lex_t, to make it work like a Streamable...
#    include <s11n.net/s11n/data_node_functor.hpp>        // <-- s11n::streamable_type_serialization_proxy class
#    include <s11n.net/s11n/pods_streamable.hpp>          // <-- required for s11n 0.9.14+
#    define S11N_TYPE lext::lex_t                         // <-- type of object to be treated as a Serializable
#    define S11N_TYPE_NAME "lex_t"                        // <-- class name (for the classloader)
#    define S11N_SERIALIZE_FUNCTOR ::s11n::streamable_type_serialization_proxy
                                                          // ^^^^ our de/s11n implementation (proxy functor)
#    include <s11n.net/s11n/reg_serializable_traits.hpp>  // <-- register the above data with s11n
#  endif // s11n_LEX_T_REGISTERED_WITH_S11N
#endif // s11n_S11N_INCLUDED
////////////////////////////////////////////////////////////////////////

