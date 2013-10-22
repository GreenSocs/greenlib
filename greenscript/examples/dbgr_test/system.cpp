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

#include <systemc>
#include <greenscript/greenscript.h>
#include <iostream>


class mod : public sc_core::sc_module {
SC_HAS_PROCESS(mod);
public:
  mod(sc_core::sc_module_name n): sc_core::sc_module(n) {
    SC_THREAD(go);
  }
  void go() {
    while(true) {
      wait(1.0,sc_core::SC_NS);
      for(int i=0; i<80; i++) {
        std::cout << '.';
        for(int j=0; j<10000; j++) ;
      }
      std::cout << std::endl;
    }
  }
};


int sc_main(int argc, char *argv[])
{
  std::cout << "in sc_main\n";

  // Instanciate a container module to load Python scripts
  gs::script::GreenScriptModule gs_module("gs_module", "simple.py");
  std::cout << "added a Python interpreter\n";

  mod my_mod("dumdum");
  std::cout << "added a dummy C++ component\n";

  sc_core::sc_start(1.0,sc_core::SC_US);
  sc_core::sc_stop();

  std::cout << "sc_main completed simulation and exiting\n";

  return 0;
}

