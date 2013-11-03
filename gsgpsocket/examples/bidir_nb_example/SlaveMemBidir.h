// LICENSETEXT
// 
//   Copyright (C) 2007-2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel, Christian Schröder
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

// This file is based on the PLBSlave.h (see GreenBus example 'plb')


//
// GenericProtocol Slave with bidirectional port
//
// This is a module owning a bidirectional port.
// The behaviour is a slave-behavior so this is not an example how
// to use a bidirectional port meaningful!
//


#include <boost/config.hpp> // needed for SystemC 2.1
#include <systemc>
#include <list>
using std::list;

#include "gsgpsocket/transport/GSGPSlaveSocket.h"
using namespace gs;
using namespace gs::gp;

#include <iomanip>
#include "nb_example/gs_dump.h"  // needed for GS_DUMP macro if not USE_GPSOCKET

#define DELAY(x) 10*x, sc_core::SC_NS


class SlaveMemBidir 
: public sc_core::sc_module
, public tlm_b_if<GenericBidirectionalPort<32>::accessHandle> // = b_transact(tr)
, public payload_event_queue_output_if<bidir_atom>  // = notify(atom)
{
public:
  GenericBidirectionalPort<32> target_port;
  typedef GenericBidirectionalPort<32>::accessHandle accessHandle;
  typedef GenericBidirectionalPort<32>::phase phase;
  typedef GenericBidirectionalPort<32>::atom atom;
  
  unsigned char MEM[MEMSIZE];

  bool inWrite;
  int m_delay;

  MAddr m_base, m_high;

  // SlaveMemBidir functions 
  // tlm_b_if implementation
  void b_transact(accessHandle);
  // payload_event_queue_output_if implementation
  void notify(atom&);
  void PVTProcess(accessHandle, phase);  
  int IPmodel( accessHandle t);

  // tlm_slave_if implementation
  virtual void setAddress(MAddr  base, MAddr  high);
  virtual void getAddress(MAddr& base, MAddr& high);

    
  SC_HAS_PROCESS(SlaveMemBidir);

  /**
   * Constructor. 
   * @param name_ Module name.
   * @param delay_ Access delay in cycles.
   */
  SlaveMemBidir(sc_core::sc_module_name name_, int delay_) :
    sc_core::sc_module(name_),
    target_port("tport"),
    inWrite(false),
    m_delay(delay_),
    m_base(0),
    m_high(0)
  {
    target_port.bind_b_if(*this);    
    target_port.peq.out_port(*this);
    GS_DUMP("memory size: " << MEMSIZE);
    // Configure the socket
    GSGPSocketConfig cnf;
    cnf.use_wr_resp = false;
    target_port.set_config(cnf);
  }
};


void SlaveMemBidir::b_transact( accessHandle t)
{
  GS_DUMP("blocking b_transact");
  (void) IPmodel(t); // we dont care about timing
}


void SlaveMemBidir::notify(atom &tc)
{
  GS_DUMP("non-blocking notify " << tc.second.to_string() << ", " << 
                       gs::tlm_command_writer::to_string(tc.first.get_tlm_transaction()->get_command()));
  PVTProcess(tc.first, tc.second);
}

/* This method plays out the protocol. We can do this how we like, so long as we
 * stick to the generic protocol. We can hard wire in some number, ask the IP
 * block, be random, whatever. These numbers determin which "real" bus protocol
 * we are essencially modelling */
void SlaveMemBidir::PVTProcess(  accessHandle tah,  phase p)
{
  switch (p.state) {
    
  case GenericPhase::RequestValid:
    {
      if (inWrite) {
        //keep writes in order !!!
        GS_DUMP("I am busy... error");
        target_port.ErrorRequest(tah, p, DELAY(1));
      } 
      else {
        target_port.AckRequest(tah, p, DELAY(1));
        
        if (tah->getMCmd() == Generic_MCMD_RD) {
          GS_DUMP("accepted READ cmd.");
          int del = IPmodel(tah);
          gs_uint64 bvalid;
          bvalid = tah->getMBurstLength();
          p.setBytesValid( bvalid );
          GS_DUMP("sending data now (delay " << del << ").");
          tah->setSResp(Generic_SRESP_DVA);
          target_port.Response(tah, p, DELAY(del));
        } else {
          GS_DUMP("accepted WRITE cmd (in 1 cycle), now waiting for data.");
          inWrite=true;
        }
      }
    }
    break;
    
    
  case GenericPhase::DataAccepted:
    {
      GS_DUMP("master sent DataAccepted. Transaction finished OK.");
    }
    break;
    
    
  case GenericPhase::DataValid:
    {
      GS_DUMP("receiving data from master.");
      GS_DUMP("data valid: 0x" << (std::hex) << (gs_uint64)p.getBytesValid() << (std::dec) 
              << " (32bit="<< (gs_uint32)p.getBytesValid() <<")");
      
      if (tah->getMBurstLength() > MEMSIZE) {
        // send DataError
        target_port.ErrorData(tah,p,DELAY(p.getBytesValid()-1));
        break;
      }
      if (p.getBytesValid() < tah->getMBurstLength()) {
        GS_DUMP("ack data chunk (0x" <<(std::hex)<< (gs_uint64)p.getBytesValid() <<(std::dec)
                << " bytes valid) (not yet all bytes valid).");
        // send DataAck
        target_port.AckData(tah,p,DELAY(p.getBytesValid()-1));
        break;
      }
      
      GSDataType my_data;
      my_data.set(tah->getMData());
      GS_DUMP("Got " << (unsigned)tah->getMBurstLength() <<" data bytes sent to addr=0x" <<(std::hex)
              <<(gs_uint64)tah->getMAddr()<<" (local addr=0x"<<(gs_uint64)(tah->getMAddr()-m_base) <<(std::dec)<<"):");
      for (unsigned int i=0; i<tah->getMBurstLength(); i++){
        MEM[i+(tah->getMAddr()-m_base)]=my_data[i];
      }
      GSDataType::dtype* vec = &my_data.getData();
      assert(vec->size() >= tah->getMBurstLength()); std::cout << "            ";
      for (unsigned int i = 0; i<tah->getMBurstLength(); i++) { std::cout << (unsigned int)vec->at(i) << " "; } std::cout << std::endl;

      // send DataAck
      target_port.AckData(tah,p,DELAY(tah->getMBurstLength()-1));
      
      inWrite =false;
    }
    break;
    
  case GenericPhase::ResponseAccepted:
    {
      GS_DUMP("master sent ResponseAccepted.");

      // nothing needed to be done
    }
    break;
    
    
  default:
    {
      std::stringstream ss; ss << "error: phase "; ss << p.to_string();
      SC_REPORT_WARNING(name(),ss.str().c_str());
    }
    break;
  }  
}
  


/* Here is what the user should end up writing
   a very simple model,
   to work in a 'co-war' like environment, where the model writer thinks about
   timing up front, then this model can return a timing number, or a strucutre
   of timing numbers if required */

int SlaveMemBidir::IPmodel( accessHandle t)
{
  if (t->getMCmd()==Generic_MCMD_RD){
    GS_DUMP("processing (blocking or nb) READ: burstlength="<<(unsigned)t->getMBurstLength()
            <<", local addr=0x"<<(std::hex)<< (gs_uint64)(t->getMAddr()-m_base) <<(std::dec)<<", data=");
    std::cout << "         stored data to transmit: ";
    for (unsigned int i=0; i<t->getMBurstLength(); i++) {
      std::cout << (unsigned int)MEM[i+(t->getMAddr()-m_base)] << " ";
    } std::cout << std::endl;
    for (unsigned int i=0; i<t->getMBurstLength(); i++) {
      t->getSData()[i] = MEM[i+(t->getMAddr()-m_base)];
    }
    return m_delay;
  }

  if (t->getMCmd()==Generic_MCMD_WR){
    GS_DUMP("processing (blocking or nb) WRITE: burstlength="<<(unsigned)t->getMBurstLength()
            <<", local addr=0x"<<(std::hex)<< (gs_uint64)(t->getMAddr()-m_base) <<(std::dec)<<", data=");
    GSDataType my_data;
    my_data.set(t->getMData());
    std::cout << "         ";
    for (unsigned int i=0; i<t->getMBurstLength(); i++) {
      std::cout << (unsigned int)my_data[i] << " ";
      MEM[i+(t->getMAddr()-m_base)]=my_data[i];
    } std::cout << std::endl;
    return m_delay*2;
  }

  return 0;

}



void SlaveMemBidir:: setAddress(MAddr  base, MAddr  high) 
{
  target_port.base_addr =base;
  m_base=base;
  target_port.high_addr =high;
}

void SlaveMemBidir:: getAddress(MAddr& base, MAddr& high)
{
  base =m_base;
  high =m_high;
}
