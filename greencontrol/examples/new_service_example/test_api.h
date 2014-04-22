//   GreenControl framework
//
// LICENSETEXT
//
//   Copyright (C) 2010 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
//
//
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT

// doxygen comments

#ifndef __test_api_h__
#define __test_api_h__

// GreenControl Core
#include "greencontrol/core.h"

// Service related stuff
#include "test_globals.h"
#include "test_command_if.h"


namespace gs {
namespace test {

  
  /// Test Service User API
  /**
   * This API is the connection to the Test_Plugin via the 
   * GreenControl framework.
   *
   * The API derives from test_command_if to get the command_if methods implemented
   * in the same way for the Plugin and the User API.
   *
   * The API derives from test_service_ID to be able to specify the Service ID when
   * constructing the gc_port. Alternatively this could have a member of type test_service_ID;
   * it does not matter how much object of test_service_ID exist in the system, they will all
   * map to the same Service ID.
   */
  class Test_Api
    : public gs::ctr::gc_port_if
    , public test_command_if
    , public test_service_ID
  {
  public:

    /// Constructor Test_Api
    /**
     * The User API name (given to m_gc_port and m_name) can be chosen arbitraryly!
     * The name chosen in this example adds the "_API" postfix to the service name,
     * which is not required this way. Just don't use the same name as the Plugin
     * or any other User API (there is no automatic check for duplicated!).
     */
    Test_Api(const char* name)
    : m_gc_port(*this/*test_service_ID*/, test_service_ID::get_Service_ID_string()+"_API", false, this)
    , m_name(test_service_ID::get_Service_ID_string()+"_API")
    { 
      std::cout << "This is User API '" << m_name << std::endl;
    }

    /// Destructor Test_Api
    ~Test_Api() {
    }
    
    /// Called by gc_port through gc_port_if when transaction arrives (gc_port_if).
    /**
     * Implements pc_port_if.
     * This method starts whenever a Service Plugin or User API sent a transaction to this User API
     */
    void transport(gs::ctr::ControlTransactionHandle &tr)
    {
      TEST_DUMP_N(name(), " Got transaction, command="<<getCommandName(tr->get_mCmd()));

      // show received Transaction
      TEST_DUMP_N(name(), "  received transaction: "<<(tr->toString()).c_str());      

      bool success = false;
      // According to the command fill the transaction or make actions
      switch (tr->get_mCmd()) {

          // TODO cases
          
        default:
          SC_REPORT_WARNING(name(), "Unknown command in transaction");
          success = false;
        }

        if (!success)
          tr->set_mError(1);
    }
    
    /// Returns the name of the API (implements command_if and test_command_if).
    const std::string getName() { return m_name; }
    
    /// Returns the name of the plugin
    const char* name() const { return m_name.c_str(); }
    
    // ///////////////////////////  /////////////////////////  ///////////////////////// //
    // ///////////////////////////  User API specific methods  ///////////////////////// //
    // ///////////////////////////  /////////////////////////  ///////////////////////// //

    /// Test API call sending something and returning something
    unsigned int get_random(const std::string salt) {
      
      gs::ctr::ControlTransactionHandle th = m_gc_port.createTransaction(); // get transaction from pool
      
      // set data
      th->set_mService(get_Service_ID());
      th->set_mCmd(CMD_TEST_1);
      // set some extension data
      test_extension snd_ext;
      snd_ext.m_extension_data.push_back("DATA1");
      snd_ext.m_extension_data.push_back("DATA2");
      snd_ext.m_extension_data.push_back(salt);
      th->set_extension(&snd_ext); // Note: this is only possible once per extension type per transaction!
      // set some extension data for return message
      test_return_extension ret_ext;
      th->set_extension(&ret_ext); // Note: this is only possible once per extension type per transaction!
            
      m_gc_port->transport(th); // send to plugin
      
      // handle returned errors
      if (th->get_mError() != 0) {
        std::cout << "some error" << std::endl;
        return 0;
      }
      assert(ret_ext.m_extension_data != 0 && "The plugin does never return a 0 here if no error was there!");
      return ret_ext.m_extension_data;
    }
    
    
    
    // ///////////////////////////  /////////////////////////  ///////////////////////// //
    // ///////////////////////////  /////////////////////////  ///////////////////////// //

  protected:
    
    /// Port to send and receive ControlTransactionContiners.
    gs::ctr::gc_port m_gc_port;

    /// API name for debug
    const std::string m_name;

  };

    
} // end namespace av
} // end namespace gs

#endif
