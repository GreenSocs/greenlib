#ifndef MULTI_EXAMPLE_H_
#define MULTI_EXAMPLE_H_

#include "tlm.h"
#include "greensignalsocket/green_signal.h"

using namespace gs_generic_signal;
GS_GUARD_ONLY_EXTENSION(foo);
GS_GUARD_ONLY_EXTENSION(bar);
GS_GUARD_ONLY_EXTENSION(baa);

SC_MODULE(Master1)
{

  gs_generic_signal::initiator_signal_socket  socket;

  SC_CTOR(Master1): socket("master1_socket")
  {
    SC_THREAD(run);
    socket.register_nb_transport_bw(this, &Master1::nb_trans);
   
    gs::socket::config<gs_generic_signal_protocol_types> tmp;
    tmp.use_mandatory_extension<bar>();
    tmp.use_optional_extension<foo>();
    socket.set_config(tmp);
  }
  
  void b_trans(unsigned int id, tlm::tlm_generic_payload &trans, sc_core::sc_time& delay)
  {
	  
  }

  tlm::tlm_sync_enum nb_trans(gs_generic_signal_payload& trans, signal_phase& ph, sc_core::sc_time& delay){
      return tlm::TLM_COMPLETED;
  }

  void run(){
    wait(10,sc_core::SC_NS);
    gs_generic_signal_payload* p=socket.get_transaction();
    signal_phase  ph;
    sc_core::sc_time t=sc_core::SC_ZERO_TIME;
    unsigned char data=0x0f;
    bool flag =true;
    p->set_data_ptr(&data);
    p->set_ack_requirement(flag);
    socket.validate_extension<bar>(*p);
    socket->nb_transport_fw(*p,ph,t);
    //check the phase now
    std::cout << ph << "\n";
    wait(10,sc_core::SC_NS);
    flag =false;
    data=0x04;
    ph=gs_generic_signal::REQ;
    p->set_data_ptr(&data);
    p->set_ack_requirement(flag);
    socket.invalidate_extension<bar>(*p);
    //The user is allowed to use the same payload for sending different signals,
    //it is ok to not call clear_extension before reusing. At the slave end, 
    //socket.get_extension is called which returns true only for validated ones.
    //(*p).clear_extension<bar>();
    socket.validate_extension<foo>(*p);
    socket->nb_transport_fw(*p,ph,t);
    //check the phase now
    std::cout << ph << "\n";
    socket.release_transaction(p);
    }
};

SC_MODULE(Master2)
{

  gs_generic_signal::initiator_signal_socket  socket;

  SC_CTOR(Master2): socket("master2_socket")
  {
    SC_THREAD(run);
    socket.register_nb_transport_bw(this, &Master2::nb_trans);
   
    gs::socket::config<gs_generic_signal_protocol_types> tmp;
    tmp.use_mandatory_extension<bar>();
    socket.set_config(tmp);
  }
  
  void b_trans(unsigned int id, tlm::tlm_generic_payload &trans, sc_core::sc_time& delay)
  {
	  
  }

  tlm::tlm_sync_enum nb_trans(gs_generic_signal_payload& trans, signal_phase& ph, sc_core::sc_time& delay){
    std::cout<<"BAR"<<std::endl;
    return tlm::TLM_COMPLETED;
  }

  void run(){
    wait(15,sc_core::SC_NS);
    gs_generic_signal_payload* p=socket.get_transaction();
    signal_phase  ph;
    sc_core::sc_time t=sc_core::SC_ZERO_TIME;
    unsigned char data=0x0f;
    bool flag =true;
    p->set_data_ptr(&data);
    p->set_ack_requirement(flag);
    socket.validate_extension<bar>(*p);
    socket->nb_transport_fw(*p,ph,t);
    socket.release_transaction(p);
    }
};

SC_MODULE(Slave)
{
  
  target_signal_multi_socket<Slave> socket;

  SC_CTOR(Slave): socket("socket")//, multi("target_multi")
  {
    socket.register_nb_transport_fw(this, &Slave::trans);

    gs::socket::config<gs_generic_signal_protocol_types> tmp;
    tmp.use_mandatory_extension<bar>();
    tmp.use_optional_extension<foo>();
    socket.set_config(tmp);
    //socket.set_source(name);
    
  }
  
  tlm::tlm_sync_enum trans(unsigned int id, gs_generic_signal_payload& trans, signal_phase& ph, sc_core::sc_time& delay)
  { 
	std::cout<< "transport call received from socket" << id << "\n";   
    if (socket.get_extension<bar>(trans))
      {
    	if(trans.is_ack_required()==true)
    		ph=gs_generic_signal::ACK;
    	std::cout<<"bar enabled"<<std::endl;
      }
    else if(socket.get_extension<foo>(trans))
    {
    	if(trans.is_ack_required()==true)
    		ph=gs_generic_signal::ACK;
    	std::cout<<"foo enabled"<<std::endl;
    }
    else
      std::cout<<"foo and bar disabled"<<std::endl;
    
    //check the last signal value here
    unsigned char last_val=0;
    last_val=socket.get_last_value<bar>();
    std::cout << sc_core::sc_time_stamp() << " : last value of bar signal "<< (unsigned int)last_val << "\n"; 

    last_val=socket.get_last_value<foo>();
    std::cout << sc_core::sc_time_stamp() << " : last value of foo signal "<< (unsigned int)last_val << "\n"; 

    last_val=socket.get_last_value<baa>();
    std::cout << sc_core::sc_time_stamp() << " : last value of baa signal "<< (unsigned int)last_val << "\n"; 
    return tlm::TLM_COMPLETED;
  }
};


#endif /*EXAMPLE_H_*/
