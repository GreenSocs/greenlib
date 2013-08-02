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


#ifndef __PCIePhase_dynamic_casts_h__
#define __PCIePhase_dynamic_casts_h__


namespace tlm {
namespace PCIe { 

 
  /// PCIe phases
  /**
   * Based on th GenericPhase class (see generic.dynamic_casts.h).
   */
  class PCIePhase  // Phases for the PCIe high level protocol
  : public virtual GenericPhase
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
      //: GenericPhase()//state(Idle), sim_mode(MODE_BA), BurstNumber(0)
    { 
      state = Idle; sim_mode = MODE_BA; BurstNumber=0;
      //cout << "a create PCIe phase def=0" << endl;
    }

    PCIePhase(sc_event &_initiator_update_event, sc_event &_target_update_event) 
      : GenericPhase(_initiator_update_event, _target_update_event)// state(Idle), sim_mode(MODE_CT), BurstNumber(0)
    { 
      state = Idle; sim_mode = MODE_BA; BurstNumber=0;
      //cout << "b create PCIe phase def=0" << endl; 
    }
                                                                                      

    /**
     * Create phase.
     * @param s Phase state
     */
    PCIePhase(gs_uint32 s)
      //: GenericPhase(s)//state(s), sim_mode(MODE_BA), BurstNumber(0)
    { 
      state = s; sim_mode = MODE_BA; BurstNumber=0;
      //cout << "c create PCIe phase ("<<s<<") "<< this->state <<" ="<<toString()<< endl; 
    }

    /**
     * Create phase.
     * @param s Phase state
     * @param b Burst number
     */
    PCIePhase(gs_uint32 s, gs_uint32 b)
      //: GenericPhase(s, b)//state(s), sim_mode(MODE_BA), BurstNumber(b)
    {
      // TODO: warum funcktioniert der Aufruf vom base constructor nicht???
      state = s; sim_mode = MODE_BA; BurstNumber=b;
      //cout << "d create PCIe phase ("<<s<<") "<< this->state <<" ="<<toString()<< endl;
    }
    
    /// copy constructor
    PCIePhase(const PCIePhase &p) 
    //  : GenericPhase(p)
    {
      *this = p; // use copy-operator
      //cout << "e copy PCIe phase ("<<p.state<<") "<< this->state <<" ="<<toString()<< endl;
    }

    /// copy operator
    PCIePhase & operator=(const PCIePhase &p) {
      if (&p==this)
        return *this;
      state=p.state;
      sim_mode=p.sim_mode;
      BurstNumber=p.BurstNumber;
      return *this;
    }
/*
    /// set the simulation mode for this phase
    inline void setSimulationMode(GenericSimulationModeType m) {
      sim_mode = m;
    }

    /// get the simulation mode for this phase
    inline GenericSimulationModeType getSimulationMode() {
      return sim_mode;
    }
*/   
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
        return GenericPhase::toString();
      }
    }
  };


} // end namespace PCIe
} // end namespace tlm


#endif
