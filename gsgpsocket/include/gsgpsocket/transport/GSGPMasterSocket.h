// LICENSETEXT
// 
//   Copyright (C) 2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schröder, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
// 
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT

#ifndef __GSGPMASTERSOCKET_H__
#define __GSGPMASTERSOCKET_H__

#include "gsgpsocket/transport/GSGPSocket.h"
#include "greensocket/initiator/multi_socket.h"
#include "greensocket/bidirectional/multi_socket.h"
#include "gsgpsocket/utils/pseudo_port.h"
#include "greencontrol/gcnf/apis/utils/gs_param_sync.h"

#define GSGP_MASTER_SOCKET_TYPE_NAME "GSGPMasterSocket"

namespace gs {
  namespace gp {
    
    typedef gs::unevenpair<GenericMasterAccessHandle, GenericPhase> master_atom;
    typedef gs::unevenpair<GenericBidirectionalAccessHandle, GenericPhase> bidir_atom;
    
    // /////////////////////////////////////////////////////////////////////////////// //
    // //////// Bidir type wrapper struct for Master and Slave sockets /////////////// //
    
    template <bool BIDIR, unsigned int BUSWIDTH>
    struct bidir_master_type_wrapper { };
    
    template < unsigned int BUSWIDTH >
    struct bidir_master_type_wrapper<false, BUSWIDTH> {
      /// Socket type dependent on bidir template parameter
      typedef typename gs::socket::initiator_multi_socket<BUSWIDTH> sock_type;
    };
    template < unsigned int BUSWIDTH >
    struct bidir_master_type_wrapper<true, BUSWIDTH> {
      /// Socket type dependent on bidir template parameter
      typedef typename gs::socket::bidirectional_multi_socket<BUSWIDTH> sock_type;
    };
    
    // /////////////////////////////////////////////////////////////////////////////// //
    // /////////////////////////////////////////////////////////////////////////////// //
    
    template <unsigned int BUSWIDTH, typename TRANSACTION, typename CONFIG, typename PHASE,
    bool BIDIR, typename SOCK_TYPE>
    class GSGPMasterSocket;
    
    // ///////// structs for GSGPMasterSocket calls ///////////////// //

    GSGP_CREATE_BIG_FUNCTOR_HEADER(RequestObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(SendDataObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(AckResponseObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(ErrorResponseObj);
    
    // ///////////////////  GenericInitatorSocket  ////////////////////// //
    // /////////////////  optionally bidirectional  ///////////////////// //
    
    /// GSGP Master Socket with optional switch for bidirectional port
    /**
     * - template parameter BIDIR: if to use bidirectional GreenSocket sockets
     * - _never_ set the last template parameter SOCK_TYPE!
     */
    template <unsigned int BUSWIDTH, typename TRANSACTION, typename CONFIG, typename PHASE, 
    bool BIDIR = false, typename SOCK_TYPE = typename bidir_master_type_wrapper<BIDIR, BUSWIDTH>::sock_type >
    class GSGPMasterSocket
    : virtual public GSGPSocket_base< SOCK_TYPE, CONFIG >
    {
    public:
      typedef TRANSACTION transaction;
      typedef Handle<TRANSACTION> accessHandle;
      typedef PHASE  phase;
      typedef Handle<TRANSACTION> created_transaction_handle;
      typedef TRANSACTION created_transaction;  
      typedef gs::unevenpair<accessHandle, phase> atom; // most time this is master_atom or bidir_atom
      typedef CONFIG config_type;
    protected:
      typedef GSGPMasterSocket<BUSWIDTH, TRANSACTION, CONFIG, PHASE, BIDIR, SOCK_TYPE> my_type;
      typedef SOCK_TYPE socket_type;
      typedef GSGPSocket_base<socket_type, CONFIG> base_socket_type;
      typedef base_socket_type virtual_base_type; // type whose constructor needs to be called by derived classes

      /// peq dummy struct (member peq) that represents the peq, real peq in this slave is the m_fw_peq
      struct peq_struct {
        /// port which the user needs to bind ONE of them to to receive nb calls (notify(atom))
        //sc_core::sc_port<payload_event_queue_output_if<atom> > out_port;
        pseudo_port<payload_event_queue_output_if<atom> > out_port;
        pseudo_port<payload_event_queue_multi_output_if<atom> > out_multi_port;
      };

      /// Unique number of this socket
      gs_handle MasterPortNumber;
      
      /// The (only) phase object that is passed to the user
      PHASE m_phase;

      // allow these objects to access the protected function socket_name
      friend class Socket_name_proxy<my_type>;
      
      // target address param syncronizer (used when hierarchical binding)
      gs::gs_param_sync<gs_uint64> target_addr_sync;

    public:

      /// Payload event queue dummy for legacy support (user module binds to its out_port)
      peq_struct peq;

      /// PEQ which delays the transactions befor being called to the user (does automatic copy of fields valid only at call time)
      gp_peq m_bw_peq;
      
      /// configurable target address of this initiator port
      gs_param<gs_uint64> target_addr;  
      /// Deprecated: data width of this port
      const gs_uint32 data_width; // read-only! initialize to template param
      
      /// GSGP Master Socket constructor
      /**
       * Note: is_bidirectional now needs to be set as template parameter BIDIR
       * @param port_name  Name for underlying port, forwarded in constructor
       * @param socket_type_name  Socket type name with default (="GSGPMasterSocket"), modified e.g. by Bidirectional socket
       */
      GSGPMasterSocket (const char* port_name, 
                        const std::string socket_type_name = GSGP_MASTER_SOCKET_TYPE_NAME )
      : virtual_base_type(port_name, socket_type_name)
      , m_bw_peq(this, &my_type::bw_peq_cb, this)
      , target_addr(std::string(port_name) + ".target_addr")
      , data_width(BUSWIDTH)
      , Request(*this)
      , SendData(*this)
      , AckResponse(*this)
      , ErrorResponse(*this)
      { 
        // init m_name
        base_socket_type::socket_name();

        // register callback function to this Socket, see start_of_simulation

        // use in-port pointer address as unique master ID (this is important to make genericRouter work)
        MasterPortNumber = (gs_handle) this;
      }
      
      /// Destructor
      ~GSGPMasterSocket() { }
      
      /// master sends a request atom
      RequestObj<my_type, PHASE>          Request;
      /// master sends a data atom
      SendDataObj<my_type, PHASE>         SendData;
      /// master acknowledges a slave response
      AckResponseObj<my_type, PHASE>      AckResponse;
      /// master signals an error on a slave response
      ErrorResponseObj<my_type, PHASE>    ErrorResponse;
      
      /**
       * PV blocking transact method. It is used in PV mode 
       * to send a whole transaction container at once.
       * @param index  Outgoing socket of the multi socket.
       */
      void Transact(accessHandle t, unsigned int index = 0);
      
      /**
       * PV blocking temporal decoupled transact method. It is used in PV mode 
       * to send a whole transaction container at once.
       * @param index  Outgoing socket of the multi socket.
       */
      void Transact(accessHandle t, sc_core::sc_time& td, unsigned int index = 0);
      
      /// Create a transaction
      accessHandle create_transaction();

      /// Create a transaction of template type
      template<typename ACCESS_TYPE>
      ACCESS_TYPE create_transaction();

      /// Needs to be called after a transaction
      void release_transaction(accessHandle &th);
      
      gs_handle get_master_port_number();

      /// Registers the callback according to the configuration
      virtual void start_of_simulation();

      /// Callback function for NON-BLOCKING backward path (called by GreenSocket)
      /**
       * If connected to GSGPSocket, m_is_connected_to_GPSocket_vec[index] == true   <br>
       *    Forwards the call to the internal tlm peq.
       * else                                                              <br>
       *    Handles conversions from other sockets to GSGPSocket
       *    and (in some cases) forwards the call to the internal tlm peq.
       */
      tlm::tlm_sync_enum bw_nb_transport_cb(unsigned int index,
                                            tlm::tlm_generic_payload& tlmtr, 
                                            tlm::tlm_phase& tph,
                                            sc_core::sc_time& t);
      
      /// Forwards the transaction to the local peq (calls notify) after having acquired (the callback function will release)
      inline void enqueue_to_bw_peq(tlm::tlm_generic_payload& tlmtr, tlm::tlm_phase& tph, const sc_core::sc_time& delay, unsigned int index);

      /// Callback function called on backward path called by peq (either initiated by target or the return path)
      void bw_peq_cb(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr, 
                  const tlm::tlm_base_protocol_types::tlm_phase_type& tph, unsigned int index);

    };

    /// User class for a standard GSGP Master Socket
    template<unsigned int BUSWIDTH>
    class GenericMasterPort
    : public GSGPMasterSocket<BUSWIDTH, GenericMasterAccess, GSGPSocketConfig, GenericPhase, false> {
    protected:
      typedef GSGPMasterSocket<BUSWIDTH, GenericMasterAccess, GSGPSocketConfig, GenericPhase> base_socket_type;
      typedef typename base_socket_type::virtual_base_type virtual_base_type; // type whose constructor needs to be called by derived classes
    public:
      // is_bidirectional now needs to be set as template parameter BIDIR
      GenericMasterPort( const char* port_name )
      : virtual_base_type(port_name, GSGP_MASTER_SOCKET_TYPE_NAME)
      , GSGPMasterSocket<BUSWIDTH, GenericMasterAccess, GSGPSocketConfig, GenericPhase, false>(port_name)
      { }
    };

    /**
     * Create a unique number (there may be better ways to implement this...)
     */
    class UniqueNumber {
    public:
      static inline gs_uint64 get() 
      {
        static gs_uint64 unique_number = 0;
        return unique_number++;
      }
    };
    
#include "gsgpsocket/transport/GSGPMasterSocket.tpp"
    
    /**
     * Deprecated: Helper function to get master access handle from a master_atom
     */
    inline GenericMasterAccessHandle _getMasterAccessHandle(/*const*/ master_atom &tc) { return tc.first; }
    
    /**
     * Deprectated: Helper function to get master access handle from a GenericTransaction handle (formerly used in b_transact)
     */
    inline GenericMasterAccessHandle _getMasterAccessHandle(const GenericMasterAccessHandle &t) { return t; }
    
    /**
     * Helper function to get the phase from a atom
     */
    inline GenericPhase _getPhase(const master_atom &tc) { return tc.second; }
    
  } // end namespace gp
} // end namespace gs


#endif
