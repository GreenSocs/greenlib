#ifndef __INITIATOR_H__
#define __INITIATOR_H__

#include "greensignalsocket/green_signal.h"
#include "common.h"
using namespace gs_generic_signal;

class Initiator1 : public sc_module {
  public : 
    initiator_signal_socket out_signal_port;

    SC_HAS_PROCESS(Initiator1);
    Initiator1(sc_module_name name) : sc_module(name),
    out_signal_port("outPort"), m_value(48) {

      gs::socket::config<gs_generic_signal_protocol_types> tmpCnf;
      tmpCnf.use_mandatory_extension<GATE>();
      out_signal_port.set_config(tmpCnf);

      SC_THREAD(run);

    }
  private:
    unsigned char m_value;
    void run() {
      gs_generic_signal_payload* p = out_signal_port.get_transaction();
      unsigned char data = ++m_value;
      p->set_data_ptr(&data);
      out_signal_port.validate_extension<GATE>(*p);
      sc_core::sc_time t = sc_core::SC_ZERO_TIME;
      while(1) {
        wait(10, SC_NS);
        out_signal_port->b_transport(*p, t);
        data = ++m_value;
      }
      out_signal_port.release_transaction(p);
      return;
    }
};
class Initiator2 : public sc_module {
  public : 
    initiator_signal_socket out_signal_port;

    SC_HAS_PROCESS(Initiator2);
    Initiator2(sc_module_name name) : sc_module(name),
    out_signal_port("outPort"), m_value(55) {

      gs::socket::config<gs_generic_signal_protocol_types> tmpCnf;
      tmpCnf.use_mandatory_extension<GATE>();
      out_signal_port.set_config(tmpCnf);

      SC_THREAD(run);

    }
  private:
    unsigned char m_value;
    void run() {
      gs_generic_signal_payload* p = out_signal_port.get_transaction();
      unsigned char data = ++m_value;
      p->set_data_ptr(&data);
      out_signal_port.validate_extension<GATE>(*p);
      sc_core::sc_time t = sc_core::SC_ZERO_TIME;
      while(1) {
        wait(10, SC_NS);
        out_signal_port->b_transport(*p, t);
        data = ++m_value;
      }
      out_signal_port.release_transaction(p);
      return;
    }
};
#endif
