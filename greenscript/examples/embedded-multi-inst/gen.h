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

#ifndef __GEN_H__
#define __GEN_H__

#include <systemc>

#include <greencontrol/config.h>


SC_MODULE(gen)
{
  sc_core::sc_in<bool> clk;
  sc_core::sc_out<int> out;

  void do_gen()
  {
    std::cout << name() << ": gen " << val << "\n";
    out.write( val++ );
  }

  SC_HAS_PROCESS(gen);

  gen(sc_core::sc_module_name name)
    : sc_module(name), val("val")
  {
    SC_METHOD(do_gen);
    sensitive<< clk.pos();
  }

 protected:
  gs::cnf::gs_param<int> val;

};

#endif // __GEN_H__
