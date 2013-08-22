// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
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

#ifndef __simpleAddressMap_h__
#define __simpleAddressMap_h__

#include "systemc.h"

#ifndef USE_STATIC_CASTS
#include "greenbus/transport/generic.h"
#endif

#include "greenbus/core/tlm.h"
#include "greenbus/utils/gb_configurable.h"
#include <map>

using namespace tlm;

//class GenericRouter;

//--------------------------------------------------------------------------
/**
 * Simple address map implementation for the generic protocol.
 */
//--------------------------------------------------------------------------
template<class ROUTER>
class SimpleAddressMap 
{

  typedef sc_export<payload_event_queue_if<GS_ATOM> > * destinationPort;  
  typedef std::map<MAddr, unsigned int>::iterator addressMapIterator;
  
public:
  SimpleAddressMap() {}

  //--------------------------------------------------------------------------
  /**
   * Generate an address map from the connected slaves
   */
  //--------------------------------------------------------------------------
  void generateMap(GenericRouterInitiatorPort* port)
  {
    if(port == NULL) 
    {
      SC_REPORT_ERROR("Address Map", "Null pointer passed to GenerateMap().");
    }
    
    MAddr b,h;
#ifdef VERBOSE
    const char* target_name;
#endif
    
    GS_TRACE("SimpleAddressMap", "Generating address map:");
    GS_TRACE("SimpleAddressMap", "Number of connected slaves: %d", (int)( *port ).connected_in_ports.size());
    
    for (unsigned int i=0; i<(*port).connected_in_ports.size(); i++) {
      // get target_port
      destinationPort destination = (*port).connected_in_ports[i];

      // get address parameters
      gb_configurable *api = dynamic_cast<gb_configurable*>(destination->get_parent());
      sc_object *o = dynamic_cast<sc_object*>(destination->get_parent());
#ifdef VERBOSE
      if (o->get_parent()){
        target_name = o->get_parent()->name();
      }
      else
        target_name = o->name();
#endif
      
      ROUTER* r =dynamic_cast<ROUTER*>(o->get_parent());
      if (r){
        r->end_of_elaboration();
        b=r->m_addressMap.get_min();
        h=r->m_addressMap.get_max();
      }
      else {
        std::stringstream bs(api->getParam("base_addr"));
        std::stringstream hs(api->getParam("high_addr"));
        bs >> b;
        hs >> h;
      }
    
      GS_TRACE("SimpleAddressMap", "Target [%s] is connected to port number %d, Baseaddress=0x%x, Highaddress=0x%x", 
               target_name, i, (unsigned int)b, (unsigned int)h);

      insert(b,h,i);
    }
  }


  //--------------------------------------------------------------------------
  /**
   * Check for overlapping address ranges
   */
  //--------------------------------------------------------------------------
  void checkSanity() 
  {
    addressMapIterator pos;
    for (pos=m_addressMap.begin();pos!=m_addressMap.end();++pos){
      if(pos->second!=255) {
        dumpMap();
        SC_REPORT_ERROR("SimpleAddressMap","Overlapping Address Regions.");
      }
      else
        ++pos;
      if(pos->second==255) {
        dumpMap();
        SC_REPORT_ERROR("SimpleAddressMap","Overlapping Address Regions.");
      }
    }
    GS_TRACE("SimpleAddressMap", "Address sanity check successfull.");
  }


  //--------------------------------------------------------------------------
  /**
   * Print map 
   */
  //--------------------------------------------------------------------------
  void dumpMap()
  {
    std::cout<<"SimpleAddressMap: printing the sorted MAP:"<<std::endl<<std::flush;
    addressMapIterator pos;
    for (pos=m_addressMap.begin();pos!=m_addressMap.end();++pos){
      if(pos->second==255)
        printf("key: %x    value: %i \n", (unsigned int) ((pos->first+1)>>1)-1, pos->second);
      else
        printf("key: %x    value: %i \n", (unsigned int) (pos->first>>1)-1, pos->second);
    }
  }
  

  //--------------------------------------------------------------------------
  /**
   * Decode slave address.
   * @param address_ A slave address.
   * @return The decoded slave port number. On error, the value 255 is returned.
   */
  //--------------------------------------------------------------------------
  unsigned int decode(MAddr address_)
  {
    addressMapIterator lbound;
    
    if(address_>0xffffffff)
      SC_REPORT_ERROR("SimpleAddressMap", "Address must not exceed 32 bits in width.");
    
    lbound=m_addressMap.lower_bound((address_+1)<<1);
    if((lbound->second == 255) | (lbound==m_addressMap.end())){
      GS_TRACE("SimpleAddressMap", "Decode attempt for address 0x%x failed:\n", (unsigned int)address_);
#ifdef VERBOSE
      dumpMap();
#endif
      std::ostringstream err_str;
      err_str << "Address does not match any registered address range. " << address_;
      SC_REPORT_ERROR("SimpleAddressMap", err_str.str().c_str());
    }
    else{
      return lbound->second;
    }
    return 255;
  }
  
  const MAddr& get_max(){
    if (m_addressMap.size()){
      addressMapIterator i=(m_addressMap.end());
      i--;
      retval=(i->first>>1)-1;
      return retval;
    }
    else {
      SC_REPORT_ERROR("SimpleAddressMap", "get_max() called on empty address map.");
      return retval;
    }    
  }

  const MAddr& get_min(){
    if (m_addressMap.size()){
      addressMapIterator i=(m_addressMap.begin());
      retval=((i->first+1)>>1)-1;
      return retval;
    }
    else {
      SC_REPORT_ERROR("SimpleAddressMap", "get_min() called on empty address map.");
      return retval;
    }
  }


private:
  typedef std::map<MAddr, unsigned int> mapType;
  MAddr retval;

  //--------------------------------------------------------------------------
  /**
   * Insert a slave into the address map
   */
  //--------------------------------------------------------------------------
  void insert(MAddr baseAddress_, MAddr highAddress_, unsigned int portNumber_)
  {
    if(baseAddress_>highAddress_)
      SC_REPORT_ERROR("Address Map", "Base address must be lower than high address.");
    if((baseAddress_>0xffffffff) | (highAddress_>0xffffffff))
      SC_REPORT_ERROR("Address Map", "Addresses must not exceed 32 bits in width.");
    if(portNumber_>=255)
      SC_REPORT_ERROR("Address Map", "Only ;-) 255 targets can be handled.");
    m_addressMap.insert(mapType::value_type(((baseAddress_+1)<<1)-1,    255    ));
    m_addressMap.insert(mapType::value_type( (highAddress_+1)<<1   ,portNumber_));
  }

  /// the address map
  mapType m_addressMap;
};




#endif
