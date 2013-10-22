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

#include "gsgpsocket/transport/GSGPMasterBlockingSocket.h"
  using namespace gs;
  using namespace gs::gp;

#include <time.h>
#include <iomanip>

#define SHOW_SC_TIME(msg) std::cout << "time " << std::setw(3)                 \
<< sc_core::sc_time_stamp() << ": " << std::setw(6) << name() << ": " << msg   \
<< std::endl

class sillysort
: public sc_core::sc_module, 
  public payload_event_queue_output_if<master_atom>
{
public:
  GenericMasterBlockingPort<32> init_port;
  typedef GenericMasterBlockingPort<32>::accessHandle transactionHandle;
  typedef GenericMasterBlockingPort<32>::phase phase;
  
  unsigned char mem[MEMSIZE];
  unsigned char ch;

  void sendPVT(transactionHandle);
  
  sc_core::sc_event ft;
  int pending;
  
  void run();
  void run2();
  void notify(master_atom&);
  
  SC_HAS_PROCESS(sillysort);
  //Constructor
  sillysort(sc_core::sc_module_name name, unsigned int target_address=0, const char* data="As molt example in...") 
  : sc_core::sc_module(name)
  , init_port("iport") 
    
  {
    initial_data=data;
    strcpy((char *)(mem),data);
    tadd=target_address;
    pending=0;
    init_port.out_port(*this);
    
    SC_THREAD( run );

    // Configure the socket
    GSGPSocketConfig cnf;
    // This master awaits a write response
    cnf.use_wr_resp = true;
    init_port.set_config(cnf);
    
  }
  
  std::string initial_data;
  GSDataType data;
  unsigned int tadd;
};



// This master has two threads, one which uses the blocking transaction
// interface, and the other uses the non-blocking interface
void sillysort::run()
{
  unsigned char ch1, ch2;
  int j;
  unsigned int str_len = strlen((char *)mem);

  for (j =0 ; j < str_len; j++ )
  {
    ch = mem[j];
    transactionHandle t1 = init_port.create_transaction();
    t1->setMCmd(Generic_MCMD_WR);
    t1->setMAddr(tadd + j);
    t1->setMBurstLength(1);
    data.setData(gs::GSDataType::dtype(&ch, sizeof((char *)&ch) ));
    t1->setMData(data);
    init_port.Transact(t1);
    init_port.release_transaction(t1);
   }

   SHOW_SC_TIME( "run: Initally memory contains: \"" << (char *)mem << "\"");

   int swaps;

    do 
    {
      while (pending) {
        wait(ft);
      }
      
      swaps=0;
      for (unsigned int i=0; i<str_len-1; i++) 
      {
         // get the data to our local cache, then run
         transactionHandle t1 = init_port.create_transaction();
         t1->setMCmd(Generic_MCMD_RD);
         t1->setMAddr(i);
         t1->setMBurstLength(1);

         data.setData(gs::GSDataType::dtype(&ch1, sizeof((char *)&ch1) ));
         t1->setMData(data);
         init_port.Transact(t1);
   
         data.setData(gs::GSDataType::dtype(&ch2, sizeof((char *)&ch2) ));
         t1->setMData(data);
         t1->setMAddr(i +1);
         init_port.Transact(t1);
   
         init_port.release_transaction(t1);
   
         SHOW_SC_TIME( "run: Got '" << ch1 << "' and '" << ch2 << "' from address " << tadd + i );

         while(pending) wait(ft);

         if (ch1 > ch2) {

           SHOW_SC_TIME( "run: Swapping Characters ");
           transactionHandle t1 = init_port.create_transaction();
           t1->setMCmd(Generic_MCMD_WR);
           t1->setMAddr(i);
           t1->setMBurstLength(1);
 
           data.setData(gs::GSDataType::dtype(&ch2, sizeof((char *)&ch2) ));
           t1->setMData(data);
           init_port.Transact(t1);
 
           data.setData(gs::GSDataType::dtype(&ch1, sizeof((char *)&ch1) ));
           t1->setMData(data);
           t1->setMAddr(i+1);
           init_port.Transact(t1);
 
           init_port.release_transaction(t1);
           swaps++;
         }
      }

      for (j =0 ; j < str_len; j++ )
      {
        transactionHandle t1 = init_port.create_transaction();
        t1->setMCmd(Generic_MCMD_RD);
        t1->setMAddr(tadd + j);
        t1->setMBurstLength(1);
        data.setData(gs::GSDataType::dtype(&ch, sizeof((char *)&ch) ));
        t1->setMData(data);

        init_port.Transact(t1);
        mem[j] = ch;
        init_port.release_transaction(t1);
       }

       mem[j++] = '\0';

       SHOW_SC_TIME( "run: After another pass memory contains:: \"" << (char *)mem << "\"");

    } while (swaps);
  
}


void sillysort::sendPVT(transactionHandle t)
{
  
  pending++;
  phase answer;
  init_port.Request.block(t, answer);
  //cout<<endl<<sc_time_stamp()<<" answer is "<<answer.state<<endl;
  SHOW_SC_TIME( "run: Slave Accepted Request, sending data.");
  answer.setBytesValid((gs_uint64) t->getMBurstLength() );
  init_port.SendData(t,answer);
}

void sillysort::notify(master_atom& tc)
{
  transactionHandle tah = _getMasterAccessHandle(tc);
  phase p=_getPhase(tc);
  
  switch (p.state) {
    case GenericPhase::RequestAccepted: //not needed any more since we use Request.block
      break;
    case GenericPhase::DataAccepted:
      // ok send the next burst
      if (p.getBytesValid() < tah->getMBurstLength()) {
        p.setBytesValid((gs_uint64)tah->getMBurstLength());
        SHOW_SC_TIME( "run2: Sending data to slave." << p.getBytesValid());
        init_port.SendData(tah,p);
      } 
      else if  (p.getBytesValid() > tah->getMBurstLength()) {
        SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Fell of the end of the burst?" );
      }
      break;
    case GenericPhase::ResponseValid:
      SHOW_SC_TIME( "run2: (burst done) got data \"" << (char *)mem << "\" from slave.");
      init_port.AckResponse(tah,p);
      pending--;

      init_port.release_transaction(tah);
      ft.notify();
      break;
    default:
      SC_REPORT_ERROR( sc_core::SC_ID_INTERNAL_ERROR_, "Phase not recognized" );
  }
  
}

