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

#ifndef __GSGPSLAVESOCKET_H__
#define __GSGPSLAVESOCKET_H__

#include <iostream>

#include "gsgpsocket/transport/GSGPSocket.h"
#include "greensocket/target/multi_socket.h"
#include "greensocket/bidirectional/multi_socket.h"
#include "greensocket/bidirectional/single_socket.h"
#include "tlm_utils/instance_specific_extensions.h"
#include "gsgpsocket/core/tlm_b_if.h"
#include "gsgpsocket/core/tlm_f_if.h"
#include "greencontrol/config.h"
#include "gsgpsocket/utils/pseudo_port.h"
#include "greensocket/utils/greensocketaddress_base.h"
#include "greencontrol/gcnf/apis/utils/gs_param_sync.h"

#define GSGP_SLAVE_SOCKET_TYPE_NAME "GSGPSlaveSocket"

namespace gs {
  namespace gp {
    
    template <unsigned int BUSWIDTH, typename TRANSACTION, typename CONFIG, typename PHASE,
    bool BIDIR, typename SOCK_TYPE>
    class GenericTargetSocket;
    
    typedef gs::unevenpair<GenericSlaveAccessHandle, GenericPhase> slave_atom;

    // ///////// structs for GPSlaveSocket calls ///////////////// //
        
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(AckRequestObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(ErrorRequestObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(AckDataObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(ErrorDataObj);
    GSGP_CREATE_SMALL_FUNCTOR_HEADER(ResponseObj);
    
    // ////////////  Base class for address parameters  /////////////// //
    
    // /////////////////////////////////////////////////////////////////////////////// //
    // //////// Bidir type wrapper struct for Master and Slave sockets /////////////// //
    
    template <bool BIDIR, unsigned int BUSWIDTH>
    struct bidir_slave_type_wrapper { };
    
    template < unsigned int BUSWIDTH >
    struct bidir_slave_type_wrapper<false, BUSWIDTH> {
      /// Socket type dependent on bidir template parameter
      typedef typename gs::socket::target_multi_socket<BUSWIDTH> sock_type;
    };
    template < unsigned int BUSWIDTH >
    struct bidir_slave_type_wrapper<true, BUSWIDTH> {
      /// Socket type dependent on bidir template parameter
      typedef typename gs::socket::bidirectional_multi_socket<BUSWIDTH> sock_type;
    };
    

    // ///////////////////  GenericTargetSocket  ////////////////////// //
    // //////////////////  optionally bidirectional /////////////////// //
    /// GSGP Slave Socket with optional switch for bidirectional port
    /**
     * - template parameter BIDIR: if to use bidirectional GreenSocket sockets
     * - _never_ set the last template parameter SOCK_TYPE!
     */
    template <unsigned int BUSWIDTH, typename TRANSACTION, typename CONFIG, typename PHASE,
    bool BIDIR = false, typename SOCK_TYPE = typename bidir_slave_type_wrapper<BIDIR, BUSWIDTH>::sock_type>
    class GSGPSlaveSocket
    : virtual public GSGPSocket_base< SOCK_TYPE, CONFIG >
    , public gs::socket::GreenSocketAddress_base
      {
      public:        
        typedef TRANSACTION transaction;
        typedef Handle<TRANSACTION> accessHandle;
        typedef PHASE  phase;
        typedef Handle<TRANSACTION> created_transaction_handle;
        typedef TRANSACTION created_transaction;  
        typedef gs::unevenpair<accessHandle, phase> atom; // most time this is slave_atom or bidir_atom
        typedef CONFIG config_type;
      protected:
        typedef GSGPSlaveSocket<BUSWIDTH, TRANSACTION, CONFIG, PHASE, BIDIR, SOCK_TYPE> my_type;
        typedef SOCK_TYPE socket_type;
        typedef GSGPSocket_base<socket_type, CONFIG> base_socket_type;
        typedef base_socket_type virtual_base_type; // type whose constructor needs to be called by derived classes
        using base_socket_type::at_mode;

        typedef tlm_b_if<accessHandle> b_if_type;
        typedef tlm_multi_b_if<accessHandle> multi_b_if_type;
        typedef tlm_td_b_if<accessHandle> td_b_if_type;
        typedef tlm_td_multi_b_if<accessHandle> td_multi_b_if_type;
        typedef tlm_f_if<accessHandle> f_if_type;
        typedef tlm_multi_f_if<accessHandle> multi_f_if_type;
        typedef payload_event_queue_if<atom> peq_if_type;
        
        /// peq dummy struct (member peq) that represents the peq, real peq in this slave is the m_fw_peq
        struct peq_struct {
          /// port which the user needs to bind ONE of them to to receive nb calls (notify(atom))
          //sc_core::sc_port<payload_event_queue_output_if<atom> > out_port;
          pseudo_port<payload_event_queue_output_if<atom> > out_port;
          pseudo_port<payload_event_queue_multi_output_if<atom> > out_multi_port;
        };
        
        /// PEQ which delays the transactions befor being called to the user (does automatic copy of fields valid only at call time)
        gp_peq m_fw_peq;

        /// number of outstanding non-blocking transactions
        unsigned m_outstanding_nb_count;

        /// signals when there is no more outstanding non-blocking transactions
        sc_core::sc_event m_outstanding_nb_drained;

        /// The (only) phase object that is passed to the user
        PHASE m_phase;

        // allow these objects to access the protected function socket_name
        friend class Socket_name_proxy<my_type>;

        // address range param syncronizers (used when hierarchical binding)
        gs::gs_param_sync<gs_uint64> base_addr_sync;
        gs::gs_param_sync<gs_uint64> high_addr_sync;

      public:

        /// Deprecated: data width of this port
        const gs_uint32 data_width; // read-only! initialize to template param
        
        /// Payload event queue dummy for legacy support (user module binds to its out_port)
        peq_struct peq;
        
        //sc_core::sc_export<b_if_type> b_in;
        pseudo_port<b_if_type> b_in;
        pseudo_port<multi_b_if_type> multi_b_in;
        pseudo_port<td_b_if_type> td_b_in;
        pseudo_port<td_multi_b_if_type> td_multi_b_in;
        
        pseudo_port<f_if_type> f_in;
        pseudo_port<multi_f_if_type> multi_f_in;
        
        /// GSGP Slave Socket constructor
        /**
         * Note: is_bidirectional now needs to be set as template parameter BIDIR
         * @param port_name  Name for underlying port, forwarded in constructor
         * @param socket_type_name  Socket type name with default (="GSGPSlaveSocket"), modified e.g. by Bidirectional socket
         */
        GSGPSlaveSocket(const char* port_name, 
                        const std::string socket_type_name = GSGP_SLAVE_SOCKET_TYPE_NAME )
        : virtual_base_type(port_name, socket_type_name)
        , gs::socket::GreenSocketAddress_base(port_name)
        , m_fw_peq(this, &my_type::fw_peq_cb, this)
        , m_outstanding_nb_count(0)
        , data_width(BUSWIDTH)
        , AckRequest(*this)
        , ErrorRequest(*this)
        , AckData(*this)
        , ErrorData(*this)
        , Response(*this)
        , is_single_port(false)
        , is_multi_port(false)
        , has_f(false)
        , has_b(false)
        , has_td_b(false)
        , has_nb(false)
        { 
          // init m_name
          base_socket_type::socket_name();
          
          // register callback function to this Socket, see start_of_simulation
        }

        
        /// Destructor
        ~GSGPSlaveSocket() { }
        
        /// slave acknowledges a request atom
        AckRequestObj<my_type, PHASE>       AckRequest;
        /// slave replies an error to a request atom
        ErrorRequestObj<my_type, PHASE>     ErrorRequest;
        /// slave acknowledges data atom
        AckDataObj<my_type, PHASE>          AckData;
        /// slave replies an error to a data atom
        ErrorDataObj<my_type, PHASE>        ErrorData;
        /// slave sends a response atom
        ResponseObj<my_type, PHASE>         Response;
        
        /// Binding function to bind this socket to a user slave
        void bind_b_if(b_if_type& other);

        /// Binding function to bind this socket to a user slave
        void bind_b_if(multi_b_if_type& other);
        
        /// Binding function to bind this socket to a user slave
        void bind_b_if(td_b_if_type& other);

        /// Binding function to bind this socket to a user slave
        void bind_b_if(td_multi_b_if_type& other);
        
        /// Binding function to bind this socket to a user slave
        void bind_f_if(f_if_type& other);

        /// Binding function to bind this socket to a user slave
        void bind_f_if(multi_f_if_type& other);
        
        /// Callback function for BLOCKING forward path (called by GreenSocket or by the internal return path handling)
        /**
         * Either call the time-decoupled version with time or wait and call the non-td version ( b_in->b_transact(tr) ).
         */
        void fw_b_transport_cb(unsigned int index, tlm::tlm_generic_payload& tlmtr, 
                               sc_core::sc_time& t);
        
        /// Callback function for NON-BLOCKING forward path (called by GreenSocket)
        /**
         * If connected to GSGPSocket, m_is_connected_to_GPSocket_vec[index] == true   <br>
         *    Forwards the call to the internal tlm peq.
         * else                                                              <br>
         *    Handles conversions from other sockets to GSGPSocket
         *    and (in some cases) forwards the call to the internal tlm peq.
         */
        tlm::tlm_sync_enum fw_nb_transport_cb(unsigned int index,
                                              tlm::tlm_generic_payload& tlmtr, 
                                              tlm::tlm_phase& tph,
                                              sc_core::sc_time& t);
        
        /// Forwards the transaction to the local peq (calls notify) after having acquired (the callback function will release)
        inline void enqueue_to_fw_peq(tlm::tlm_generic_payload& tlmtr, tlm::tlm_phase& tph, const sc_core::sc_time& delay, unsigned int index);
        
        /// Callback function for NON-BLOCKING forward path called by inwards tlm peq (either initiated by target or the return path)
        /**
         * Forwards the call to the user.
         */
        void fw_peq_cb(tlm::tlm_generic_payload& tlmtr, const tlm::tlm_phase& tph, unsigned int index);
        
        /// Registers the callback according to the configuration
        virtual void start_of_simulation();

      protected:
        /// Simulate nb transaction with the f_transact
        inline void nb_from_f(atom &tc, unsigned int index = 0);

        // Some private flags set at start_of_simulation() to simplify callback handling
        bool is_single_port;
        bool is_multi_port;
        bool has_f;
        bool has_b;
        bool has_td_b;
        bool has_nb;
      };
    
    /// User class for a standard GSGP Slave Socket
    template<unsigned int BUSWIDTH>
    class GenericSlavePort
    : public GSGPSlaveSocket<BUSWIDTH, GenericSlaveAccess, GSGPSocketConfig, GenericPhase> {
    protected:
      typedef GSGPSlaveSocket<BUSWIDTH, GenericSlaveAccess, GSGPSocketConfig, GenericPhase> base_socket_type;
      typedef typename base_socket_type::virtual_base_type virtual_base_type; // type whose constructor needs to be called by derived classes
    public:
      GenericSlavePort( const char* port_name )
      : virtual_base_type(port_name, GSGP_SLAVE_SOCKET_TYPE_NAME)
      , GSGPSlaveSocket<BUSWIDTH, GenericSlaveAccess, GSGPSocketConfig, GenericPhase>(port_name)
      { }
    };

#include "gsgpsocket/transport/GSGPSlaveSocket.tpp"

    /**
     * Deprecated: Helper function to get slave access handle from a slave_atom
     */
    inline GenericSlaveAccessHandle _getSlaveAccessHandle(const slave_atom &tc) { return tc.first; }

    /**
     * Deprectated: Helper function to get slave access handle from a GenericTransaction handle (formerly used in b_transact)
     */
    inline GenericSlaveAccessHandle _getSlaveAccessHandle(const GenericSlaveAccessHandle &t) { return t; }
    
    /**
     * Helper function to get the phase from a slave_atom
     */
    inline GenericPhase _getPhase(const slave_atom &tc) { return tc.second; }
    
  } // end namespace gp
} // end namespace gs


#endif
