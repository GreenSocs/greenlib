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

#ifndef __ship_datatypes_h__
#define __ship_datatypes_h__


namespace tlm {

//---------------------------------------------------------------------------
/**
 * SHIP command definitions 
 * @see shipSlaveAPI.waitEvent()
 */
//---------------------------------------------------------------------------
///SHIP commands
enum ship_command_enum {
  ///A master requests data from a slave
  SHIP_REQUEST=0, 
  ///A master sends data to a slave
  SHIP_SEND, 
  ///No active command available
  SHIP_NONE
};

///SHIP operation modes
enum ship_mode {
  ///PV mode, uses b_transact
  SHIP_MODE_PV=0,
  ///BA performance, transfers SHIP object by reference (requires SHIP APIs at both ends of the channel)
  SHIP_MODE_BA_P=1,
  ///BA compatibility mode, transfers serialized SHIP object (connect a SHIP API with another user API)
  SHIP_MODE_BA_C=2,
  ///CT mode, transfers serialized SHIP object and sends a notification for each data cycle
  SHIP_MODE_CT=3
};
  
  

///This struct is returned by the waitEvent method. 
/**
 * @see ship_slave_if.waitEvent()
 */
struct ship_command {
  ///The actual command of this ship-command
  /**
   * @see command
   */
  ship_command_enum cmd;

  ///The burstlength of the current command.
  /**
   * This burstlength information may be necessary when 
   * the serialisation length isn't fixed
   */
  gs_uint32 burstlength;
};

} // namespace tlm


#endif
