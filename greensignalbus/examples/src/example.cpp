#include "systemc.h"
#include "Initiator.h"
#include "Target.h"
#include "greensignalbus/genericSignalBus.h"

int sc_main(int argc, char** argv) {
  Initiator1 initInst1("myInitiator1");
  Initiator2 initInst2("myInitiator2");
  Target targetInst1("myTarget1");
  Target targetInst2("myTarget2");
  
  //initInst.out_signal_port(targetInst.in_signal_port);

  gs::gp::GenericSignalBus<32> sb("signalbus");
  //gs_generic_signal::genericSignalBus sb("signalBus");

  initInst1.out_signal_port(sb.target_socket);
  initInst2.out_signal_port(sb.target_socket);

  sb.init_socket(targetInst1.in_signal_port);
  sb.init_socket(targetInst2.in_signal_port);

  //targetInst1.in_signal_port.set_source<GATE>(initInst1.out_signal_port.name());
  targetInst2.in_signal_port.set_source<GATE>(initInst2.out_signal_port.name());
  std::string srcName("myInitiator.outPort");
  //targetInst1.in_signal_port.set_source(srcName);
  //targetInst2.in_signal_port.set_source(srcName);

  sc_start(11, SC_NS);
  return 0;
}
