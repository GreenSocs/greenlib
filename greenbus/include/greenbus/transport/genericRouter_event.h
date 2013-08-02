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

#ifndef __genericRouter_event_h__
#define __genericRouter_event_h__

using namespace tlm;

//--------------------------------------------------------------------------
/**
 * The GenericRouter. It owns a multi-initiator port 
 * and a multi-target port for connecting arbitrary numbers 
 * of master and slave modules. All specific router implementations
 * inherit from this class.
 */
//--------------------------------------------------------------------------

class GenericRouter : public sc_module, 
                      public GenericRouter_if,
                      public tlm_b_if<GenericTransactionHandle>
{
public:

  typedef GenericRouterInitiatorPort INITPORT;
  typedef GenericRouterTargetPort TARGETPORT;
  typedef SimpleAddressMap<GenericRouter> ADDRESSMAP;  
  
  /// target multiport
  TARGETPORT target_port;
  
  /// slave multiport
  INITPORT init_port;

  /// port to the protocol
  sc_port<GenericProtocol_if> protocol_port;

  /// slave address map 
  ADDRESSMAP m_addressMap;



  SC_HAS_PROCESS(GenericRouter);
  //--------------------------------------------------------------------------
  /**
   * Constructor; bind ports and register SC_METHODs with the kernel.
   */
  //--------------------------------------------------------------------------
  GenericRouter(sc_module_name name_)
    : sc_module(name_),
      target_port("tport"),
      init_port("iport"),
      protocol_port("protocol_port")
  {
    GS_TRACE(name(), "I am a generic router.");

    // DUST structure analysis
    #ifdef USE_DUST
    DUST_BUS("GenericRouter");
    #endif

    // we implement the tlm_b_if for PV bypassing
    target_port.bind_b_if(*this);
    
    SC_METHOD( MasterAccessMonitor );
    sensitive << target_port.default_event();
    dont_initialize();
    
    SC_METHOD( SlaveAccessMonitor );
    sensitive << init_port.default_event();
    dont_initialize();
    
    SC_METHOD( ProcessMasterAccess );
    p_PMAMethod = sc_get_last_created_process_handle();
    dont_initialize();
    
    SC_METHOD( ProcessSlaveAccess );
    p_PSAMethod = sc_get_last_created_process_handle();
    dont_initialize();
  }
  
  
  //--------------------------------------------------------------------------
  /**
   * This method starts whenever a master triggers a payload-event.
   */
  //--------------------------------------------------------------------------
  void MasterAccessMonitor()
  {
    // get payload (transaction)
    GS_ATOM t_p = target_port.get_payload();
    
    // forward payload to protocol (there it can be queued for later arbitration or whatever)
    protocol_port->registerMasterAccess(t_p);
  }
  

  //--------------------------------------------------------------------------
  /**
   * This method can be made sensitive by the bus protocol to control
   * the processing of transfers from master to slave by triggering an event.
   */
  //--------------------------------------------------------------------------
  void ProcessMasterAccess()
  {
    // tell protocol that it's time to process the master accesses
    // this could include arbitrate between accesses that were enqueued earlier with registerMasterAccess...
    protocol_port->processMasterAccess();
  }


  //--------------------------------------------------------------------------
  /**
   * This method starts whenever a slave triggers a payload-event.
   */
  //--------------------------------------------------------------------------
  void SlaveAccessMonitor()
  {  
    // get payload  
    GS_ATOM t_p = init_port.get_payload();
    
    // forward payload to protocol class (there it can be queued or parsed or whatever)
    protocol_port->registerSlaveAccess(t_p);    
  }


  //--------------------------------------------------------------------------
  /**
   * This method can be made sensitive by the protocol to control
   * the processing of transfers from slave to master by triggering an event.
   */
  //--------------------------------------------------------------------------
  void ProcessSlaveAccess()
  {
    // tell bus protocol that it's time to process the slave accesses
    // this could include arbitrate between accesses that were enqueued earlier with registerSlaveAccess...
    protocol_port->processSlaveAccess();
  }


  //--------------------------------------------------------------------------
  /** 
   * Provide access to the initiator port (used by bus protocol).
   */
  //--------------------------------------------------------------------------
  virtual INITPORT* getInitPort(){ return &init_port;}


  //--------------------------------------------------------------------------
  /** 
   * Provide access to the target port (used by bus protocol).
   */
  //--------------------------------------------------------------------------
  virtual TARGETPORT* getTargetPort(){ return &target_port;}


  //--------------------------------------------------------------------------
  /**
   * End of elab simulation kernel callback
   */
  //--------------------------------------------------------------------------
  virtual void end_of_elaboration()
  {
    GS_TRACE(name(), "end_of_elaboration called.");

    // tell the protocol to assign sensitivies to the "process" methods
    protocol_port->assignProcessMasterAccessSensitivity(p_PMAMethod);
    protocol_port->assignProcessSlaveAccessSensitivity(p_PSAMethod);

    // create the slave address map 
    GS_TRACE(name(), "Creating address map...");
    m_addressMap.generateMap(getInitPort());
    //m_addressMap.dumpMap();

    GS_TRACE(name(), "Checking address map sanity (overlapping address regions)...");
    m_addressMap.checkSanity();
  }


  //--------------------------------------------------------------------------
  /**
   * This is the PV TLM blocking if implementation
   */
  //--------------------------------------------------------------------------
  void b_transact(GenericTransactionHandle t)
  {
    // TODO: catch decodeAddress exceptions
    
    GS_TRACE(name(), "forwarding PV transaction from master id=0x%x directly to slave at address=0x%x",
             (unsigned int)t->getMID(), (unsigned)t->getMAddr());
    (*(init_port.connected_b_in_ports[decodeAddress(t->getMAddr())]))->b_transact(t);
  }


  //--------------------------------------------------------------------------
  /**
   * GenericRouter_if's decodeAddress function.
   */
  //--------------------------------------------------------------------------
  virtual unsigned int decodeAddress(MAddr addr) 
  {
    return m_addressMap.decode(addr);
  }





private:

  //--------------------------------------------------------------------------
  /**
   * Process handle to SC_METHOD processMasterAccess 
   * that will be triggered by the bus protocol class.
   */
  //--------------------------------------------------------------------------
  sc_process_b* p_PMAMethod;


  //--------------------------------------------------------------------------
  /**
   * Process handle to SC_METHOD processSlaveAccess 
   * that will be triggered by the bus protocol class.
   */
  //--------------------------------------------------------------------------
  sc_process_b* p_PSAMethod;



};

#endif
