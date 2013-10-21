// LICENSETEXT
//
//   Copyright (C) 2005 - 2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info.com
//
//   Developed by :
//
//  Robert Guenzel, Christian Schroeder
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

 
/*
This simple examample shows how the greensocket configuration
mechanism can be used to find out what extensions may be used on
which link.

The master configures all its sockets indicating that he is able
to use the cacheable and the priority extensions.
The connected slaves do not support all extensions so the green
sockets will figure out what may be done on each link.

One of the slaves is a plain OSCI slave (taken from the OSCI
examples).

One of the slaves reqires an extension that the master doesn't know.
A direct connection will fail, so there is an adapter adds the extension
to the transaction. The adapter does not care about anything else
so the rest of the configuration is transparently forwarded from
master to slave.


if you provide provoke_bind_error as a argument for the executable
you'll see that master and slave are not bindable without the
adapter.


NOTE: when running the example you can see that the master
gets the callback from the socket3 twice. This is because
at first the master is just bound to the adapter that tells
the master it can do what it wants, and only then when the adapter
checks its binding to the slave, it updates the information
for the master.
*/

#include "simple_b_gs_master_multi.h"
#include "simple_b_gs_slave.h"
#include "simple_b_gs_secure_slave.h"
#include "simple_b_gs_secure_adapter.h"
#include "simple_b_gs_secure_adapter.h"
#include "ExplicitLTTarget.h"

#ifdef USE_GS_MONITOR
#include "greensocket/monitor/green_socket_monitor.h"
#ifdef USE_GREEN_CONFIG
#include "greencontrol/core/gc_core.h"  // include GreenControl Core
#include "greencontrol/gcnf/plugin/configplugin.h"   // ConfigPlugin
#include "greencontrol/gcnf/plugin/configdatabase.h" // ConfigDatabase to be bound to port of ConfigPlugin
#include "greencontrol/gcnf/apis/GCnf_Api/GCnf_Api.h"
#include "greencontrol/gav/apis/gav_api/GAV_Api.h"
// GreenAV
#include "greencontrol/gav/plugin/gav_plugin.h"
#include "greencontrol/gav/plugin/Stdout_OutputPlugin.h"
#include "greencontrol/gav/plugin/File_OutputPlugin.h"
#else
#include "greensocket/monitor/green_socket_observer_base.h"
template<unsigned int BUSWIDTH> 
struct my_probe 
  : gs::socket::gp_observer_base
{
  my_probe(gs::socket::monitor<BUSWIDTH>* mon)
    :  gs::socket::gp_observer_base(mon)
  {}
  
  //small helper to avoid repeated "calculations" of the name
  std::string& get_my_name()
  {
    if (!m_name.size())
    {
      m_name="My probe for the link between ";
      //the connected init is returned to us as a bindability base ptr. 
      // if you need something else dyn cast the thing into what you need
      if (gs::socket::gp_observer_base::get_connected_initiator())      
        m_name+=gs::socket::gp_observer_base::get_connected_initiator()->get_name(); 
      else
        m_name+="Some_none_green_initiator_socket";
      m_name+=" and ";
      if (gs::socket::gp_observer_base::get_connected_target())
        m_name+=gs::socket::gp_observer_base::get_connected_target()->get_name();
      else
        m_name+="Some_none_green_target_socket";
    }
    return m_name;
  }
  
  //helper for dumping extensions
  void dump_exts(tlm::tlm_generic_payload& txn)
  {
    std::cout<<"Let's check the extensions "<<std::endl;
    //And now something that I think is another advantage of greensocket (and its extensions): you can introspect the extensions like that
    for (unsigned int i=0; i<tlm::max_num_extensions(); i++)
    {
      if (txn.get_extension(i) && gs::ext::extension_cast()[i]) //there is an extension and it is a greensocket style one
      {
        gs::ext::gs_extension_base* tmp=gs::ext::extension_cast()[i](txn.get_extension(i));
        assert(tmp);
        unsigned int potential_guard_id;
        switch(tmp->get_type(potential_guard_id)){
          case gs::ext::gs_data:
            std::cout<<"   Found a GS extension named: "<<tmp->get_name()<<" and the content is "<<tmp->dump()<<std::endl;
            break;
          case gs::ext::gs_guarded_data:
            if (txn.get_extension(potential_guard_id) && tmp->is_valid()) //only dump if guard is there
              std::cout<<"   Found a GS extension named: "<<tmp->get_name()<<" and the content is "<<tmp->dump()<<std::endl;
            break;
          case gs::ext::gs_array_guard:;
        }
      }
      else
      if (txn.get_extension(i))
      {
        std::cout<<"   There is a plain TLM extension. All I know is its ID: "<<i<<std::endl;
      }
    }
    std::cout<<"    Done checking the extensions "<<std::endl;  
  }
  
  //we get this one after nb_transport is called but before it reaches the callee
  virtual void nb_call_callback(bool fwNbw, tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase, const sc_core::sc_time& time)
  {
    //now some "analysis" to see it works
    std::cout<<get_my_name()<<": at "<<sc_core::sc_time_stamp()<<" I see a CALL to:"<<std::endl
             <<" nb_transport_"<<(fwNbw?"fw":"bw")<<std::endl
             <<"   the payload ptr is "<<&txn<<std::endl
             <<"   the phase is "<<phase<<std::endl
             <<"   the time is "<<time<<std::endl;
    dump_exts(txn);
  }
  
  //we get this one after nb_transport has returned from the callee but before it returns  at the caller side
  virtual void nb_return_callback(bool fwNbw, tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase, const sc_core::sc_time& time, tlm::tlm_sync_enum retVal)
  {
    std::cout<<get_my_name()<<": at "<<sc_core::sc_time_stamp()<<" I see a RETURN from:"<<std::endl
             <<" nb_transport_"<<(fwNbw?"fw":"bw")<<std::endl
             <<"   the payload ptr is "<<&txn<<std::endl
             <<"   the phase is "<<phase<<std::endl
             <<"   the time is "<<time<<std::endl
             <<"   the return value is "<<((retVal==tlm::TLM_ACCEPTED)?"TLM_ACCEPTED":(retVal==tlm::TLM_UPDATED)?"TLM_UPDATED":"TLM_COMPLETED")<<std::endl;
    dump_exts(txn);
  }

  //for this example I do not care about b_transport
  virtual void b_call_callback(tlm::tlm_generic_payload& txn, const sc_core::sc_time& time)
  {
    //now some "analysis" to see it works
    std::cout<<get_my_name()<<": at "<<sc_core::sc_time_stamp()<<" I see a CALL to:"<<std::endl
             <<" b_transport"<<std::endl
             <<"   the payload ptr is "<<&txn<<std::endl
             <<"   the time is "<<time<<std::endl;
    dump_exts(txn);
  }
  
  virtual void b_return_callback(tlm::tlm_generic_payload& txn, const sc_core::sc_time& time){
    std::cout<<get_my_name()<<": at "<<sc_core::sc_time_stamp()<<" I see a RETURN from:"<<std::endl
             <<" b_transport"<<std::endl
             <<"   the payload ptr is "<<&txn<<std::endl
             <<"   the time is "<<time<<std::endl;
    dump_exts(txn);
  }

  std::string m_name;

};
#endif
#endif

int sc_main(int argc, char** argv){
  bool provoke_bind_error=false;
  if (argc>1){
    if (std::string(argv[1])=="provoke_bind_error")
      provoke_bind_error=true;
  }

#ifdef USE_GS_MONITOR
#ifdef USE_GREEN_CONFIG
  /// GreenControl Core instance
  gs::ctr::GC_Core       core("ControlCore");
  
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);  
  gs::av::GAV_Plugin analysisPlugin("AnalysisPlugin", gs::av::STDOUT_OUT);    
#endif
#endif
  

  
  simple_b_gs_secure_adapter* a;
  if (!provoke_bind_error) a=new simple_b_gs_secure_adapter("A");
  simple_b_gs_master_multi m("M");
  simple_b_gs_slave  s0("S0",true, false);
  simple_b_gs_slave  s1("S1", false, true);
  ExplicitLTTarget   s2("S2");

  simple_b_gs_secure_slave  s3("S3");

#ifdef USE_GS_MONITOR  
  gs::socket::monitor<>* mon1=0, *mon2=0, *mon3=0, *mon4=0, *mon5=0;
  
  mon1=gs::socket::connect_with_monitor<32,tlm::tlm_base_protocol_types>(m.socket, s0.socket);
  mon2=gs::socket::connect_with_monitor<32,tlm::tlm_base_protocol_types>(m.socket, s1.socket);
  mon3=gs::socket::connect_with_monitor<32,tlm::tlm_base_protocol_types>(m.socket, s2.socket);
#else
  m.socket(s0.socket);
  m.socket(s1.socket);
  m.socket(s2.socket);
#endif
  
  if (provoke_bind_error)
    m.socket(s3.socket);
  else{
#ifdef USE_GS_MONITOR  
    mon4=gs::socket::connect_with_monitor<32,tlm::tlm_base_protocol_types>(m.socket, a->t_socket);
    mon5=gs::socket::connect_with_monitor<32,tlm::tlm_base_protocol_types>(a->i_socket, s3.socket);
#else
    m.socket(a->t_socket);
    a->i_socket(s3.socket);
#endif
  }

#ifdef USE_GS_MONITOR
#ifdef USE_GREEN_CONFIG
#define TEXT_OUT(mon) \
  if (mon) \
    gs::av::GAV_Api::getApiInstance(NULL)->add_to_default_output(\
      gs::av::TXT_FILE_OUT, \
      gs::cnf::GCnf_Api::getApiInstance(NULL)->getPar(mon->get_param().getName()))
  
  TEXT_OUT(mon1);
  TEXT_OUT(mon2);
  TEXT_OUT(mon3);
  TEXT_OUT(mon4);
  TEXT_OUT(mon5);

#undef TEXT(OUT)
#else
  my_probe<32> p1(mon1);
  my_probe<32> p2(mon2);
  my_probe<32> p3(mon3);
  my_probe<32> p4(mon4);
  my_probe<32> p5(mon5);
#endif
#endif
  
  sc_core::sc_start();
#ifdef USE_GS_MONITOR  
  delete mon1;
  delete mon2;
  delete mon3;
  delete mon4;
  delete mon5;
#endif  
  
  return 0;
}
