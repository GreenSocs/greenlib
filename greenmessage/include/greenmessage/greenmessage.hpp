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

#ifndef GS_MSG_H
#define GS_MSG_H

#include <map>
#include <set>
#include <string>
#include <stdexcept> // for std::runtime_error
#include <systemc>
#include <iostream>

#include "tree_map.hpp"
#include "lex_t.hpp"

namespace gs {
  namespace msg {


    //////////////////////////////////////////////////
    // Types

    // Message
    typedef gs::tree_map<lext::lex_t, lext::lex_t> Message;

    // WriteIf
    typedef sc_core::sc_fifo_out_if<Message> WriteIf;

    struct duplicated_name : public std::exception
    {
      duplicated_name(const std::string name, const std::string msg)
        : name(name)
        , msg(msg)
      {};
      virtual ~duplicated_name() throw() {};
      virtual const char* what() const throw() {
        return msg.c_str();
      };
      const std::string name;
      const std::string msg;
    };

    // forward declarations for friend declarations in receiver_base //
    static WriteIf &findReceiver(const std::string&);
    class WriteIfGroup;
    static void findReceivers(WriteIfGroup&);

    // base class for Message receivers //
    class receiver_base : public WriteIf {
      public:
        receiver_base(std::string name) {init(name);}
        receiver_base(const char *name) {init(name);}
        receiver_base(const sc_core::sc_module_name &name) {
          // illegal ctor, user needs explanation
          std::cout << "GreenMessage FATAL:\n";
          std::cout << "  Do not use sc_module_name to construct a receiver_base\n";
          std::cout << "  Use full hierarchical sc_object::name()\n";
          exit(1);
        }
        static void print_registry(std::ostream &os = std::cout) {
          for(std::map<std::string,WriteIf*>::iterator i=receiver_base::registry().begin();
            i!=receiver_base::registry().end(); i++) {
            os << i->first << std::endl;
          }
        }
        virtual ~receiver_base() {
           std::map<std::string,WriteIf*>::iterator i = registry().find(m_registered_name);
           registry().erase(i);
        }
      private:
        std::string m_registered_name;
        void init(std::string name) {
          if(registry().find(name) != registry().end())
            throw duplicated_name(name, "FATAL: GreenMessage receiver with duplicate name: `" + name + "'.");
          registry()[name] = this;
          m_registered_name = name;
        }
        // singleton registry
        static std::map<std::string,WriteIf*> &registry() {
          static std::map<std::string,WriteIf*> the_registry;
          return the_registry;
        };
        friend class receiver_proxy;
        friend WriteIf &findReceiver(const std::string&);
        friend void findReceivers(WriteIfGroup&);
    };


    //////////////////////////////////////////////////
    // Map command strings to handlers

    template<class T>
    class CommandMap {

    public:

      // Pointer to method that handle a command
      typedef void (T::*MethodPtr)(const Message&);

      // Register commands in the map<name,handler>
      void operator()(std::string name, MethodPtr handler) {
        m[name] = handler;
      }

      // Get a command handler function by command name
      MethodPtr operator[](std::string s) {
        return m[s];
      }

    private:
      std::map<std::string,MethodPtr> m;

    };


    //////////////////////////////////////////////////
    // Receiver: basically a workaround for diamond inheritance

    // GetWriteIf
    struct GetWriteIf
    {
      virtual WriteIf& getWriteIf() = 0;
      virtual ~GetWriteIf() {};
    };


    struct receiver_proxy
      : public GetWriteIf
    {
      // Register the object that is controled
      receiver_proxy(WriteIf& obj, std::string name) : obj(obj) {
        init(name);
      }
      receiver_proxy(WriteIf& obj, const char *name) : obj(obj) {
        init(std::string(name));
      }
      ~receiver_proxy() {
        std::map<std::string,WriteIf*>::iterator i =
          receiver_base::registry().find(m_registered_name);
        receiver_base::registry().erase(i);
      }

      WriteIf& getWriteIf() {
        return obj;
      }

      std::string registered_name() {
        return m_registered_name;
      }

    private:
      WriteIf& obj;

      void init(std::string name) {
        if(receiver_base::registry().find(name) != receiver_base::registry().end())
          throw duplicated_name(name, "FATAL: GreenMessage receiver with duplicate name: `" + name + "'.");
        receiver_base::registry()[name] = &obj;
        m_registered_name = name;
      }

      std::string m_registered_name;
    };


    // MessageFifo
    class MessageFifo : public sc_core::sc_fifo<Message>, public receiver_proxy {
      public:
        // explicit prefix given - used by Greenscript
        MessageFifo(const char *prefix, const char *name, int size = 16) :
          sc_core::sc_fifo<Message>(size),
          receiver_proxy(*this, build_name(prefix,name)) {}
        // if no prefix given, use SC to build it automatically
        MessageFifo(const char *name, int size = 16) :
          sc_core::sc_fifo<Message>(name, size),
          receiver_proxy(*this, sc_core::sc_fifo<Message>::name()) {}
        // no name given at all, let SC invent one
        MessageFifo(int size = 16) :
          sc_core::sc_fifo<Message>(size),
          receiver_proxy(*this, std::string(sc_core::sc_fifo<Message>::name())) {}
      private:
        std::string build_name(const char *prefix, const char *name) {
          if(*prefix == 0) return std::string(name);
          return std::string(prefix)+std::string(".")+std::string(name);
        }
    };


    //////////////////////////////////////////////////
    // Function: Get a messenger by module name

    struct invalid_receiver : public std::exception
    {
      invalid_receiver(const std::string name, const std::string msg)
        : name(name)
        , msg(msg)
      {};
      virtual ~invalid_receiver() throw() {};
      virtual const char* what() const throw() {
        return msg.c_str();
      };
      const std::string name;
      const std::string msg;
    };

    static
    WriteIf&
    findReceiver(const std::string& name)
    {
      if(receiver_base::registry().find(name) == receiver_base::registry().end()) {
        // receiver_base::print_registry();
        throw invalid_receiver(name, "FATAL: GreenMessage receiver named `" + name + "' not found.");
      }

      return *receiver_base::registry()[name];
    }

    struct WriteIfGroup
      : public WriteIf
    {
      /// Implement WriteIf
      void write(const gs::msg::Message& msg)
      {
        // send msg to group
        std::set<WriteIf*>::iterator i;
        for (i = _set.begin(); i != _set.end(); ++i) {
          (*i)->write(msg);
        }
      }

      // write ALL or NONE, if possible
      bool nb_write(const gs::msg::Message& msg) {
        if(num_free() == 0) return false;
        bool result = true;
        std::set<WriteIf*>::iterator i;
        for (i = _set.begin(); i != _set.end(); ++i) {
          if(!(*i)->nb_write(msg)) result = false;
        }
        return result;
      }
      // minimum of all num_free() allows cascading
      int num_free() const {
        int min_num_free;
        std::set<WriteIf*>::iterator i;
        for (i = _set.begin(); i != _set.end(); ++i) {
          int a = (*i)->num_free();
          if((i == _set.begin()) || (a < min_num_free)) min_num_free = a;
        }
        return min_num_free;
      }
      // event from fifo with smallest num_free()
      const sc_core::sc_event &data_read_event() const {
        int min_num_free;
        std::set<WriteIf*>::iterator i, ilow;
        for (i = _set.begin(); i != _set.end(); ++i) {
          int a = (*i)->num_free();
          if((i == _set.begin()) || (a < min_num_free)) {
            min_num_free = a;
            ilow = i;
          }
        }
        return (*i)->data_read_event();
      }

      void operator=(gs::msg::WriteIf& wi)
      {
        WriteIfGroup& newset = dynamic_cast<WriteIfGroup&>(wi);
        _set = newset._set;
      }

      void insert(WriteIf& wi)
      {
        _set.insert(&wi);
      }

      void erase(WriteIf& wi)
      {
        _set.erase(&wi);
      }

    protected:
      std::set<WriteIf*> _set;
    };

    static
    void
    findReceivers(WriteIfGroup& receivers)
    {
      for(std::map<std::string,WriteIf*>::iterator i=receiver_base::registry().begin();
          i!=receiver_base::registry().end(); i++) {
        receivers.insert(*(i->second));
      }
    }

    inline WriteIf&
    broadcast()
    {
      static bool initialized = false;

      // Create empty receivers group
      static WriteIfGroup receivers;

      if (!initialized) {
        // fill receivers vector
        findReceivers(receivers);
        // remove duplicates
        // TODO: remove duplicates
        initialized = true;
      }

      return receivers;
    }


    //////////////////////////////////////////////////
    // Function: A high level API to send a message
    inline
    void sendMsg(const char* receiver_name, const gs::msg::Message& msg)
    {
      try {
        WriteIf & receiver = findReceiver(receiver_name);
        receiver.write(msg);
      }
      catch (invalid_receiver e) {
        std::cout << e.what() << std::endl;
        exit(1);
      }
    }


  } // namespace msg
} // namespace gs

#endif // GS_MSG_H
