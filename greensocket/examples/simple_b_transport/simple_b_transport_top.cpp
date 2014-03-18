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
#include "greencontrol/config.h"  // include GreenControl Core
// GreenAV
#include "greencontrol/gav/plugin/gav_plugin.h"
#include "greencontrol/gav/plugin/File_OutputPlugin.h"

#define USE_GREEN_CONFIG
#include "greensocket/monitor/green_socket_monitor.h"
#endif

int sc_main(int argc, char** argv){

  bool provoke_bind_error=false;
  if (argc>1){
    if (std::string(argv[1])=="provoke_bind_error")
      provoke_bind_error=true;
  }

#ifdef USE_GS_MONITOR
  // GreenAV Plugin
  gs::av::GAV_Plugin analysisPlugin("AnalysisPlugin", gs::av::TXT_FILE_OUT);    
  // Get text file output plugin
  gs::av::OutputPlugin_if* myop = gs::av::GAV_Api::getApiInstance(NULL)->get_default_output(gs::av::TXT_FILE_OUT);
#endif
  
  simple_b_gs_secure_adapter* a;
  if (!provoke_bind_error) a=new simple_b_gs_secure_adapter("A");
  simple_b_gs_master m("M");
  simple_b_gs_slave  s0("S0",true, false);
  simple_b_gs_slave  s1("S1", false, true);
  ExplicitLTTarget   s2("S2");

  simple_b_gs_secure_slave  s3("S3");

#ifdef USE_GS_MONITOR
  GS_BIND_AND_OBSERVE(m.socket0, s0.socket, myop);
  GS_BIND_AND_OBSERVE(m.socket1, s1.socket, myop);
#else
  GS_BIND_AND_OBSERVE(m.socket0, s0.socket, NULL);
  GS_BIND_AND_OBSERVE(m.socket1, s1.socket, NULL);
#endif

  // Example for the other GS_BIND macro, to observe in more then one output plugin
  GS_BIND_AND_DEFINE_MONITOR_PARAM(m.socket2, s2.socket, myparam);
#ifdef USE_GS_MONITOR
  myop->observe(myparam);
#endif

  
  if (provoke_bind_error)
    m.socket3(s3.socket);
  else{
    GS_BIND_AND_OBSERVE(m.socket3, a->t_socket, myop);
    GS_BIND_AND_OBSERVE(a->i_socket, s3.socket, myop);
  }

  sc_core::sc_start();

  return 0;
}
