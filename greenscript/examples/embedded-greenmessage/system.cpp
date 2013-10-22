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

// Defines GreenScriptModule
#include <greenscript/greenscript.h>

// Defines the sender_module
#include "bidir.h"

#include <string>
#include <iostream>

using namespace sc_core;


int sc_main(int argc, char *argv[])
{
  //
  // SystemC world
  //
  bidir_module    bidir("sc-bidir");


  //
  // Python world
  //

  // Instanciate a container module to load Python scripts
  gs::script::GreenScriptModule gs_module("gs_module", "bidir.py");

  // Instantiate the receiver module
  std::string writeif_type("no_process = ");
  writeif_type += ((argc < 2) ? "blocker" : argv[1]);
  std::cout << writeif_type << std::endl;
  gs_module.run(writeif_type.c_str());
  gs_module.run("mybidir = bidir()");

  //
  // RUN
  //
  sc_start();

  // Need to delete stuff to test Python memory management
  gs_module.run("windup()");

  return 0;
}

