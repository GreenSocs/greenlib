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

#include "simple_b_gs_master.h"
#include "simple_b_gs_slave.h"
#include "simple_b_gs_secure_slave.h"
#include "simple_b_gs_secure_adapter.h"
#include "ExplicitLTTarget.h"

#ifdef USE_GS_MONITOR
#include "greencontrol/core/gc_core.h"  // include GreenControl Core
#include "greencontrol/gcnf/plugin/configplugin.h"   // ConfigPlugin
#include "greencontrol/gcnf/plugin/configdatabase.h" // ConfigDatabase to be bound to port of ConfigPlugin
#include "greencontrol/gcnf/apis/GCnf_Api/GCnf_Api.h"
#include "greencontrol/gav/apis/gav_api/GAV_Api.h"
//#include "greencontrol/ShowSCObjects.h"
// GreenAV
#include "greencontrol/gav/plugin/gav_plugin.h"
#include "greencontrol/gav/plugin/Stdout_OutputPlugin.h"
#include "greencontrol/gav/plugin/File_OutputPlugin.h"

#define USE_GREEN_CONFIG
#include "greensocket/monitor/green_socket_monitor.h"
#endif


SC_MODULE(simple_b_gs_slave_hier)
{
  typedef conf_fwd_target_multi_socket<simple_b_gs_slave> socket_type;

  socket_type socket;

  simple_b_gs_slave child;
  
  simple_b_gs_slave_hier(sc_core::sc_module_name name_, bool use_cachable_, bool use_prio_)
    : sc_core::sc_module(name_)
    , socket("socket")
    , child("child", use_cachable_, use_prio_)
  {
    socket(child.socket);
  }

};


int sc_main(int argc, char** argv){

  bool provoke_bind_error=false;
  if (argc>1){
    if (std::string(argv[1])=="provoke_bind_error")
      provoke_bind_error=true;
  }

#ifdef USE_GS_MONITOR
  /// GreenControl Core instance
  gs::ctr::GC_Core       core("ControlCore");
  
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);  
  gs::av::GAV_Plugin analysisPlugin("AnalysisPlugin", gs::av::STDOUT_OUT);    
#endif
  

  
  simple_b_gs_secure_adapter* a;
  if (!provoke_bind_error) a=new simple_b_gs_secure_adapter("A");
  simple_b_gs_master m("M");
  simple_b_gs_slave  s0("S0",true, false);
  simple_b_gs_slave_hier  s1("S1", false, true);
  ExplicitLTTarget   s2("S2");

  simple_b_gs_secure_slave  s3("S3");

#ifdef USE_GS_MONITOR  
  gs::socket::monitor<>* mon1=0, *mon2=0, *mon3=0, *mon4=0, *mon5=0;
  
  mon1=gs::socket::connect_with_monitor<32, tlm::tlm_base_protocol_types>(m.socket0, s0.socket);
  mon2=gs::socket::connect_with_monitor<32, tlm::tlm_base_protocol_types>(m.socket1, s1.socket);
  mon3=gs::socket::connect_with_monitor<32, tlm::tlm_base_protocol_types>(m.socket2, s2.socket);
#else
  m.socket0(s0.socket);
  m.socket1(s1.socket);
  m.socket2(s2.socket);
#endif
  
  if (provoke_bind_error)
    m.socket3(s3.socket);
  else{
#ifdef USE_GS_MONITOR  
    mon4=gs::socket::connect_with_monitor<32, tlm::tlm_base_protocol_types>(m.socket3, a->t_socket);
    mon5=gs::socket::connect_with_monitor<32, tlm::tlm_base_protocol_types>(a->i_socket, s3.socket);
#else
    m.socket3(a->t_socket);
    a->i_socket(s3.socket);
#endif
  }

#ifdef USE_GS_MONITOR
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
