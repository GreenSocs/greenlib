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

#ifdef USE_STATIC_CASTS
  #ifndef USE_PLB_TRANSACTION
    #error Define USE_PLB_TRANSACTION has to be set, when using the PLB Protocol (with static casts)
    #define __busProtocol_h__
  #endif
#endif

#ifndef __busProtocol_h__
#define __busProtocol_h__

#include <systemc.h>
#include "greenbus/transport/genericProtocol_if.h"
#include "greenbus/transport/genericRouter_if.h"
#include "greenbus/transport/genericScheduler_if.h"

#include "greenbus/utils/gs_trace.h"
//#include "greenbus/utils/phase_trace.h"

#include <map>


//Attention: Phase Trace does not work with OSCI SystemC 2.2 (segfault for unknown reasons)
//#define PHASE_TRACE

/**
 *
 */

using namespace tlm;
using namespace PLB;

class PLBBusProtocol
: public GenericProtocol_if, 
  public sc_module
{
public:

  //typedef tlm::unevenpair<GenericTransactionHandle,GenericPhase> GS_ATOM;
#ifdef USE_STATIC_CASTS
  typedef sc_export<payload_event_queue_if<tlm::unevenpair<tlm::shared_ptr<GenericTransaction>,tlm::GenericPhase > > > * sourcePort;
  typedef sc_export<payload_event_queue_if<tlm::unevenpair<tlm::shared_ptr<GenericTransaction>,tlm::GenericPhase > > > * destinationPort;
#else
  typedef sc_export<payload_event_queue_if<tlm::unevenpair<boost::shared_ptr<GenericTransaction>,tlm::GenericPhase > > > * sourcePort;
  typedef sc_export<payload_event_queue_if<tlm::unevenpair<boost::shared_ptr<GenericTransaction>,tlm::GenericPhase > > > * destinationPort;
#endif

  //sc_port<GenericRouter_if<initiator_multi_port<GenericTransaction,GenericRouterAccess,GenericPhase>, 
  //                         target_multi_port<GenericTransaction,GenericRouterAccess,GenericPhase> > > router_port;
  sc_port<GenericRouter_if> router_port;

  //sc_port<GenericScheduler_if<GenericTransaction, GenericPhase> > scheduler_port;
  sc_port<GenericScheduler_if> scheduler_port;

  SC_HAS_PROCESS(PLBBusProtocol);
  PLBBusProtocol(sc_module_name name, double time, sc_time_unit base);
  PLBBusProtocol(sc_module_name name, sc_time &t);
  ~PLBBusProtocol(){ 
#ifdef PHASE_TRACE
    if (p_trace)
      delete p_trace; 
#endif
  } //TODO check for NULL

  virtual bool registerMasterAccess(GS_ATOM& transaction);
  virtual bool processMasterAccess();
  virtual bool registerSlaveAccess(GS_ATOM& transaction);
  virtual bool processSlaveAccess();

  virtual void assignProcessMasterAccessSensitivity(sc_process_b* pMethod);
  virtual void assignProcessSlaveAccessSensitivity(sc_process_b* pMethod);

  virtual void end_of_elaboration();

  
private:
  void init();
  
  sc_event_queue startMasterAccessProcessing;
  sc_event startSlaveAccessProcessing;

  std::map<unsigned long, unsigned int> m_masterMap;

  //well... what do we need for PLB?
  //TRANS_STORAGE(m_queue); //a request queue, I guess
  sc_time m_clkPeriod; //a clock period of course!
  sc_time m_halfClkPeriod; //half of the clock period, to avoid run time division
  GS_ATOM m_waitingSecondaryWriteData; //a place where we can "park" secondary write data
  bool m_secoWriteDataValid; //a bool indicating that there is valid write data waiting
  GS_ATOM m_activeSecoRdReq; //a link to the active secondary request, to signal promotion to primary
  GS_ATOM m_activeSecoWrReq; //a link to the active secondary request, to signal promotion to primary
  GS_ATOM m_activePrimReq;   //a link to the active (un-acknowledged) 
  unsigned int m_activePrimWrMaster; //the ID of the primary writing master
  unsigned int m_activeSecoWrMaster; //the ID of the secondary writing master
  unsigned int m_activePrimRdMaster; //the ID of the primary writing master
  unsigned int m_activeSecoRdMaster; //the ID of the secondary writing master
  sc_time m_wrCompTime; //the time the last write was completed
  sc_time m_rdCompTime; //the time the last read  was completed
  bool m_secoRdAck; //a bool indicating whether the seq request was acked yet or not
  bool m_secoWrAck; //a bool indicating whether the seq request was acked yet or not
  unsigned int m_activePrimWrSlavePortRank; //the port rank of the recent targeted (write) slave (saves decoding time)
  unsigned int m_activePrimRdSlavePortRank; //the port rank of the recent targeted (read)  slave (saves decoding time)
  unsigned int m_activeSecoWrSlavePortRank; //the port rank of the recent targeted (write) slave (saves decoding time)
  unsigned int m_activeSecoRdSlavePortRank; //the port rank of the recent targeted (read)  slave (saves decoding time)
  sc_event m_TOEvent; //an event for timeouts
  bool m_addressStageBusy; //indicator that address stage is busy
  sc_time m_addressAckTimeWr; //time of last wr aAck
  sc_time m_addressAckTimeRd; //time of last rd aAck
  sc_time m_nextAllowedResponse;
  sc_event m_promotionEvent; //event that gets triggered, when a secondary request gets promoted to primary
  sc_time m_rdReqDeliverTime; //the time a request arrives at the slave
  sc_time m_wrReqDeliverTime; //the time a request arrives at the slave
  bool m_promoteWr; //a bool indicating whether there's a seco write to promote
  bool m_promoteRd; //a bool indicating whether there's a seco read  to promote
  volatile bool m_timedOut; //a semaphore
  GenericRouterAccessHandle m_timedOutTransaction;
  const GenericRouterAccessHandle m_constNULLTransactionP;


  void promote(bool rNw, int delay); //helper function

  void delayedPromotion(); //sc_method
#ifdef PHASE_TRACE
  phase_trace* p_trace;
#endif
  
};

#include "PLBBusProtocol.tpp"

#endif
