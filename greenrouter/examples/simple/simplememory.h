// LICENSETEXT
// 
//   Copyright (C) 2007-2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel, Christian Schroeder
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
//     GreenSocs Ltd
// 
// 
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT

//
//  - Fixed BytesValid behaviour! (does not use faulty BurstNumber any longer)
//
//


#include <boost/config.hpp> // needed for SystemC 2.1
#include <systemc>
#include <list>
using std::list;

# include "gsgpsocket/transport/GSGPSlaveSocket.h"
using namespace gs;
using namespace gs::gp;

#include <iomanip>

/*#define SHOW_SC_TIME(module, msg) cout << "time " << std::setw(3) \
<< sc_time_stamp() << ": " << std::setw(6) << #module << ": " << msg << endl */

#ifndef SHOW_SC_TIME
#define SHOW_SC_TIME(msg)  // nothing
#endif

#define CYCLES(x) 3*x, sc_core::SC_NS

class simplememory 
: public sc_core::sc_module
, public tlm_b_if<GenericSlaveAccessHandle> // = b_transact(tr)
, public payload_event_queue_output_if<slave_atom>  // = notify(atom)
{
public:
  GenericSlavePort<32> target_port;
  typedef GenericSlavePort<32>::accessHandle accessHandle;
  typedef GenericSlavePort<32>::phase phase;
  
  unsigned char MEM[MEMSIZE];

  std::list<std::pair<accessHandle, phase > > waiting;
  bool inWrite;

  // tlm_b_if implementation
  void b_transact(GenericSlaveAccessHandle);
  // payload_event_queue_output_if implementation
  void notify(slave_atom&);
  void PVTProcess(accessHandle, phase);
  
  int IPmodel(accessHandle t);

  //Constructor
  SC_HAS_PROCESS(simplememory);
  simplememory(sc_core::sc_module_name name, unsigned int delay=10 ) 
    : sc_core::sc_module(name)
    , target_port ("tport")
    , m_delay(delay)
  {
    target_port.bind_b_if(*this);
    target_port.peq.out_port(*this);
    inWrite=false;
    target_port.base_addr = 0;
    // Configure the socket
    GSGPSocketConfig cnf;
    // This slave sends a write response!
    cnf.use_wr_resp = true;
    target_port.set_config(cnf);
  }
  
  GSDataType data;
  unsigned int m_delay;
};


void simplememory::b_transact( GenericSlaveAccessHandle t)
{
  (void) IPmodel(_getSlaveAccessHandle(t)); // we dont care about timing
}


void simplememory::notify(slave_atom& tc)
{
  accessHandle tah=_getSlaveAccessHandle(tc);
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
      SHOW_SC_TIME( "PVT : RequestValid ");
      if (inWrite) {
        //keep writes in order !!!
        SHOW_SC_TIME( "PVT : can't process this write for now ");
        waiting.push_back(std::pair<accessHandle,  phase > (tah,p));

      } else {
      
        if (tah->getMCmd() == Generic_MCMD_RD) {
          SHOW_SC_TIME( "PVT : Accepted Read (in 10) sending master data in 50");
                                  // because we can, copy it all on beat one but
                                  // it takes some time, get the IP model to
                                  // tell us
          target_port.AckRequest(tah,p,CYCLES(IPmodel(tah)));
        } else {
          SHOW_SC_TIME( "PVT : Accepted write (in 10) waiting for Data ");
          inWrite=true;
          target_port.AckRequest(tah,p,CYCLES(IPmodel(tah)));
        }
      }
      break;
      
    case GenericPhase::DataValid:
      SHOW_SC_TIME( "PVT : DataValid ");
      std::cout<<"DataValid got "<<(tah->getSData()[0])<<(tah->getSData()[1])<<" for address "<<(MAddr)tah->getMAddr()<<std::endl;
      if (p.getBytesValid() >= tah->getMBurstLength()) {
        SHOW_SC_TIME( "PVT : Response send");
        // it takes some time after the data has all come in for it to be
        // consumed, get the IP model to tell us.
        tah->setSResp(Generic_SRESP_DVA);
        target_port.Response(tah,p,CYCLES(IPmodel(tah)));
      }
      target_port.AckData(tah,p,CYCLES(1));
      break;
      
    case GenericPhase::ResponseAccepted:
      SHOW_SC_TIME( "PVT : ResponseAccepted " << tah->getMCmd());
      if (tah->getMCmd() == Generic_MCMD_WR) {
        inWrite=false;
      }
      if (!inWrite) {
        if (!waiting.empty()) {
          SHOW_SC_TIME( "PVT : RE-AWAKE pending write  ");
          std::pair<accessHandle, phase > pair = waiting.front();
          waiting.pop_front();
          PVTProcess(pair.first,pair.second);
        }
      }
      
      // Always nice to know
      break;
      
    default:
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
  //SHOW_SC_TIME( "handle cmd " << t->getMCmd());

  if (t->getMCmd() == Generic_MCMD_RD) {
    memcpy(&(t->getSData()[0]), &MEM[t->getMAddr()-target_port.base_addr], t->getMBurstLength());
    return m_delay;
  } else {
    memcpy( &MEM[t->getMAddr()-target_port.base_addr], &(t->getSData()[0]), t->getMBurstLength());
    return 2*m_delay;
  }

}


