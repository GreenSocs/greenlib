// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf, Robert Guenzel
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

#ifndef __PCIeRouter_if_h__
#define __PCIeRouter_if_h__

namespace tlm {
namespace PCIe {

//---------------------------------------------------------------------------
/**
 * The PCIe router interface.
 */
//---------------------------------------------------------------------------
class PCIeRouter_if 
: public virtual sc_interface 
{
  
public:

  virtual PCIeRouterDevicePort* getDownstreamPort() =0;
  virtual PCIeBidirectionalPort* getUpstreamPort() =0;
  void add_Memory_address_range_to_device(MAddr base, MAddr high, 
                                          unsigned int bus_no, unsigned int device_no);
  void add_IO_address_range_to_device(MAddr base, MAddr high, 
                                          unsigned int bus_no, unsigned int device_no);
  //virtual unsigned int decodeAddress(MAddr addr) =0;
  void add_IO_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port);
  void add_Memory_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port);
  void add_Memory_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port,
                        MAddr base, MAddr high);
  void add_IO_address_range(tlm_port<tlm_b_if<PCIeTransactionHandle >, 
                        unevenpair<PCIeTransactionHandle, PCIePhase> > &port,
                        MAddr base, MAddr high);
  
};

} // end namespace PCIe
} // end namespace tlm

#endif
