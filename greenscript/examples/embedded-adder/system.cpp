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

#include "gen.h"
#include "disp.h"

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
  gs::ctr::GC_Core       core("ControlCore");
  gs::cnf::ConfigDatabase *db = new gs::cnf::ConfigDatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin("ConfigPlugin", db);
  gs::cnf::LuaFile_Tool luareader("luareader");
  //////////////////////////////////////////////


  //
  // SystemC world
  //

  // Signals
  sc_signal<int> A("A"), B("B"), C("C");
  sc_clock clk("clk", 10, SC_NS);

  // Module: number generators
  gen g1("g1"), g2("g2");
  g1.out(A);
  g1.clk(clk);
  g2.out(B);
  g2.clk(clk);
  luareader.config("config.lua");   // config for both generators

  // Module: display
  disp d("d");
  d.in(C);


  //
  // Python world
  //

  // Instanciate a container module to load Python scripts
  GreenScriptModule gs_module("gs_module", "adder.py");

  // Module: adder (and port bind)
  gs_module.run("my_adder = adder('A','B','C')");


  //
  // RUN
  //

  sc_start(sc_time(100, SC_NS));

  return 0;
}
