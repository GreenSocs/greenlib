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

#include "globals.h"
#include "greenbus/transport/GP/GP.h"

using namespace tlm;


#include <iomanip>

#ifndef SHOW_SC_TIME
//#define SHOW_SC_TIME_(module,msg) cout << "time " << std::setw(3) << sc_simulation_time() << ": " \
//                                       << std::setw(6) << #module << ": " << msg
//#define SHOW_SC_TIME(module,msg)  SHOW_SC_TIME_(module,msg) << endl


#define SHOW_SC_TIME_(module,msg) // nothing
#define SHOW_SC_TIME(module,msg)  // nothing
#endif

#define CYCLES(x) 3*x, SC_NS

class simplememory 
  : public sc_module, 
    public tlm_b_if<GenericTransactionHandle>, 
    public payload_event_queue_output_if<GS_ATOM>
{
public:
  GenericSlavePort target_port;
  typedef GenericSlavePort::accessHandle accessHandle;
  typedef GenericSlavePort::phase phase;
  
  unsigned char MEM[MEMSIZE];

  std::list<std::pair<accessHandle, phase > > waiting;
  bool inWrite;
  

  void notify(GS_ATOM&);
  void PVTProcess(accessHandle, phase);
  

  void b_transact(GenericTransactionHandle);

  int IPmodel(accessHandle t);
  
  

  //Constructor
  SC_CTOR( simplememory ) :
    target_port ("tport")
  {
    target_port.bind_b_if(*this);
    target_port.peq.out_port(*this);
    inWrite=false;
  }
  
  GSDataType data;
  
};


void simplememory::b_transact( GenericTransactionHandle t)
{
  (void) IPmodel(_getSlaveAccessHandle(t)); // we dont care about timing
}


void simplememory::notify(GS_ATOM& tc)
{
  accessHandle tah = _getSlaveAccessHandle(tc);
  phase p=_getPhase(tc);
  PVTProcess(tah,p);
}

/* This method plays out the protocol. We can do this how we like, so long as we
 * stick to the generic protocol. We can hard wire in some number, ask the IP
 * block, be random, whatever. These numbers determin which "real" bus protocol
 * we are essencially modelling */
void simplememory::PVTProcess(accessHandle tah, phase p)
{
  
  switch (p.state) {

    case GenericPhase::RequestValid:
      if (inWrite) {
        //keep writes in order !!!
        SHOW_SC_TIME(simplememory, "PVT : can't process this write for now ");
        waiting.push_back(std::pair<accessHandle,  phase > (tah,p));

      } else {
      
        if (tah->getMCmd() == Generic_MCMD_RD) {
          SHOW_SC_TIME(simplememory, "PVT : Accepted Read (in 10) sending master data in 50");
                                  // because we can, copy it all on beat one but
                                  // it takes some time, get the IP model to
                                  // tell us
          target_port.AckRequest(tah,p,CYCLES(IPmodel(tah)));
        } else {
          SHOW_SC_TIME(simplememory, "PVT : Accepted write (in 10) waiting for Data ");
          inWrite=true;
          target_port.AckRequest(tah,p,CYCLES(IPmodel(tah)));
        }
      }
      break;
      
    case GenericPhase::DataValid:
      SHOW_SC_TIME(simplememory, "PVT : Got Data Ack ");
      if (p.BurstNumber >= tah->getMBurstLength()) {
        SHOW_SC_TIME(simplememory, "PVT : Response send");
        // it takes some time after the data has all come in for it to be
        // consumed, get the IP model to tell us.
        target_port.Response(tah,p,CYCLES(IPmodel(tah)));
      } 
      target_port.AckData(tah,p,CYCLES(1));
      break;
      
    case GenericPhase::ResponseAccepted:
      SHOW_SC_TIME(simplememory, "PVT :  " << tah->getMCmd());
      if (tah->getMCmd() == Generic_MCMD_WR) {
        inWrite=false;
      }
      if (!inWrite) {
        if (!waiting.empty()) {
          SHOW_SC_TIME(simplememory, "PVT : RE-AWAKE pending write  ");
          std::pair<accessHandle, phase > pair = waiting.front();
          waiting.pop_front();
          PVTProcess(pair.first,pair.second);
        }
      }
      
      // Always nice to know
      break;
      
    default:
      cout << "Phase = "<<p.state << endl;
      SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Phase not recognized" );
      break;
  }
  
  
}
  


/* Here is what the user should end up writing
   a very simple model,
   to work in a 'co-war' like environment, where the model writer thinks about
   timing up front, then this model can return a timing number, or a strucutre
   of timing numbers if required */

int simplememory::IPmodel(accessHandle t)
{
  SHOW_SC_TIME_(simplememory, "blocking: cmd " << t->getMCmd());

  if (t->getMCmd() == Generic_MCMD_RD) {
    data.setPointer((void*)&MEM[t->getMAddr()]);
    return 10;
  } else {
    return 20;
  }

}


