// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
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


#ifndef __PCIeAddressMap_h__
#define __PCIeAddressMap_h__


#include <systemc.h>

#include "greenbus/utils/gb_configurable.h"
#include <map>
#include <set>

namespace tlm {
namespace PCIe {


/// Enumeration for the outgoing port
enum OutgoingPort {
  OutgoingPortError = 0,
  PCIeDownstreamPort,      // downstream_port in PCIeRouter
  PCIeDownstreamBroadcast, // all downstream ports
  PCIeUpstreamPort,        // upstream_port in PCIeRouter
  LAST_PORT
};

/// Struct returned by the decode methods
/**
 * Holds the information if Upstream Port or if Downstream Port 
 * and which port of the downstream multi port.
 */
class RouterPort {
public:
  /// Outgoing (multi) port
  OutgoingPort  outgoing_port ;
  /// Outgoing port within multi port (if it is a multi port)
  unsigned int  connected_port;

  RouterPort ()
  : outgoing_port(OutgoingPortError), connected_port(0) { }
 
  RouterPort (OutgoingPort p, unsigned int cp) 
  : outgoing_port(p), connected_port(cp) { }
  
  std::string to_string() {
    std::stringstream ss;
    switch (outgoing_port) {
      case OutgoingPortError:
        ss << "Error/Base ";        break;
      case PCIeDownstreamPort: {
          ss << "Downstream ";
          ss << connected_port;
        } break;
      case PCIeDownstreamBroadcast:
        ss << "PCIeDownstreamBroadcast ";        break;
      case PCIeUpstreamPort:
        ss << "Upstream   ";        break;
      default:
        ss << "unknown    ";        break;
    }
    return ss.str();
  }
  
};


// ////////////////////////////////////////////////////// //
// ////////////////////////////////////////////////////// //
// ////////////////////////////////////////////////////// //

/// PCIe Memory Address, I/O Address, ID and Implicit routing map implementation for the PCIe router.
/**
 * Functions:
 * - Four address spaces: Memory addresses, I/O addresses, IDs, Iplicit.
 * - Address maps for each address space expect implicit:
 * - Generates three address maps: two address based maps and an ID based map
 *   - ID based map is generated during end_of_elaboration (fixed map)
 *   - Address based maps have to be generated at runtime when devices get their 
 *     address by PCIe TLP (transaction).
 *   - Propagation of addresses and IDs to other routers is done internally 
 *     with method calls. A Router calls in the upstream direction (and not 
 *     downstream). So a router always knows which addresses/IDs can be reached
 *     on which Downstream Port. All unknown addresses/IDs are routed upstream.
 * - returns outgoing port (out of address, ID or implicit)
 *
 * - routing:
 *   For address based and ID based routing: Look for address range or ID in the
 *   address maps of the Downstream Ports and return the port if existing.
 *   If address/ID does not match, return Upstream Port as target.
 *
 * - On changes in the address map, the add_address... method in the upstream
 *   switch has to be called. Then the uptream switch adds the new address range
 *   in its own address map to the concerning downstream ports (where the change was made).
 *   Afterwards the add method of its upstream switch has to be called etc...
 *
 */
template<class ROUTER>
class PCIeAddressMap 
 : public sc_object
{
public:
  typedef sc_export<payload_event_queue_if<PCIeTransactionContainer> > * destinationPCIePort;  
private:
  /// Address Map type
  typedef std::map<MAddr, RouterPort> addressMapType; // addr -> RouterPort
  typedef addressMapType::iterator addressMapIterator;
  /// ID Map type: map<bus number, downstream port>  bus_no -> Downstream Port
  typedef std::map<unsigned int, unsigned int> idMapType;
  typedef idMapType::iterator idMapIterator;
  typedef std::vector<unsigned int> busNoVecType;
  typedef std::set<unsigned int>    busNoSetType;

public:

  PCIeAddressMap(const char * name, PCIeRouterDevicePort* downstream_multi_port_, 
                                    PCIeBidirectionalPort* upstream_port_) 
   : sc_object(name),
     m_ret_UpstreamPort(PCIeUpstreamPort, 0), // default return port for address decode method
     downstream_multi_port(downstream_multi_port_),
     upstream_port(upstream_port_)
  {
    if(downstream_multi_port_ == NULL) 
      SC_REPORT_ERROR(this->name(), "Null pointer downstream_multi_port_ passed address map.");

    m_internal_bus_no = -1;
    m_ID_map_generated = false;
  }

  
  /// Called by the device's PCIeAPI with device number and called by other switches (without device number): Adds a new address range to the specified device
  /**
   * An End Point device has to call this method when it gets a new address range via a
   * Configuration Register writing transaction (TLP).
   *
   * Another switch located downstream calls this method mhen it has updated its own address map.
   * When called by another switch the device_no parameter is not specified.
   *
   * Updates the local address map
   * and forwards the update to the upstream switch.
   *
   * @param is_IO_map  If this address range is for the I/O Address map (true) or the Memory Address map (false).
   * @param base   Base address of the new added address range.
   * @param high   High address of the new added address range.
   * @param bus_no     Bus Number of the port which is connected to the device whose address is added.
   * @param device_no  (optional:) Device Number of the connected device. MUST be set by a device which is connected directly to the switch where the method is called.
   *
   * Not time critical.
   */
  void add_address_range(bool is_IO_map, MAddr base, MAddr high, 
                         unsigned int bus_no, unsigned int device_no = 32) {
    // store if the device was specified
    bool device_specified = true;
    if (device_no == 32) { 
      device_specified = false;
      device_no = 0;
    }

    // Check if device no is ok
    if (!device_specified && 
        m_downstream_bus_nos.find(bus_no) != m_downstream_bus_nos.end()) {
      //SC_REPORT_WARNING(name(), "When adding an address to a bus directly served by this switch, the device number should be specified when calling add_address_range!");
    }
    
    addressMapType *used_map;
    if (is_IO_map) { used_map = &m_IO_AddressMap; PCIeDEBUG("Add address range to IO Address map."); }
    else { used_map = &m_Memory_AddressMap; PCIeDEBUG("Add address range to Memory Address map."); }

    sc_assert(m_ID_map_generated);
    // decode the out port
    RouterPort p = decode_ID_based(bus_no, device_no);
    if (p.outgoing_port != PCIeDownstreamPort) {
      PCIeDEBUG2(name(), "add_address_range: Cannot find port assigned to the specified bus number %u.", bus_no);
      SC_REPORT_WARNING(name() , "add_address_range: Cannot find port assigned to the specified bus number.");
      // will be done later at end_of_elavoration in generate_address_map()
      return;
    }

    if (device_specified) {
      PCIeDEBUG2(name(), "Add address range 0x%llx, 0x%llx to local address map (bus %u, device %u = port index %u).", (long long unsigned int)base, (long long unsigned int)high, bus_no, device_no, p.connected_port);
    } else {
      PCIeDEBUG2(name(), "Add address range 0x%llx, 0x%llx to local address map (bus %u = port index %u).", (long long unsigned int)base, (long long unsigned int)high, bus_no, p.connected_port);
    }

    // Check if not yet in map
    bool already_in_map = false;;
    addressMapIterator iter;
    iter=used_map->find(high);
    if(iter!=used_map->end()){
      // if failure
      if (iter->second.outgoing_port != PCIeDownstreamPort || iter->second.connected_port != p.connected_port) {
        SC_REPORT_ERROR(name(), "This high address is already associated to another device!");
      }
      // if this range already is registered
      iter=used_map->find(base);
      if (iter->second.outgoing_port == OutgoingPortError) {
        already_in_map = true;
      }
      else {
        SC_REPORT_ERROR(name(), "Error while adding address range: base address entry already exists.");
      }
    }

    if (!already_in_map) {
      // Add the address range to the local address map
      insert_to_addr_map(is_IO_map, base, high, p.outgoing_port, p.connected_port);
      check_sanity(); // Check sanity each time because no central time point can be chosen for this check (not end_of_elab etc.)
#ifdef PCIeDEBUG_ON
      // only show if this is an add during simulation - otherwise the end_of_elaboration call dumps the map.
      if (sc_start_of_simulation_invoked())
        dump_map();
#endif
          
      // Add the same address range to the upstream switch
      sc_interface *inf         = ((*upstream_port).out.get_interface());
      sc_object    *destination = dynamic_cast<sc_object*>(inf);
      sc_object *o = dynamic_cast<sc_object*>(destination->get_parent());
      const char* target_name;
      if (o->get_parent()) target_name = o->get_parent()->name();
      else target_name = o->name();
      ROUTER* r =dynamic_cast<ROUTER*>(o->get_parent());
      if (r) {
        if ((void*)(&r->m_PCIeAddressMap) != (void*)this) { // avoid call of router itself (e.g. in a root complex which derives from PCIeRouter and owns a port which is connected to the Upstream Port)
          r->m_PCIeAddressMap.add_address_range(is_IO_map, base, high, bus_no);
          PCIeDEBUG2(name(), "Forward address range adding to Upstream Port's switch %s", target_name);
        } else PCIeDEBUG("Address range not forwarded to myself.");
      } else {
        PCIeDEBUG2(name(), "Device at Upstream Port (%s) is not a switch, no forwarding.", target_name);
      }
    } 
    else {
      PCIeDEBUG2(name(), "Address range already inserted.");
    }
  }

  /// Generates the ID map.
  /**
   * Generate m_idMap with switches (buses) located at Downstream Ports (not Upstream Port, automatically 
   * send to Upstream if bus number is not assigned to a Downstream Port).
   *
   * Calls generate_ID_map on all Downstream located switches to be able to call get_served_bus_numbers
   * on each of them.
   * Calls generate_ID_map on the Upstream located switch to make sure that all switches generate their
   * ID maps once one of these methods was called.
   *
   * Sets downstream_port_is_router vector: marks all ports where routers are connected to with true.
   * This allows the decode method to detect errors when a router port is addressed as target.
   *
   * Should be called by the router or by other address maps during end_of_elaboration.
   *
   * Not time critical.
   */
  void generate_ID_map()
  {
    if (!m_ID_map_generated) {
      m_ID_map_generated = true;

      // create bus numbers
      bool enable_internal_bus = true; // TODO: false if this is the map of a root complex!
      generate_bus_nos(enable_internal_bus);

      const char* target_name;
      
      PCIeDEBUG2(name(), "Generate ID map:");
      PCIeDEBUG2(name(), "Number of connected PCIe devices/switches at Downstream Port: %d", (int)( *downstream_multi_port ).connected_in_ports.size()-1);

      // /// generate ID map for Downstream PCIe ports connected to switches (not devices) /// //

      //downstream_port_is_router.resize((*downstream_multi_port).connected_in_ports.size());
      // start at port 1 (not 0) to ignore dummys
      for (unsigned int i=1; i<(*downstream_multi_port).connected_in_ports.size(); i++) {
        // get device_port
        destinationPCIePort destination = (*downstream_multi_port).connected_in_ports[i];

        sc_object *o = dynamic_cast<sc_object*> (destination->get_parent());
        if (o->get_parent()) {
          target_name = o->get_parent()->name();
        }
        else {
          target_name = o->name();
        }

        std::vector<unsigned int> vec;
        // if connected with PCIe switch (type PCIeRouter), read all served bus numbers of that router
        ROUTER* r =dynamic_cast<ROUTER*>(o->get_parent());
        if (r) {
          //downstream_port_is_router[i] = true;
          r->m_PCIeAddressMap.generate_ID_map(); // call eoe no avoid repeated call of generate_ID_map()
          r->m_PCIeAddressMap.collect_served_bus_numbers(vec);
        }
        //else downstream_port_is_router[i] = false;
        
        // add all numbers to map
        std::vector<unsigned int>::iterator iter;
        for( iter = vec.begin(); iter != vec.end(); ++iter ) {
          insert_to_ID_map(*iter, i);
        }

        PCIeDEBUG2(name(), "Target [%s] is connected to port number %d", target_name, i);
      }
      
      // /// call generate_ID_map on upstream port /// //
      sc_interface *inf         = ((*upstream_port).out.get_interface());
      sc_object    *destination = dynamic_cast<sc_object*>(inf);
      if (destination) {
        sc_object *o = dynamic_cast<sc_object*>(destination->get_parent());
        ROUTER* r =dynamic_cast<ROUTER*>(o->get_parent());
        if (r){
          if ((void*)(&r->m_PCIeAddressMap) != (void*)this) { // avoid call of router itself (e.g. in a root complex which derives from PCIeRouter and owns a port which is connected to the Upstream Port)
            PCIeDEBUG("call generate_ID_map on "<<o->get_parent()->name());
            r->m_PCIeAddressMap.generate_ID_map();
          } else PCIeDEBUG("call not forwarded to myself.");
        }
        else {
          PCIeDEBUG("Upstream is no router.");
        }
      } else {
        SC_REPORT_ERROR(name(), "Upstream not connected.");
      }
      PCIeDEBUG2(name(), "Generate ID map done");
    }
  }

  /// Generates both address maps.
  /**
   * Has to be called by the router during the end_of_elaboration only 
   * _after_ the ID map was generated!
   *
   * Only needed for propagation of manually set addresses. 
   * Normally the address map is generated dynamically during simulation with special TLP
   * transactions.
   *
   * This address map generation may be used by the user for debugging purpose:
   * When an address range was set manually with add_address_range_to_port before
   * end_of_elaboration the forwarding to other routers is done here.
   *
   * For each stored address entry the add_address_range_to_bus_no of the Upstream 
   * located switch is called.
   *
   * TODO Also generic device addresses may be set here.
   *
   * Not time critical.
   */
  void generate_address_map() {
    //SC_REPORT_WARNING(name(), "generate_address_map(): The call of this method is only needed if any addresses were set manually!");
    sc_assert(m_ID_map_generated);

    // generate IO Address map
    generate_address_map(true);
    // generate Memory Address map
    generate_address_map(false);
    
    PCIeDEBUG2(name(), "Generate address map done");
    
  }
private:
  /// Help function for generate_address_map()
  void generate_address_map(bool is_IO_map) {
    addressMapType *used_map;
    if (is_IO_map) { used_map = &m_IO_AddressMap; PCIeDEBUG("Generate IO Address map."); }
    else { used_map = &m_Memory_AddressMap; PCIeDEBUG("Generate Memory Address map."); }
    
    // Send all added addresses (which are manually set or set by generic devices) to upstream port.
    MAddr base, high;
    
    addressMapIterator pos;
    for (pos=used_map->begin();pos!=used_map->end();++pos){
      if (pos->second.outgoing_port != OutgoingPortError) {
        SC_REPORT_ERROR(name(), "Error while inserting new address range. Check for overlapping address regions.");
      }
      base = pos->first; //old: (pos->first+1)<<1;
      ++pos;
      if (pos->second.outgoing_port != PCIeDownstreamPort) {
        SC_REPORT_ERROR(name(), "Error while inserting new address range. Check for overlapping address regions.");
      }
      high = pos->first; //old: (pos->first+1)<<1;
      
      // Add the address range to the upstream switch
      sc_interface *inf         = ((*upstream_port).out.get_interface());
      sc_object    *destination = dynamic_cast<sc_object*>(inf);
      sc_object *o = dynamic_cast<sc_object*>(destination->get_parent());
      const char* target_name;
      if (o->get_parent()) target_name = o->get_parent()->name();
      else target_name = o->name();
      ROUTER* r =dynamic_cast<ROUTER*>(o->get_parent());
      if (r){
        if ((void*)(&r->m_PCIeAddressMap) != (void*)this) { // avoid call of router itself (e.g. in a root complex which derives from PCIeRouter and owns a port which is connected to the Upstream Port)
          PCIeDEBUG2(name(), "Forward address range adding to Upstream Port's switch %s", target_name);
          r->m_PCIeAddressMap.add_address_range(is_IO_map, base, high, m_internal_bus_no);
        } else PCIeDEBUG("Address range not forwarded to myself.");
      } else {
        PCIeDEBUG2(name(), "Device at Upstream Port (%s) is not a switch, no forwarding.", target_name);
      }
    }
  }

public:
  /// Adds the bus numbers which are served by this router to the parameter vector
  /**
   * Go through all downstream switches and collect 
   * their bus numbers and add the own one.
   *
   * @param served_busses Vector of bus numbers where the served ones are added.
   *
   * Not time critical.
   */
  void collect_served_bus_numbers(busNoVecType &served_busses) {

    // add internal Bus Number
    served_busses.push_back(m_internal_bus_no);

    // add own Bus Numbers
    served_busses.insert(served_busses.end(), m_downstream_bus_nos.begin(), m_downstream_bus_nos.end());
    
    // add bus numbers of all downstream switches
    for (unsigned int i=1; i<(*downstream_multi_port).connected_in_ports.size(); i++) {
      destinationPCIePort destination = (*downstream_multi_port).connected_in_ports[i];
      sc_object *o = dynamic_cast<sc_object*> (destination->get_parent());

      // if connected with PCIe switch (type PCIeRouter), read all served bus numbers of that router
      ROUTER* r =dynamic_cast<ROUTER*>(o->get_parent());
      if (r){
        r->m_PCIeAddressMap.generate_ID_map(); // call eoe no avoid repeated call of generate_ID_map()
        std::vector<unsigned int> downvec;
        r->m_PCIeAddressMap.collect_served_bus_numbers(downvec);
        served_busses.insert(served_busses.end(), downvec.begin(), downvec.end());
      }
    }
  }
  

  /// Print the maps.
  /**
   * Showing ID, address maps.
   *
   * Not time critical.
   */
  void dump_map()
  {
    
    std::vector<unsigned int> vec;
    collect_served_bus_numbers(vec);

    cout << "  Dump maps of module '" << name() << "':"<<endl;

    // ** Busses
    cout << "    My Bus Numbers = ";
    busNoSetType::iterator theIterator;
    for( theIterator = m_downstream_bus_nos.begin(); theIterator != m_downstream_bus_nos.end(); theIterator++ ) {
     cout << *theIterator << ", ";
    } cout << endl;
    cout << "    My internal Bus Number = "<< m_internal_bus_no<< endl;
    cout << "    Served busses:  ";
    busNoVecType::iterator iter;
    for( iter = vec.begin(); iter != vec.end(); ++iter ) {
      if (iter != vec.begin()) cout << ", ";
      cout << *iter; 
    }
    cout << endl;
    
    // ** ID
    cout << "    ID map:"<<endl;
      cout << "      Bus "<< m_internal_bus_no << " -> internal"<<endl;
    for( theIterator = m_downstream_bus_nos.begin(); theIterator != m_downstream_bus_nos.end(); theIterator++ ) {
      cout << "      Bus "<< *theIterator << " -> "<< decode_ID_based(*theIterator, 0).to_string() <<endl;
    }
    idMapIterator pos_id;
    for (pos_id=m_idMap.begin();pos_id!=m_idMap.end();++pos_id) {
      cout << "      Bus "<<pos_id->first<<" -> Downstream "<<pos_id->second<<endl;
    }

    // ** MEMORY ADDRESS
    cout << "    Memory Address map" << endl;
    cout << "      size = "<< m_Memory_AddressMap.size() << endl;
    addressMapIterator pos;
    for (pos=m_Memory_AddressMap.begin();pos!=m_Memory_AddressMap.end();++pos){
      if(pos->second.outgoing_port == OutgoingPortError) {
        printf("      key: 0x%llx", (long long unsigned int) (pos->first));
        cout <<"   value: "<< pos->second.to_string() << endl;
      } else {
        printf("      key: 0x%llx", (long long unsigned int) (pos->first));
        cout <<"   value: "<< pos->second.to_string() << endl;
      }
    }

    // ** I/O ADDRESS
    cout << "    I/O Address map" << endl;
    cout << "      size = "<< m_IO_AddressMap.size() << endl;
    for (pos=m_IO_AddressMap.begin();pos!=m_IO_AddressMap.end();++pos){
      if(pos->second.outgoing_port == OutgoingPortError) {
        printf("      key: 0x%llx", (long long unsigned int) (pos->first));
        cout <<"   value: "<< pos->second.to_string() << endl;
      } else {
        printf("      key: 0x%llx", (long long unsigned int) (pos->first));
        cout <<"   value: "<< pos->second.to_string() << endl;
      }
    }
  }
  

  /// Decode Memory Address to port for address based routing. Uses the Mermory Address map.
  /**
   * If the address in not in the map, Upstream Port is returned.
   *
   * @param address A slave memory address.
   * @return The decoded port.
   *
   * Time critical!!
   */
  RouterPort decode_Memory_Address(MAddr address) {
    addressMapIterator lbound;
    
    //if(address>0xffffffffffffffffLL)
    //  SC_REPORT_ERROR(name(), "Address must not exceed 64 bits in width.");
    
    // Searches for the high address entry (with the port as second value) which
    // is found by searching with lower_bound the next entry greater or equal the address.
    // If the lower bound itself is the target address that is handled in the if case.
    lbound=m_Memory_AddressMap.lower_bound(address);//old: lower_bound((address+1)<<1);
    if(lbound->second.outgoing_port == OutgoingPortError | lbound==m_Memory_AddressMap.end()){
      // if address = base address return next map entry
      if (address == lbound->first) {
        lbound=m_Memory_AddressMap.upper_bound(address); // catch the upper bound entry
      } else {
        // Address does not match any registered address range.
        PCIeDEBUG2(name(), "Decode attempt for memory address 0x%llx failed. Send Upstream.", (long long unsigned int)address);
        return m_ret_UpstreamPort;
      }
    }
    return lbound->second;
  }

  /// Decode I/O Address to port for address based routing. Uses the I/O Address map.
  /**
   * If the address in not in the map, Upstream Port is returned.
   *
   * @param address A slave I/O address.
   * @return The decoded port.
   *
   * Time critical!!
   */
  RouterPort decode_IO_Address(MAddr address) {
    addressMapIterator lbound;
    
    CHECK_RULES(if(address>0x00000000ffffffffLL) SC_REPORT_ERROR(name(), "I/O address must not exceed 32 bits in width."););
    
    // Searches for the high address entry (with the port as second value) which
    // is found by searching with lower_bound the next entry greater or equal the address.
    // If the lower bound itself is the target address that is handled in the if case.
    lbound=m_IO_AddressMap.lower_bound(address);//old: lower_bound((address+1)<<1);
    if(lbound->second.outgoing_port == OutgoingPortError | lbound==m_IO_AddressMap.end()){
      // if address = base address return next map entry
      if (address == lbound->first) {
        lbound=m_IO_AddressMap.upper_bound(address); // catch the upper bound entry
      } else {
        // Address does not match any registered address range.
        PCIeDEBUG2(name(), "Decode attempt for I/O address 0x%llx failed. Send Upstream.", (long long unsigned int)address);
        return m_ret_UpstreamPort;
      }
    }
    return lbound->second;
  }
  
  /// Decode ID to port for ID based routing.
  /**
   * If the address in not in map, Upstream Port is returned.
   *
   * This decoding can performed fixed to the port position in the multi port.
   * The values for the device number need not to be coordinated with other busses/switches
   * (but when a bus is managed by _more_ switches but one!)
   *
   * @param bus_no     target bus number (0...255)
   * @param device_no  target device number (0...31)
   * @return The decoded port number. On error, outgoing_port is OutgoingPortError.
   *
   * Little time critical! (only used in initialization)
   */
  RouterPort decode_ID_based(const unsigned int bus_no, const unsigned int device_no) {
    /*
      - Vorbedingungen: 
        - eine Bus Nr. pro Switch.
        - Switchs wissen, wenn andere Bus-Nummern am Downstream port hängen
          ( wie in Erfahrung bringen??? )
      
      Ausführen:
      - wenn busNo die Nr. dieses Switches -> an entsprechendes Device ausliefern.
        - Nummerierung: PCIe Downlink ports Durchnummerieren: Device 0 = Port position 1, Dev 1 = Port pos 2 etc.
                        Generic Downlink ports weiter Durchnummerieren: Device n+1 = Port position 1, Dev n+2 = Port pos 2 etc.
      - wenn busNo von einem Switch bedient wird, der an einen Downstream Port angeschlossen ist -> dorthin weiterleiten
        -  Die Switches müssen untereinander bekanntgeben, welche Bus Nummern an den jeweiligen Upstream Ports
           bedient werden. Das wird mit end_of_elaboration gemacht (siehe generate_ID_map())
      - sonst: an upstream_port weiterleiten
    */
    RouterPort p;
    // if target device is managed by this router (this bus no)
    if ( is_served_bus(bus_no) ) {
      p.outgoing_port = PCIeDownstreamPort;
      p.connected_port = bus_no - *m_downstream_bus_nos.begin() +1;
    }
    // if target device is managed by another switch
    else {
      // if target switch is connected to a Downstream Port
      idMapIterator iter = m_idMap.find(bus_no);
      if( iter != m_idMap.end() ) {
        p.outgoing_port = PCIeDownstreamPort;
        p.connected_port = m_idMap[bus_no];
      }
      // if target switch can be reached through Upstream Port 
      else {
        p.outgoing_port = PCIeUpstreamPort;
      }
    }
    return p;
  }
  
  /// Decode implicit routing to one port or all Downstream Ports (broadcast) for implicit routing.
  /**
   * @param  message_type Message type which specifies the routing target(s)
   * @return The decoded ports. On error or no forwarding, outgoing_port is OutgoingPortError.
   *
   * Time critical!!
   */
  RouterPort decode_implicit(MMessageType message_type) {
    /*
      je nach message type einen oder mehrere Ausgangsports in den Rückgabevektor schreiben
      (für broadcast z.B. für alle ports einen Eintrag im Vektor)
    */
    RouterPort p;
    switch ((unsigned int)message_type) {
      case RoutedToRootComplex: // 000
      {
        p.outgoing_port = PCIeUpstreamPort;
      } break;
      case BroadcastFromRootComplex:       // 011
      {
        // send to all Downstream Ports
        p.outgoing_port = PCIeDownstreamBroadcast;
      } break;
      case LocalTerminateAtReceiver:       // 100
      {
        SC_REPORT_INFO(name(), "TLP terminated at this switch, not forwarded!");
        p.outgoing_port = OutgoingPortError;
      } break;
      case GatheredAndRoutedToRootComplex: // 101
      { // Has to be handled separately in the router
        PCIeDEBUG("TLP terminated at this switch, forwarded later to Root Complex!");
        p.outgoing_port = OutgoingPortError;
      } break;
      case ReservedTerminateAtReceiver:    // 101
      {
        SC_REPORT_WARNING(name(), "Implicit routing: reserved message type. TLP terminated at this switch, not forwarded!");
        p.outgoing_port = OutgoingPortError;
      } break;
      default:
      {
        SC_REPORT_ERROR(name(), "Implicit routing: unknown or not allowed message type.");
        // RoutedByAddress not allowed: call decode_address_based instead
        // RoutedByID      not allowed: call decode_ID_based      instead
        p.outgoing_port = OutgoingPortError;
      }
    }
    return p;
    
  }
  
  /// Decodes a port number to a bus number
  unsigned int decode_port(unsigned int port) {
    return port + *m_downstream_bus_nos.begin() - 1;
  }

  /// Returns if the overgiven bus number matches the internal bus number of this PCIe switch
  const bool is_internal_switch_bus(int bus_no) {
    return (m_internal_bus_no == bus_no);
  }

  /// Returns if the overgiven bus number is served by this switch directly at one Downstream Port
  /**
   * Returns false in the case of the internal bus number. To check that use is_internal_switch_bus(no).
   */
  const bool is_served_bus(int bus_no) {
    return (m_downstream_bus_nos.find(bus_no) != m_downstream_bus_nos.end());
  }

  /// Resets the address map (I/O addresses and Memory addresses)
  void reset() {
    PCIeDEBUG("Reset address map");
    // reset IO Memory Address map
    m_IO_AddressMap.clear();
    // reset Memory Address map
    m_Memory_AddressMap.clear();
  }

protected:

  /// Sanity checks for both address maps etc.
  /**
   * - Check for overlapping address ranges
   * - Check for maximum allowed devices (32, 0...31) per bus
   * - Check for maximum allowed busses (256, 0...255) in the system
   * - ...
   */
  void check_sanity() 
  {
    // TODO: check if correct after change of stored addresses (not shifted and de/incremented)
    addressMapIterator pos;
    addressMapType *used_map;
    
    for (unsigned int i = 0; i<=1; i++) {
      if (i) { used_map = &m_Memory_AddressMap; /*PCIeDEBUG("Check Memory Address map.");*/ }
      else { used_map = &m_IO_AddressMap; /*PCIeDEBUG("Check IO Address map.");*/  }
      
      for (pos=used_map->begin();pos!=used_map->end();++pos){
        if(pos->second.outgoing_port!=OutgoingPortError)
          //SC_REPORT_WARNING(name(),"Overlapping Address Regions.");
          SC_REPORT_ERROR(name(),"Overlapping Address Regions.");
        else
          ++pos;
        if(pos->second.outgoing_port==OutgoingPortError)
          //SC_REPORT_WARNING(name(),"Overlapping Address Regions.");
          SC_REPORT_ERROR(name(),"Overlapping Address Regions.");
      }
    }
    PCIeDEBUG("Address sanity check (Memory and I/O) successfull.");
  }

  /// Insert an address range to one of the address maps.
  /**
   * @param is_IO_map  If this address range is for the I/O Address map (true) or the Memory Address map (false).
   * @param base   Base address of the new added address range.
   * @param high   High address of the new added address range.
   * @param mport  Outgoing port (Downstream / Upstream).
   * @param port_pos  Port index of the Downstream multi port.
   *
   * Not time critical.
   */
  void insert_to_addr_map(bool is_IO_map, MAddr baseAddress_, MAddr highAddress_, OutgoingPort mport, unsigned int port_pos)
  {
    if(baseAddress_>highAddress_)
      SC_REPORT_ERROR(name(), "Base address must be lower than high address.");
    if (is_IO_map) {
      if(baseAddress_>0x00000000ffffffffLL | highAddress_>0x00000000ffffffffLL)
        SC_REPORT_ERROR(name(), "IO Addresses must not exceed 32 bits in width.");
    }
    if(port_pos>=32)
      SC_REPORT_ERROR(name(), "PCIe can only handle 32 devices per bus. The internal bus only can address 32 Downstream Ports represented as PCI-to-PCI Bridges.");
    addressMapType *used_map;
    if (is_IO_map) { used_map = &m_IO_AddressMap; PCIeDEBUG("Generate IO Address map."); }
    else { used_map = &m_Memory_AddressMap; PCIeDEBUG("Generate Memory Address map."); }
    // old: does not allow usage of MSB
    //m_addressMap.insert(addressMapType::value_type(((baseAddress_+1)<<1)-1,  RouterPort(OutgoingPortError,0)  ));
    //m_addressMap.insert(addressMapType::value_type( (highAddress_+1)<<1   ,  RouterPort(mport, port_pos)      ));
    used_map->insert(addressMapType::value_type(baseAddress_,  RouterPort(OutgoingPortError,0)  ));
    used_map->insert(addressMapType::value_type(highAddress_,  RouterPort(mport, port_pos)      ));
  }

  /// Insert an ID to the ID map
  /**
   * Multiple adding is allowed!
   * A warning is thrown if the bus number is already existing but 
   * inserted with a different port_pos.
   *
   * Not time critical.
   */
  void insert_to_ID_map(const unsigned int bus_no, const unsigned int port_pos) {
    // check if existing
    idMapIterator iter = m_idMap.find(bus_no);
    if( iter != m_idMap.end() && iter->second != port_pos) {
      SC_REPORT_WARNING(name(), "insert_to_ID_map: Insert failed because of changed position!");
    }
    m_idMap.insert(  idMapType::value_type( bus_no, port_pos )  );
  }
  
  /// Generates the Bus Numbers of the devices connected to the Downstream Ports
  /**
   * Uses the static member variable m_max_bus_no to set the downstream numbers and the internal number
   *
   * Has to be called during end_of_elaboration after all ports have been connected
   *
   * @param enable_internal_bus  Whether this switch should get an internal bus number (false if root complex)
   * Not time critical.
   */
  void generate_bus_nos(bool enable_internal_bus) {
    if (m_downstream_bus_nos.empty() && m_internal_bus_no == -1) { // may be called only once!
      // set internal Bus Number
      if (enable_internal_bus) {
        m_internal_bus_no = ++m_max_bus_no;
        PCIeDEBUG2(name(), "generate_bus_no: Internal Bus Number of this switch is %d", m_internal_bus_no);
      }
      // set Downstream Bus Numbers
      std::ostringstream ss;
      for (unsigned int i=1; i<(*downstream_multi_port).connected_in_ports.size(); i++) {
        m_downstream_bus_nos.insert(++m_max_bus_no);
        ss << m_max_bus_no << ", ";
      }
      PCIeDEBUG2(name(), "generate_bus_no: Bus Numbers of this switch are %s", ss.str().c_str());
      if (m_max_bus_no >= 256) {
        SC_REPORT_ERROR(name(), "Too many PCIe switches! Only 256 busses are allowed in PCIe.");
      }
    }
    else {
      SC_REPORT_ERROR(name(), "Bus number is already assigned!");
    }
  }

protected:

  /// Set of Bus Numbers of the Downstream Ports of this switch. Port 1 belongs to the lowest number etc.
  busNoSetType m_downstream_bus_nos;

  /// Bus number of the internal PCI-to-PCI Bridge connection bus of this switch
  int m_internal_bus_no;

  /// Temporary return address
  MAddr m_tmp_ret_addr;

  /// Return port for Memory Address decoding in case of upstream
  const RouterPort m_ret_UpstreamPort;

  /// Static member variable to allow all PCIeAddressMaps to select an unique bus number; initialized to -1
  static int m_max_bus_no;

  /// If the ID map was already generated. (To be checked in generate_address_map)
  bool m_ID_map_generated;

  /// IO Memory Address map
  addressMapType m_IO_AddressMap;
  /// Memory Address map
  addressMapType m_Memory_AddressMap;

  /// the ID map for switches located at any Downstream Port (bus_no -> port) (not Upstream)
  idMapType m_idMap;

  /// Downstream multi Port of the Router (needed for generating/updating maps)
  PCIeRouterDevicePort* downstream_multi_port;
  /// Stores for each connected port if the target is a router
  //std::vector<bool>     downstream_port_is_router;

  /// Upstream Port of the Router (needed for generating/updating maps)
  PCIeBidirectionalPort* upstream_port;
  
};

/// Initialize max bus number to -1 (so first bus number starts with 0)
template<class T> int PCIeAddressMap<T>::m_max_bus_no = -1; 

} // end namespace PCIe
} // end namespace tlm




#endif
