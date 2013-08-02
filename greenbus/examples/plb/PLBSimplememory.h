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

#include <systemc.h>
#include <list>
using std::list;

#include "greenbus/transport/PLB/PLB.h"

using namespace tlm;
using namespace PLB;

#include <iomanip>


//this slave has one problem:
//in case the slave delays its request ack for a very long time, it could happen that in the mean time the
//request timed out and a new one arrived, which would confuse the slave...
class PLBSimplememory 
: public sc_module, 
  public tlm_b_if<GenericTransactionHandle>,
  public payload_event_queue_output_if<GS_ATOM>
{
public:
 
  PLBSlavePort target_port;
  typedef PLBSlavePort::phase phaseHandle;
  
  unsigned char MEM[50];

  int m_delay;

  void react();
  void PVTProcess(  PLBSlavePort::accessHandle,  PLBSlavePort::phase);
  void test_sl(  PLBSlavePort::accessHandle,  PLBSlavePort::phase);
  void doRead();
  void b_transact(GenericTransactionHandle);
  void ackChecker();

  int IPmodel( PLBSlavePort::accessHandle t);
  
  void setAddress(MAddr  base, MAddr  high);
  void getAddress(MAddr& base, MAddr& high);
  
  unsigned long long m_base;
  unsigned long long m_high;
  
  PLBSlavePort::accessHandle m_tah;
  PLBSlavePort::phase m_ph;
  bool secAck;
  bool IDLE;
  MID m_activeMaster;
 
  sc_event ackCheck_event, receive_event;
  
  virtual void notify (GS_ATOM& tc);

  unsigned bvalid;
  
    
  //Constructor
  SC_HAS_PROCESS(PLBSimplememory);
  PLBSimplememory(sc_module_name name_, int delay ) :
    sc_module(name_),
    target_port ("tport")
  {
    target_port.bind_b_if(*this);
    m_delay=delay;
    IDLE=true;
    secAck=false;

    SC_METHOD( react );
    sensitive << receive_event;
    dont_initialize();
    
    SC_METHOD(ackChecker);
    sensitive << ackCheck_event;
    dont_initialize();
    
    target_port.peq.out_port(*this);
  }
  
  ~PLBSimplememory();
};

PLBSimplememory::~PLBSimplememory(){
}

void PLBSimplememory::ackChecker(){
  if (m_tah->getSError()==GenericError::Generic_Error_TimeOut){
    GS_TRACE(name(), "Request timed out");
    return;
  }
  switch (m_ph.state){
    case GenericPhase::RequestValid:
#ifdef VERBOSE
      if (secAck) {
        GS_TRACE(name(), "Request became primary during ack Cycle (so basically it was acked as prim).");
      }
      else {
        GS_TRACE(name(), "Acked the Request as primary.");
      }
#endif
      secAck=false;
      if (m_tah->getMCmd()==Generic_MCMD_RD){
        doRead();
      }    
      break;
    case PLBPhase::SecondaryRequest:
      secAck=true;
      GS_TRACE(name(), "Acked the Req as secondary. Waiting for prim now.");
      //next_trigger(target_port.default_event()); //this should be delayed for 1 delta, because of race with react (so it should be triggred by react
      break;
    case PLBPhase::WrPrim:
      secAck=false;
      GS_TRACE(name(), "WrPrim occured.");
      break;
    case PLBPhase::RdPrim:
      secAck=false;
      GS_TRACE(name(), "RdPrim occured.");
      doRead();
      break;
  }
}

void PLBSimplememory::doRead(){
    std::vector<gs_uint8> my_data;
    strncpy((char *)(MEM),"the answer is golden",m_tah->getMBurstLength());
    for (unsigned int i=0; i<m_tah->getMBurstLength(); i++){
      (m_tah->getSData())[i]=MEM[i];
    }
    GS_TRACE(name(), "send reply in %d ns", 2*CLK_CYCLE+(m_tah->getMBurstLength()-1)*CLK_CYCLE);
    target_port.Response(m_tah,m_ph,CLK_CYCLE+CLK_CYCLE+ //PLB: 2 clock cycle gap between posedge addrAck/promotion and posedge of first rdDAck
                         (m_tah->getMBurstLength()-1)*CLK_CYCLE,SC_NS);
    IDLE=true;
}

void PLBSimplememory::test_sl(PLBSlavePort::accessHandle tah,  PLBSlavePort::phase p){
  switch (p.state){
  case GenericPhase::RequestValid:
    if (!IDLE) return;

    GS_TRACE(name(), "got primary request!");

    /*
    if(tah->getMCmd()==Generic_MCMD_WR){
      cout<<"Write Command"<<endl
          <<"BurstLength="<<tah->getMBurstLength()<<endl
          <<flush;
    }
    if(tah->getMCmd()==Generic_MCMD_RD){
      cout<<"Read Command"<<endl
          <<"BurstLength="<<tah->getMBurstLength()<<endl
          <<flush;
    }
    */
    
    bvalid=0;
    target_port.AckRequest(tah,p, m_delay*CLK_CYCLE,SC_NS);
    ackCheck_event.notify(m_delay*CLK_CYCLE,SC_NS); //awake at own posedge ack
    IDLE=false;
    break;
    
  case PLBPhase::SecondaryRequest:
    if (!IDLE) return;
    GS_TRACE(name(), "slave got secondary request.");
    
    /*
    if(tah->getMCmd()==Generic_MCMD_WR){
      cout<<"Write Command"<<endl
          <<"BurstLength="<<tah->getMBurstLength()<<endl
          <<flush;
      }
    if(tah->getMCmd()==Generic_MCMD_RD){
      cout<<"Read Command"<<endl
          <<"BurstLength="<<tah->getMBurstLength()<<endl
          <<flush;
      }
     */

    bvalid=0;
    target_port.AckRequest(tah,p, m_delay*CLK_CYCLE,SC_NS);
    ackCheck_event.notify(m_delay*CLK_CYCLE,SC_NS); //awake at own posedge ack
    IDLE=false;    
    break;  
    
  case GenericPhase::DataValid:
    {
      if (IDLE)
        SC_REPORT_ERROR(name(), "Got Data in IDLE mode! Unexpected behaviour!");
      GS_TRACE(name(), "Got data, MSBytesValid=%d, MBurstLength=%d", (int)tah->getMSBytesValid(), (int)tah->getMBurstLength());
#ifdef USE_CC
      //cout<<name()<<": "<<bvalid<<endl;
      target_port.AckData(tah,p,((tah->getMSBytesValid()-bvalid)>>3)*CLK_CYCLE,SC_NS);
      bvalid=tah->getMSBytesValid();
      if (tah->getMSBytesValid()==tah->getMBurstLength()) // received last data atom
        IDLE=true;
#else
      target_port.AckData(tah,p,(tah->getMSBytesValid()>>3)*CLK_CYCLE,SC_NS );
      IDLE=true;
#endif
    }
    break;
  }
}

void PLBSimplememory:: setAddress(MAddr  base, MAddr  high){
  target_port.base_addr=base;
  target_port.high_addr=high;
}

void PLBSimplememory:: getAddress(MAddr& base, MAddr& high){
  base=target_port.base_addr;
  high=target_port.high_addr;
}

void PLBSimplememory::b_transact( GenericTransactionHandle t)
{
  PLBSlavePort::accessHandle tt=_getSlaveAccessHandle(t);
   wait(m_delay*CLK_CYCLE+(tt->getMBurstLength())*CLK_CYCLE, SC_NS);
}


void PLBSimplememory::react()
{
  if (IDLE){
    m_activeMaster=m_tah->getMID();
    test_sl(m_tah,m_ph);
  }
  else {
    if (m_activeMaster==m_tah->getMID()){
      if(secAck) ackCheck_event.notify();
      test_sl(m_tah,m_ph);
    }
  }
}

/* Here is what the user should end up writing
   a very simple model,
   to work in a 'co-war' like environment, where the model writer thinks about
   timing up front, then this model can return a timing number, or a strucutre
   of timing numbers if required */

int PLBSimplememory::IPmodel( PLBSlavePort::accessHandle t)
{
  SC_REPORT_ERROR("PLBSimplememory","Not implemented!");

  return 0;
}

void PLBSimplememory::notify(GS_ATOM &tc) {
  m_tah = _getSlaveAccessHandle(tc);
  m_ph = _getPhase(tc);
  receive_event.notify();
}

