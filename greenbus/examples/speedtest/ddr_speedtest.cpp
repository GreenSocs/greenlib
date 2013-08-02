// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
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

#include "ddr_speedtest.h"

#include "speedtest_top.h"
#include "greenbus/utils/gb_config.h"

/**
 * GreenBus benchmark
 */
int main(int argc, char *argv[]) {

  if(argc<2) {
    printf("\nUsage: %s <gb_config_file> <simulation_time_in_ns>\n\n", argv[0]);
    return EXIT_SUCCESS;
  }

  std::string s(argv[1]); 
  if (s.find("nodes")!=std::string::npos) {
    if(argc<3) {
      printf("\n%s: Usage with nodes config file requires delay_max specification!\n\n", argv[0]);
      return EXIT_FAILURE;
    }
    printf("\n------------------------------------------------------------------------------------\nSetting delay max to: %d\n", atoi(argv[2]));
    unsigned delay_max=atoi(argv[2]);
    speedtest_top *top = new speedtest_top("top", argv[1], delay_max);
    gb_config(argv[1]);  
    sc_start();
    return EXIT_SUCCESS;
  }

  else {
    pseudo_master_simplebus *master = new pseudo_master_simplebus("master");
    simplebusSlavePort *slaveport = new simplebusSlavePort("memoryport");
    ddr *memory = new ddr("memory");
    slaveport->slave_port(*memory);
    gb_config(argv[1]);  
  }

  if (argc>2) {
    cout<<endl<<"Speedtest running for "<<atoi(argv[2])<<"ns"<<endl<<endl;
    sc_start(atoi(argv[2]), SC_NS);
  }
  else {
    cout<<endl<<"Speedtest running until end of simulation"<<endl<<endl;
    sc_start();
  }
  
  return EXIT_SUCCESS;
}

