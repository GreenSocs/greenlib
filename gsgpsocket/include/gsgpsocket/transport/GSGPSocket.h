// LICENSETEXT
// 
//   Copyright (C) 2008 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
// 
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT

#ifndef __GSGPSOCKET_H__
#define __GSGPSOCKET_H__

#ifndef USE_GPSOCKET // this define can be used by users to check wether this GP Socket is used (or e.g. the old GreenBus)
# define USE_GPSOCKET
#endif

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
# error "please use SC_INCLUDE_DYNAMIC_PROCESSES"
#endif

#include "gsgpsocket/utils/helpfunctions.h"
#include "gsgpsocket/utils/gs_trace.h"
#include "gsgpsocket/transport/generic_transaction.h"
#include "gsgpsocket/transport/generic_phase.h"
#include "gsgpsocket/utils/gsgp_peq.h"
#include "tlm_utils/instance_specific_extensions.h"
#include "greencontrol/config.h"
#include "gsgpsocket/core/tlm_b_if.h"
#include "gsgpsocket/utils/payload_event_queue.h"
#include "gsgpsocket/transport/GSGPconfig.h"
#include "greencontrol/core/gs_sc_api_detection.h"


#define GSGP_RESET_INSTANCE_SPECIFIC_COMPLETED_EXTENSION(socket, transaction) \
  transaction_completed_extn* completed_extn; \
  socket->inst_spec_extn_accessor(transaction).get_extension(completed_extn); \
  if (completed_extn) { \
    completed_extn->value = false; \
  } else { \
    completed_extn = new transaction_completed_extn; \
    completed_extn->value = false; \
    socket->inst_spec_extn_accessor(transaction).set_extension(completed_extn); \
  }

#define GSGP_RESET_INSTANCE_SPECIFIC_STATUS_EXTENSION(socket, transaction) \
  transaction_status_extn* phase_status_extn; \
  socket->inst_spec_extn_accessor(transaction).get_extension(phase_status_extn); \
  if (phase_status_extn) { \
    phase_status_extn->i_sent_an_error = false; \
    phase_status_extn->is_in_req_cycle = false; \
  } else { \
    phase_status_extn = new transaction_status_extn; \
    phase_status_extn->i_sent_an_error = false; \
    phase_status_extn->is_in_req_cycle = false; \
    socket->inst_spec_extn_accessor(transaction).set_extension(phase_status_extn); \
  }

#define GSGP_CREATE_BIG_FUNCTOR_HEADER(name) \
  template <typename SOCKET, typename PHASE>   \
  struct name : Socket_name_proxy<SOCKET> { \
    name(SOCKET& soc) : Socket_name_proxy<SOCKET>(&soc) { } \
  }; \
  template <typename SOCKET> \
  struct name<SOCKET, GenericPhase> : Socket_name_proxy<SOCKET> { \
    name(SOCKET& soc) : Socket_name_proxy<SOCKET>(&soc) { this->soc = &soc; } \
    void operator()(typename SOCKET::accessHandle &th, const GenericPhase &ph, const sc_core::sc_time &d, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th, const GenericPhase &ph, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th, const GenericPhase &ph, double d, sc_core::sc_time_unit u, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th , const sc_core::sc_time &d, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th , double d, sc_core::sc_time_unit u, unsigned int index = 0); \
    SOCKET *soc; \
  }

#define GSGP_CREATE_SMALL_FUNCTOR_HEADER(name) \
  template <typename SOCKET, typename PHASE>   \
  struct name : Socket_name_proxy<SOCKET> { \
    name(SOCKET& soc) : Socket_name_proxy<SOCKET>(&soc) { } \
  }; \
  template <typename SOCKET>  \
  struct name<SOCKET, GenericPhase> : Socket_name_proxy<SOCKET> { \
    name(SOCKET& soc) : Socket_name_proxy<SOCKET>(&soc) { this->soc = &soc; } \
    void operator()(typename SOCKET::accessHandle &th, const GenericPhase &_p, const sc_core::sc_time &d, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th, const GenericPhase &_p, unsigned int index = 0); \
    void operator()(typename SOCKET::accessHandle &th, const GenericPhase &_p, double d, sc_core::sc_time_unit u, unsigned int index = 0); \
    SOCKET *soc; \
  }

namespace gs {
  namespace gp {

    // /////////////////////////////////////////////////////////////////////////////// //
    // //////// payload_event_queue_multi_output_if for notify with multi socket index information /////////////// //

    template<typename PAYLOAD>
    class payload_event_queue_multi_output_if : public virtual sc_core::sc_interface
    {
    public:
      virtual void notify (PAYLOAD& p, unsigned int index) =0;
    };
    
    /// TLM2.0 instance-specific extension to store if the callee has completed the transaction
    struct transaction_completed_extn 
    : tlm_utils::instance_specific_extension<transaction_completed_extn> {
      bool value; // User-defined extension attribute 
    };

    /// TLM2.0 instance-specific extension to store if the socket sent an error within this extension and if the transaction is within the request cycle
    struct transaction_status_extn 
    : tlm_utils::instance_specific_extension<transaction_status_extn> {
      /// if the transaction is within the request cycle (true if last phase was BEGIN_REQ, false as soon as a END_REQ was received)
      bool is_in_req_cycle;
      /// if I sent an error (GreenBus) phase 
      bool i_sent_an_error;
    };
    
    /// Converts GreenBus transaction and phase to -> TLM transaction and phase
    inline void synchronize_transaction_phase(GenericTransaction &t, const GenericPhase &ph, 
                                              tlm::tlm_generic_payload &tlmtr, tlm::tlm_phase &tph) {
      // Convert GreenBus phase to TLM Phase + response_status
      switch (ph.state) {
        case GenericPhase::Idle:
          tph = tlm::UNINITIALIZED_PHASE;
          break;
        case GenericPhase::RequestValid:
          tph = tlm::BEGIN_REQ;
          break;
        case GenericPhase::RequestAccepted:
          tph = tlm::END_REQ;
          break;
        case GenericPhase::RequestError:
          tph = tlm::BEGIN_RESP; // !!!
          tlmtr.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
          break;
        case GenericPhase::DataValid:
          tph = BEGIN_DATA;
          break;
        case GenericPhase::DataAccepted:
          tph = END_DATA;
          break;
        case GenericPhase::DataError:
          tph = END_DATA;
          tlmtr.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
          break;
        case GenericPhase::ResponseValid:
          tph = tlm::BEGIN_RESP;
          break;
        case GenericPhase::ResponseAccepted:
          tph = tlm::END_RESP;
          break;
        case GenericPhase::ResponseError:
          tph = tlm::END_RESP; // TODO: what if receiver is OSCI?
          tlmtr.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
          break;
        default:
          tph = tlm::UNINITIALIZED_PHASE;
          tlmtr.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
          SC_REPORT_WARNING("GreenBus -> TLM2.0 conversion", "Got unknown GreenBus phase. Conversion to TLM2.0 failed!");
          break;
      }
      // Copy BurstNumber and MSBytesValid
      //t.get_extension_support_base()->get_extension<burst_number>(tlmtr)->value = ph.BurstNumber;
      t.get_extension_support_base()->get_extension<bytes_valid>(tlmtr)->value  = ph.getBytesValid();
    }

    /// Converts TLM transaction and phase to -> GreenBus transaction and phase
    inline void synchronize_transaction_phase(tlm::tlm_generic_payload &tlmtr, const tlm::tlm_phase &tph, 
                                              GenericTransaction &t, GenericPhase &ph, bool is_in_req_cycle) {
      // Convert TLM phase + response_status -> GreenBus Phase
      switch (tph) {
        case tlm::UNINITIALIZED_PHASE:
          ph = GenericPhase::Idle;
          break;
        case tlm::BEGIN_REQ:
          ph = GenericPhase::RequestValid;
          break;
        case tlm::END_REQ:
          assert(tlmtr.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE
                 || tlmtr.get_response_status() == tlm::TLM_OK_RESPONSE);
          ph = GenericPhase::RequestAccepted;          
          break;
        case tlm::BEGIN_RESP:
          if (tlmtr.get_response_status() != tlm::TLM_OK_RESPONSE
              && is_in_req_cycle)
            ph = GenericPhase::RequestError;
          else
            ph = GenericPhase::ResponseValid; // this _may_ be a valid with error code, which is allowed
          break;
        case tlm::END_RESP:
          if (tlmtr.get_response_status() != tlm::TLM_OK_RESPONSE
              && tlmtr.get_response_status() != tlm::TLM_INCOMPLETE_RESPONSE) // TODO: allowed for OSCI?
            ph = GenericPhase::ResponseError;
          else
            ph = GenericPhase::ResponseAccepted;
          break;
        default:
          if (tph == BEGIN_DATA)
            ph = GenericPhase::DataValid;
          else if (tph == END_DATA) {
            if (tlmtr.get_response_status() != tlm::TLM_OK_RESPONSE && tlmtr.get_response_status() != tlm::TLM_INCOMPLETE_RESPONSE)
              ph = GenericPhase::DataError;
            else
              ph = GenericPhase::DataAccepted;
          } else {
            ph = GenericPhase::Idle;
            t.setSResp(Generic_SRESP_ERR);
            SC_REPORT_WARNING("TLM2.0 -> GreenBus conversion", "Got unknown TLM2.0 phase. Conversion to GreenBus failed!");
          }
          break;
      }
      // Copy BurstNumber and MSBytesValid
      //ph.BurstNumber = t.get_extension_support_base()->get_extension<burst_number>(tlmtr)->value;
      ph.setBytesValid( t.get_extension_support_base()->get_extension<bytes_valid>(tlmtr)->value );
    }      
    
    
    // /////////////////////////////////////////////////////////////////////////////// //
    /// Proxy class which allows access to the socket's protected sock_name function 
    template <typename SOCKET>
    class Socket_name_proxy {
    protected:
      SOCKET* soc;
    public:
      Socket_name_proxy(SOCKET *_soc) { soc = _soc; }
      /// Return the name (socket_name) of the socket
      const char* socket_name() { return soc->socket_name(); };
    };
    

    // /////////////////////////////////////////////////////////////////////////////// //
    /// GSGP Socket base (virtual base of Master/Slave Sockets)
    template <typename BASE_SOCKET, typename CONFIG /*= GSGPSocketConfig*/>
    class GSGPSocket_base
    : public BASE_SOCKET 
    , public configure_gs_params
    {
    protected:
      
      typedef BASE_SOCKET socket_type;
      
      /// If this socket (=0) is unconfigured, (=1) has been configured manually or (=2) already run through the automatic configuration mechanism (=2) at before_end_of_elaboration
      unsigned int m_configured;
      
      /// String returned by get_type_string()
      std::string m_type_string;

      // my name, returned by socket_name()
      std::string m_name;
      
      /// Configuration set for this socket
      CONFIG *mConfig;
      
      /// If the multi socket according the vector position is connected to another GSGPSocket, set during bound_to callback
      std::vector<bool> m_is_connected_to_GPSocket_vec;
      
    public:
      
      /// Instance-specific extension accessor
      tlm_utils::instance_specific_extension_accessor  inst_spec_extn_accessor;
      
      /// Constructor with port name (for GreenSocket) and type_string for the get_type_string() function
      GSGPSocket_base(const char* port_name, const std::string type_string) 
      : socket_type(port_name)
      , m_configured(0)
      , m_type_string(type_string)
      , mConfig(NULL)
      , at_mode(std::string(port_name) + ".at_mode", "false")
      //, m_is_connected_to_GPSocket(false) // will be updated in start_of_simulation
      { 
        mConfig = create_config_with_params<CONFIG>(port_name);
        assert(mConfig != NULL);
      }

      /// Destructor
      ~GSGPSocket_base() {
        delete mConfig;
      }

      /// Returns if this socket has already been configured (done during bound_to)
      bool is_configured() const { return m_configured; }

      /// Get the type string of this socket. This function is required by greensocket. Overloads extension_support_base function returning the mTypeString.
      const std::string& get_type_string() {
        return m_type_string;
      }

      /// Set the socket configuration using a GenericProtocol configuration object.
      /**
       * The user either can create a GPSocketConifg object and use this function
       * to set the configuration of this socket - or the configuration can be applied
       * by setting the gs_params of the config owned by this socket.
       *
       * The call of this functions prevents the automatic configuration during 
       * before_end_of_elaboration!
       */
      inline void set_config(const CONFIG& cnf) {
        // check if already configured -> Error
        if (m_configured == 2) // if already before_end_of_elaboration
          SC_REPORT_ERROR(m_name.c_str(), "The configuration during before_end_of_elaboration is already done. You should not change configuration now.");
        *mConfig = cnf;
        GS_DUMP_N(m_name.c_str(), "manual configuration");
        socket_type::set_config(convert_to_GreenSocket_conf(*mConfig));
        m_configured = 1; // marked as manually configured
      }
      
      /// Returns the GeneralProtocol configuration of this GSGP socket. (Use get_recent_config() to get the GreenSocket configuration!)
      inline CONFIG& get_GSGP_config() const { return *mConfig; }

      /// Returns if the connected socket is a GreenBus GenericProtocol Socket which supports the data phase etc.
      /**
       * @return true if config has data phase.
       */
      bool is_connected_to_GPSocket(unsigned int index) const { return m_is_connected_to_GPSocket_vec[index]; }

      /// Set the configuration before_end_of_elaboration (where it is needed). MUST call the base class function, too!
      void before_end_of_elaboration() {
        socket_type::before_end_of_elaboration();
        if (m_configured == 0) { // if nothing yet configured
          GS_DUMP_N(m_name.c_str(), "automatic configuration");
          socket_type::set_config(convert_to_GreenSocket_conf(*mConfig));
        } else { // (==1) if already manually configured
          GS_DUMP_N(m_name.c_str(), "automatic configuration SKIPPED because of manual configuration");
        }
        m_configured = 2;
      }
      
      /// GreenSocket callback: Get the configurated data phase and set local m_is_connected_to_GPSocket
      void bound_to(const std::string& socketTypeName, socket::bindability_base<tlm::tlm_base_protocol_types>* soc, unsigned int) {
        GS_DUMP_N(m_name.c_str(), "I am a socket of type " << socketTypeName);
        m_is_connected_to_GPSocket_vec.resize(socket_type::size());
        for (unsigned int i = 0; i < socket_type::size(); i++) {
          // Check if the connected socket supports data phase and BytesValid and set local m_is_connected_to_GPSocket
          if (   socket_type::get_recent_config(i).template has_extension<bytes_valid>()!=gs::ext::gs_reject
              && socket_type::get_recent_config(i).has_phase(BEGIN_DATA)!=gs::ext::gs_reject
              && socket_type::get_recent_config(i).has_phase(END_DATA)!=gs::ext::gs_reject) {
            m_is_connected_to_GPSocket_vec[i] = true;
            GS_DUMP_N(m_name.c_str(), "(#"<<i<<":) I am connected to a GSGPSocket");
          }
          else {
            m_is_connected_to_GPSocket_vec[i] = false;
            GS_DUMP_N(m_name.c_str(), "(#"<<i<<":) I am connected to another socket (not-GSGPSocket)");
            // change my configuration to only OSCI (remove data phase, BytesValid)
            socket_type::get_recent_config(i).remove_phase(BEGIN_DATA);
            socket_type::get_recent_config(i).remove_phase(END_DATA);
            socket_type::get_recent_config(i).template remove_extension<bytes_valid>();
            socket_type::set_config(socket_type::get_recent_config(i), i); // TODO: richtig so?
          }
        }
      }
      

      void start_of_simulation() {
        socket_type::start_of_simulation();
        for (unsigned int i = 0; i < m_is_connected_to_GPSocket_vec.size(); i++) {
          if (m_is_connected_to_GPSocket_vec[i]){
            if (mConfig->use_wr_resp){
              if (socket_type::get_recent_config(i).template has_extension<wr_resp_dummy>()==gs::ext::gs_reject) {
                std::ostringstream ss; 
                ss << "Connected socket #" <<i<< " does not support write responses, but this socket requires them.";
                SC_REPORT_ERROR(m_name.c_str(), ss.str().c_str());
              }
            }
            else {
              if (socket_type::get_recent_config().template has_extension<wr_resp_dummy>()!=gs::ext::gs_reject) {
                std::ostringstream ss;
                ss << "Connected socket #" <<i<< " requires write responses, but this socket does not support them."; 
                SC_REPORT_ERROR(m_name.c_str(), ss.str().c_str());
              }
            }
          }
        }
      }

      /// Returns if the response status is an error (is not INCOMPLETE or OK)
      inline bool has_error(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        return (tlmtr.get_response_status() != tlm::TLM_INCOMPLETE_RESPONSE 
                && tlmtr.get_response_status() != tlm::TLM_OK_RESPONSE      );
      }
      
      
      // ///////////////////  Instance specific extension handling  //////////////////// //

      
      /// Marks the given transaction as completed (using the instance specific extension)
      /**
       * @param is_completed  If the transaction should be marked as completed (default) or not (=reset)
       */
      void mark_transaction_completed(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr, bool is_completed = true) {
        transaction_completed_extn* completed_extn;
        inst_spec_extn_accessor(tlmtr).get_extension(completed_extn);
        if (completed_extn) {
          completed_extn->value = is_completed;
        } else {
          completed_extn = new transaction_completed_extn;
          completed_extn->value = is_completed;
          inst_spec_extn_accessor(tlmtr).set_extension(completed_extn);
        }
      }

      /// Resets the given transaction as NOT completed (using the instance specific extension)
      inline void reset_transaction_completed(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        mark_transaction_completed(tlmtr, false);
      }
      
      /// Checks if the given tlm transaction is marked as completed (checks the instance specific extension)
      bool is_transaction_completed(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        // Handle completed return status of this transaction
        transaction_completed_extn* completed_extn; 
        inst_spec_extn_accessor(tlmtr).get_extension(completed_extn); // Get existing extension 
        if (completed_extn)
          return completed_extn->value;
        return false;
      }
      
      
      /// Marks the given transaction as sent as error (using the instance specific extension)
      /**
       * @param is_error  If the transaction should be marked as error (default) or not (=reset)
       */
      void mark_transaction_sent_as_error(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr, bool is_error = true) {
        transaction_status_extn* extn;
        inst_spec_extn_accessor(tlmtr).get_extension(extn);
        if (extn) {
          extn->i_sent_an_error = is_error;
        } else {
          extn = new transaction_status_extn;
          extn->i_sent_an_error = is_error;
          inst_spec_extn_accessor(tlmtr).set_extension(extn);
        }
      }

      /// Resets the given transaction as NOT sent as error (using the instance specific extension)
      inline void reset_transaction_sent_as_error(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        mark_transaction_sent_as_error(tlmtr, false);
      }
      
      /// Checks if the given transaction was sent as an error by me
      bool is_transaction_sent_as_error(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        // Handle completed return status of this transaction
        transaction_status_extn* extn; 
        inst_spec_extn_accessor(tlmtr).get_extension(extn); // Get existing extension 
        if (extn)
          return extn->i_sent_an_error;
        return false;
      }

      
      /// Sets the given transaction to be within the request cycle (using the instance specific extension)
      void mark_transaction_is_in_req_cycle(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr, 
                                            bool _is_in_req_cycle = true) {
        transaction_status_extn* extn;
        inst_spec_extn_accessor(tlmtr).get_extension(extn);
        if (extn) {
          extn->is_in_req_cycle = _is_in_req_cycle;
        } else {
          extn = new transaction_status_extn;
          extn->is_in_req_cycle = _is_in_req_cycle;
          inst_spec_extn_accessor(tlmtr).set_extension(extn);
        }
      }

      /// Resets the given transaction to be NOT within the request cycle (using the instance specific extension)
      void reset_transaction_is_in_req_cycle(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        mark_transaction_is_in_req_cycle(tlmtr, false);
      }
      
      /// Returns if the given transaction is within the request phase
      bool transaction_is_within_req_cycle(tlm::tlm_base_protocol_types::tlm_payload_type& tlmtr) {
        transaction_status_extn* phase_status_extn; 
        inst_spec_extn_accessor(tlmtr).get_extension(phase_status_extn); // Get existing extension 
        if (phase_status_extn) {
          return phase_status_extn->is_in_req_cycle;
        }
        //assert(false); exit(1);
        return false;
      }

      const char* socket_name() {
        if (m_name.length() == 0) {
#if SYSTEMC_API == 210
          sc_core::sc_module *mod = dynamic_cast<sc_core::sc_module*>(dynamic_cast<gs::socket::bindability_base<tlm::tlm_base_protocol_types>*>(this)->get_parent());
          assert(mod != NULL);
          m_name = mod->name();
#else
          m_name = socket_type::get_parent_object()->name();
#endif
          m_name += ".";
          m_name += m_type_string;
        }
        return m_name.c_str();
      }
      
      /// Allow switch between Loosely Timed (default) or Approximately Timed modes.
      /**
       * The LT/AT switch can change at runtime, so it is not part of the socket
       *  configuration to bind.
       * It is up to the user to use this parameter and implement the two modes.
       */
      gs_param<bool> at_mode; // default to false
      
    };

  } // end namespace gp
} // end namespace gs


#endif
