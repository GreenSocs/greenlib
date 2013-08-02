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

SC_MODULE(disp)
{
  sc_core::sc_in<int> in;

  void do_disp()
  {
    //printf("disp: time %lf val is %d\n", sc_core::sc_time_stamp(), in.read());
    std::cout << "disp: time " << sc_core::sc_time_stamp() << " val is " <<  in.read() << std::endl;
  }

  SC_CTOR(disp)
  {
    SC_METHOD(do_disp);
    sensitive << in;
  }
};
