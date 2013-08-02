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

#ifndef __dummy_protocol_h__
#define __dummy_protocol_h__

//------------------------------------------------------------------------------------
/**
* Dummy protocol class to be used if no protocol class is connected to the router.
 */
//------------------------------------------------------------------------------------
template<typename ROUTER>
class DummyProtocol 
  : public GenericProtocol_if, 
    public sc_module,
    public gb_configurable
{
public:
  typedef sc_export<payload_event_queue_if<GS_ATOM> > * SourcePort;
  typedef sc_export<payload_event_queue_if<GS_ATOM> > * DestinationPort;
  
  sc_port<GenericRouter_if> router_port;
  
  SC_HAS_PROCESS(DummyProtocol);
  
  gb_param<gs_uint32> delay_MasterAccess;
  gb_param<gs_uint32> delay_SlaveAccess;
  
  DummyProtocol(sc_module_name _name)
    : sc_module(_name),
      router_port("router_port")
  {
      GS_TRACE(name(), "I am a dummy protocol.");
        
      GB_PARAM(delay_MasterAccess, gs_uint32, 0); // if >0, forward master atoms with given delay (in ns)
      GB_PARAM(delay_SlaveAccess, gs_uint32, 0); // if >0, forward slave atoms with given delay (in ns)
  }
  
  // forward all master atoms directly to the slave
  virtual bool registerMasterAccess(GS_ATOM& tc)
  {
    GenericPhase p = tc.second;
    GenericRouterAccess &t = tc.first->getRouterAccess();
    DestinationPort destination = ( *router_port->getInitPort() ).connected_in_ports[router_port->decodeAddress(t.getMAddr())];
    if (delay_MasterAccess == 0)
      (*destination)->notify(tc);
    else 
      (*destination)->notify(tc, delay_MasterAccess, SC_NS);
      
    return true;
  }      
  
  // forward all slave atoms directly to the master
  virtual bool registerSlaveAccess(GS_ATOM& tc)
  {
    GenericRouterAccess &t = tc.first->getRouterAccess();
    GenericPhase p = tc.second;
    SourcePort source = ( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[t.getMID()]];
    if (delay_SlaveAccess == 0)
	    (*source)->notify(tc); 
    else
	    (*source)->notify(tc, delay_SlaveAccess, SC_NS); 
      
    return true;
  }      
  
  
  virtual bool processMasterAccess() {
    return true;
  }
  
  virtual bool processSlaveAccess() {
    return true;
  }
  
  void end_of_elaboration() {
    // create the master <=> port-num map
    for (unsigned int i=0; i<router_port->getTargetPort()->connected_in_ports.size(); i++) {
      unsigned long id = (unsigned long)(router_port->getTargetPort()->connected_in_ports[i]);
      // create (port pointer <=> master number) pairs
      m_masterMap[id] = i;
    }
  }
  

  
  
  void assignProcessMasterAccessSensitivity(sc_process_b* pMethod) {}
  void assignProcessSlaveAccessSensitivity(sc_process_b* pMethod) {}
  
protected:
  ROUTER *mRouter;
  std::map<unsigned long, unsigned int> m_masterMap; 
};



#endif // __dummy_protocol_h__
