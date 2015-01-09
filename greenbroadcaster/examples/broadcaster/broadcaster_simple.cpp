#include "greenbroadcaster/genericBroadcaster.h"

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include "systemc.h"

#ifdef USE_TLM_SOCKETS
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#else
#include "greensocket/target/single_socket.h"
#include "greensocket/initiator/single_socket.h"
#include "greensocket/generic/green_socket_config.h"
#endif

using namespace gs;
using namespace gs::gp;

void print_transaction_object(tlm::tlm_generic_payload *trans)
{
    std::cout << "trans: = cmd = " << (trans->get_command() ? 'W' : 'R')
        << ", addr = " << hex <<  trans->get_address()
        << ", data_ptr = " << hex << (void *)trans->get_data_ptr()
        << ", data_ptr_len = " << hex << trans->get_data_length()
        << ", data = " << hex << *((uint32_t *)trans->get_data_ptr())
        << ", byte_enable_ptr = " << hex << (void *)trans->get_byte_enable_ptr()
        << ", byte_enable_len = " << hex << trans->get_byte_enable_length()
        << ", streaming width = " << hex << trans->get_streaming_width()
        << " at time " << sc_time_stamp() << std::endl;
}

SC_MODULE(DummyInitiator)
{
#ifdef USE_TLM_SOCKETS
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_initiator_socket<DummyInitiator> socket;
#else
    gs::socket::initiator_socket<> socket;
#endif


    SC_CTOR(DummyInitiator)
    : socket("master_socket")  // Construct and name socket
    {
#ifndef USE_TLM_SOCKETS
        gs::socket::config<tlm::tlm_base_protocol_types> conf;
        socket.set_config(conf);
#endif
        SC_THREAD(thread_process);
    }

    void thread_process()
    {
        // TLM-2 generic payload transaction, reused across calls to b_transport
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
        sc_time delay = sc_time(10, SC_NS);
        
        // Generate a random sequence of reads and writes
        for (int i = 0; i < 5; i++)
        {
            std::cout << "i = " << i << std::endl;
            tlm::tlm_command cmd = tlm::TLM_WRITE_COMMAND;
            uint32_t data = 0xFF000000 | i;
            // Initialize 8 out of the 10 attributes, byte_enable_length and
            // extensions being unused
            trans->set_command( cmd );
            trans->set_address( i );
            trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
            trans->set_data_length( 4 );
            trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
            trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
            trans->set_dmi_allowed( false ); // Mandatory initial value

            print_transaction_object(trans);

            socket->b_transport( *trans, delay );  // Blocking transport call

            // Initiator obliged to check response status and delay
            if ( trans->is_response_error() ) {
                SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
            }
            std::cout << std::endl;
            // Realize the delay annotated onto the transport call
            wait(delay);
        }
    }
};

// Target module representing a simple memory
SC_MODULE(DummyTarget)
{
    private:
    int id;
    public:
    static int object_count;
#ifdef USE_TLM_SOCKETS
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<DummyTarget> socket;
#else
    gs::socket::target_socket<>  socket;
#endif

    SC_CTOR(DummyTarget)
    : socket("socket")
    {
#ifndef USE_TLM_SOCKETS
        gs::socket::config<tlm::tlm_base_protocol_types> conf;
        socket.set_config(conf);
#endif
        id = object_count++;
        // Register callback for incoming b_transport interface method call
        socket.register_b_transport(this, &DummyTarget::b_transport);
    }
    
    // TLM-2 blocking transport method
    void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
    {
        std::cout << "Inside DummyTarget::" << __FUNCTION__
                  << ", id = " << id <<std::endl;
        print_transaction_object(&trans);
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }
};

int DummyTarget::object_count = 0;

int sc_main(int argc, char **argv)
{
    GenericBroadcaster<32> b_obj("broadcaster");
    DummyInitiator init_obj("master_initiator");
    DummyTarget  t1_obj("slave_target_1");
    DummyTarget  t2_obj("slave_target_2");

    // binding
    init_obj.socket(b_obj.target_socket);
    b_obj.init_socket(t1_obj.socket);
    b_obj.init_socket(t2_obj.socket);

    sc_start();
    return 0;
}
