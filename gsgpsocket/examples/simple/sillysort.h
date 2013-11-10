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

#ifdef USE_GPSOCKET
# include "gsgpsocket/transport/GSGPMasterBlockingSocket.h"
  using namespace gs;
  using namespace gs::gp;
#else
# include "greenbus/transport/GP/GP.h"
  using namespace tlm;
#endif

#include <time.h>




#include <iomanip>
#ifndef SPEEDTEST
# define SHOW_SC_TIME(msg) std::cout << "time " << std::setw(3) << sc_core::sc_time_stamp() << ": " \
                                       << std::setw(6) << name() << ": " << msg << std::endl
#else
# define SHOW_SC_TIME(msg)  // nothing
#endif

class sillysort
: public sc_core::sc_module, 
#ifdef USE_GPSOCKET
    public payload_event_queue_output_if<master_atom>
#else
    public payload_event_queue_output_if<GS_ATOM>
#endif
{
public:
#ifdef USE_GPSOCKET
  GenericMasterBlockingPort<32> init_port;
  typedef GenericMasterBlockingPort<32>::accessHandle transactionHandle;
  typedef GenericMasterBlockingPort<32>::phase phase;
#else
  GenericMasterBlockingPort init_port;
  typedef GenericMasterBlockingPort::accessHandle transactionHandle;
  typedef GenericMasterBlockingPort::phase phase;
#endif
  
  unsigned char mem[MEMSIZE];

  void sendPV(transactionHandle);
  void sendPVT(transactionHandle);
  
  sc_core::sc_event ft;
  int pending;
  
  void run();
  void run2();
#ifdef USE_GPSOCKET
  void notify(master_atom&);
#else
  void notify(GS_ATOM&);
#endif
  
  SC_HAS_PROCESS(sillysort);
  //Constructor
  sillysort(sc_core::sc_module_name name, unsigned int target_address=0, const char* data="As molt...") 
  : sc_core::sc_module(name)
  , init_port("iport") 
    
  {
    initial_data=data;
    strcpy((char *)(mem),data);
    tadd=target_address;
    pending=0;
    init_port.out_port(*this);
    
    SC_THREAD( run );
//    SC_THREAD( run2 );

#ifdef USE_GPSOCKET
    // Configure the socket
    GSGPSocketConfig cnf;
    // This master awaits a write response
    cnf.use_wr_resp = true;
    init_port.set_config(cnf);
#endif
    
  }
  
  std::string initial_data;
  GSDataType data;
  unsigned int tadd;
};



// This master has two threads, one which uses the blocking transaction
// interface, and the other uses the non-blocking interface


void sillysort::run()
{

#ifdef SPEEDTEST
  clock_t start=clock();
#endif
  unsigned long long  pvts=0;
  unsigned long long pvs=0;
    
#ifdef SPEEDTEST
  for (int speedtest=0;speedtest<1000;speedtest++)
  {
#endif      
    // set things up at the slave
    strcpy((char *)(mem), initial_data.c_str());

    transactionHandle t1 = init_port.create_transaction();
    t1->setMCmd(Generic_MCMD_WR);
    t1->setMAddr(tadd);
    t1->setMBurstLength(strlen((char *)mem)+1);

#ifdef USE_GPSOCKET
    data.setData(gs::GSDataType::dtype(&mem[0], strlen((char *)mem)+1));
    t1->setMData(data);
#else
    std::vector<gs_uint8> data_v;
    data_v.resize(strlen((char *)mem)+1);
    memcpy(&data_v[0], &mem[0], data_v.size());
    GSDataType dt(data_v);
    t1->setMData(dt);
#endif
    
    SHOW_SC_TIME( "run: start writing \"" << (char *)mem << "\" to slave");
    init_port.Transact(t1);
    pvs++;
    SHOW_SC_TIME( "run: finished writing to slave");

#ifdef USE_GPSOCKET
    init_port.release_transaction(t1);
#endif
    int swaps;

    do {
      while (pending) {
        wait(ft);
      }
      
  //    wait(); // sync point

      // get the data to our local cache, then run
      transactionHandle t1 = init_port.create_transaction();
      t1->setMCmd(Generic_MCMD_RD);
      t1->setMAddr(tadd);
      t1->setMBurstLength(strlen((char *)mem)+1);
#ifdef USE_GPSOCKET      
      data.setData(gs::GSDataType::dtype(&mem[0], strlen((char *)mem)+1));
      t1->setMData(data);
#else    
      data_v.resize(strlen((char *)mem)+1);
      dt.setData(data_v);
      t1->setMData(dt);
#endif

      SHOW_SC_TIME( "run: PV read from slave");
      init_port.Transact(t1);
#ifndef USE_GPSOCKET
      memcpy(&mem[0], &t1->getSData()[0], t1->getMBurstLength());
#endif
      pvs++;
      SHOW_SC_TIME( "run: PV got \"" << (char *)mem << "\" from slave");

#ifdef USE_GPSOCKET
      init_port.release_transaction(t1);
#endif

      swaps=0;
      for (unsigned int i=0; i<strlen((char *)mem)-1; i++) {
        while(pending) wait(ft);
        if (mem[i]>mem[i+1]) {
          unsigned char t=mem[i];
          mem[i]=mem[i+1];
          mem[i+1]=t;
          transactionHandle t1 = init_port.create_transaction();
          t1->setMCmd(Generic_MCMD_WR);
          t1->setMAddr(tadd+i);
          t1->setMBurstLength(2);
#ifdef USE_GPSOCKET          
          data.setData(gs::GSDataType::dtype(&mem[i],2));
          t1->setMData(data);
#else
          data_v.resize(2);
          memcpy(&data_v[0], &mem[i], data_v.size());
          dt.setData(data_v);
          t1->setMData(dt);
#endif
          //data.setPointer((void*)&mem[i]);
          SHOW_SC_TIME( "run: start writing \"" << (char *)mem << "\" (bytes "<<i<<" to "<<i+1<<") to slave");
          sendPVT(t1);
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
  //cout<<endl<<sc_time_stamp()<<" answer is "<<answer.state<<endl;
  SHOW_SC_TIME( "run: Slave Accepted Request, sending data.");
#ifdef USE_GPSOCKET
  answer.setBytesValid((gs_uint64) t->getMBurstLength() );
#else
  std::cout<<"TO ADDR "<<t->getMAddr()<<std::endl;
  t->setMSBytesValid((gs_uint64) t->getMBurstLength() );
#endif
  init_port.SendData(t,answer);
}

#ifdef USE_GPSOCKET
void sillysort::notify(master_atom& tc)
#else
void sillysort::notify(GS_ATOM& tc)
#endif
{
  transactionHandle tah = _getMasterAccessHandle(tc);
  phase p=_getPhase(tc);
  
  switch (p.state) {
    case GenericPhase::RequestAccepted: //not needed any more since we use Request.block
      break;
    case GenericPhase::DataAccepted:
      // ok send the next burst
#ifdef USE_GPSOCKET
      if (p.getBytesValid() < tah->getMBurstLength()) {
        p.setBytesValid((gs_uint64)tah->getMBurstLength());
        SHOW_SC_TIME( "run2: Sending data to slave." << p.getBytesValid());
#else
      if (tah->getMSBytesValid() < tah->getMBurstLength()) {
        tah->setMSBytesValid((gs_uint64)tah->getMBurstLength());
        SHOW_SC_TIME( "run2: Sending data to slave." << tah->getMSBytesValid());
#endif
        init_port.SendData(tah,p);
      } 
#ifdef USE_GPSOCKET
      else if  (p.getBytesValid() > tah->getMBurstLength()) {
#else
      else if  (tah->getMSBytesValid() > tah->getMBurstLength()) {
#endif
        SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Fell of the end of the burst?" );
      }
      break;
    case GenericPhase::ResponseValid:
      SHOW_SC_TIME( "run2: (burst done) got data \"" << (char *)mem << "\" from slave.");
      init_port.AckResponse(tah,p);
      pending--;
#ifdef USE_GPSOCKET
      init_port.release_transaction(tah);
#endif      
      ft.notify();
      break;
    default:
      SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Phase not recognized" );
  }
  
}

