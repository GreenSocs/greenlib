// LICENSETEXT
//
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//
//   Wolfgang Klingauf, Robert Guenzel
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

#ifndef __INCLUDED_BY_GENERIC_ROUTER_B_H__
# error "DummyProtocol.h may never be included directly!"
#endif


template<unsigned int BUSWIDTH, typename TRAITS>
class DummyProtocol
  : public GenericProtocol_if<TRAITS>
  , public sc_core::sc_module
{
  typedef typename TRAITS::tlm_payload_type              payload_type;
  typedef typename TRAITS::tlm_phase_type                phase_type;  
  typedef tlm::tlm_sync_enum                            sync_enum_type;
  typedef typename GenericRouter_if<BUSWIDTH, TRAITS>::target_socket_type target_socket_type;
  typedef typename GenericRouter_if<BUSWIDTH, TRAITS>::init_socket_type init_socket_type;
  typedef typename GenericRouter_if<BUSWIDTH,TRAITS>::sender_ids sender_ids_type;

public:
  
  sc_core::sc_port<GenericRouter_if<BUSWIDTH,TRAITS> > router_port;

  
  DummyProtocol(sc_core::sc_module_name name_)
    : sc_core::sc_module(name_)
    , router_port("router_port")
    , router_id(0)
  {
      GS_DUMP("I am a dummy protocol.");
  }
  
  void before_end_of_elaboration(){
    router_id=router_port->getRouterID();
    t_sock=router_port->getTargetPort();
    i_sock=router_port->getInitPort();
    gs::socket::config<TRAITS> conf;
    conf.treat_unknown_as_optional();
    if (t_sock) t_sock->set_config(conf);
    if (i_sock) i_sock->set_config(conf);
  }
  
  virtual sync_enum_type registerMasterAccess(unsigned int from,
                                              payload_type& txn, phase_type& ph,
                                              sc_core::sc_time& time)
  {
    sender_ids_type* send_ids=t_sock->template get_extension<sender_ids_type>(txn);
    if (send_ids->value.size()<=router_id) send_ids->value.resize(router_id+1);
    send_ids->value[router_id]=from; //gotta set it every time, since we are protocol agnostic

    bool decode_ok = false;
    Port_id_t tar_port_num = router_port->decodeAddress(txn, decode_ok)[0];

    if (decode_ok)
    {
      return (*i_sock)[tar_port_num]->nb_transport_fw(txn, ph, time);
    }
    else
    {
      /*
       * FIXME: Need to update the txn..
       */
      return tlm::TLM_UPDATED;
    }
  }

  virtual sync_enum_type registerSlaveAccess(unsigned int, payload_type& txn, phase_type& ph, sc_core::sc_time& time){
    sender_ids_type* send_ids=t_sock->template get_extension<sender_ids_type>(txn);
    return (*t_sock)[send_ids->value[router_id]]->nb_transport_bw(txn, ph, time);
  }

  virtual bool processMasterAccess() {
    return true;
  }
  
  virtual bool processSlaveAccess() {
    return true;
  }

  virtual void before_b_transport(unsigned int, payload_type&, sc_core::sc_time&){
    GS_DUMP("Doing nothing to b_transport txn");
  }
  
  virtual void invalidate_direct_mem_ptr(unsigned int, sc_dt::uint64, sc_dt::uint64){
    GS_DUMP("I do not support DMI");
    exit(1);
  }
  
  virtual unsigned int transport_dbg(unsigned int, payload_type& trans){
    GS_DUMP("I do not support debug transport");
    exit(1);
  }
  
  virtual bool get_direct_mem_ptr(unsigned int, payload_type& trans, tlm::tlm_dmi&  dmi_data){
    GS_DUMP("I do not support DMI");
    exit(1);
    return false;
  }
  
  virtual bool assignProcessMasterAccessSensitivity(sc_core::sc_spawn_options& opts){return false;}
  virtual bool assignProcessSlaveAccessSensitivity(sc_core::sc_spawn_options& opts){return false;}
  
protected:
  unsigned int router_id;
  target_socket_type* t_sock;
  init_socket_type*   i_sock;  
};

