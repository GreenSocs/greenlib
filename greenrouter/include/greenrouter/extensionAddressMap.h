// LICENSETEXT
//
//   Copyright (C) 2007 - 2009 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//
//   Puneet Arora, CircuitSutra Technologies
//   puneet@circuitsutra.com
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
#ifndef __routeOnExtensions__
#define __routeOnExtensions__
namespace gs {
namespace gp {

typedef sc_dt::uint64 Map_address_t;                  ///< address type
typedef unsigned int  Port_id_t;                      ///< port ID type
template < typename TRAITS, unsigned int PORTMAX = 255>
class ExtensionMap {
  public:
    ExtensionMap () : m_max_port(PORTMAX) {}

    virtual ~ExtensionMap () {}

    template < typename SOCKET >
    void generateMap (SOCKET& init_socket) {
      unsigned int maxExts = 0;
      gs::socket::config<TRAITS> tmp_conf;
      //iterate over all the connections of init_socket i.e. over all the targets
      for (unsigned int i=0; i<init_socket.size(); ++i) {
         //get config of the target 'i'
         tmp_conf = init_socket.get_recent_config(i);
         //for all the extensions present in this config, make an entry in the address map.
         //address map keeps a record of targets that accept a particular extension.
         maxExts = tmp_conf.get_upper_bound_for_extension_ids();
         for (unsigned int j=0; j<maxExts; ++j) {
           if (tmp_conf.has_extension(j) != gs::ext::gs_reject)
             m_AddressMap[j].push_back(i); //push target 'i' in the list of extension 'j'
         }
      }
    }

    void checkSanity () { }

 //--------------------------------------------------------------------------
 // Define GS_VERBOSE_DUMPMAP if not defined in the compilation command line
 #if ( ! defined ( GS_VERBOSE_DUMPMAP ) )
   #if ( defined ( GS_VERBOSE ) )
     #define GS_VERBOSE_DUMPMAP 1
   #else /* GS_VERBOSE */
     #define GS_VERBOSE_DUMPMAP 0
   #endif  /*GS_VERBOSE  */
 #endif  /* GS_VERBOSE_DUMPMAP */

 /** --------------------------------------------------------------------------
  * dumpMap
  * @param verbose Only show the map if value is true. Default to true only when macro GS_VERBOSE is defined.
 -------------------------------------------------------------------------- */
    void dumpMap (bool verbose = GS_VERBOSE_DUMPMAP) {
    }

 typedef typename TRAITS::tlm_payload_type payload_type;
    virtual std::vector<Port_id_t>& 
    decode (payload_type& txn, bool &success,
            gs::socket::config<TRAITS>* tmp_conf, unsigned int from) {
      tlm::tlm_extension_base* ext = 0;
      gs::ext::gs_extension_base* gs_ext = 0;

      success = false;
      
      //for all the extensions present in this config, check which is present in
      //payload. Then broadcast this payload to all the targets that accept this
      //extension. Note that certain targets will reject this signal if 'from' is
      //not a valid source for them.
      for (unsigned int j=0; j<tmp_conf->get_upper_bound_for_extension_ids(); ++j) {

        if (tmp_conf->has_extension(j) != gs::ext::gs_reject) { //if the config has extension 'j'

          if ((ext=txn.get_extension(j))) { //if the payload has extension 'j'

            if (gs::ext::extension_cast()[j]==NULL)
              continue;
            gs_ext = (gs::ext::extension_cast()[j](ext));
            if (gs_ext && gs_ext->is_valid()) { //if the extension 'j' is validated one
              success = true;
              return m_AddressMap[j];
             // for (unsigned int i=0; i<m_AddressMap[j].size(); ++i) {
             //   //broadcast to all the targets that accept extension 'j'
             //   init_socket[m_AddressMap[j][i]]->b_transport(txn, time);
             // }
             // break;
            }
          }
        }
      }
      //std::cout << "ERROR: ExtensionMap::decode the payload has none of the expected extension validated. Returning reference to dummy vector\n";
      GS_DUMP("ERROR: ExtensionMap::decode the payload has none of the expected extension validated. Returning reference to dummy vector\n");
      return m_tempVec;
    }

    Port_id_t get_max_port () {
      return m_max_port;
    }

    void insert (Map_address_t address_low, Map_address_t address_high, Port_id_t port_id) {
      std::cout << "This function should not be called because routing on the basis of extensions only\n";
    };
  private:
    const Port_id_t m_max_port;                           ///< maximum port ID
    std::vector<unsigned int> m_tempVec;
    //KEY is extId and VALUE is list of targets that take this extension
    std::map<unsigned int, std::vector<unsigned int> > m_AddressMap;
};

}
}

#endif
