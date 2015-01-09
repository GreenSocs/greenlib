#ifndef __genericBroadcaster_h__
#define __genericBroadcaster_h__

#include <boost/config.hpp> // needed for SystemC 2.1
#include <systemc>
#include "greensocket/initiator/multi_socket.h"
#include "greensocket/target/single_socket.h"
#include "gsgpsocket/utils/gs_trace.h"
#include "greenbroadcaster/genericBroadcaster_if.h"
#include <sstream>
#include <set>

namespace gs {
namespace gp {

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int PORTMAX = 255,
          typename RESP_TYPE=tlm::tlm_response_status,
          RESP_TYPE ADDR_ERR_RESP=tlm::TLM_ADDRESS_ERROR_RESPONSE,
          void (TRAITS::tlm_payload_type::*SET_RESP_CALL)(const RESP_TYPE) = &(TRAITS::tlm_payload_type::set_response_status)
         >
class GenericBroadcaster_b : public sc_core::sc_module,
                             public GenericBroadcaster_if<BUSWIDTH, TRAITS>,
                             public GenericBroadcasterBase<TRAITS, PORTMAX>
{
    private:
    class broadcaster_init_socket : public GenericBroadcaster_if<BUSWIDTH, TRAITS>::init_socket_type 
    {
        public:
        broadcaster_init_socket(const char* name, GenericBroadcaster_b& owner)
        : GenericBroadcaster_if<BUSWIDTH, TRAITS>::init_socket_type(name),
          m_owner(owner)
        {}

        virtual void bound_to(const std::string& other_type,
                              gs::socket::bindability_base<TRAITS>* other,
                              unsigned int index) {
            m_owner.bound_to_target(other_type, other, index);
        }
        private:
        GenericBroadcaster_b& m_owner;
    };

    class broadcaster_target_socket : public GenericBroadcaster_if<BUSWIDTH, TRAITS>::target_socket_type
    {
        public:
        broadcaster_target_socket(const char* name, GenericBroadcaster_b& owner)
        : GenericBroadcaster_if<BUSWIDTH, TRAITS>::target_socket_type(name),
          m_owner(owner)
        {}

        virtual void bound_to(const std::string& other_type,
                              gs::socket::bindability_base<TRAITS>* other,
                              unsigned int index) {
            m_owner.bound_to_initiator(other_type, other, index);
        }
        private:
        GenericBroadcaster_b& m_owner;
    };

    public:
    typedef broadcaster_target_socket         target_socket_type;
    typedef broadcaster_init_socket           init_socket_type;
    typedef typename TRAITS::tlm_payload_type payload_type;
    typedef typename TRAITS::tlm_phase_type   phase_type;
    typedef tlm::tlm_sync_enum                sync_enum_type;

    //--------------------------------------------------------------------------
    /**
     * Input/Output sockets
     */
    //--------------------------------------------------------------------------
    /// target slave single port
    target_socket_type                        target_socket;
    
    /// master multiport
    init_socket_type                          init_socket;

    SC_HAS_PROCESS(GenericBroadcaster_b);
    //--------------------------------------------------------------------------
    /**
     * Constructor; bind ports and register SC_METHODs with the kernel.
     */
    //--------------------------------------------------------------------------
    GenericBroadcaster_b(sc_core::sc_module_name name_)
    : sc_core::sc_module(name_),
      target_socket("tsocket", *this),
      init_socket("isocket", *this)
    {
        std::cout << "I am a generic broadcaster." << std::endl;
        target_socket.register_b_transport(this, &GenericBroadcaster_b::b_tr);
    }

    virtual void bound_to_target(const std::string& other_type,
                                 gs::socket::bindability_base<TRAITS>* other,
                                 unsigned int index) {
        std::cout << "Initiator socket index "
                << index << " is bound to a socket of type " << other_type
                << std::endl;
        gs::socket::config<TRAITS> conf = init_socket.get_recent_config(0);
        for (unsigned int i = 1; i < init_socket.size(); i++) {
            std::stringstream s1,s2;
            s1 << "Merged Config of Targets 0 to " << (i-1)
               << " of " << init_socket.name();
            s2 << "Target at index " << i << " of " << init_socket.name();
            conf.merge_with(s1.str().c_str(), s2.str().c_str(),
                            init_socket.get_recent_config(i));
        }
        target_socket.set_config(conf);
        init_socket.set_config(conf);
    }
  
    virtual void bound_to_initiator(const std::string& other_type,
                                    gs::socket::bindability_base<TRAITS>* other,
                                    unsigned int index) {
        // [TODO] Is It really required ?
        if (target_socket.size() > 1)
        {
            std::cout << "Target socket is already binded, Maximum allowed number of"
                    " binding = 1." << std::endl;
            return;
        }

        std::cout << "Target socket index " << index << " is bound to a socket of"
                " type " << other_type << std::endl;
        gs::socket::config<TRAITS> conf = target_socket.get_recent_config(0);
        init_socket.set_config(conf);
        target_socket.set_config(conf);

    }

    void before_end_of_elaboration() {
        std::cout << "Inside GenericBroadcaster_b::" << __FUNCTION__ << std::endl;
#ifdef USE_GPSOCKET
        GSGPSocketConfig conf;
        target_socket.set_config(configure_gs_params::convert_to_GreenSocket_conf(conf));
        init_socket.set_config(configure_gs_params::convert_to_GreenSocket_conf(conf))
#else
        gs::socket::config<TRAITS> conf = target_socket.get_recent_config(0);
        target_socket.set_config(conf);
        init_socket.set_config(conf);
#endif
    }

    void start_of_simulation() {
        std::cout << "Resolved configs are" << std::endl
                << "  For initiators:" << std::endl
                << target_socket.get_recent_config(0).to_string() << std::endl
                << "  For targets:" << std::endl
                << init_socket.get_recent_config(0).to_string() << std::endl;
    }

    //--------------------------------------------------------------------------
    /**
     * This is the PV TLM blocking if implementation
     */
    //--------------------------------------------------------------------------
    void b_tr(payload_type& txn, sc_core::sc_time& time)
    {
        std::cout << "Inside " << __FUNCTION__ << std::endl;
        for (unsigned int i = 0; i < init_socket.size(); ++i) {
            // Forward transaction to all the targets
            init_socket[i]->b_transport(txn, time);
        }
    }
};

template <unsigned int BUSWIDTH, typename TRAITS=tlm::tlm_base_protocol_types,
          unsigned int PORTMAX = 255,
          typename RESP_TYPE=tlm::tlm_response_status,
          RESP_TYPE ADDR_ERR_RESP=tlm::TLM_ADDRESS_ERROR_RESPONSE,
          void (TRAITS::tlm_payload_type::*SET_RESP_CALL)(const RESP_TYPE) = &TRAITS::tlm_payload_type::set_response_status
        >
class GenericBroadcaster : public GenericBroadcaster_b<BUSWIDTH, TRAITS,
                                                       PORTMAX, RESP_TYPE,
                                                       ADDR_ERR_RESP,
                                                       SET_RESP_CALL
                                                      >
{
    public:
    GenericBroadcaster(const char* name):
        GenericBroadcaster_b<BUSWIDTH, TRAITS, PORTMAX, RESP_TYPE,
                             ADDR_ERR_RESP, SET_RESP_CALL>(name)
    {}
};

}
}
#endif

