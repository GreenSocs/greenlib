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


#define MEMSIZE 10000

#include <systemc.h>

#include "greenbus/transport/GP/GP.h"

using namespace tlm;



/**
 * A simple bus master that plays with the protocol.
 */
class sillysort 
: public sc_module,
  public payload_event_queue_output_if<GS_ATOM>
{
public:
  GenericMasterBlockingPort init_port;
  typedef GenericMasterBlockingPort::accessHandle accessHandle;
  typedef GenericMasterBlockingPort::phase phaseHandle;

  unsigned char mem[MEMSIZE];

  void sendPV(accessHandle );
  
  sc_event ft;
  MAddr tadd;
  
  void run();
  void test_wr();
  void test_rd();
  void notify(GS_ATOM&);

public:

  //Constructor
  SC_HAS_PROCESS(sillysort);

  /**
   * Sillysort constructor.
   * @param name_ The module name.
   * @param targetAddress The target memory address. 
   * @param data Initial memory content
   */
  sillysort(sc_module_name name_, MAddr targetAddress, const char* data = NULL) : 
    sc_module(name_),
    init_port("iport") 
  {
    tadd=targetAddress;

    // init memory
    bzero((char*)mem, MEMSIZE);
    if (data != NULL)
      strcpy((char *)(mem),data);

    cout<<"Hello my name is "<<name()<<" and my port's id is "<<init_port.get_master_port_number()<<endl<<flush;

    // This master has two threads, one which uses the blocking transaction
    // interface, and the other uses the non-blocking interface

    SC_THREAD(run);
    
    init_port.out_port(*this);    
  }
};


void sillysort::run()
{
  // send a blocking PV write ... /////////////////////////////////////////////
  // set things up at the slave
  accessHandle t1 = init_port.create_transaction(); 
  t1->setMCmd(Generic_MCMD_WR);
  t1->setMAddr(tadd);
  std::vector<gs_uint8> data;
  data.resize(strlen((char *)mem)+1);
  for (unsigned int i=0; i<strlen((char *)mem)+1; i++){
    data[i]=mem[i];
  }
  GSDataType dt(data);
  t1->setMData(dt);
  t1->setMBurstLength(strlen((char *)mem)+1);
  
  GS_TRACE(name(), "run: PV-writing [%s] to slave", mem);
  sendPV(t1);
  GS_TRACE(name(), "run: finished PV-writing to slave");

  unsigned char swaps;
  
  do {
    
    // send a blocking PV read ////////////////////////////////////////////////
    accessHandle t1 = init_port.create_transaction(); 
    t1->setMCmd(Generic_MCMD_RD);
    t1->setMAddr(tadd);
    t1->setMBurstLength(strlen((char *)mem)+1);
    data.resize(strlen((char *)mem)+1);
    dt.setData(data);
    t1->setMData(dt);
    GS_TRACE(name(), "run: PV-read from slave");
    sendPV(t1);
    GS_TRACE(name(), "run: PV got [%s] from slave", mem);


    // now do a PVT write /////////////////////////////////////////////////////
    swaps=0;

    t1 = init_port.create_transaction();
    t1->setMCmd(Generic_MCMD_WR);
    t1->setMAddr(tadd);
    t1->setMBurstLength(strlen((char *)mem)+1);
    data.resize(strlen((char *)mem)+1);
    for (unsigned int i=0; i<strlen((char *)mem)+1; i++){
      data[i]=mem[i];
    }
    dt.setData(data);
    t1->setMData(dt);
    
    // we can do a blocking PVT call like this...
    phaseHandle ph;
    init_port.Request.block(t1, ph); 
    GS_TRACE(name(), "run: slave accepted request.");  
    
    for (unsigned int i=0; i<strlen((char *)mem)-1; i++) {
      if (mem[i]>mem[i+1]) {
        unsigned char t=mem[i];
        mem[i]=mem[i+1];
        mem[i+1]=t;
        swaps++;
      }      
    }

    // and  we can do a non-blocking PVT call like this...
    init_port.SendData(t1, ph);
    wait(ft); // wait until notify() was informed that the slave accepted the data
    GS_TRACE(name(), "run: slave accepted data.");

  } while (swaps);
}


void sillysort::sendPV(accessHandle t)
{
  init_port.Transact(t);
}


void sillysort::notify(GS_ATOM& tc)
{
  accessHandle tah = _getMasterAccessHandle(tc);
  phaseHandle phase = tc.second;
  
  GS_TRACE(name(), "notify started");
  
  switch (phase.state) {    
  case GenericPhase::RequestValid: // someone sends a request to us
    SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Listen mate, I'm the one giving the orders round here!" );
    break;
    
  case GenericPhase::RequestAccepted: // someone acknowledges a request from us
    if (tah->getMCmd() == Generic_MCMD_WR) {
      GS_TRACE(name(), "notify:  Slave Accepted, sending data.");
      //init_port.SendData(tah,phase);
    } else {
      GS_TRACE(name(), "notify:  Slave Accepted, waiting to receive data.");
      // init_port.Repass(tah,phase);
    }
    break;
        
  case GenericPhase::DataAccepted: // someone has accepted our data
    GS_TRACE(name(), "notify: got data-ACK from slave [address=0x%x]", (unsigned)tah->getMAddr());
    ft.notify();
    break;
    
  case GenericPhase::ResponseValid: // someone sends a response to us
    GS_TRACE(name(), "notify: got response [%s] from slave [address=0x%x]", (char *)mem, (unsigned)tah->getMAddr());
    init_port.AckResponse(tah,phase);
    ft.notify();
    break;
    
  case GenericPhase::ResponseAccepted: // someone has accepted our response (uhoh, I thought we were a master?!)
    SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Only master should accept responses" );
    break;
    
  default: // only will come here if someone uses a user-specific phase we do not know
    SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Phase not recognized" );
  }
  
}

