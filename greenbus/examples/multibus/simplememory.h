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

#include "greenbus/transport/GP/GP.h"

using namespace tlm;


#include <iomanip>

#define DELAY(x) 10*x, SC_NS

class simplememory 
: public sc_module, 
  public tlm_b_if<GenericTransactionHandle>,
  public payload_event_queue_output_if<GS_ATOM>  
{
public:
  GenericSlavePort target_port;
  typedef GenericSlavePort::accessHandle accessHandle;
  typedef GenericSlavePort::phase phaseHandle;
  
  unsigned char MEM[MEMSIZE];

  std::list<std::pair<accessHandle, phaseHandle > > waiting;
  bool inWrite;
  int m_delay;

  MAddr m_base, m_high;

  // simplememory functions 
  void notify(GS_ATOM&);
  void PVTProcess(GenericSlavePort::accessHandle,  GenericSlavePort::phase);  
  int IPmodel( GenericSlaveAccess* t);

  // tlm_b_if implementation
  void b_transact(GenericTransactionHandle);

  // tlm_slave_if implementation
  virtual void setAddress(MAddr  base, MAddr  high);
  virtual void getAddress(MAddr& base, MAddr& high);

    
  SC_HAS_PROCESS(simplememory);

  /**
   * Constructor. 
   * @param name_ Module name.
   * @param delay_ Access delay in cycles.
   */
  simplememory(sc_module_name name_, int delay_) :
    sc_module(name_),
    target_port("tport"),
    inWrite(false),
    m_delay(delay_),
    m_base(0),
    m_high(0)
  {
    target_port.bind_b_if(*this);    
    target_port.peq.out_port(*this);
  }
};


void simplememory::b_transact( GenericTransactionHandle t)
{
  (void) IPmodel(&(t->getTargetAccess())); // we dont care about timing
}


void simplememory::notify(GS_ATOM &tc)
{
  GenericSlavePort::accessHandle tah=_getSlaveAccessHandle(tc);
  GenericSlavePort::phase p=tc.second;
  PVTProcess(tah,p);
}

/* This method plays out the protocol. We can do this how we like, so long as we
 * stick to the generic protocol. We can hard wire in some number, ask the IP
 * block, be random, whatever. These numbers determin which "real" bus protocol
 * we are essencially modelling */
void simplememory::PVTProcess(  GenericSlavePort::accessHandle tah,  GenericSlavePort::phase p)
{
  
  switch (p.state) {
    
  case GenericPhase::RequestValid:
    {
      if (inWrite) {
        //keep writes in order !!!
	GS_TRACE(name(), "I am busy... can't process this write for now");
        waiting.push_back(std::pair<GenericSlavePort::accessHandle,  GenericSlavePort::phase > (tah,p));        
      } 
      else {
        target_port.AckRequest(tah, p, DELAY(1));
        
        if (tah->getMCmd() == Generic_MCMD_RD) {
          GS_TRACE(name(), "accepted READ cmd, sending data now.");
          target_port.Response(tah, p, DELAY(IPmodel(tah.get())));
        } else {
          GS_TRACE(name(), "accepted WRITE cmd (in 1 cycle), now waiting for data.");
          inWrite=true;
        }
      }
    }
    break;
    
    
  case GenericPhase::DataAccepted:
    {
      GS_TRACE(name(), "master sent DataAccepted. Transaction finished OK.");
    }
    break;
    
    
  case GenericPhase::DataValid:
    {
      GS_TRACE(name(), "receiving data from master.");
      GSDataType my_data;
      my_data.set(tah->getMData());
      GS_TRACE(name(), "Got %d data bytes: [%s]", my_data.getSize(), my_data.serialize());

      for (unsigned int i=0; i<tah->getMBurstLength(); i++){
        MEM[i+(tah->getMAddr()-m_base)]=my_data[i];
      }

      // send DataAck
      target_port.AckData(tah,p,DELAY(tah->getMBurstLength()-1));
      
      inWrite =false;
      if (!waiting.empty()) {
        GS_TRACE(name(), " RE-AWAKE pending write  ");
        std::pair<GenericSlavePort::accessHandle,  GenericSlavePort::phase > 
          pair =waiting.front();
        waiting.pop_front();
        PVTProcess(pair.first,pair.second);
      }
    }
    break;
    
  case GenericPhase::ResponseAccepted:
    {
      GS_TRACE(name(), "master sent ResponseAccepted. Transaction finished OK.");
    }
    break;
    
    
  default:
    {
      SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Phase not recognized" );
    }
    break;
  }  
}
  


/* Here is what the user should end up writing
   a very simple model,
   to work in a 'co-war' like environment, where the model writer thinks about
   timing up front, then this model can return a timing number, or a strucutre
   of timing numbers if required */

int simplememory::IPmodel( GenericSlaveAccess* t)
{
  if (t->getMCmd()==Generic_MCMD_RD){
    GS_TRACE(name(), "processing blocking READ: burstlength=%d, addr=0x%x, data=[%s]", (unsigned)t->getMBurstLength(), (unsigned)(t->getMAddr()-m_base), MEM+t->getMAddr()-m_base);
    for (unsigned int i=0; i<t->getMBurstLength(); i++){
      t->getSData()[i]=MEM[i+(t->getMAddr()-m_base)];
    }
    return m_delay;
  }

  if (t->getMCmd()==Generic_MCMD_WR){
    GS_TRACE(name(), "processing blocking WRITE");
    GSDataType my_data;
    my_data.set(t->getMData());
    for (unsigned int i=0; i<t->getMBurstLength(); i++){
      MEM[i+(t->getMAddr()-m_base)]=my_data[i];
    }
    return m_delay*2;
  }

  return 0;

}



void simplememory:: setAddress(MAddr  base, MAddr  high) 
{
  target_port.base_addr =base;
  m_base=base;
  target_port.high_addr =high;
}

void simplememory:: getAddress(MAddr& base, MAddr& high)
{
  base =m_base;
  high =m_high;
}

