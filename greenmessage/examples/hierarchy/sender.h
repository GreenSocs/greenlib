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


class sender_module
  : public sc_core::sc_module
{

public:

  SC_HAS_PROCESS(sender_module);

  /// Constructor
  sender_module(sc_core::sc_module_name name)
    : sc_module(name)
  {
    SC_THREAD(main_action);
  }

  /// Main action
  void main_action()
  {
    wait(1,sc_core::SC_NS);

    gs::msg::Message msg;
    msg["sender"] = "sender";
    msg["command"] = "start";
    msg["configset.cache.size"] = 10;
    msg["configset.CPU"][0] = "adder";
    msg["configset.CPU"][1] = "multiplexor";
    msg["reply"] = reply.name();

    // Send to receiver
    std::cout << name() << ": sending a message to receiver: start" << std::endl;
    try {
      gs::msg::WriteIf & receiver1 = gs::msg::findReceiver("receiver");
      receiver1.write(msg);
    }
    // test some error when sending
    // (eg. (a) receiver not found; (b) found but don't have WriteIf)
    catch (gs::msg::invalid_receiver e) {
      // do something about the error
      std::cout << name() << ": receiver named " << e.name << " is invalid."
                << " (" << e.what() << ")" << std::endl;
      exit(1);
    }

    // Read reply
    std::cout << name() << ": waiting for reply" << std::endl;
    gs::msg::Message replyMsg = reply.read();
    std::cout << name() << ": read reply. Field `foo' is " << replyMsg["foo"] << std::endl;
  }

private:
  gs::msg::MessageFifo reply;
};
