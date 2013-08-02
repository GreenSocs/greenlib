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
  : public sc_core::sc_module, public gs::msg::receiver_base
{

public:

  SC_HAS_PROCESS(sender_module);

  /// Constructor
  sender_module(sc_core::sc_module_name n)
    : sc_core::sc_module(n),
      gs::msg::receiver_base(std::string("writeif_") + name())
      // note: separate names for module and module-as-receiver,
      // but it would also work to use the same name
  {
    SC_THREAD(send_action);
  }

  /// Send action: send a message then exit
  void send_action()
  {
    wait(1,sc_core::SC_NS);

    gs::msg::Message msg;
    msg["sender"] = "sender";
    msg["command"] = "start";
    msg["reply"] = std::string("writeif_") + name();
    gs::msg::Message subm;
    subm["note"] = "james";
    subm[2] = "jon";
    subm[21] = 57.57;
    msg["ten"] = subm;
    gs::msg::Message subm2;
    for(int i=0; i<7; i++) subm2[i] = i*i;
    msg["squares"] = subm2;
    msg["x.y.z"] = "hello";

    msg["jj"] = 10;
    msg["jj"]["kk"] = 20;
    std::cout << msg["jj"] << " " << msg["jj.kk"] << std::endl;
    const gs::msg::Message &x = msg["rowlocks"];
    std::cout << msg["non-present"] << " " << x << std::endl;

#define FIND(x) ((msg.find(x) != 0) ? x : "-") << " "
    std::cout << "some found keys " <<
      FIND("jj") << FIND("non-present") << FIND("jj.kk") <<
      FIND("kk") << FIND("sender") << FIND("ten.21") << std::endl;

    msg.erase("jj.kk");
    msg.erase("non-present");
    msg.erase("kk");
    std::cout << "some found keys " <<
      FIND("jj") << FIND("non-present") << FIND("jj.kk") <<
      FIND("kk") << FIND("sender") << FIND("ten.21") << std::endl;

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
  }

  /// Callback, used for replies
  void write(const gs::msg::Message &replyMsg)
  {
    // Read reply
    std::cout << name() << ": got a reply" << std::endl;
    std::cout << name() << ": read reply. Field `foo' is " << replyMsg["foo"] << std::endl;
    std::cout << name() << ": read reply. Field `ten' is " << float(replyMsg["ten"][21]) << " ";
    std::cout << replyMsg["ten"][2] << " " << replyMsg["ten"]["note"] << std::endl;
    std::cout << name() << ": read reply.  Field 'squares' contains ";
    int m = replyMsg["squares"].max_numerical_key();
    for(int i=0; i<=m; i++) {
      std::cout << int(replyMsg["squares"][i]) << " ";
    }
    std::cout << std::endl;
    std::cout << "x = ]" << replyMsg["x"] << "[\n";
    std::cout << "x.y = ]" << replyMsg["x.y"] << "[  ]" << replyMsg["x"]["y"] << "[\n";
    std::cout << "x.y.z = ]" << replyMsg["x.y.z"] << "[  ]" << replyMsg["x.y"]["z"] << "[  ]" << replyMsg["x"]["y.z"] << "[  ]" << replyMsg["x"]["y"]["z"] << "[\n";

    std::cout << replyMsg["jj"] << std::endl;
    const gs::msg::Message &x = replyMsg["rowlocks"];
    std::cout << x << std::endl;

#define CFIND(x) (replyMsg.find(x) ? x : "-") << " "
    std::cout << "some found keys " <<
      CFIND("jj") << CFIND("non-present") << CFIND("jj.kk") <<
      CFIND("kk") << CFIND("sender") << CFIND("ten.21") << std::endl;

    std::cout << "\n\n";
    for(gs::msg::Message::children_map::const_iterator ix = replyMsg.begin();
      ix != replyMsg.end(); ix++)  std::cout << ix->first << " ]" << ix->second << "[\n";
  }

  /// Other elements of the WriteIf must be implemeted
  bool nb_write(const gs::msg::Message &m)
  {
    write(m);  // in this component, always non-blocking
    return true;
  }
  int num_free() const {return 1;}
  const sc_core::sc_event &data_read_event() const {return m_event;}

private:
  sc_core::sc_event m_event;
};

