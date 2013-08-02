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

#ifndef __ADDER_H__
#define __ADDER_H__

#include <systemc>


SC_MODULE(adder)
{
  sc_core::sc_in<int> in1;
  sc_core::sc_in<int> in2;
  sc_core::sc_out<int> out;

  void doit()
  {
    out.write(in1.read() + in2.read());
  }

  SC_HAS_PROCESS(adder);

  adder(sc_core::sc_module_name name)
    : sc_module(name)
  {
    SC_METHOD(doit);
    sensitive << in1 << in2;
  }
};

#endif // __ADDER_H__
