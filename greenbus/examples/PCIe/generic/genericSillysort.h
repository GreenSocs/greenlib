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

#include "globals.h"
#include "greenbus/transport/GP/GP.h"

#include <time.h>

using namespace tlm;

#define USE_PV
//#define SPEEDTEST

#include <iomanip>
#ifndef SPEEDTEST
#define SHOW_SC_TIME_(module,msg) cout << "time " << std::setw(3) << sc_time_stamp() << ": " \
                                       << std::setw(6) << #module << ": " << msg
#define SHOW_SC_TIME(module,msg)  SHOW_SC_TIME_(module,msg) << endl
#else
#define SHOW_SC_TIME_(module,msg) // nothing
#define SHOW_SC_TIME(module,msg)  // nothing
#endif

class sillysort
  : public sc_module, 
    public payload_event_queue_output_if<GS_ATOM>
{
public:
  GenericMasterBlockingPort init_port;
  typedef GenericMasterBlockingPort::accessHandle transactionHandle;
  typedef GenericMasterBlockingPort::phase phase;

  unsigned char mem[MEMSIZE];

  void sendPV(transactionHandle);
  void sendPVT(transactionHandle);
  
  sc_event ft;
  int pending;
  
  void run();
  void run2();
  void notify(GS_ATOM&);
  

  //Constructor
  SC_CTOR( sillysort ) : init_port("iport") {
    strcpy((char *)(mem),"As molt...");

    pending=0;
    init_port.out_port(*this);
    
    SC_THREAD( run );
//    SC_THREAD( run2 );

  }
  
  GSDataType data;
};



// This master has two threads, one which uses the blocking transaction
// interface, and the other uses the non-blocking interface


void sillysort::run()
{

    clock_t start=clock();
    unsigned long long  pvts=0;
    unsigned long long pvs=0;
    
#ifdef SPEEDTEST
    for (int i=0;i<1000;i++)
    {
#endif      
    // set things up at the slave
    strcpy((char *)(mem), "As molt...");
    data.setPointer((void*)&mem[0]);

    transactionHandle t1 = init_port.create_transaction();
    t1->setMCmd(Generic_MCMD_WR);
    t1->setMAddr(0x0);
    t1->setMData(data);
    t1->setMBurstLength(strlen((char *)mem)+1);

    SHOW_SC_TIME(sillysort, "run: start writing \"" << (char *)mem << "\" to slave");
    init_port.Transact(t1);
    pvs++;
    SHOW_SC_TIME(sillysort, "run: finished writing to slave");

    int swaps;

  do {
    while (pending) {
      wait(ft);
    }
    
//    wait(); // sync point

    // get the data to our local cache, then run
    transactionHandle t1 = init_port.create_transaction();
    t1->setMCmd(Generic_MCMD_RD);
    t1->setMAddr(0x0);
    t1->setMBurstLength(strlen((char *)mem)+1);
    SHOW_SC_TIME(sillysort, "run: PV read from slave");
    init_port.Transact(t1);
    pvs++;
    SHOW_SC_TIME(sillysort, "run: PV got \"" << (char *)mem << "\" from slave");

    swaps=0;
    for (int i=0; i<strlen((char *)mem)-1; i++) {
      if (mem[i]>mem[i+1]) {
        unsigned char t=mem[i];
        mem[i]=mem[i+1];
        mem[i+1]=t;
        transactionHandle t1 = init_port.create_transaction();
        t1->setMCmd(Generic_MCMD_WR);
        t1->setMAddr(i);
        t1->setMBurstLength(2);
        data.setPointer((void*)&mem[i]);
        SHOW_SC_TIME(sillysort, "run: start writing \"" << (char *)mem << "\" (bytes "<<i<<" to "<<i+1<<") to slave");
#ifdef USE_PV
        sendPV(t1);
#else
        sendPVT(t1);
#endif
        pvts++;
        swaps++;
      }
      
    }
  } while (swaps);
  
#ifdef SPEEDTEST
    }
    
  clock_t end=clock();
  
  cout << "start:"<<start<<" end:"<<end<<" elapsed(s):"<<(float)(end-start)/CLOCKS_PER_SEC<<endl;
  cout << "pvt transactions:"<<pvts<<" pv transactions:"<<pvs<<endl;

  cout << "pvts/second:"<<(float)pvts/((float)(end-start)/CLOCKS_PER_SEC)<<endl;
#endif
}


void sillysort::sendPV(transactionHandle t)
{
  init_port.Transact(t);
}

void sillysort::sendPVT(transactionHandle t)
{
  
  pending++;
  phase answer;
  init_port.Request.block(t, answer);
  cout<<endl<<sc_time_stamp()<<" answer is "<<answer.state<<endl;
  SHOW_SC_TIME(sillysort, "run: Slave Accepted.");
  SHOW_SC_TIME(sillysort, "run:  Slave Accepted, sending data.");
  init_port.SendData(t,answer);
}

void sillysort::notify(GS_ATOM& tc)
{
  transactionHandle tah = _getMasterAccessHandle(tc);
  phase p=tc.second;
  
  switch (p.state) {
    case GenericPhase::RequestAccepted: //not needed any more since we use Request.block
      break;
    case GenericPhase::DataAccepted:
        // ok send the next burst
        if (p.BurstNumber < tah->getMBurstLength()) {
          SHOW_SC_TIME(sillysort, "run2: Sending data to slave." << p.BurstNumber);
          init_port.SendData(tah,p);
        } else if  (p.BurstNumber > tah->getMBurstLength()) {
            SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Fell of the end of the burst?" );
          }
      break;
    case GenericPhase::ResponseValid:
      SHOW_SC_TIME(sillysort, "run2: (burst done) got data \"" << (char *)mem << "\" from slave.");
      init_port.AckResponse(tah,p);
      pending--;
      ft.notify();
      break;
    default:
      SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Phase not recognized" );
  }
  
}

