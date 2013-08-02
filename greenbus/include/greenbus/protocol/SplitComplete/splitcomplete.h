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

#ifndef __SPLITCOMPLETE_H__
#define __SPLITCOMPLETE_H__

#include "greenbus/core/attributes.h"
#include "greenbus/transport/generic.h"

/**
 *
 */

using namespace tlm;

namespace SplitComplete {

  namespace SplitCompletePhase
  {
    enum {
      Split = GenericPhase::LAST_GENERIC_PHASE,
      Complete
    };
  };


  // Functor to be used with Notifications
  struct setSplitPhase : public GenericPhase
  {
    setSplitPhase(const GenericPhase & p) : GenericPhase(SplitCompletePhase::Split) {};
    setSplitPhase() : GenericPhase(SplitCompletePhase::Split) {};
  };

  // Functor to be used with Notifications
  struct setCompletePhase : public GenericPhase
  {
    setCompletePhase(const GenericPhase & p) : GenericPhase(SplitCompletePhase::Complete) {};
    setCompletePhase() : GenericPhase(SplitCompletePhase::Complete) {};
  };


  template <class PORT>
  class SplitCompleteTargetAPI :
    public GenericTargetAPI<PORT>
  {

  public:

    SplitCompleteTargetAPI (const char *port_name) :
      GenericTargetAPI<PORT> (port_name),
      Split(*this),
      Complete(*this)
    {}

    Notifications<PORT, setSplitPhase>    Split;
    Notifications<PORT, setCompletePhase> Complete;
  };


  typedef SplitCompleteTargetAPI<target_port<GenericTransaction,GenericSlaveAccess,GenericPhase> > SplitCompleteTargetPort;

}

#endif
