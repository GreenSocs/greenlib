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

#include "greenbus/transport/PLB/PLB.h"

using namespace tlm;

#include <iomanip>

#define MEMSIZE 100


class PLBMaster 
: public sc_module,
  public payload_event_queue_output_if<GS_ATOM>,
  public gb_configurable
{
public:
  PLBMasterPort init_port;
  typedef PLBMasterPort::accessHandle accessHandle;
  typedef PLBMasterPort::phase phaseHandle;
  typedef PLBMasterPort::created_transaction_handle PLBTransactionHandle;

  unsigned char mem[MEMSIZE];

  void sendPV(accessHandle );
  void sendPVT(accessHandle );
    
  void test_wr();
  void test_rd();
  
  virtual void notify (GS_ATOM& tc);

  virtual void end_of_simulation();
  
  sc_event receive_event;
  accessHandle tah;
  phaseHandle phase;


  unsigned c;
  
  GSDataType mdata;
  
  // configurable parameters
  gb_param<gs_uint32> burst_length; // burst length in byte
  gb_param<gs_uint32> initial_delay; // wait this number of clock cycles initially before start of operation
  gb_param<gs_uint32> loops; // how many transactions shall I generate? (set to 0 for infinite)
  
public:
  //Constructor
  SC_HAS_PROCESS(PLBMaster);
  PLBMaster(sc_module_name name_, bool rNw) //, unsigned long long targetAddress, const char* data, bool rNw) 
    : sc_module(name_),
      init_port("iport"),
	    c(0),
      mdata(10)
  {
    // configurable parameters
    GB_PARAM(burst_length, gs_uint32, 10);
    GB_PARAM(initial_delay, gs_uint32, 0);
    GB_PARAM(loops, gs_uint32, 0);
        
    if (rNw) {
      SC_THREAD(test_rd);
      sensitive << receive_event;
    }
    else{
      SC_THREAD(test_wr);
      sensitive << receive_event;
    }
    
    
    // bind PEQ in interface
    init_port.peq.out_port(*this);

    
  }
  
  ~PLBMaster(){
  }
};


void PLBMaster::test_rd(){
  wait (initial_delay*CLK_CYCLE, SC_NS);

  while(true){
    tah = init_port.create_transaction();
    
    if (mdata.getSize()<burst_length)
      mdata.getData().resize(burst_length);

    tah->setMCmd(Generic_MCMD_RD);
    tah->setMBurstLength(burst_length.value);
    tah->setMData(mdata);
    tah->setMPrio(3);

    init_port.Request(tah);

    wait();

    if(phase.state==GenericPhase::RequestAccepted) {
      GS_TRACE(name(), "Slave sent RequestAccepted.");
    }
    else {
      SC_REPORT_ERROR(name(),"wrong phase");
    }
    
    wait();
    tah->setMPrio(3);
    if(phase.state==GenericPhase::ResponseValid){
      GS_TRACE(name(), "Slave sent data.");
    }
    else
      if(phase.state==GenericPhase::ResponseError){
        GS_TRACE(name(), "Oh no, my request timed out... :-(");
        return;
      }
    else
      SC_REPORT_ERROR(name(),"wrong phase!");
  }
}

void PLBMaster::test_wr(){
  wait (initial_delay*CLK_CYCLE, SC_NS);
  
  GS_TRACE(name(), "Running in write mode");
  
  // fill in the write data
  if (mdata.getSize()<burst_length)
    mdata.getData().resize(burst_length);
  for (unsigned int i=0; i<strlen((char *)mem)+1; i++){
    mdata[i]=i;
  }


#ifdef USE_PV
    c++;
    tah = init_port.create_transaction();
    tah->setMCmd(Generic_MCMD_WR);
        
    tah->setMData(mdata);
    tah->setMBurstLength(burst_length.value);
    tah->setMPrio(3);

  while(c!=312498) {
    c++;
    init_port.Transact(tah);
  }

#else
  unsigned bvalid;
  unsigned num_loops = 0;

  while (1) {
    c++;

    bvalid=0;

    tah = init_port.create_transaction();

    tah->setMCmd(Generic_MCMD_WR);
        
    tah->setMData(mdata);
    tah->setMBurstLength(burst_length.value);
    tah->setMPrio(3);

    init_port.Request(tah);
    wait();

    if(phase.state==GenericPhase::RequestAccepted) {
      GS_TRACE(name(), "Slave sent RequestAccepted.");
    }
    else {
      SC_REPORT_ERROR(name(),"wrong phase");
    }


    do {
#ifdef USE_CC
      bvalid+=8; // send 64 bit per data atom
      if (bvalid>burst_length.value) 
        bvalid=burst_length.value;
      tah->setMSBytesValid(bvalid);
#else
      bvalid=burst_length.value; // send all data with one data atom
#endif

      init_port.SendData(tah,phase);
      
      wait();
      tah->setMPrio(2);
      if(phase.state==GenericPhase::DataAccepted) {
        GS_TRACE(name(), "Slave accepted the data.");
      }
      else {
        if(phase.state==GenericPhase::DataError) {
          GS_TRACE(name(), "Oh no, my request timed out... :-(");
          return;
        }      
        else {
          SC_REPORT_ERROR(name(),"wrong phase");
        }
      }
    } while (bvalid!=burst_length.value);

    if (loops!=0) {
      num_loops++;
      if (num_loops==loops) break;
    }
  }
#endif
}


void PLBMaster::end_of_simulation() {
  cout << c << " Transaktionen." << endl << flush;
}


void PLBMaster::notify(GS_ATOM &tc) {
#ifdef USE_STATIC_CASTS
  tah = _getMasterAccessHandle(tc);
#else
  tah =  boost::dynamic_pointer_cast<PLBMasterAccess>(tc.first);
#endif
  phase = _getPhase(tc);
  receive_event.notify();
}
