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

// This file is based on the PLBMaster.h (see GreenBus example 'plb')


//
// This is a module owning a bidirectional port.
// The behaviour is a master-behavior so this is not an example how
// to use a bidirectional port meaningful!
//
// This bidirectioinal module first sends multiple (=loops) write 
// transactions to the address bidir_port.target_addr and increases 
// address automatically regarding the burst length of one transaction
// (=burst_length).
// Afterwards the same data are read again.
//
//
// Variable master device 
//  can be configured to different behaviour:
//
// use #define USE_PV to send PV transaction (Transact)
//

//#define USE_PV

#define MEMSIZE 262144 // for address space 0x00000 to 0x40000

#include <boost/config.hpp> // needed for SystemC 2.1
#include <systemc>
#include <iostream>

#include "gsgpsocket/transport/GSGPBidirectionalSocket.h"
using namespace gs;
using namespace gs::gp;

#include "tlm_utils/peq_with_get.h"           // Payload event queue FIFO



#include <iomanip>
#include "nb_example/gs_dump.h"  // needed for GS_DUMP macro if not USE_GPSOCKET


class BidirModule 
: public sc_core::sc_module
, public tlm_b_if<GenericBidirectionalPort<32>::accessHandle> // = b_transact(tr)
, public payload_event_queue_output_if<bidir_atom> // = notify
{
public:
  GenericBidirectionalPort<32> bidir_port; // bidirectional port!!
  typedef GenericBidirectionalPort<32>::accessHandle accessHandle;
  typedef GenericBidirectionalPort<32>::phase phase;
  typedef pair<accessHandle, phase> peq_pair;
  tlm_utils::peq_with_get<peq_pair> m_receive_peq;
  
  unsigned char mem[MEMSIZE];

  void sendPV(accessHandle );
  void sendPVT(accessHandle );
    
  void main_action();
  void perform_writes();
  void perform_reads();
  
  peq_pair* wait_for_next_atom();

  virtual void notify (bidir_atom& tc); // master+slave part
  void b_transact(accessHandle); // slave part

  virtual void end_of_simulation();
  
  GSDataType mdata;
  
  // configurable parameters
  gs::gs_param<gs_uint32> burst_length; // burst length in byte
  gs::gs_param<gs_uint32> initial_delay; // wait this number of clock cycles initially before start of operation
  gs::gs_param<gs_uint32> loops; // how many transactions shall I generate?
  //gs_uint32 burst_length;
  //gs_uint32 initial_delay;
  //gs_uint32 loops;
  
public:
  // Constructor
  SC_HAS_PROCESS(BidirModule);
  BidirModule(sc_core::sc_module_name name_) //, unsigned long long targetAddress, const char* data, bool rNw) 
  : sc_module(name_)
  , bidir_port("bidir_m_port")
  , m_receive_peq("receive_peq")
  , mdata(20)
  , burst_length("burst_length", 20)
  , initial_delay("initial_delay", 10)
  , loops("loops", 5)
  {
    SC_THREAD(main_action);

    // Master/Slave port bind
    // bind PEQ in interface
    bidir_port.peq.out_port(*this);
    // Slave port bind
    bidir_port.bind_b_if(*this);    
    
    // Configure the socket
    GSGPSocketConfig cnf;
    cnf.use_wr_resp = false;
    bidir_port.set_config(cnf);
  }
  
  ~BidirModule(){
  }
};


// ----------- main action -----------------------------------------
void BidirModule::main_action() {
  wait (initial_delay*CLK_CYCLE, sc_core::SC_NS);
  perform_writes();
  mdata.getData().resize(burst_length, 0); // reset data
  wait (initial_delay*CLK_CYCLE, sc_core::SC_NS);
  std::cout << std::endl << "----------------------------------------------------------" << std::endl << std::endl;
  perform_reads();
}


// ----------- WRITE action -----------------------------------------
void BidirModule::perform_writes() {
  GS_DUMP("write mode");
  
  unsigned char data_cnt = 0;
  gs_uint64 addr;
  unsigned num_loops = 0;

  // fill in the write data
  if (mdata.getSize()<burst_length)
    mdata.getData().resize(burst_length);
  

#ifdef USE_PV

  accessHandle tah = bidir_port.create_transaction();
  tah->setMCmd(Generic_MCMD_WR);
  tah->setMData(mdata);
  tah->setMBurstLength(burst_length.getValue());

  while(1) {
    std::cout << std::endl;

    // fill in new write data
    for (unsigned int i=0; i<burst_length; i++){
      mdata[i]=data_cnt++;
    }

    addr = bidir_port.target_addr; addr += (num_loops*burst_length);
    tah->setMAddr( addr );

    std::cout << "(" << name() << "): data to send: "; for (unsigned int a = 0; a < burst_length; a++) std::cout << (unsigned int) mdata[a] << " "; std::cout << std::endl;
    GS_DUMP("Master send blocking.");
    bidir_port.Transact(tah);

    if (loops!=0) {
      num_loops++;
      if (num_loops==loops) break;
    }
  }

#else // if not USE_PV

  peq_pair *atom;
  phase ph;
  unsigned bvalid;

  while (1) {
    wait(sc_core::SC_ZERO_TIME); // to beautify output
    std::cout << std::endl;

    // make sure no transaction outstanding
    assert(m_receive_peq.get_next_transaction() == NULL);
    bvalid=0;

    // fill in new write data
    for (unsigned int i=0; i<burst_length; i++){
      mdata[i]=data_cnt++;
    }

    accessHandle tah = bidir_port.create_transaction();
    tah->setMCmd(Generic_MCMD_WR);
    addr = bidir_port.target_addr; addr += (num_loops*burst_length);
    tah->setMAddr( addr );
    tah->setMData(mdata);
    tah->setMBurstLength(burst_length.getValue());
    
    std::cout << "(" << name() << "): data to send: "; for (unsigned int a = 0; a < burst_length; a++) std::cout << (unsigned int) mdata[a] << " "; std::cout << std::endl;
    GS_DUMP("Master send Request (RequestValid).");
    bidir_port.Request(tah);
    atom = wait_for_next_atom();
    tah = atom->first; ph = atom->second;

    if (ph.state == GenericPhase::RequestAccepted) {
      GS_DUMP("Slave sent RequestAccepted.");
    }
    else if (ph.state==GenericPhase::RequestError) {
      SC_REPORT_WARNING(name(), "Oh no, request error.");
      return;
    }
    else {
      std::stringstream ss; ss << "wrong phase: "; ss << ph.to_string();
      SC_REPORT_ERROR(name(),ss.str().c_str());
    }
 

    do {
      // make sure no transaction outstanding
      assert(m_receive_peq.get_next_transaction() == NULL);
      bvalid = burst_length; // send all data with one data atom
      ph.setBytesValid( bvalid );
      GS_DUMP("Master send data (DataValid).");
      bidir_port.SendData(tah,ph);
      atom = wait_for_next_atom();
      tah = atom->first; ph = atom->second;
      if (ph.state == GenericPhase::DataAccepted) {
        GS_DUMP("Slave accepted the data (DataAccepted).");
      }
      else if (ph.state == GenericPhase::DataError) {
        GS_DUMP("Oh no, data error.");
        return;
      }      
      else {
        std::stringstream ss; ss << "wrong phase: "; ss << ph.to_string();
        SC_REPORT_ERROR(name(),ss.str().c_str());
      }
    } while (bvalid!=burst_length);

    bidir_port.release_transaction(tah);
    
    if (loops!=0) {
      num_loops++;
      if (num_loops==loops) break;
    }
  }
#endif // end if USE_PV
}


// ----------- READ action -----------------------------------------
void BidirModule::perform_reads() {
  GS_DUMP("read mode");

#ifndef USE_PV
  peq_pair *atom;
  phase ph;
#endif
  gs_uint64 addr;
  
  unsigned num_loops = 0;
  while(true) {
    wait(sc_core::SC_ZERO_TIME); // to beautify output
    std::cout << std::endl;
    // make sure no transaction outstanding
    assert(m_receive_peq.get_next_transaction() == NULL);

    accessHandle tah = bidir_port.create_transaction();
    
    if (mdata.getSize()<burst_length)
      mdata.getData().resize(burst_length);
    
    tah->setMCmd(Generic_MCMD_RD);
    addr = bidir_port.target_addr; addr += (num_loops*burst_length);
    tah->setMAddr( addr );
    tah->setMBurstLength(burst_length.getValue());
    tah->setMData(mdata);
    
#ifdef USE_PV

    GS_DUMP("Master send blocking.");
    bidir_port.Transact(tah);
    std::cout << "(" << name() << "): data received: "; for (unsigned int a = 0; a < tah->getMBurstLength(); a++) std::cout << (unsigned int) tah->getMData()[a] << " "; std::cout << std::endl;

#else // if not USE_PV
    
    GS_DUMP("Master send Request (RequestValid).");
    bidir_port.Request(tah);
    atom = wait_for_next_atom();
    tah = atom->first; ph = atom->second;
    if (ph.state == GenericPhase::RequestAccepted) {
      GS_DUMP("Slave sent RequestAccepted.");
    }
    else if (ph.state == GenericPhase::RequestError) {
      SC_REPORT_WARNING(name(), "Oh no, request error.");
      return;
    }
    else {
      std::stringstream ss; ss << "wrong phase: "; ss << ph.to_string();
      SC_REPORT_ERROR(name(),ss.str().c_str());
    }
    
    do {
      GS_DUMP("master waits for data (chunk).");
      atom = wait_for_next_atom();
      tah = atom->first; ph = atom->second;
      if(ph.state==GenericPhase::ResponseValid){
        GS_DUMP("Slave sent data.");
        GS_DUMP("data valid: 0x"<<(std::hex)<< (gs_uint64)ph.getBytesValid() <<(std::dec)
                <<" (32bit=" << (gs_uint32)ph.getBytesValid() << ")");
      }
      else {
        std::stringstream ss; ss << "wrong phase: "; ss << ph.to_string();
        SC_REPORT_ERROR(name(),ss.str().c_str());
      }
      bidir_port.AckResponse(tah, ph);
    } 
    while (ph.getBytesValid() < tah->getMBurstLength());
    std::cout << "(" << name() << "): data received: "; for (unsigned int a = 0; a < tah->getMBurstLength(); a++) std::cout << (unsigned int) tah->getMData()[a] << " "; std::cout << std::endl;
#endif // end if USE_PV
    
    bidir_port.release_transaction(tah);
    
    if (loops!=0) {
      num_loops++;
      if (num_loops==loops) break;
    }
  } // end while
}


// ----------- others -----------------------------------------
void BidirModule::end_of_simulation() {
}


// ----------- notify -----------------------------------------
void BidirModule::notify(bidir_atom &tc) {
  GS_DUMP("non-blocking notify "<< tc.second.to_string()<<", "<< gs::tlm_command_writer::to_string(tc.first.get_tlm_transaction()->get_command()));
  // legacy code, leave out _getMasterAccessHandle and _getPhase !!
  peq_pair *atom = new peq_pair(tc.first, tc.second); // TODO: Memory Leak, delete after used
  sc_core::sc_time t = sc_core::SC_ZERO_TIME;
  m_receive_peq.notify(*atom, t);
}

BidirModule::peq_pair* BidirModule::wait_for_next_atom() {
  peq_pair *atom = NULL;
  atom = m_receive_peq.get_next_transaction();
  if (!atom) {
    wait(m_receive_peq.get_event()); // wait for BEGIN_RESP (END_RESP/completed will be sent by nb_transport_bw)
    atom = m_receive_peq.get_next_transaction();
  }
  GS_DUMP("got "<< atom->second.to_string()<<" "<<
          gs::tlm_command_writer::to_string(atom->first->get_tlm_transaction()->get_command()) <<
          " out of local peq");
  return atom;
}


// ----------- slave part -----------------------------------------
void BidirModule::b_transact( accessHandle t)
{
  assert(false); // not supported!
}
