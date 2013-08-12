#ifndef __TARGET_H__
#define __TARGET_H__

#include "greensignalsocket/green_signal.h"
#include "common.h"
using namespace gs_generic_signal;

class Target : public sc_module {
  public : 
    target_signal_socket<Target> in_signal_port;

    SC_HAS_PROCESS(Target);
    Target(sc_module_name name) : sc_module(name),
    in_signal_port("inPort") {

      in_signal_port.register_b_transport(this, &Target::b_trans);
      gs::socket::config<gs_generic_signal_protocol_types> tmpCnf;
      tmpCnf.use_mandatory_extension<GATE>();
      in_signal_port.set_config(tmpCnf);

    }
    void b_trans(gs_generic_signal_payload &trans, sc_time& delay) {
      if (in_signal_port.get_extension<GATE>(trans)) {
        std::cout << name() << " : Recvd value " << in_signal_port.get_last_value<GATE>() << " for GATE signal at " << sc_time_stamp() << ".\n";
      }
    }
};
#endif
