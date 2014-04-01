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

This simple example shows how to keep track of the temporal decopled
(TD) time. It uses the GreenSocs Quantum Keeper to handle this time as
a global gs_param that is outputed using the GreenAV mechanism.

The slave is a plain OSCI slave (taken from the OSCI examples) only
slightly changed to use the quantum keeper.

*/

#include "b_master_qk.h"
#include "ExplicitLTTarget_gs_quantumkeeper.h"

#include "greencontrol/config.h"
#include "greencontrol/gav/apis/gav_api/GAV_Api.h"
// GreenAV
#include "greencontrol/gav/plugin/Stdout_OutputPlugin.h"
#include "greencontrol/gav/plugin/FileWithTd_OutputPlugin.h"


int sc_main(int argc, char** argv){

  /// GreenControl Core instance
  gs::ctr::GC_Core       core();
  
  // GreenConfig Plugin
  gs::cnf::ConfigDatabase* cnfdatabase = new gs::cnf::ConfigDatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(cnfdatabase);  
  gs::av::GAV_Plugin analysisPlugin(gs::av::STDOUT_OUT);    
  
  // Master and Slave
  b_master_qk m("M");
  ExplicitLTTarget s("S");

  m.socket(s.socket);

  // Register the command and address that is processed in the target
  gs::av::GAV_Api::getApiInstance()->add_to_default_output(
      gs::av::TXT_TD_FILE_OUT,
      s.current_command
  );
  gs::av::GAV_Api::getApiInstance()->add_to_default_output(
      gs::av::TXT_TD_FILE_OUT,
      s.current_addr
  );

  // Register the end of a transaction (as seen by the master)
  gs::av::GAV_Api::getApiInstance()->add_to_default_output(
      gs::av::TXT_TD_FILE_OUT,
      m.done_transaction
  );

  sc_core::sc_start();

  return 0;
}
