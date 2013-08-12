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

#include "sender.h"


  /// Constructor
  sender_module::sender_module(sc_core::sc_module_name name)
    : sc_module(name)
  {
    SC_THREAD(main_action);
  };

  /// Main action
  void sender_module::main_action()
  {
    wait(1,sc_core::SC_NS);

    gs::msg::Message msg;
    msg["from"] = "sender";
    msg["command"] = "start";
    msg["reply"] = reply.name();

    // Create a group of receivers by adding members
    std::cout << name() << ": creating a group of receivers by adding members receiver1 and receiver3" << std::endl;
    gs::msg::WriteIfGroup my_receivers;
    my_receivers.insert(gs::msg::findReceiver("receiver1"));
    my_receivers.insert(gs::msg::findReceiver("receiver3"));

    // Send to the group
    std::cout << name() << ": sending a message to the group: start" << std::endl;
    my_receivers.write(msg);

    // Read 2 replies
    for (int i=0; i<2; ++i) {
      std::cout << name() << ": waiting for reply" << std::endl;
      gs::msg::Message replyMsg = reply.read();
      std::cout << name() << ": read reply. Field `from' is " << replyMsg["from"] << std::endl;
    }

    // Create a group of receivers by removing members
    std::cout << name() << ": creating a group of receivers by removing members self and receiver2" << std::endl;
    gs::msg::WriteIfGroup my_receivers2;
    my_receivers2 = gs::msg::broadcast();
    my_receivers2.erase(gs::msg::findReceiver(reply.name()));
    my_receivers2.erase(gs::msg::findReceiver("receiver2"));

    // Send to the group
    std::cout << name() << ": sending a message to the group: stop" << std::endl;
    msg["command"] = "stop";
    my_receivers2.write(msg);

    // Read 2 replies
    for (int i=0; i<2; ++i) {
      std::cout << name() << ": waiting for reply" << std::endl;
      gs::msg::Message replyMsg = reply.read();
      std::cout << name() << ": read reply. Field `from' is " << replyMsg["from"] << std::endl;
    }
  };
