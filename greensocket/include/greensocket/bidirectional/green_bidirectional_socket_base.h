//   GreenSocket
//
// LICENSETEXT
//
//   Copyright (C) 2008 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//    Robert Guenzel <guenzel@eis.cs.tu-bs.de>, Christian Schroeder
//
//
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT


#ifndef __GREEN_BIDIRECTIONAL_SOCKET_BASE_H__
#define __GREEN_BIDIRECTIONAL_SOCKET_BASE_H__

#include <vector>
#include "greensocket/generic/gs_extension.h"
#include "greensocket/generic/gs_callbacks.h"
#include "greensocket/generic/green_initiator_socket_callback_base.h"
#include "greensocket/generic/green_target_socket_callback_base.h"
#include "greensocket/generic/green_socket_bind_checker.h"
#include "greensocket/generic/green_socket_mm_base.h"
#include "greensocket/generic/green_socket_default_pool.h"
#include "greencontrol/core/gs_sc_api_detection.h"
#include "greensocket/utils/gs_msg_output.h"

namespace gs {
namespace socket {


/// This is the green_bidirectional_socket from which other bidirectional_sockets
/// shall be derived
template <unsigned int BUSWIDTH=32,
          typename TRAITS=tlm::tlm_base_protocol_types,
          unsigned int N=0, 
          typename BIND_BASE=bind_checker<TRAITS>,
          typename MM_INTERFACE=tlm::tlm_mm_interface,
          typename EXT_FILLER=mm_base<TRAITS,MM_INTERFACE>,
          typename POOL=default_pool<TRAITS, EXT_FILLER>
          >
class bidirectional_socket_base: 
                             public initiator_socket_callback_base<BUSWIDTH,TRAITS, N, typename BIND_BASE::root_type>
                            ,public target_socket_callback_base<BUSWIDTH,TRAITS, N, typename BIND_BASE::root_type>
                            ,public BIND_BASE
                            //,public BIND_BASE::ext_support_type
                            ,public mm_base<TRAITS,MM_INTERFACE>
{
public:
  typedef TRAITS                                                       traits_type;
  typedef initiator_socket_callback_base<BUSWIDTH,TRAITS,N, typename BIND_BASE::root_type>            init_base_type; // equivalent to base_type
  typedef typename init_base_type::base_target_socket_type             init_base_target_socket_type;
  typedef target_socket_callback_base<BUSWIDTH,TRAITS,N, typename BIND_BASE::root_type>               target_base_type;
  typedef typename TRAITS::tlm_payload_type                            payload_type;
  typedef typename TRAITS::tlm_phase_type                              phase_type;
  typedef tlm::tlm_sync_enum                                           sync_enum_type;
  //typedef typename BIND_BASE::ext_support_type                  ext_support_type;
  typedef mm_base<TRAITS,MM_INTERFACE>              mm_base_type;
  typedef BIND_BASE bind_checker_type;
protected:
  typedef bidirectional_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, MM_INTERFACE, EXT_FILLER, POOL> my_type;

protected:
  /// Accessor class to access functions of initiator and target ports
  /**
   * TODO: this class may need to be extended with further functions being disjuct 
   *       between init and target socket classes.
   */
  class SubPortAccessor {
  public:
    SubPortAccessor(my_type *owner)
    : m_owner(owner) { }
    // initiator socket functions
    sync_enum_type nb_transport_fw(typename TRAITS::tlm_payload_type& txn,
                                   typename TRAITS::tlm_phase_type& p,
                                   sc_core::sc_time& t) { return m_owner->fw[index]->nb_transport_fw(txn, p, t); }
    void b_transport(typename TRAITS::tlm_payload_type& trans,sc_core::sc_time& t) { return m_owner->fw[index]->b_transport(trans, t); }

    // target socket functions
    sync_enum_type nb_transport_bw(typename TRAITS::tlm_payload_type& txn,
                                   typename TRAITS::tlm_phase_type& p,
                                   sc_core::sc_time& t) { return m_owner->bw[index]->nb_transport_bw(txn,p, t); }
    unsigned int index;
  protected:
    my_type* m_owner;
  };
  
  /// Accessor class to access operators of target
  class SubTargetPortAccessor {
  public:
    SubTargetPortAccessor(target_base_type *target)
    : m_target_base(target) { }
    tlm::tlm_bw_transport_if<TRAITS>* operator[](int i){return (*m_target_base)[i];}
    tlm::tlm_bw_transport_if<TRAITS>* operator->(){return (*m_target_base).operator->();}
  protected:
    target_base_type* m_target_base;
  };
  /// Accessor class to access operators of initiator
  class SubInitPortAccessor {
  public:
    SubInitPortAccessor(init_base_type *init)
    : m_init_base(init) { }
    tlm::tlm_fw_transport_if<TRAITS>* operator[](int i){return (*m_init_base)[i];}
    tlm::tlm_fw_transport_if<TRAITS>* operator->(){return (*m_init_base).operator->();}
  protected:
    init_base_type* m_init_base;
  };
  
public:
  //constructor requires an name. The last argument is for future use and shall not be used.
  bidirectional_socket_base(const char* name, allocation_scheme alloc_scheme=GS_TXN_ONLY, EXT_FILLER* ext_filler=NULL);
  ~bidirectional_socket_base();
  
  //get a memory managed transaction from the pool.
  // Its ref count is already set to 1
  payload_type* get_transaction();

  //Return a memory managed transaction to the pool.
  // It shall be called when an initiator has no longer use for a transaction
  void release_transaction(payload_type* txn);
/*   
  //Register a callback for the nb_transport_bw TLM2 interface method
  template <typename MODULE>
  void register_nb_transport_bw(MODULE* mod, sync_enum_type (MODULE::*cb)(payload_type& trans, phase_type& phase, sc_core::sc_time& t));

  //Register a callback for the invalidate_dmi TLM2 interface method
  template <typename MODULE>
  void register_invalidate_dmi(MODULE* mod, void (MODULE::*cb)(sc_dt::uint64 start_range, sc_dt::uint64 end_range));
*/
  //test if a transaction is from the pool of this socket (for debug only. This call is SLOW!!)
  bool is_from(payload_type* gp){return m_pool->is_from(gp);}
  
  //bind against a bidirectional socket
  template<unsigned int M, typename _EXT_FILLER, typename _POOL>
  void bind(bidirectional_socket_base<BUSWIDTH, TRAITS, M, BIND_BASE, MM_INTERFACE, _EXT_FILLER, _POOL>& s);
  
  //operator notation for direct bind
  template<unsigned int M, typename _EXT_FILLER, typename _POOL>
  void operator() (bidirectional_socket_base<BUSWIDTH, TRAITS, M, BIND_BASE, MM_INTERFACE, _EXT_FILLER, _POOL>& s);
  
  //bind hierarchically to the given inner socket
  template<unsigned int M, typename _EXT_FILLER, typename _POOL>
  void bind_to_inner(bidirectional_socket_base<BUSWIDTH, TRAITS, M, BIND_BASE, MM_INTERFACE, _EXT_FILLER, _POOL>& s);
  
  // ///////// functions handling calls to initiator/target_socket_callback_bases /////////// //
  
  //simple helpers for warnings an errors to shorten in code notation
  void display_warning(const std::string& text) { init_base_type::display_warning(text); }
  void display_error(const std::string& text) { init_base_type::display_error(text); }
  // non-ambiguous functions: 
  // target:    register_nb_transport_fw, register_b_transport, register_transport_dbg, register_get_direct_mem_ptr
  // initiator: register_nb_transport_bw, register_invalidate_direct_mem_ptr

  // NOTE: this is only valid at end of elaboration!
  unsigned int size();
  
  //SystemC standard callback before end of elaboration
  void before_end_of_elaboration();

  void start_of_simulation();
  
  const char* name() const;
  
  /// Returns the parent of the socket - which should be the same for init and target socket
  virtual sc_core::sc_object* get_parent();
#if SYSTEMC_API == 210
  //sc_core::sc_object* get_parent() { return init_base_type::get_parent(); }
#else
  sc_core::sc_object* get_parent_object() { return init_base_type::get_parent_object(); }
#endif
  
  /// Get one of the base sc_objects (the init port one)
  /**
   * @return Init port's base sc_object
   */
  sc_core::sc_object& get_sc_object()
  {
    return static_cast<sc_core::sc_object&>(static_cast<init_base_type&>(*this));
  }    
  
public:
  // operators retuning the SubPortAccessor object which resolves the ambiguousness for some functions
  // For not supported functions (user:) use bw and fw objects (developer:) add to SubPortAccessor class
  SubPortAccessor* operator[](int i){ m_port_accessor.index = i; return &m_port_accessor; }
  SubPortAccessor* operator->()     { m_port_accessor.index = 0; return &m_port_accessor; }
  // Accessor object for target port operators [] and ->
  SubTargetPortAccessor bw;
  SubInitPortAccessor fw;
  
protected:

  //ATTENTION: derived socket should not implement end_of_elaboration
  // if they do, they have to call this end_of_elaboration before anything else
  void end_of_elaboration();
  
  virtual void free(payload_type* txn);
  
  //this function is called when we were successfully bound to someone
  // override it to react to the resolved config
  virtual void bound_to(const std::string&, typename BIND_BASE::bind_base_type*, unsigned int);
  
  //override this function when deriving from the greensocket
  virtual const std::string& get_type_string();

  
  //virtual unsigned int get_num_bindings();

  const static std::string s_kind;
  POOL* m_pool;
  EXT_FILLER * m_filler;
  bool base_eoe_done; // target socket part
  
  std::string m_name;
  
  SubPortAccessor m_port_accessor;
  
  //nb_transport_functor<TRAITS> m_nb_bw_functor;
  //invalidate_direct_mem_ptr_functor<TRAITS> m_inval_dmi_functor;

};

} //end ns socket
} //end ns gs

#include "greensocket/bidirectional/green_bidirectional_socket_base.tpp"
#endif
