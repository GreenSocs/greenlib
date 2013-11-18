// LICENSETEXT
//
//   Copyright (C) 2009 : GreenSocs Ltd
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

#include <iostream>
#include "greenmemory/sparse_mem.h"

using namespace std;


int main(int argc, char* argv[])
{
  unsigned addr = 5;

  if (argc > 1)
    addr = atoi(argv[1]);

  cout << "Searching " << addr << endl;

  gs::mem::sparse_mem<10> mem;

  mem[addr] = 10;
  mem[addr+15] = 20;
  mem[addr+115] = 43;
  cout << "mem[" << addr << "] has " << (unsigned) mem[addr] << endl;
  cout << "mem[" << addr+15 << "] has " << (unsigned) mem[addr+15] << endl;
  cout << "mem[" << addr+115 << "] has " << (unsigned) mem[addr+115] << endl;

  cout << "get_block 2 to 30" << endl;
  gs::mem::sparse_mem<10>::pointer block = mem.get_block(2,30);
  cout << "end get_block 2 to 30" << endl;
  if (addr < 30) {
    cout << "block[" << addr-2 << "] has " << (unsigned) block[addr-2] << endl;
    cout << "setting block[" << addr-2 << "] = 123" << endl;
    block[addr-2] = 123;
    cout << "check if changed the memory: mem[" << addr << "] has " << (unsigned) mem[addr] << endl;
  }

}
