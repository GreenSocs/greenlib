#ifndef __GenericBroadcaster_if_h__
#define __GenericBroadcaster_if_h__

#include "greensocket/target/single_socket.h"
#include "greensocket/initiator/multi_socket.h"

namespace gs {
namespace gp {

//---------------------------------------------------------------------------
/**
 * The generic broadcaster interface.
 */
//---------------------------------------------------------------------------

class dummy {
    public:
    dummy(void*) {}
    dummy(){}
    void free(void*) {}
};


template <unsigned int BUSWIDTH, typename TRAITS>
class GenericBroadcaster_if : public virtual sc_core::sc_interface
{
    public:
    SINGLE_MEMBER_DATA(sender_ids, gs::ext::vector_container<unsigned int>);
    typedef typename TRAITS::tlm_payload_type                payload_type;
    typedef gs::socket::target_socket<BUSWIDTH, TRAITS>      target_socket_type;
    typedef gs::socket::initiator_multi_socket<BUSWIDTH, TRAITS,
                                               gs::socket::bind_checker<TRAITS>,
                                               dummy,
                                               dummy, dummy> init_socket_type;
};

template <typename TRAITS, unsigned int PORTMAX>
class GenericBroadcasterBase {
    public:
    virtual ~GenericBroadcasterBase(){}
};

}
}
#endif
