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

#ifndef __GenericRouter_if_h__
#define __GenericRouter_if_h__

#include "greensocket/initiator/multi_socket.h"
#include "greensocket/target/multi_socket.h"
#include "greensocket/bidirectional/multi_socket.h"

namespace gs{
namespace gp{
//---------------------------------------------------------------------------
/**
 * The generic router interface.
 */
//---------------------------------------------------------------------------
//template <class INITPORT, 
//          class TARGETPORT>

class dummy{
public:
  dummy(void*){}
  dummy(){}
  void free(void*){}
};


template <unsigned int BUSWIDTH, typename TRAITS>
class GenericRouter_if 
: public virtual sc_core::sc_interface 
{

  
public:
  SINGLE_MEMBER_DATA(sender_ids, gs::ext::vector_container<unsigned int>);
  typedef typename TRAITS::tlm_payload_type              payload_type;
  typedef gs::socket::target_multi_socket<BUSWIDTH, TRAITS> target_socket_type;
  typedef gs::socket::initiator_multi_socket<BUSWIDTH, TRAITS
                                            ,gs::socket::bind_checker<TRAITS>
                                            ,dummy //tlm::tlm_mm_interface : changed to let greenrouter work with different protocols
                                            ,dummy, dummy> init_socket_type;
  typedef gs::socket::bidirectional_multi_socket<BUSWIDTH, TRAITS> bidir_socket_type;

  virtual init_socket_type* getInitPort() =0;
  virtual target_socket_type* getTargetPort() =0;
  virtual bidir_socket_type* getBiDirSocket() =0;
  virtual std::vector<unsigned int>& decodeAddress(payload_type& txn, bool& decode_status, gs::socket::config<TRAITS>* conf = 0, unsigned int from = 0) =0;
  virtual unsigned int getRouterID() =0;
  virtual unsigned int getCurrentNumRouters()=0;
  
};

template <typename TRAITS, unsigned int PORTMAX>
class SimpleAddressMap;

template <typename TRAITS, unsigned int PORTMAX, typename ADDR_MAP>
class GenericRouterBase{
public:
  virtual ~GenericRouterBase(){}
  virtual ADDR_MAP& getAddressMap()=0;
  virtual void do_eoe()=0;
};


}
}
#endif
