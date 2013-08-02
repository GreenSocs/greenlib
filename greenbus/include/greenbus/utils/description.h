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



#ifndef __DESCRIPTION_H__
#define __DESCRIPTION_H__

#include <string>


namespace tlm {
  

  //---------------------------------------------------------------------------
  /**
   * Base class to include a description in a derived class.
   */
  //---------------------------------------------------------------------------
  template <const char * d>
  class Description {

  public:

    static const char * desc; // the description string

    //---------------------------------------------------------------------------
    /**
     * Return string representation of the type description.
     */
    //---------------------------------------------------------------------------
    const std::string ToString() {return std::string(desc);}

  };

  template <const char * d>
  const char* Description<d>::desc = d;

  
} // end of namespace tlm

#endif


