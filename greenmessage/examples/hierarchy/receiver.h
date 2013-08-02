// LICENSETEXT
//
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//
//   Marcus Bartholomeu
//     GreenSocs Ltd
//
//
//   This program is free software.
//
//   If you have no applicable agreement with GreenSocs Ltd, this software
//   is licensed to you, and you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   If you have a applicable agreement with GreenSocs Ltd, the terms of that
//   agreement prevail.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
//   02110-1301  USA
//
// ENDLICENSETEXT

// Include GreenMessage
#include <greenmessage/greenmessage.hpp>


class receiver_module
  : public sc_core::sc_module,
    public gs::msg::receiver_proxy
{

public:

  SC_HAS_PROCESS(receiver_module);

  /// Constructor
  receiver_module(sc_core::sc_module_name name)
    : sc_core::sc_module(name),
      gs::msg::receiver_proxy(msgIn,name)
  {
    SC_THREAD(main_action);
  }

  /// Main action
  void main_action()
  {
    std::cout << name() << ": reading fifo for incomming messages" << std::endl;
    gs::msg::Message msg = msgIn.read();
    std::cout << name() << ": received a message with command: " << msg["command"] << std::endl;

    // Did we get a configset (tree) inside the message?
    gs::msg::Message& configset = msg["configset"];
    if (configset.size() != 0) {

      // Print first level names
      std::cout << name() << ": received a configset. First level names:" << std::endl;
      gs::msg::Message::iterator i;
      for (i = configset.begin(); i != configset.end(); ++i) {
        std::cout << name() << ":    " << i->first << std::endl;
      }

      // Test existence (value) of an specific parameter
      std::cout << name() << ": testing if cache.size parameter exists: cache.size = ";
      if (configset["cache.size"]) {
        std::cout << configset["cache.size"] << std::endl;
      } else {
        std::cout << "(not defined)" << std::endl;
      }

    }
    else {
      std::cout << name() << ": received an empty configset." << std::endl;
    }

    gs::msg::WriteIf& reply = gs::msg::findReceiver(msg["reply"]);
    msg["foo"] = 123.456;
    reply.write(msg);
    std::cout << name() << ": replyed the same msg" << std::endl;
  }

private:
  sc_core::sc_fifo<gs::msg::Message> msgIn;
};
