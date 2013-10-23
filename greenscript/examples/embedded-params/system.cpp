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

#include <greenscript/greenscript.h>
#include <greencontrol/config.h>
#include <iostream>
#include <string>

using namespace sc_core;


class mod : public sc_module {
SC_HAS_PROCESS(mod);
public:
  mod(sc_module_name n) : sc_module(n),
    james("james"),
    kate_fish_names("kate.fish.names"),
    mixup("mixup"),
    bad_dog("jamesh.bad.dog"),
    dummy("no_such_name")
  {
    SC_THREAD(go);
    if(bad_dog) std::cout << "Bad Dog, Bad Programme" << std::endl;
    else std::cout << "Good Dog, Good Programme" << std::endl;

    std::cout << "size of dummy array is " << dummy.size() << std::endl;
  }
private:
  void go() {
    wait(10, SC_NS);
    std::cout << "james = " << james << std::endl;

    for(unsigned i=0; i<kate_fish_names.size(); i++) {
      wait(10, SC_NS);
      std::cout << "kate[" << i << "] = " << kate_fish_names[i] << std::endl;
    }

    wait(10, SC_NS);
    gs::gs_param_array mb("bob", mixup);
    gs::gs_param<std::string> mbq("quality", mb);
    std::cout << "mixup.bob.quality = " << mbq << std::endl;
  }

  gs::gs_param<int> james;
  gs::gs_param<std::string *> kate_fish_names;
  gs::gs_param_array mixup;
  gs::gs_param<bool> bad_dog;
  gs::gs_param<int *> dummy;
};


int sc_main(int argc, char *argv[]) {
  std::cout << "in sc_main" << std::endl;

  // Instantiate a container module to load Python scripts
  gs::script::GreenScriptModule gs_module("gs_module", "embedded_params.py");
  std::cout << "added a Python interpreter" << std::endl;

  // And an SC module to check for correct DB access
  mod my_mod("sc_module");
  std::cout << "added a SystemC component" << std::endl;

  // Simulation
  sc_start(sc_time(1000, SC_NS));
  sc_stop();

  std::cout << "sc_main completed simulation and exiting" << std::endl;
  return 0;
}
