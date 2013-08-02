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


class bidir_module
//  : public sc_core::sc_module, public gs::msg::receiver_proxy
  : public sc_core::sc_module
{

public:

  SC_HAS_PROCESS(bidir_module);

  /// Constructor
  bidir_module(sc_core::sc_module_name name)
//    : sc_module(name), receiver_proxy(msgIn,sc_module::name())
    : sc_module(name), msgIn(name)
  {
    SC_THREAD(sender_action);
    SC_THREAD(receiver_action);
  }

  void sender_action()
  {
    wait(1,sc_core::SC_NS);

    gs::msg::Message msg;
    msg["sender"] = "sender";
    msg["command"] = "start";
    msg["reply"] = reply.name();

    // Send to receiver
    std::cout << name() << ": sending a message to receiver: start" << std::endl;
    try {
      gs::msg::WriteIf & receiver1 = gs::msg::findReceiver("gs_module.receiver");
      receiver1.write(msg);
    }
    catch (gs::msg::invalid_receiver e) {
      // do something about the error
      std::cout << name() << ": receiver named " << e.name << " is invalid."
                << " (" << e.what() << ")" << std::endl;
      exit(1);
    }

    // Read reply
    std::cout << name() << ": waiting for reply" << std::endl;
    gs::msg::Message replyMsg = reply.read();
    std::cout << name() << ": read reply. Field `foo.test' is " << replyMsg["foo"]["test"] << std::endl;
  }

  void receiver_action()
  {
    std::cout << name() << ": reading fifo for incomming messages" << std::endl;
    gs::msg::Message msg = msgIn.read();
    std::cout << name() << ": received a message with command: " << msg["command"] << ", value=" << msg["value"] << std::endl;
    std::cout << "  and boolean members " << (msg["boolt"] ? "-yes-" : "-no-") << " and " << (msg["boolf"] ? "-yes-" : "-no-") << std::endl;
    std::cout << "  and hierarchical members " << msg["sub"]["hello"] << " and " << msg["sub"]["fish"] << std::endl;

    gs::msg::WriteIf& reply = gs::msg::findReceiver(msg["reply"]);
    msg["foo.pi"] = 3.14159;
    reply.write(msg);
    std::cout << name() << ": replyed the same msg" << std::endl;

    wait(20,sc_core::SC_NS);
    gs::msg::WriteIf &rx = gs::msg::findReceiver("gs_module.py_no_process");
    for(int i=0; i<10; i++) {
      gs::msg::Message msg2;
      msg2["fish"] = 1000 - i*i;
      rx.write(msg2);
    }

    gs::msg::WriteIf &rx2 = gs::msg::findReceiver("gs_module.second_rx");
    for(int i=0; i<10; i++) {
      gs::msg::Message msg2;
      msg2["fish"] = 2000 - i*i;
      rx2.write(msg2);
    }
    for(int i=10; i<20; i++) {
      wait(1,sc_core::SC_NS);
      gs::msg::Message msg3;
      msg3["fish"] = 2000 - i*i;
      rx2.write(msg3);
    }
  }

private:
  gs::msg::MessageFifo reply;
//  sc_core::sc_fifo<gs::msg::Message> msgIn;
  gs::msg::MessageFifo msgIn;
};

