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

/*
Copyright (c) 2006 : Technical University of Braunschweig, Germany

All rights reserved.

Authors: Robert Guenzel, Wolfgang Klingauf, TU Braunschweig, E.I.S.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Technical University of Braunschweig, Germany nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//TODO: rdAddrAckTime  && wrAddrAckTime (both get usually set together but when promoting requests only the one of 
//the same type as the promoted request gets set. In this way the wrPrim->NextwrReq und rdPrim->nextRdReq relations
//stay correct and wrPrim will not effect NextRdReq anymore and vice versa)

#ifndef __PLBBusProtocol_tpp__
#define __PLBBusProtocol_tpp__

inline void PLBBusProtocol::init() {
  // DUST structure analysis
#ifdef USE_DUST
  DUST_PROTOCOL("PLBBusProtocol");
#endif

#ifdef PHASE_TRACE
  p_trace=NULL;
#endif
  // private members
  m_activePrimWrMaster=0;
  m_activeSecoWrMaster=0;
  m_activePrimRdMaster=0;
  m_activeSecoRdMaster=0;
  m_activePrimWrSlavePortRank=255;
  m_activePrimRdSlavePortRank=255;
  m_activeSecoWrSlavePortRank=255;
  m_activeSecoRdSlavePortRank=255;
  m_addressStageBusy=false;
  m_secoRdAck=false;
  m_secoWrAck=false;
  m_secoWriteDataValid=false;
  m_promoteRd=false;
  m_promoteWr=false;
  m_timedOut=false;
  m_timedOutTransaction=m_constNULLTransactionP;

  SC_METHOD(delayedPromotion);
  sensitive<<m_promotionEvent;
  dont_initialize();

  //SC_REPORT_INFO(sc_module::name(), "I am a PLB protocol.");
}


inline PLBBusProtocol::PLBBusProtocol(sc_module_name name, sc_time &t)
: sc_module(name), router_port("routerPort"),
  m_clkPeriod(t),
  m_halfClkPeriod(m_clkPeriod/2),
  m_wrCompTime(0,SC_NS),
  m_rdCompTime(0,SC_NS),
  m_addressAckTimeWr(SC_ZERO_TIME-2*m_clkPeriod),
  m_addressAckTimeRd(SC_ZERO_TIME-2*m_clkPeriod),
  m_rdReqDeliverTime(0,SC_NS),
  m_wrReqDeliverTime(0,SC_NS),
  m_constNULLTransactionP()

{
  init();
}


inline PLBBusProtocol::PLBBusProtocol(sc_module_name name, double time, sc_time_unit base) 
: sc_module(name), router_port("routerPort"),
  m_clkPeriod(time, base),
  m_halfClkPeriod(m_clkPeriod/2),
  m_wrCompTime(0,SC_NS),
  m_rdCompTime(0,SC_NS),
  m_addressAckTimeWr(SC_ZERO_TIME-2*m_clkPeriod),
  m_addressAckTimeRd(SC_ZERO_TIME-2*m_clkPeriod),
  m_rdReqDeliverTime(0,SC_NS),
  m_wrReqDeliverTime(0,SC_NS),
  m_constNULLTransactionP()
{
  init();
}

#ifdef VERBOSE_PLB
#define DUMPPLB(msg) cout<<sc_time_stamp()<<" "<<name()<<": "<<msg<<endl<<flush
#else
#define DUMPPLB(msg)
#endif

inline bool PLBBusProtocol::registerMasterAccess(GS_ATOM &atom){
  GenericPhase p = _getPhase(atom);
  GenericRouterAccess &t = *_getRouterAccessHandle(atom);

#ifdef PHASE_TRACE
   p_trace->iSetTrace(atom);
#endif

  if(p.state==GenericPhase::RequestValid){
    //Init Atoms get enqueued and will be processed at the next clock edge
    scheduler_port->enqueue(atom);
    startMasterAccessProcessing.notify(m_clkPeriod);
    return true;
  }
  else 
    if(p.state==GenericPhase::DataValid){
      if (t.getMID()==(MID)m_activePrimWrMaster){
        //prim Write Data can be directly forwarded to the target
        if (t.getSError()!=GenericError::Generic_Error_TimeOut){
          destinationPort destination= ( *router_port->getInitPort() ).connected_in_ports[router_port->decodeAddress(t.getMAddr())];
          if(sc_time_stamp()==m_wrCompTime) {
            (*destination)->notify(atom,m_clkPeriod);
            DUMPPLB("DATA Valid in One");
#ifdef PHASE_TRACE
            p_trace->tSetTrace(atom, m_clkPeriod);
#endif
          }
          else{
            (*destination)->notify(atom);
            DUMPPLB("DATA Valid Now");
#ifdef PHASE_TRACE
            p_trace->tSetTrace(atom);
#endif
          }
        }
      }
      else{
        //secondary Write Data gets stored in the "parking place"
        m_waitingSecondaryWriteData=atom;
        m_secoWriteDataValid=true;
      }
    }
    else{
      if(p.state==GenericPhase::ResponseAccepted){
        //SC_REPORT_WARNING(name(), "Response Accept is not supported in PLB. Will just be forwarded.");
        destinationPort destination= ( *router_port->getInitPort() ).connected_in_ports[router_port->decodeAddress(t.getMAddr())];
        (*destination)->notify(atom);        
      }
      else{
        GS_TRACE(name(), "Got phase=%d",p.state);
        SC_REPORT_ERROR(name(),"Masters may only put Init (RequestValid) or Data (DataValid) atoms.");
      }
    }
  return false;
}

inline bool PLBBusProtocol::processMasterAccess(){
  //since only init atoms will be processed by this method, we'll quit if address stage is busy
  //cout<<sc_time_stamp()<<" proc mst"<<endl<<flush;
  if(m_addressStageBusy)
    return false;

  destinationPort destination;
  if(scheduler_port->isPending()){
    GS_ATOM pt=scheduler_port->dequeue(false);
	  GenericRouterAccess &t = *_getRouterAccessHandle(pt);

    GenericPhase p =pt.second;
    //this init atom arrived at least one clock cycle before and so we may "see" it
    //and since it's the first one we are allowed to "see" and our queue orderes requests by priority
    //it is the init atom with the highest priority available now. So let's deal with it:
    m_addressStageBusy=true;
    if(t.getMCmd()==Generic_MCMD_WR){
      if(sc_time_stamp()<m_addressAckTimeWr+2*m_clkPeriod){ //is the address stage ready for a new address cycle?
        startMasterAccessProcessing.notify((m_addressAckTimeWr+2*m_clkPeriod)-sc_time_stamp());
        m_addressStageBusy=false;
        return false;  //it has not returned to idle after last ack so we bail out
      }
      if(m_activePrimWrMaster==0){
        //no primary write in progress, so this write req becomes the primary one
        m_activePrimWrMaster=t.getMID();
        m_activePrimWrSlavePortRank=router_port->decodeAddress(t.getMAddr());
        destination= ( *router_port->getInitPort() ).connected_in_ports[m_activePrimWrSlavePortRank];
        m_activePrimReq=pt;
        m_TOEvent.notify(16*m_clkPeriod+m_halfClkPeriod);
        scheduler_port->dequeue(true);
        DUMPPLB("PA Valid WR in One");
      }
      else if(m_activeSecoWrMaster==0){
        //there is no sec in progress so we can make this request a secondary one

        /*the block below does the following:
          Imagine there are two pending requests (all writes) the first one gets onto the bus as primary.
          The second one gets onto the bus as secondary
          Now the first one is very long (so the last address ack [the secondary one] is a long time in the past).
          Due to this the address stage would be able to process an address cycle (m_addressAckTime older than 2 clock cylces)
          and it is not busy (as both requests completed their address cycles).
          So as soon as the first request finsihes (wrComp signal) the second one gets primary and there is new room for a secondary
          request. If now a third request is issued at that time, It would get onto the bus (because old address ack and address stage is idle).
          But this is just not correct, as the "real" arbiter will get active only 2 cycles after the wrComp (and not immediately).
          So the block checks if the wrComp is old enough. If it fails we schedule a new address attempt at the correct time and bail out.
        */
        if (sc_time_stamp()<m_wrCompTime+2*m_clkPeriod){ 
          startMasterAccessProcessing.notify((m_wrCompTime+2*m_clkPeriod)-sc_time_stamp());
          m_addressStageBusy=false;
          return false;
        }

        pt.second.state=PLBPhase::SecondaryRequest;
        m_activeSecoWrMaster=t.getMID();
        m_activeSecoWrSlavePortRank=router_port->decodeAddress(t.getMAddr());
        destination= ( *router_port->getInitPort() ).connected_in_ports[m_activeSecoWrSlavePortRank];
        m_activeSecoWrReq=pt;
        scheduler_port->dequeue(true);          
        DUMPPLB("SA Valid WR in One");
      }
      else {
        //neither primary nor secondary stage is free so we must admit that we failed...
        m_addressStageBusy=false;
        return false;
      }
      //and finally we deliver the request after 1 clock cycle
      (*destination)->notify(pt, m_clkPeriod);
#ifdef PHASE_TRACE
      p_trace->tSetTrace(pt, m_clkPeriod);
#endif      
      m_wrReqDeliverTime=sc_time_stamp()+m_clkPeriod;
      m_secoWrAck=false;
      return true;
    }


    if(t.getMCmd()==Generic_MCMD_RD){
      if(sc_time_stamp()<m_addressAckTimeRd+2*m_clkPeriod){ //is the address stage ready for a new address cycle?
        startMasterAccessProcessing.notify((m_addressAckTimeRd+2*m_clkPeriod)-sc_time_stamp());
        m_addressStageBusy=false;
        return false;  //it has not returned to idle after last ack so we bail out    
      }
      if(m_activePrimRdMaster==0){
        //no primary read in progress, so this read req becomes the primary one
        m_activePrimRdMaster=t.getMID();
        m_activePrimRdSlavePortRank=router_port->decodeAddress(t.getMAddr());
        destination= ( *router_port->getInitPort() ).connected_in_ports[m_activePrimRdSlavePortRank];
        m_activePrimReq=pt;
        m_TOEvent.notify(16*m_clkPeriod+m_halfClkPeriod);
        scheduler_port->dequeue(true);
        DUMPPLB("PA Valid RD in One");
      }
      else if (m_activeSecoRdMaster==0){
        if (sc_time_stamp()<m_rdCompTime+2*m_clkPeriod){ 
          startMasterAccessProcessing.notify((m_rdCompTime+2*m_clkPeriod)-sc_time_stamp());
          m_addressStageBusy=false;
          return false;
        }        
        //there is a primary request active, so this request is secondary
        pt.second.state=PLBPhase::SecondaryRequest;
        m_activeSecoRdMaster=t.getMID();
        m_activeSecoRdSlavePortRank=router_port->decodeAddress(t.getMAddr());
        destination= ( *router_port->getInitPort() ).connected_in_ports[m_activeSecoRdSlavePortRank];
        m_activeSecoRdReq=pt;
        scheduler_port->dequeue(true);
        DUMPPLB("SA Valid RD in One");
      }
      else{
        //neither primary nor secondary stage is free so we must admit that we failed...
        m_addressStageBusy=false;
        return false;
      }
      //and finally we deliver the request after 1 clock cycle
      (*destination)->notify(pt, m_clkPeriod);
#ifdef PHASE_TRACE
      p_trace->tSetTrace(pt, m_clkPeriod);
#endif      
      m_rdReqDeliverTime=sc_time_stamp()+m_clkPeriod;
      m_secoRdAck=false;
      return true;
    }

    SC_REPORT_ERROR(name(), "Unknown command detected in process master access.");
    return false;
  }
  return false;
}


inline bool PLBBusProtocol::registerSlaveAccess(GS_ATOM& atom){
  //get access to transaction
  GenericRouterAccess &t = *_getRouterAccessHandle(atom);
  GenericPhase p = _getPhase(atom);
  
  sourcePort source;
  if(p.state==GenericPhase::RequestAccepted){
    DUMPPLB("Addr Ack Now");
    if(m_timedOutTransaction==_getRouterAccessHandle(atom)){
      cout<<"ignoring reply"<<endl<<flush;
      m_timedOutTransaction=m_constNULLTransactionP;
      return false; //if a slave answers to an already timed out request we ignore it
    }
    else
      m_TOEvent.cancel();
    if(!m_timedOut)
      startMasterAccessProcessing.notify(2*m_clkPeriod);
    if(!m_addressStageBusy)
      SC_REPORT_ERROR(name(),"Slave accepted a request, without getting a request.");
#ifdef PHASE_TRACE
      p_trace->tSetTrace(atom);
      p_trace->iSetTrace(atom);
      if (m_timedOut){
        p_trace->tResetTrace(atom, m_halfClkPeriod);
        p_trace->iResetTrace(atom, m_halfClkPeriod);
      }
      else {
        p_trace->tResetTrace(atom, m_clkPeriod);
        p_trace->iResetTrace(atom, m_clkPeriod);
      }
#endif               
    if(t.getMCmd()==Generic_MCMD_WR){
      if(m_activeSecoWrMaster!=0){
        if(m_wrCompTime==sc_time_stamp()){
          //ack and wrComp coincide, so we also signal wrPrim
          m_promotionEvent.cancel(); //disable this one, because the completion of the prim write already scheduled it
          m_promoteWr=false;
          m_activeSecoWrReq.second.state=PLBPhase::WrPrim;
          (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoWrSlavePortRank]))->notify(m_activeSecoWrReq);
          DUMPPLB("WR Prim");
#ifdef PHASE_TRACE
        p_trace->tSetTrace(m_activeSecoWrReq);
        p_trace->tResetTrace(m_activeSecoWrReq, m_clkPeriod);
#endif               
          promote(false,1);
        }
        m_secoWrAck=true;
      }
      source= ( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[t.getMID()]];
      m_addressStageBusy=false;
      if(m_timedOut){ //set both
        m_addressAckTimeWr=sc_time_stamp()-m_halfClkPeriod;
        m_addressAckTimeRd=sc_time_stamp()-m_halfClkPeriod;
      }
      else{
        m_addressAckTimeRd=sc_time_stamp();      
        m_addressAckTimeWr=sc_time_stamp();
      }
      (*source)->notify(atom); //immideately forward the ack
#ifdef PHASE_TRACE
        if (m_timedOut){
          if (m_secoWrAck)
            p_trace->tResetTrace(atom, m_halfClkPeriod, PLBPhase::SecondaryRequest);
          else
            p_trace->tResetTrace(atom, m_halfClkPeriod, GenericPhase::RequestValid);
          p_trace->iResetTrace(atom, m_halfClkPeriod, GenericPhase::RequestValid);
        }
        else {
          if (m_secoWrAck)
            p_trace->tResetTrace(atom, m_clkPeriod, PLBPhase::SecondaryRequest);
          else
            p_trace->tResetTrace(atom, m_clkPeriod, GenericPhase::RequestValid);
          p_trace->iResetTrace(atom, m_clkPeriod, GenericPhase::RequestValid);
        }
#endif                       
      return true;
    }
    if(t.getMCmd()==Generic_MCMD_RD){
      if(m_activeSecoRdMaster!=0){
        m_secoRdAck=true;
      }
      else
        m_nextAllowedResponse=sc_time_stamp()+2*m_clkPeriod; //if prim ack we know that the response may start two cycles after POSEDGE aAck
      source= ( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[t.getMID()]];
      m_addressStageBusy=false;
      if(m_timedOut){
        m_addressAckTimeWr=sc_time_stamp()-m_halfClkPeriod;
        m_addressAckTimeRd=sc_time_stamp()-m_halfClkPeriod;
      }
      else{
        m_addressAckTimeRd=sc_time_stamp();      
        m_addressAckTimeWr=sc_time_stamp();
      }
      (*source)->notify(atom); //immideately forward the ack
#ifdef PHASE_TRACE
        if (m_timedOut){
          if (m_secoRdAck)
            p_trace->tResetTrace(atom, m_halfClkPeriod, PLBPhase::SecondaryRequest);
          else
            p_trace->tResetTrace(atom, m_halfClkPeriod, GenericPhase::RequestValid);
          p_trace->iResetTrace(atom, m_halfClkPeriod, GenericPhase::RequestValid);
        }
        else {
          if (m_secoRdAck)
            p_trace->tResetTrace(atom, m_clkPeriod, PLBPhase::SecondaryRequest);
          else
            p_trace->tResetTrace(atom, m_clkPeriod, GenericPhase::RequestValid);
          p_trace->iResetTrace(atom, m_clkPeriod, GenericPhase::RequestValid);
        }
#endif                       
      return true;
    }
    SC_REPORT_ERROR(name(), "Transaction carries unknown command.");
    return false;
  }
  if(p.state==GenericPhase::DataAccepted | p.state==GenericPhase::DataError){
    DUMPPLB("DATA Ack in One");
    source= ( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[t.getMID()]];
    (*source)->notify(atom); //immideately forward the ack
    
    // HACK Warning (support multiple data phases)
    if (t.getMBurstLength() > t.getMSBytesValid()) {
      return true;
    }
    
    
#ifdef PHASE_TRACE
    if (p.state==GenericPhase::DataAccepted){
      p_trace->tSetTrace(atom);
      p_trace->tResetTrace(atom, m_clkPeriod);
    }
    p_trace->iSetTrace(atom);
    p_trace->iResetTrace(atom, m_clkPeriod);
    p_trace->iResetTrace(atom, m_clkPeriod, GenericPhase::DataValid);
    p_trace->tResetTrace(atom, m_clkPeriod, GenericPhase::DataValid);    
#endif    
    m_wrCompTime=sc_time_stamp();
    if(m_activeSecoWrMaster!=0){ //there's a secondary req active
      if(m_secoWrAck){ //the secondary one was acked already
        //we signal wrPrim immediately in this case
        m_activeSecoWrReq.second.state=PLBPhase::WrPrim;
        (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoWrSlavePortRank]))->notify(m_activeSecoWrReq);
        DUMPPLB("Wr Prim");
#ifdef PHASE_TRACE
        p_trace->tSetTrace(m_activeSecoWrReq);
        p_trace->tResetTrace(m_activeSecoWrReq, m_clkPeriod);
#endif            
      }
      else {
        if(sc_time_stamp()<m_wrReqDeliverTime){ //if req is not delivered yet, we simply make it primary (this can only be one clock cycle before delivery)
          m_TOEvent.notify(15*m_clkPeriod+m_halfClkPeriod+m_wrReqDeliverTime-sc_time_stamp());
          m_activeSecoWrReq.second.state=GenericPhase::RequestValid;
          //dangerous! two requests (sec then prim) for one xfer...
          (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoWrSlavePortRank]))->notify(m_activeSecoWrReq, m_clkPeriod);
          DUMPPLB("PA Valid WR (promoted)");
#ifdef PHASE_TRACE
          p_trace->tSetTrace(m_activeSecoWrReq, m_clkPeriod);
          p_trace->tResetTrace(m_activeSecoWrReq, m_clkPeriod, PLBPhase::SecondaryRequest);
#endif            
          m_activePrimReq=m_activeSecoWrReq;          
        }
        else{ //if it's delivered but not acked yet, it will become primary one clock cycle later
          m_promotionEvent.notify(m_clkPeriod);
          m_promoteWr=true;
          return true;  //prevent req promotion
        }
      }
    }
    promote(false,1);  //promotion when no sec request is active leads to deletion of prim
    return true;
  }
  if(p.state==GenericPhase::ResponseValid | p.state==GenericPhase::ResponseError){
    DUMPPLB("Resp");
    source= ( *router_port->getTargetPort() ).connected_in_ports[m_masterMap[t.getMID()]];
    if (sc_time_stamp()<m_nextAllowedResponse && p.state==GenericPhase::ResponseValid){
      SC_REPORT_WARNING(name(),"Response started before XILINX PLB Protocol allows it. Delay will be inserted.");
      (*source)->notify(atom, m_nextAllowedResponse-sc_time_stamp()); //immideately forward the ack 
#ifdef PHASE_TRACE
      if (p.state==GenericPhase::ResponseValid){
        p_trace->tSetTrace(atom, m_nextAllowedResponse-sc_time_stamp());
        p_trace->tResetTrace(atom, m_nextAllowedResponse-sc_time_stamp()+m_clkPeriod);
      }
      p_trace->iSetTrace(atom, m_nextAllowedResponse-sc_time_stamp());
      p_trace->iResetTrace(atom, m_nextAllowedResponse-sc_time_stamp()+m_clkPeriod);
#endif    
    }
    else {
      (*source)->notify(atom); //immideately forward the ack
#ifdef PHASE_TRACE
      if (p.state==GenericPhase::ResponseValid){
        p_trace->tSetTrace(atom);
        p_trace->tResetTrace(atom, m_clkPeriod);
      }
      p_trace->iSetTrace(atom);
      p_trace->iResetTrace(atom, m_clkPeriod);
#endif    
    }
    m_rdCompTime=sc_time_stamp();
    if(m_activeSecoRdMaster!=0){ //secondary request active
      if(sc_time_stamp()<m_rdReqDeliverTime){ //if req is not delivered yet, we simply make it primary
          m_activeSecoRdReq.second.state=GenericPhase::RequestValid;
          m_TOEvent.notify(15*m_clkPeriod+m_halfClkPeriod+m_rdReqDeliverTime-sc_time_stamp());
          (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoRdSlavePortRank]))->notify(m_activeSecoRdReq, m_clkPeriod);          
          DUMPPLB("PA Valid RD (promoted)");
#ifdef PHASE_TRACE
          p_trace->tSetTrace(atom, m_clkPeriod);
          p_trace->tResetTrace(m_activeSecoWrReq, m_clkPeriod, PLBPhase::SecondaryRequest);
#endif    
          m_activePrimReq=m_activeSecoRdReq;
          promote(true,0);
          return true;
      }
      if(!m_secoRdAck |(m_secoRdAck && sc_time_stamp()==m_addressAckTimeRd) ){
        m_promotionEvent.notify(m_clkPeriod);
        m_promoteRd=true;
        return true;  //prevent promotion
      }
      m_activeSecoRdReq.second.state=PLBPhase::RdPrim;
      (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoRdSlavePortRank]))->notify(m_activeSecoRdReq);
#ifdef PHASE_TRACE
      p_trace->tSetTrace(m_activeSecoRdReq);
      p_trace->tResetTrace(m_activeSecoRdReq, m_clkPeriod);
#endif              
    }
    promote(true,0);
    return true;
  }
  SC_REPORT_ERROR(name(),"Slave responed with unallowed phase state");
  return false;

}

inline bool PLBBusProtocol::processSlaveAccess(){
  //used for TO handling
#ifdef USE_STATIC_CASTS
  GenericRouterAccess &t = *_getRouterAccessHandle(m_activePrimReq);
#else
  GenericRouterAccess &t = dynamic_cast<GenericRouterAccess&>(*(m_activePrimReq.first));
#endif
  
  if(t.getSError()==GenericError::Generic_Error_TimeOut){ //request already timed out and completed address phase protocol, now lets complete the data phase
    startMasterAccessProcessing.notify(SC_ZERO_TIME);
    if(t.getMCmd()==Generic_MCMD_WR){
      m_activePrimReq.second.state=GenericPhase::DataError;
      registerSlaveAccess(m_activePrimReq);
    }
    else{
      m_activePrimReq.second.state=GenericPhase::ResponseError;
      registerSlaveAccess(m_activePrimReq);
    }
  }
  else{ //request just timed out
    DUMPPLB("TIMEOUT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    m_activePrimReq.second.state=GenericPhase::RequestAccepted;
    m_timedOut=true; //our cute semaphore...
    registerSlaveAccess(m_activePrimReq);  //complete address phase protocol
    m_timedOut=false;
    m_timedOutTransaction=_getRouterAccessHandle(m_activePrimReq);
    t.setSError(GenericError::Generic_Error_TimeOut); //set it here so slave can check it without getting an event
    next_trigger(m_clkPeriod+m_halfClkPeriod);
  }
  return true;
}

inline void PLBBusProtocol::promote(bool rNw, int delay){
  if (!rNw){
    m_activePrimWrMaster=m_activeSecoWrMaster;
    m_activePrimWrSlavePortRank=m_activeSecoWrSlavePortRank;
    m_activeSecoWrMaster=0;
    m_activeSecoWrSlavePortRank=255;
    if (m_secoWriteDataValid){
      destinationPort destination= ( *router_port->getInitPort() ).connected_in_ports[m_activePrimWrSlavePortRank];
      (*destination)->notify(m_waitingSecondaryWriteData, delay*m_clkPeriod);
      DUMPPLB("DATA Valid in "<<delay);
#ifdef PHASE_TRACE
      p_trace->tSetTrace(m_waitingSecondaryWriteData, delay*m_clkPeriod);
#endif      
      m_secoWriteDataValid=false;
    }
    if (m_activePrimWrMaster)
      m_addressAckTimeWr=sc_time_stamp();
  }
  else{
    m_activePrimRdMaster=m_activeSecoRdMaster;
    m_activePrimRdSlavePortRank=m_activeSecoRdSlavePortRank;
    m_activeSecoRdMaster=0;
    m_activeSecoRdSlavePortRank=255;
    if (m_activePrimRdMaster) {
      m_nextAllowedResponse=sc_time_stamp()+2*m_clkPeriod;
      m_addressAckTimeRd=sc_time_stamp();
    }
  }
  if(!scheduler_port->isEmpty())
    startMasterAccessProcessing.notify(2*m_clkPeriod);
}

inline void PLBBusProtocol::delayedPromotion(){
  if(m_promoteWr){
    m_activeSecoWrReq.second.state=GenericPhase::RequestValid;
    (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoWrSlavePortRank]))->notify(m_activeSecoWrReq);          
    DUMPPLB("PA Valid WR (promoted delayed)");
#ifdef PHASE_TRACE
    p_trace->tSetTrace(m_activeSecoWrReq);
    p_trace->tResetTrace(m_activeSecoWrReq, PLBPhase::SecondaryRequest);
#endif      
    m_TOEvent.notify(15*m_clkPeriod+m_halfClkPeriod);
    m_activePrimReq=m_activeSecoWrReq;
    promote(false,0);
    m_promoteWr=false;
  }
  if(m_promoteRd){
    if(m_secoRdAck){
      m_activeSecoRdReq.second.state=PLBPhase::RdPrim;
      (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoRdSlavePortRank]))->notify(m_activeSecoRdReq);      
      DUMPPLB("Rd prim");
#ifdef PHASE_TRACE
      p_trace->tSetTrace(m_activeSecoRdReq);
      p_trace->tResetTrace(m_activeSecoRdReq, m_clkPeriod);
#endif      
    }
    else{
      m_activeSecoRdReq.second.state=GenericPhase::RequestValid;
      m_TOEvent.notify(15*m_clkPeriod+m_halfClkPeriod);
      (*(( *router_port->getInitPort() ).connected_in_ports[m_activeSecoRdSlavePortRank]))->notify(m_activeSecoRdReq);      
      DUMPPLB("PA Valid RD (promoted delayed)");
#ifdef PHASE_TRACE
      p_trace->tSetTrace(m_activeSecoRdReq);
      p_trace->tResetTrace(m_activeSecoRdReq, PLBPhase::SecondaryRequest);
#endif      
      m_activePrimReq=m_activeSecoRdReq;
    }
    promote(true,0);
    m_promoteRd=false;
  }
}

inline void PLBBusProtocol::assignProcessMasterAccessSensitivity(sc_process_b* pMethod){
  //this is the way how we can assign a sensitivity to a method that resides inside the router
  //now the ProcessMasterAccess Callback will occur whenever we trigger startMasterAccessProcessing
  sc_sensitive::make_static_sensitivity(pMethod, startMasterAccessProcessing);
}

inline void PLBBusProtocol::assignProcessSlaveAccessSensitivity(sc_process_b* pMethod){
  //this is the way how we can assign a sensitivity to a method that resides inside the router
  //now the ProcessSlaveAccess Callback will occur whenever we trigger startSlaveAccessProcessing
  sc_sensitive::make_static_sensitivity(pMethod, m_TOEvent);
}

#define INIT_PHASETRACE \
  std::map<unsigned int, std::string> phases

#define ADDPHASE2TRACE(space, name) \
  phases[space :: name]=#name

inline void PLBBusProtocol::end_of_elaboration(){
  GS_TRACE(name(), "Generating master map...");
  GS_TRACE(name(), "Found the following masters:");
  for (unsigned int i=0; i<router_port->getTargetPort()->connected_in_ports.size(); i++){
    GS_TRACE(name(), "At port %d there is the master with ID=0x%x ",i,((unsigned long)(router_port->getTargetPort()->connected_in_ports[i])));
    m_masterMap[((unsigned long)(router_port->getTargetPort()->connected_in_ports[i]))]=i;
  }

  scheduler_port->setMasterMap(&m_masterMap);
#ifdef PHASE_TRACE
  INIT_PHASETRACE;
  ADDPHASE2TRACE(GenericPhase, Idle);
  ADDPHASE2TRACE(GenericPhase, RequestValid);
  ADDPHASE2TRACE(GenericPhase, RequestAccepted);
  ADDPHASE2TRACE(GenericPhase, RequestError);
  ADDPHASE2TRACE(GenericPhase, DataValid);
  ADDPHASE2TRACE(GenericPhase, DataAccepted);
  ADDPHASE2TRACE(GenericPhase, DataError);
  ADDPHASE2TRACE(GenericPhase, ResponseValid);
  ADDPHASE2TRACE(GenericPhase, ResponseAccepted);
  ADDPHASE2TRACE(GenericPhase, ResponseError);
  ADDPHASE2TRACE(PLBPhase, SecondaryRequest);
  ADDPHASE2TRACE(PLBPhase, WrPrim);
  ADDPHASE2TRACE(PLBPhase, RdPrim);

  std::map<unsigned int, bool> i_aligns;
  std::map<unsigned int, bool> t_aligns;

  t_aligns[GenericPhase::RequestValid]=true;
  t_aligns[GenericPhase::DataValid]=true;
  t_aligns[PLBPhase::SecondaryRequest]=true;
  t_aligns[PLBPhase::WrPrim]=true;
  t_aligns[PLBPhase::RdPrim]=true;

  p_trace= new phase_trace("phase_trace", router_port, phases, i_aligns, t_aligns);
#endif
}

#endif
