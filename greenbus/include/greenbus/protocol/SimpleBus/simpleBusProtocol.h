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

#ifndef __simpleBusProtocol_h__
#define __simpleBusProtocol_h__

// #define GAV_ENABLED // Enable GreenAV, to be set by the user globally

#ifdef GAV_ENABLED
#  include "greencontrol/gav/plugin/StatCalc.h"
#endif

#include <systemc.h>
#include "greenbus/core/tlm.h"

#ifndef USE_STATIC_CASTS
#include "greenbus/transport/generic.h"
#endif

#include "greenbus/transport/genericProtocol_if.h"
#include "greenbus/transport/genericScheduler_if.h"

using namespace tlm;

#ifdef GAV_ENABLED
/// Average calculator function to be added to the Calculator<double>
static double per_second(double transaction_count, double start_time_in_seconds, bool sloppy) {
  return (transaction_count / (sc_time_stamp().to_seconds() - start_time_in_seconds));
}
#endif


/**
 * This protocol provides different analysis abilities:
 *
 * Transactions per second:
 * - Transactions per second over all ports, EXTRAPOLATED to one second using
 *   the time between the first and the last transactions that were ever 
 *   recorded in this simulation. Updated on each transaction.
 * - DYNAMIC transactions per second over all ports, counting the actual
 *   transactions that were transmitted in the last second. Updated at the 
 *   end of each second (should be recorded e.g. by using an output plugin).
 * - Transactions per second for each in and out port (EXTRAPOLATED version)
 * - TODO: DYNAMIC transactions per second for each in/out port
 *
 * Transactions per second calculation:
 * EXTRAPOLATED:
 *  Start is the first transaction transmitted on that port
 *  (for overall transactions start is is 0 sec), 
 *  the last calculation update is performed after the last 
 *  transaction _on_that_port_!
 *  Accordingly the extrapolated transactions per seconds value calculates
 *  (for each port) between the first and the last transaction
 *  on that port.
 *
 * TODO: Transaction latency:
 * - Time between sending at master and delivering in slave. 
 *   Measured in the simple bus protocol implementation 
 *   in the router (where the transaction is delayed).
 */

//--------------------------------------------------------------------------
/**
 * A simple bus protocol implementation. 
 * The bus processes transactions consecutively. The arbitration
 * is clock-synchronous. Thus, incoming requests are always processed
 * with the next positive edge of the clock. Requests are
 * granted immediately when the bus is idle, else they will have to wait
 * until the bus is available again.
 */
//--------------------------------------------------------------------------

class SimpleBusProtocol 
: public GenericProtocol_if,
  public sc_module
{
public:
  typedef sc_export<payload_event_queue_if<GS_ATOM> > * SourcePort;
  typedef sc_export<payload_event_queue_if<GS_ATOM> > * DestinationPort;
  
  /// the port to the router 
  //sc_port<GenericRouter_if<INITPORT, TARGETPORT> > router_port;
  sc_port<GenericRouter_if> router_port;

  /// the port to the scheduler
  //sc_port<GenericScheduler_if<TRANSACTION, PHASE> > scheduler_port;
  sc_port<GenericScheduler_if> scheduler_port;

#ifdef GAV_ENABLED
  /// Typedef for map  port_no  ->   StatCalc
  typedef std::map<unsigned int, gs::av::StatCalc<double>*>   statCalc_map_type;
  /// Typedef for map  port_no  ->   counter param
  typedef std::map<unsigned int, gs::gs_param<unsigned int>*> counter_map_type;
  
  /// Calculator function calculating the transaction count between now and the last call to this function
  /**
   * To be added to the Calculator<unsigned int>
   * @param transaction_count  Current number of transactions.
   * @param protocol_inst_no   Caller instance of this function, to be able to assign the correct old value (0...x)
   * @return                   Difference between the new and the old transaction count values.
   */
  static unsigned int transaction_count_diff(unsigned int transaction_count, unsigned int protocol_inst_no, bool sloppy) {
    static std::vector<unsigned int> old_values;
    if (old_values.size() < protocol_inst_no+1) old_values.resize(protocol_inst_no+1);
    unsigned int diff = (transaction_count - old_values[protocol_inst_no]);
    old_values[protocol_inst_no] = transaction_count;
    return diff;
  }
  
#endif
  
  SC_HAS_PROCESS(SimpleBusProtocol);
  //--------------------------------------------------------------------------
  /**
   * Constructor.
   */
  //--------------------------------------------------------------------------
  SimpleBusProtocol(sc_module_name name_, sc_time clkPeriod) :
    sc_module(name_),
    router_port("router_port"),
    scheduler_port("scheduler_port"),
    m_clkPeriod(clkPeriod),
    m_idle(true)
#ifdef GAV_ENABLED
    ,tr_cnt("tr_cnt")
#endif
  {
    GS_TRACE(name(), "I am a simple bus protocol.");

    // DUST structure analysis
#ifdef USE_DUST
    DUST_PROTOCOL("SimpleBusProtocl");
#endif

#ifdef BUS_DEBUG
    time_bus_accessed = SC_ZERO_TIME;
    time_bus_used = SC_ZERO_TIME;
#endif

#ifdef GAV_ENABLED
    // Analysis

    // ** DYNAMIC hit rate Statistics Calculator over all ports
    // **  for actual counted transactions per second
    m_inst_no = getInstNumber(true) -1; // increment instance number

    // Add user defined calculation function to Calculator class
    gs::av::Calculator<unsigned int>::addFunc(&SimpleBusProtocol::transaction_count_diff,"private_transaction_count_diff"); 

    // Formula Calculator
    gs::av::Calculator<unsigned int> *dy_trpsec_calc
      = new gs::av::Calculator<unsigned int>("dynamic_tr_p_sec_overall");
    dy_trpsec_calc->calc("private_transaction_count_diff", tr_cnt, getInstNumber());
    // Trigger
    gs::av::Trigger *dy_tr = new gs::av::Trigger(1, SC_SEC);
    // StatCalc
    dy_overall_tr_per_sec = new gs::av::StatCalc<unsigned int>
         ("dynamic_overall_trans_per_sec_StC", dy_tr, dy_trpsec_calc);
    //cout << "transactions per second parameter name: " << overall_tr_per_sec->get_result_param()->getName() << endl;


    // ** Transactions per second Statistics Calculator over all ports
    // **  for extrapolated transactions per second

    // Add user defined calculation function to Calculator class
    gs::av::Calculator<double>::addFunc(&per_second,"per_second"); 
    
    gs::av::Calculator<double> *trpsec_calc
      = new gs::av::Calculator<double>("tr_p_sec_overall");
    trpsec_calc->enable_sloppy(); // allow division by zero
    trpsec_calc->calc("per_second", tr_cnt, sc_time_stamp().to_seconds()); // set formula: (tr_cnt 'per seconds')
    overall_tr_per_sec = new gs::av::StatCalc<double>("overall_trans_per_sec_StC", trpsec_calc); // create Statistics Calculator
    //cout << "transactions per second parameter name: " << overall_tr_per_sec->get_result_param()->getName() << endl;
    
#endif
  }

  ~SimpleBusProtocol() {
#ifdef GAV_ENABLED
    // delete dynamic overall Transactions per second StatCalc (and Trigger, Calculator)
    // do not delete the trigger before deleting the StatCalc!!
    delete dy_overall_tr_per_sec->get_calculator();
    gs::av::trigger_if* t = dy_overall_tr_per_sec->get_trigger();
    delete dy_overall_tr_per_sec;
    delete t;

    // delete overall Transactions per second StatCalc (and Calculator)
    delete overall_tr_per_sec->get_calculator();
    delete overall_tr_per_sec;
    
    // delete all counter objects of the source map
    for (counter_map_type::iterator 
          iter = source_transaction_counter_map.begin(); iter != source_transaction_counter_map.end(); iter++) {
      delete iter->second; // delete counter parameter
    }
    source_transaction_counter_map.clear();
    
    // delete all StatCalcs of the source map
    for (statCalc_map_type::iterator iter = source_tr_per_sec_StatCalc_map.begin(); iter != source_tr_per_sec_StatCalc_map.end(); iter++) {
      delete iter->second->get_calculator(); // delete StatCalc's Calculator
      delete iter->second; // delete StatCalc
    }
    source_tr_per_sec_StatCalc_map.clear();

    // delete all counter objects of the destination map
    for (counter_map_type::iterator 
         iter = destination_transaction_counter_map.begin(); iter != destination_transaction_counter_map.end(); iter++) {
      delete iter->second; // delete counter parameter
    }
    destination_transaction_counter_map.clear();
    
    // delete all StatCalcs of the destination map
    for (statCalc_map_type::iterator iter = destination_tr_per_sec_StatCalc_map.begin(); iter != destination_tr_per_sec_StatCalc_map.end(); iter++) {
      delete iter->second->get_calculator(); // delete StatCalc's Calculator
      delete iter->second; // delete StatCalc
    }
    destination_tr_per_sec_StatCalc_map.clear();
#endif
  }

  
#ifdef VERBOSE
  //--------------------------------------------------------------------------
  /**
   * Debug helper function.
   */
  //--------------------------------------------------------------------------
  inline const char *getModuleName(unsigned long id) {
    SourcePort port = router_port->getTargetPort()->connected_in_ports[m_masterMap[id]];
    return (dynamic_cast<sc_object*>(port->get_parent()->get_parent()))->name();
  }
#endif


  //--------------------------------------------------------------------------
  /**
   * A master accesses the channel
   */
  //--------------------------------------------------------------------------
  virtual bool registerMasterAccess(GS_ATOM& atom) 
  {
    GenericRouterAccessHandle ah = _getRouterAccessHandle(atom); 
    GenericPhase p = _getPhase(atom);

#ifdef GAV_ENABLED
    // overall counter
    tr_cnt++;

    // counter for the sending source port
    counter_map_type::iterator it = source_transaction_counter_map.find(m_masterMap[ah->getMID()]);
    if (it != source_transaction_counter_map.end()) {
      (*(it->second))++; // increment counter parameter
    }
    
    // create new StatCalc and counter for the source port
    else {
      // get Module Name (name of the initiator module of this transaction)
      SourcePort source =( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[ah->getMID()]];
      sc_object *obj = dynamic_cast<sc_object*>(source->get_parent()->get_parent());
      std::string sendnam;
      if (obj != NULL) sendnam = obj->name();
      else sendnam = "unknown";
      std::string parnam  = "in_port_tr_counter_" + sendnam;  // name for counter param
      std::string calcnam = "in_port_tr_p_sec_" + sendnam;       // name for calculator object
      std::string stcnam  = "in_port_tr_p_sec_StC_" + sendnam;   // name for StatCalc object
      // create cnt param
      gs::gs_param<unsigned int>* cnt = new gs::gs_param<unsigned int>(parnam);
      // add cnt param to map
      source_transaction_counter_map.insert(std::make_pair(m_masterMap[ah->getMID()], cnt));
      // create hit rate Statistics Calculator
      gs::av::Calculator<double> *trpsec_calc // Create Calculator
        = new gs::av::Calculator<double>(calcnam.c_str());
      trpsec_calc->enable_sloppy(); // allow division by zero
      trpsec_calc->calc("per_second", *cnt, sc_time_stamp().to_seconds()); // set formula: (tr_cnt 'per seconds')
      gs::av::StatCalc<double> *stc = new gs::av::StatCalc<double>(stcnam.c_str(), trpsec_calc); // create Statistics Calculator
      //cout << "transactions per second parameter name: " << stc->get_result_param()->getName() << endl;
      // add StatCalc to map
      source_tr_per_sec_StatCalc_map.insert(std::make_pair(m_masterMap[ah->getMID()], stc));
      //cout << "start transaction count statistics for " << stcnam << " at " << sc_time_stamp().to_seconds() << " sec" << endl;
      (*cnt)++;
    }
#endif
    
    if (p.isRequestValid()) {
      // this is a new request, so let's schedule it for execution
      scheduler_port->enqueue(atom);
      // process the request at the next posedge clk
      GS_TRACE(name(), "got RequestValid atom from master [%s] --> requesting its execution for the next posedge clk from the scheduler.", 
               getModuleName((unsigned)ah->getMID()))
      startMasterAccessProcessingEvent.notify(m_clkPeriod);
      return true;
    }

    else {
      // this is a master who is in the data handshake phase, so forward the data to the slave
      DestinationPort destination = ( *router_port->getInitPort() ).connected_in_ports[router_port->decodeAddress(ah->getMAddr())];
      GS_TRACE(name(), "got %s atom from master [%s]", p.toString().c_str(), getModuleName((unsigned)ah->getMID()));
      (*destination)->notify(atom);
      return true;
    }
  }

  //--------------------------------------------------------------------------
  /**
   * A master's request is to be processed now.
   */
  //--------------------------------------------------------------------------
  virtual bool processMasterAccess() 
  {
    // if bus is idle, check for new request
    if (m_idle && scheduler_port->isPending()) {

#ifdef BUS_DEBUG
      time_bus_accessed = sc_time_stamp();
#endif

      // get next pending request from scheduler...
      GS_ATOM atom = scheduler_port->dequeue();
      GenericRouterAccessHandle ah = _getRouterAccessHandle(atom);
      
      // ...and forward it to the slave
      DestinationPort destination = ( *router_port->getInitPort() ).connected_in_ports[router_port->decodeAddress(ah->getMAddr())];
      GS_TRACE(name(), "notified RequestValid from master [%s]", getModuleName((unsigned)ah->getMID()));      
      (*destination)->notify(atom);

#ifdef GAV_ENABLED
      // overall counter
      tr_cnt++;

      // counter for this destination port
      counter_map_type::iterator it = destination_transaction_counter_map.find(router_port->decodeAddress(ah->getMAddr()));
      if (it != destination_transaction_counter_map.end()) {
        (*(it->second))++; // increment counter parameter
      }
      
      // create new StatCalc and counter for this destination port
      else {
        // getModuleName (name of the target module of this transaction)
        sc_object *obj = dynamic_cast<sc_object*>(destination->get_parent()->get_parent());
        std::string sendnam;
        if (obj != NULL) sendnam = obj->name();
        else sendnam = "unknown";
        std::string parnam  = "out_port_tr_counter_" + sendnam;  // name for counter param
        std::string calcnam = "out_port_tr_p_sec_" + sendnam;       // name for calculator object
        std::string stcnam  = "out_port_tr_p_sec_StC_" + sendnam;   // name for StatCalc object
        // create cnt param
        gs::gs_param<unsigned int>* cnt = new gs::gs_param<unsigned int>(parnam);
        // add cnt param to map
        destination_transaction_counter_map.insert(std::make_pair(router_port->decodeAddress(ah->getMAddr()), cnt));
        // create hit rate Statistics Calculator
        gs::av::Calculator<double> *trpsec_calc // Create Calculator
          = new gs::av::Calculator<double>(calcnam.c_str());
        trpsec_calc->enable_sloppy(); // allow division by zero
        trpsec_calc->calc("per_second", *cnt, sc_time_stamp().to_seconds()); // set formula: (tr_cnt 'per seconds')
        gs::av::StatCalc<double> *stc = new gs::av::StatCalc<double>(stcnam.c_str(), trpsec_calc); // create Statistics Calculator
        //cout << "transactions per second parameter name: " << stc->get_result_param()->getName() << endl;
        // add StatCalc to map
        destination_tr_per_sec_StatCalc_map.insert(std::make_pair(router_port->decodeAddress(ah->getMAddr()), stc));
        //cout << "start transaction count statistics for " << stcnam << " at " << sc_time_stamp().to_seconds() << " sec" << endl;
        (*cnt)++;
      }
#endif
      
      m_idle =false;
      return true;
    }

    return false;
  }


  //--------------------------------------------------------------------------
  /**
   * A slave accesses the channel
   */
  //--------------------------------------------------------------------------
  virtual bool registerSlaveAccess(GS_ATOM& atom) 
  {
    GenericRouterAccessHandle ah = _getRouterAccessHandle(atom);
    GenericPhase p = _getPhase(atom);
    SourcePort source =( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[ah->getMID()]];
    (*source)->notify(atom); // immideately forward the ack
    
    switch(p.state) {
    case GenericPhase::DataAccepted:
      GS_TRACE(name(), "got DataAccepted atom from slave at address=0x%x", (unsigned)ah->getMAddr());
      m_idle =true; // transaction finished
      startMasterAccessProcessingEvent.notify(m_clkPeriod); // handle next request
      break;

    case GenericPhase::DataError:
      GS_TRACE(name(), "got DataError atom from slave at address=0x%x", (unsigned)ah->getMAddr());
      m_idle =true; // transaction finished
      startMasterAccessProcessingEvent.notify(m_clkPeriod); // handle next request
      break;
      
    case GenericPhase::ResponseValid:
      GS_TRACE(name(), "got ResponseValid atom from slave at address=0x%x", (unsigned)ah->getMAddr());
      m_idle =true;
      startMasterAccessProcessingEvent.notify(m_clkPeriod); // handle next request
      break;

    case GenericPhase::RequestError:
      GS_TRACE(name(), "got RequestError atom from slave at address=0x%x", (unsigned)ah->getMAddr());
      m_idle =true;
      startMasterAccessProcessingEvent.notify(m_clkPeriod); // handle next request
      break;

    default:
      GS_TRACE(name(), "got %s atom from slave at address=0x%x", p.toString().c_str(), (unsigned)ah->getMAddr());
    }

#ifdef BUS_DEBUG      
    if (m_idle) 
      time_bus_used += sc_time_stamp()-time_bus_accessed;
    //cout << "time_bus_used: " << time_bus_used << endl;
#endif
      
    return true;
  }


  //--------------------------------------------------------------------------
  /**
   * A slave's request should be processed now.
   */
  //--------------------------------------------------------------------------
  virtual bool processSlaveAccess() 
  {
    // this method can be used for implementing advanced protocol functions, 
    // e.g. timeout handling
    return true;
  }
  

  //--------------------------------------------------------------------------
  /**
   * Make the router's processMasterAccess function sensitive to our event.
   */
  //--------------------------------------------------------------------------
  void assignProcessMasterAccessSensitivity(sc_process_b* pMethod) 
  {
    GS_TRACE(name(), "assignProcessMasterAccessSensitivity triggered.");

    // register our startMasterAccessProcessing event_queue with the router's processMasterAccess callback
    sc_sensitive::make_static_sensitivity(pMethod, startMasterAccessProcessingEvent);
  }


  //--------------------------------------------------------------------------
  /**
   * Make the router's processSlaveAccess function sensitive to our event.
   */
  //--------------------------------------------------------------------------
  void assignProcessSlaveAccessSensitivity(sc_process_b* pMethod)
  {
    GS_TRACE(name(), "assignProcessMasterSlaveSensitivity triggered.");

    // register our startSlaveAccessProcessing event with the router's processMasterAccess callback
    sc_sensitive::make_static_sensitivity(pMethod, startSlaveAccessProcessingEvent);
  }

  

  //--------------------------------------------------------------------------
  /**
   * After end of elaboration, scan the init and target ports
   * and generate an master to port-num map.
   */
  //--------------------------------------------------------------------------
  void end_of_elaboration(){
    GS_TRACE(name(), "protocol configuration:");

    // create the master <=> port-num map
    for (unsigned int i=0; i<router_port->getTargetPort()->connected_in_ports.size(); i++) {
      unsigned long id = (unsigned long)(router_port->getTargetPort()->connected_in_ports[i]);
      // create (port pointer <=> master number) pairs
      m_masterMap[id] = i;
      GS_TRACE(name(), "At port slot %d there is the master %s with id=0x%lx", i, getModuleName(id), id);      
    }
    // announce the master map to the scheduler
    scheduler_port->setMasterMap(&m_masterMap);
  }


#ifdef BUS_DEBUG
  //--------------------------------------------------------------------------
  /**
   * Debug output at end of simulation
   */
  //--------------------------------------------------------------------------
  virtual void end_of_simulation() {
    cout << name() << ": Bus was in use " << time_bus_used << " time. This is a bus workload of " 
         << time_bus_used/sc_time_stamp()*100 << "%" << endl;
  }
#endif



private:
  sc_event_queue startMasterAccessProcessingEvent;
  sc_event startSlaveAccessProcessingEvent;

  
  /**
   * \brief master <=> port-num map.
   * This map is needed by the protocol to identify the master destination port of a slave's reply.
   */
  std::map<unsigned long, unsigned int> m_masterMap;

  /// clock period duration
  sc_time m_clkPeriod;

  /// bus state
  bool m_idle;

#ifdef BUS_DEBUG
  sc_time time_bus_accessed, time_bus_used;
#endif

#ifdef GAV_ENABLED

  /// Returns the number of this protocol instance.
  /**
   * To be used for the transactions per second calculator function.
   * @param increment  If to increment the intance number (default = false).
   * @return           Instance number (beginning at 1 within the first increment).
   */
  inline unsigned int getInstNumber(bool increment = false) {
    static unsigned int cnt = 0;
    if (increment) cnt++;
    return cnt;
  }
  
  /// Protocol instance number
  unsigned int m_inst_no;
  
  /// Analysis Statistics Calculator
  gs::av::StatCalc<double>* overall_tr_per_sec;
  /// Analysis Statistics Calculator for actual dynamic caluclation
  gs::av::StatCalc<unsigned int>* dy_overall_tr_per_sec;
  /// Parameter which counts the overall transactions
  gs::gs_param<unsigned int> tr_cnt;

  /// Map storing pointers to one StatCalc per destination port
  statCalc_map_type destination_tr_per_sec_StatCalc_map;
  /// Map storing pointers to one Counter per destination port
  counter_map_type destination_transaction_counter_map;
  /// Map storing pointers to one StatCalc per source port
  statCalc_map_type source_tr_per_sec_StatCalc_map;
  /// Map storing pointers to one Counter per source port
  counter_map_type source_transaction_counter_map;
#endif

};
 


#endif
