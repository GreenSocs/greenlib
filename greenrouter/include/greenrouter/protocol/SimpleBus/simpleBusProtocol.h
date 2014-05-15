// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Robert Guenzel
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

#ifndef __simpleBusProtocol_h__
#define __simpleBusProtocol_h__

#include <stdint.h>
#include "greenrouter/genericRouter.h"
#include "greenrouter/genericScheduler_if.h"
#include "gsgpsocket/utils/gsgp_peq.h"
#include "greensocket/generic/gs_callbacks.h"
//--------------------------------------------------------------------------
/**
 * A simple bus protocol implementation. 
 * The bus processes transactions consecutively. The arbitration
 * is clock-synchronous. Thus, incoming requests are always processed
 * with the next positive edge of the clock. Requests are
 * granted immediately when the bus is idle, else they will have to wait
 * until the bus is available again.
 */
//--------------------------------------------------------------------------
namespace gs{
namespace gp{


template<unsigned int BUSWIDTH>
class SimpleBusProtocol 
: public GenericProtocol_if<tlm::tlm_base_protocol_types>,
  public sc_core::sc_module,
  protected gs::socket::extension_support_base<tlm::tlm_base_protocol_types>,
  protected configure_gs_params
{
  std::string m_type;
  virtual void bound_to(const std::string&, gs::socket::bindability_base<tlm::tlm_base_protocol_types>*, unsigned int){}
  virtual const std::string& get_type_string(){return m_type;}
  virtual gs::socket::bindability_base<tlm::tlm_base_protocol_types>* get_other_side(unsigned int,unsigned int&){return NULL;}
  virtual unsigned int get_num_bindings(){return 0;}

  typedef GenericProtocol_if<tlm::tlm_base_protocol_types>::payload_type              payload_type;
  typedef GenericProtocol_if<tlm::tlm_base_protocol_types>::phase_type                phase_type;  
  typedef tlm::tlm_sync_enum                            sync_enum_type;
  
  typedef gs::socket::extension_support_base<tlm::tlm_base_protocol_types> ext_base_type;
  typedef gs::socket::bind_checker_base<tlm::tlm_base_protocol_types>      bind_checker_base_type;

  SINGLE_MEMBER_GUARDED_DATA(txn_states, gs::ext::vector_container<unsigned char>);  

  gp_peq m_fw_peq, m_bw_peq;
public:  
  /// the port to the router 
  //sc_port<GenericRouter_if<INITPORT, TARGETPORT> > router_port;
  sc_core::sc_port<GenericRouter_if<BUSWIDTH, tlm::tlm_base_protocol_types> > router_port;

  sc_core::sc_export<GenericProtocol_if<tlm::tlm_base_protocol_types> > router_target;

  /// the port to the scheduler
  sc_core::sc_port<GenericScheduler_if<tlm::tlm_base_protocol_types> > scheduler_port;
  typedef typename GenericRouter<BUSWIDTH>::sender_ids sender_ids_type;
  typedef typename GenericRouter_if<BUSWIDTH, tlm::tlm_base_protocol_types>::target_socket_type target_socket_type;
  typedef typename GenericRouter_if<BUSWIDTH, tlm::tlm_base_protocol_types>::init_socket_type init_socket_type;
  typedef typename GenericScheduler_if<tlm::tlm_base_protocol_types>::pair_type pair_type;
  //--------------------------------------------------------------------------
  /**
   * Constructor.
   */
  //--------------------------------------------------------------------------
  SimpleBusProtocol(sc_core::sc_module_name name_, uint16_t clkPeriod = (uint16_t) -1) :
    sc_core::sc_module(name_),
    ext_base_type(),
    m_fw_peq(this, &SimpleBusProtocol::fw_peq_cb, this),
    m_bw_peq(this, &SimpleBusProtocol::bw_peq_cb, this),
    router_port("router_port"),
    router_target("router_target"),
    scheduler_port("scheduler_port"),
    m_clkPeriod("m_clkPeriod", clkPeriod),
    m_idle(true)
  {
    GS_DUMP("I am a simple bus protocol.");
    m_end_req=tlm::END_REQ;
    m_begin_resp=tlm::BEGIN_RESP;
    router_target(*this);
    // assert that the m_clkPeriod is set by either the second argument or by a config file
    if (m_clkPeriod == (uint16_t) -1) {
      SC_REPORT_ERROR(name(), "The clkPeriod should be set either using the gs_param m_clkPeriod or as the second constructor parameter.");
    }
  }

  ~SimpleBusProtocol(){
  }

  void before_end_of_elaboration(){
    router_id=router_port->getRouterID();
    num_routers=router_port->getCurrentNumRouters();
    t_sock=router_port->getTargetPort();
    i_sock=router_port->getInitPort();
    GSGPSocketConfig conf;
    conf.use_wr_resp=true;
    t_sock->set_config(configure_gs_params::convert_to_GreenSocket_conf(conf));
    i_sock->set_config(configure_gs_params::convert_to_GreenSocket_conf(conf));
  }

  void start_of_simulation(){
    gs::ext::gs_extension_bindability_enum t_b_data_bind=t_sock->get_recent_config().has_phase(BEGIN_DATA);
    gs::ext::gs_extension_bindability_enum t_e_data_bind=t_sock->get_recent_config().has_phase(END_DATA);
    gs::ext::gs_extension_bindability_enum i_b_data_bind=i_sock->get_recent_config().has_phase(BEGIN_DATA);
    gs::ext::gs_extension_bindability_enum i_e_data_bind=i_sock->get_recent_config().has_phase(END_DATA);
    gs::ext::gs_extension_bindability_enum t_wr_bind=t_sock->get_recent_config().template has_extension<wr_resp_dummy>();
    gs::ext::gs_extension_bindability_enum i_wr_bind=i_sock->get_recent_config().template has_extension<wr_resp_dummy>();
    gs::ext::gs_extension_bindability_enum t_b_valid_bind=t_sock->get_recent_config().template has_extension<bytes_valid>();
    gs::ext::gs_extension_bindability_enum i_b_valid_bind=i_sock->get_recent_config().template has_extension<bytes_valid>();
    
    assert(t_b_data_bind==t_e_data_bind);
    assert(i_b_data_bind==i_e_data_bind);
    assert(t_b_data_bind==i_b_data_bind);
    assert(t_wr_bind==i_wr_bind);
    assert(t_b_valid_bind==i_b_valid_bind);
    
    if ((t_b_data_bind!=gs::ext::gs_reject) && (t_wr_bind==gs::ext::gs_reject)) has_write_resp=false;
    else has_write_resp=true;
    
    has_bytes_valid=i_b_valid_bind!=gs::ext::gs_reject; //TODO set using config
    
    GS_DUMP("The simple bus protocol will "<<((t_b_data_bind!=gs::ext::gs_reject)?"expect":"not expect")<<" data phases.");
    GS_DUMP("The simple bus protocol will "<<((has_write_resp)?"expect":"not expect")<<" write responses.");
    GS_DUMP("The simple bus protocol will "<<((has_bytes_valid)?"support":"not support")<<" transfer chunking.");
  }

  void fw_peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase, unsigned int){
    if (phase==tlm::BEGIN_REQ) {
      sender_ids_type* send_ids=ext_base_type::get_extension<sender_ids_type>(trans);
      // this is a new request, so let's schedule it for execution
      scheduler_port->enqueue(&trans, phase, send_ids->value[router_id]);
      // process the request at the next posedge clk
      GS_DUMP("got RequestValid txn --> requesting its execution for the next posedge clk from the scheduler.");
      startMasterAccessProcessingEvent.notify(sc_core::sc_time(m_clkPeriod, sc_core::SC_NS));
    }
    else {
      // this is a master who is in the data handshake phase, so forward the data to the slave
      txn_states* txn_sts;
      bool has_txn_sts=ext_base_type::get_extension<txn_states>(txn_sts, trans);
      assert(has_txn_sts);
      if ((txn_sts->value[router_id]&0x4)==0){
        tlm::tlm_phase ph=phase;
        sc_core::sc_time ti;
        GS_DUMP("send "<<ph<<" to target at 0x"<<std::hex<<trans.get_address());

        bool decode_ok = false;
        Port_id_t tar_port_num = router_port->decodeAddress(trans,
                                                            decode_ok)[0];
        if (decode_ok)
        {
          switch((*i_sock)[tar_port_num]->nb_transport_fw(trans, ph, ti))
          {
            case tlm::TLM_ACCEPTED:
            break;
            case tlm::TLM_UPDATED:
              //can only be BEGIN_DATA or END_RESP. None can be updated
              //can only be an ignorable phase
              m_bw_peq.notify(trans, ph, ti, 0);
            break;
            case tlm::TLM_COMPLETED:
              txn_sts->value[router_id] |= 0x4;
              if (phase == BEGIN_DATA)
              {
                ph = END_DATA;
                m_bw_peq.notify(trans, ph, ti, 0);
              }
            break;
          }
        }
      }
      else
        GS_DUMP("Swallowing phase "<<phase<<" because txn was already completed by target.");
      bool is_over=true;
      if (has_bytes_valid)
        is_over=ext_base_type::get_extension<bytes_valid>(trans)->value==trans.get_data_length();
      if (phase==tlm::END_RESP  //last resp accept or resp error
          && (is_over | (trans.get_response_status()!=tlm::TLM_INCOMPLETE_RESPONSE && trans.get_response_status()!=tlm::TLM_OK_RESPONSE))) {
        m_idle=true; //TODO: sc_signal?
        if ((trans.get_response_status()!=tlm::TLM_INCOMPLETE_RESPONSE && trans.get_response_status()!=tlm::TLM_OK_RESPONSE)) 
          GS_DUMP("Transaction finished due to a "<<((txn_sts->value[router_id]&0x1)?"response":"request")<<" error");
        else 
          GS_DUMP("Transaction finished due to acceptance of final response");
        GS_DUMP("Release of "<<std::hex<<&trans);
        trans.release();
        startMasterAccessProcessingEvent.notify(sc_core::sc_time(m_clkPeriod, sc_core::SC_NS)); // handle next request
      }
    }  
  }

  void bw_peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase, unsigned int){
  txn_states* txn_sts;
  bool has_txn_sts=ext_base_type::get_extension<txn_states>(txn_sts, trans);
  assert(has_txn_sts);
  #ifdef VERBOSE
    if (phase==tlm::END_REQ) txn_sts->value[router_id]|=0x1; //1 == in response cylce
  #endif
    tlm::tlm_phase ph=phase;
    sc_core::sc_time ti;
    sender_ids_type* send_ids=ext_base_type::get_extension<sender_ids_type>(trans);
    if ((txn_sts->value[router_id] & 0x2)==0){
      GS_DUMP(std::hex<<&trans<<" "<<send_ids->value.size()<<" "<<router_id);
      GS_DUMP("send "<<ph<<" to master at index"<<std::dec<<send_ids->value[router_id]);
      switch ( (*t_sock)[send_ids->value[router_id]]->nb_transport_bw(trans, ph, ti)){
        case tlm::TLM_ACCEPTED: break;
        case tlm::TLM_UPDATED: //cannot come from GSGP
          //could be END_RESP or an ignorable phase
          m_fw_peq.notify(trans, ph, ti, 0);
          break;
        case tlm::TLM_COMPLETED:
          txn_sts->value[router_id]|=0x2;
          if (phase==tlm::BEGIN_RESP) {
            ph=tlm::END_RESP;
            m_fw_peq.notify(trans, ph, ti, 0);
          }
          break;
      }
    }
    else
        GS_DUMP("Swallowing phase "<<phase<<" because txn "<<&trans<<" was already completed by init.");

    if (phase==END_DATA){
      bool is_over=!has_write_resp;
      if (has_bytes_valid)
        is_over=is_over && ext_base_type::get_extension<bytes_valid>(trans)->value==trans.get_data_length();
      if (is_over | (trans.get_response_status()!=tlm::TLM_INCOMPLETE_RESPONSE && trans.get_response_status()!=tlm::TLM_OK_RESPONSE)) {
        m_idle=true;
        if ((trans.get_response_status()!=tlm::TLM_INCOMPLETE_RESPONSE && trans.get_response_status()!=tlm::TLM_OK_RESPONSE)) 
          GS_DUMP("Transaction finished due to a data error");
        else 
          GS_DUMP("Transaction finished due to acceptance of final data phase");        
        trans.release();
        startMasterAccessProcessingEvent.notify(sc_core::sc_time(m_clkPeriod, sc_core::SC_NS)); // handle next request
      }
    }
  }

  //--------------------------------------------------------------------------
  /**
   * A master accesses the channel
   */
  //--------------------------------------------------------------------------
  virtual sync_enum_type registerMasterAccess(unsigned int from, payload_type& txn, phase_type& ph, sc_core::sc_time& time){
    GS_DUMP("Incoming call from init "<<from<<" ph="<<ph<<" time="<<time);
    txn_states* txn_sts;
    if (!ext_base_type::get_extension<txn_states>(txn_sts, txn)){ //we are the first router that sees this new txn, so we reset the states
      if (txn_sts->value.size()<num_routers) txn_sts->value.resize(num_routers);
      for (unsigned int i=0; i<num_routers; i++) txn_sts->value[i]=0;
      ext_base_type::validate_extension<txn_states>(txn);
    }
    switch (ph){
    case tlm::BEGIN_REQ:{
      sender_ids_type* send_ids=ext_base_type::get_extension<sender_ids_type>(txn);
      if (send_ids->value.size()<=router_id) send_ids->value.resize(router_id+1);
      send_ids->value[router_id]=from;
      GS_DUMP("Acquisition of "<<std::hex<<&txn);
      txn.acquire();}
    case tlm::END_RESP:
      m_fw_peq.notify(txn, ph, time, 0);
      return tlm::TLM_ACCEPTED;
    default:
      if (ph==BEGIN_DATA){
        m_fw_peq.notify(txn, ph, time, 0);
        return tlm::TLM_ACCEPTED;
      }
    }
    //in case of phases that are outside of what we allow (GSGP or BP)
    // we assume they are ignorable and we send them directly to the other side
    if ((txn_sts->value[router_id] & 0x4)==0){ //txn is not complete on target side      
      //This could even happen before a BEGIN_REQ so we gotta store the sender id
      GS_DUMP("Forwarding ignorable phase "<<ph<<" to target");
      sender_ids_type* send_ids=ext_base_type::get_extension<sender_ids_type>(txn);
      if (send_ids->value.size()<=router_id) send_ids->value.resize(router_id+1);
      send_ids->value[router_id]=from;
      bool decode_ok;
      Port_id_t tar_port_num = router_port->decodeAddress(txn, decode_ok)[0];
      if (decode_ok)
      {
        return (*i_sock)[tar_port_num]->nb_transport_fw(txn, ph, time);
      }
      else
      {
        //XXX: not sure this is good.
        return tlm::TLM_ACCEPTED;
      }
    }
    GS_DUMP("Ignoring ignorable phase "<<ph<<" because the txn was already completed by the target.");
    return tlm::TLM_ACCEPTED;
  }

  //--------------------------------------------------------------------------
  /**
   * A master's request is to be processed now.
   */
  //--------------------------------------------------------------------------
  virtual bool processMasterAccess() 
  {
    // if bus is idle, check for new request
    if (m_idle && scheduler_port->isPending()) {
      m_idle =false;
      // get next pending request from scheduler...
      pair_type tp = scheduler_port->dequeue();
      tlm::tlm_phase ph=tp.second;
      sc_core::sc_time ti;
      GS_DUMP("send "<<ph<<" to target at 0x"<<std::hex<<tp.first->get_address());

      bool decode_ok;
      Port_id_t tar_port_num = router_port->decodeAddress(*(tp.first),
                                                          decode_ok)[0];

      if (decode_ok)
      {
        switch( (*i_sock)[tar_port_num]->nb_transport_fw(*tp.first, ph, ti)){
          case tlm::TLM_ACCEPTED: break;
          case tlm::TLM_UPDATED:
            m_bw_peq.notify(*tp.first, ph, ti, 0);
            break;
          case tlm::TLM_COMPLETED:
            if ((tp.first->get_command()==tlm::TLM_WRITE_COMMAND && has_write_resp) || tp.first->get_command()==tlm::TLM_READ_COMMAND)
              m_bw_peq.notify(*tp.first, m_begin_resp, ti, 0);
            else
              m_bw_peq.notify(*tp.first, m_end_req, ti, 0);
            txn_states* txn_sts;
            bool has_txn_sts=ext_base_type::get_extension<txn_states>(txn_sts, *tp.first);
            assert(has_txn_sts);
            txn_sts->value[router_id]|=0x4;
        }
        return true;
      }
      else
      {
        return false;
      }
    }
    return false;
  }


  //--------------------------------------------------------------------------
  /**
   * A slave accesses the channel
   */
  //--------------------------------------------------------------------------
  virtual sync_enum_type registerSlaveAccess(unsigned int from, payload_type& txn, phase_type& ph, sc_core::sc_time& time)
  {
    GS_DUMP("Incoming call from target "<<from<<" ph="<<ph<<" time="<<time);
    switch(ph){
      case tlm::END_REQ:
      case tlm::BEGIN_RESP:
        m_bw_peq.notify(txn, ph, time, 0);
        return tlm::TLM_ACCEPTED;
      default:
        if (ph==END_DATA){
          m_bw_peq.notify(txn, ph, time, 0);
          return tlm::TLM_ACCEPTED;
        }
    }
    txn_states* txn_sts;
    bool has_txn_sts=ext_base_type::get_extension<txn_states>(txn_sts, txn);
    assert(has_txn_sts);
    
    //in case of phases that are outside of what we allow (GSGP or BP)
    // we assume they are ignorable and we send them directly to the other side
    if ((txn_sts->value[router_id] & 0x2)==0){ //txn is not complete on init side      
      //This could even happen before a BEGIN_REQ so we gotta store the sender id
      GS_DUMP("Forwarding ignorable phase "<<ph<<" to init");
      sender_ids_type* send_ids=ext_base_type::get_extension<sender_ids_type>(txn);
      return (*t_sock)[send_ids->value[router_id]]->nb_transport_bw(txn, ph, time);
    }
    GS_DUMP("Ignoring ignorable phase "<<ph<<" because the txn was already completed by the init.");
    return tlm::TLM_ACCEPTED;
  }
  //--------------------------------------------------------------------------
  /**
   * A slave's request should be processed now.
   */
  //--------------------------------------------------------------------------
  virtual bool processSlaveAccess() 
  {
    // this method can be used for implementing advanced protocol functions, 
    // e.g. timeout handling
    return true;
  }
  
  

  virtual void before_b_transport(unsigned int, payload_type&, sc_core::sc_time&){
    GS_DUMP("Doing nothing to b_transport txn");
  }
  
  virtual void invalidate_direct_mem_ptr(unsigned int, sc_dt::uint64, sc_dt::uint64){
    //GS_DUMP("I do not support DMI");
    //exit(1);
    GS_DUMP("I now support DMI");
  }
  
  virtual unsigned int transport_dbg(unsigned int, payload_type& trans){
    GS_DUMP("I do not support debug transport");
    exit(1);
  }
  
  virtual bool get_direct_mem_ptr(unsigned int, payload_type& trans, tlm::tlm_dmi&  dmi_data){
    //GS_DUMP("I do not support DMI");
    GS_DUMP("I now support DMI");
    //exit(1);
    return true;
  }
  
  virtual bool assignProcessMasterAccessSensitivity(sc_core::sc_spawn_options& opts){
    GS_DUMP("assignProcessMasterAccessSensitivity triggered.");
    opts.set_sensitivity(&startMasterAccessProcessingEvent);
    return true;
  }
  
  virtual bool assignProcessSlaveAccessSensitivity(sc_core::sc_spawn_options& opts){
    return false;
  }


private:
  sc_core::sc_event_queue startMasterAccessProcessingEvent;
  tlm::tlm_phase m_end_req, m_begin_resp;
  unsigned int router_id, num_routers;
  target_socket_type* t_sock;
  init_socket_type*   i_sock;  
  /// clock period duration
  gs::gs_param<uint16_t> m_clkPeriod;

  /// bus state
  bool m_idle, has_write_resp, has_bytes_valid;

};
 
}
}

#endif
