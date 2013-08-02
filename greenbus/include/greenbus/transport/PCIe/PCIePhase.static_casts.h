// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
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

// included by PCIe.h


#ifndef __PCIePhase_static_casts_h__
#define __PCIePhase_static_casts_h__


namespace tlm {
namespace PCIe { 

 
  /// PCIe phases
  /**
   * Based on the gp::GenericPhase class
   */
  class PCIePhase  // Phases for the PCIe high level protocol
  : public virtual gp::GenericPhase
  {
    
  public:
    
    /// the phases of the PCIe protocol
    enum {
      Request = LAST_GENERIC_PHASE,
      Ack,
      LAST_PCIE_PHASE
    };
    
//    gs_uint32 state;
//    GenericSimulationModeType sim_mode;
    
    /// Number of data atoms sent in this transaction
//    gs_uint32 BurstNumber;
    


    
    /**
     * Create phase with default state (Idle).
     */
    PCIePhase() 
      : gp::GenericPhase()
    { 
    }

    PCIePhase(sc_event &_initiator_update_event, sc_event &_target_update_event) 
      : gp::GenericPhase(_initiator_update_event, _target_update_event)
    { 
    }
                                                                                      

    /**
     * Create phase.
     * @param s Phase state
     */
    PCIePhase(gs_uint32 s)
      : gp::GenericPhase(s)
    { 
    }

    /**
     * Create phase.
     * @param s Phase state
     * @param b Burst number
     */
    PCIePhase(gs_uint32 s, gs_uint32 b)
      : gp::GenericPhase(s, b)
    {
    }
    
    /// copy constructor
    PCIePhase(const PCIePhase &p) 
      : gp::GenericPhase(p)
    {
    }

    /// copy operator
    PCIePhase & operator=(const PCIePhase &p) {
      gp::GenericPhase::operator=(p);
      return *this;
    }

    /// has a request atom been sent by the master?
    inline bool isRequest() { return state == Request; }
    /// has a request been acknowledged by the slave?
    inline bool isAck() { return state == Ack; }
    
    /// get a string description of the current protocol phase
    std::string toString() { 
      if (isRequest()) return std::string("Request");
      else if (isAck()) return std::string("Ack");
      else {
        //std::stringstream ss;
        //ss << "unknown phase (state=" << state 
        //   << ", simulation mode=" << sim_mode << ")";
        //return ss.str();
        return gp::GenericPhase::toString();
      }
    }
  };


} // end namespace PCIe
} // end namespace tlm


#endif
