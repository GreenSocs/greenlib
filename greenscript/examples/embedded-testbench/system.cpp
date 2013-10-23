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

#include "adder.h"

// GreenControl
#include <greencontrol/core/gc_core.h>    // Core

// GreenConfig
#include <greencontrol/config.h> // ConfigPlugin
#include <greencontrol/gcnf/plugin/configdatabase.h> // ConfigDatabase to be bound to port of ConfigPlugin
#include <greencontrol/gcnf/apis/luaFileApi/luafile_tool.h> // API Tool to read lua configuration file(s)


using namespace sc_core;
using namespace gs::script;


int sc_main(int argc, char *argv[])
{

  ////////// Defines for GreenControl //////////
  gs::ctr::GC_Core       core();
  gs::cnf::ConfigDatabase *db = new gs::cnf::ConfigDatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin(db);
  gs::cnf::LuaFile_Tool luareader("luareader");
  //////////////////////////////////////////////

  // Signals
  sc_signal<int> A("A"), B("B"), C("C");

  // Read configuration
  luareader.config("config.lua");


  //
  // SystemC world
  //

  // Device to Test: adder
  adder a("a");
  a.in1(A);
  a.in2(B);
  a.out(C);


  //
  // Python world
  //

  // Instanciate a container module to load Python scripts
  GreenScriptModule gs_module("gs_module", "testbench.py");

  // Module: testbench (and port bind)
  gs_module.run("my_tb = testbench('A','B','C')");


  //
  // RUN
  //

  sc_start();

  return 0;
}
