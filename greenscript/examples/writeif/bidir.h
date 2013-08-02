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


class bidir_module: public sc_core::sc_module {
SC_HAS_PROCESS(bidir_module);
public:
  bidir_module(sc_core::sc_module_name name):
    sc_module(name), msgIn(name, 3) {
    SC_THREAD(sender_action);
    SC_THREAD(receiver_action);
  }

  void sender_action() {
    unsigned x = 0;
    gs::msg::WriteIf &dest = gs::msg::findReceiver("gs_module.rx");
    while(true) {
      wait(sc_core::SC_ZERO_TIME);
      gs::msg::Message msg;
      msg["sender"] = "sender";
      msg["command"] = "test";
      msg["data"] = x++;
      std::cout << "SC send new msg " << x << " to " << dest.num_free()
        << std::endl;
      while(!dest.nb_write(msg)) wait(dest.data_read_event());
    }
  }

  void receiver_action() {
    while(true) {
      wait(11,sc_core::SC_NS);
      while(msgIn.num_available() == 0)
        wait(msgIn.data_written_event());
      std::cout << "          received new msg " << msgIn.read()["data"]
        << std::endl;
    }
  }

private:
  gs::msg::MessageFifo msgIn;
};

