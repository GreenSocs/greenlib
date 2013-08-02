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


#ifndef __PCIeDefs_h__
#define __PCIeDefs_h__

#include "greenbus/core/attributes.h"

namespace tlm {
namespace PCIe { 


  GSGB_ATTRIBUTE (MPCIeCmd, gs_uint32);      // PCIe command (used with enum PCIeCmd)
  GSGB_ATTRIBUTE (MTrafficClass, gs_uint32); // Traffic Class (0 to 7)
  GSGB_ATTRIBUTE (MDigestExists, bool);      // If a digest exists
  GSGB_ATTRIBUTE (MECRCpoisoned, gs_uint32); // If data is corrupt
  // MBurstLength already exists, see attributes.h
  // MAddr        already exists, see attributes.h // Address  or  bus and device numbers
  GSGB_ATTRIBUTE (MIsIDbasedRouting, bool);  // if mXXNo fields have to be interpreted for ID based routing
  GSGB_ATTRIBUTE (MIsImplicitRouting, bool); // if mMessageType field has to be interpreted for inplicit routing
  // MConnID      already exists, see attributes.h // Function number
  GSGB_ATTRIBUTE (MBusNo, gs_uint32);     // bus      number for ID based routing
  GSGB_ATTRIBUTE (MDevNo, gs_uint32);     // device   number for ID based routing
  typedef MConnID MFuncNo;           // function number for ID based routing
  GSGB_ATTRIBUTE (MRegNo, gs_uint32);     // register number for ID based routing
  // MByteEn      already exists, see attributes.h // Byte Enable for first and last 64bit DW
  // MID          already exists, see attributes.h // Address for Requester ID (route back)
  // MTagID       already exists, see attributes.h // Unique identifier for outstanding requests (0...32 or 0...256)
  GSGB_ATTRIBUTE (MConfigAddr, gs_uint32);   // Address for function registers
  GSGB_ATTRIBUTE (MMessageType, gs_uint32);  // (Routing) type of the Message (see enum)
  GSGB_ATTRIBUTE (MMessageCode, gs_uint32);  // Code dependent on Message Type
  GSGB_ATTRIBUTE (MComplStatus, gs_uint32);  // Status of the Completion (see enum)
  // MBurstLength already exists, see attributes.h // Length of the payload in bytes
  GSGB_ATTRIBUTE (MVC, gs_uint32);           // Virtual Channel (VC) (0...7)
  //GSGB_ATTRIBUTE (MHdrFlowControl, gs_uint32);  // Flow Control counter for headers
  //GSGB_ATTRIBUTE (MDataFlowControl, gs_uint32); // Flow Control counter for data
  GSGB_ATTRIBUTE (MDLLPtype, gs_uint32);
  GSGB_ATTRIBUTE (MVendorID, gs_uint32);         // Vendor ID for Vendor_Defined Messages
  GSGB_ATTRIBUTE (MVendorHeaderData, gs_uint32); // Vendor specific header data for Vendor_Defined Messages

  /// Enumeration for PCIe quark mPCIeCmd (TLP type)
  enum PCIeCmd {
    NO_CMD = Generic_MCMD_IDLE,
    MemWrite = Generic_MCMD_WR,
    MemRead = Generic_MCMD_RD,
    IORead,
    CnfgReadTy0,
    CnfgReadTy1,
    IOWrite,
    CnfgWriteTy0,
    CnfgWriteTy1,
    Msg,
    MsgWithData,
    MemReadLocked,
    // Last Value for extensions
    LAST_PCIeCmd_VALUE
  };
  //typedef PCIeCmd TLPtype;

  /// Enumeration for PCIe quark mMessageType 
  enum MessageType {
    NO_MESSAGE,
    RoutedToRootComplex, // 000
    RoutedByAddress,     // 001
    RoutedByID,          // 010
    BroadcastFromRootComplex,       // 011
    LocalTerminateAtReceiver,       // 100
    GatheredAndRoutedToRootComplex, // 101
    ReservedTerminateAtReceiver,    // 101
    // Last Value for extensions
    LAST_MessageType_VALUE
  };
  
  /// Enumeration for PCIe quark mMessageCode 
  enum MessageCode {
    NO_MESSAGE_CODE,
    // INTx Interrupt Signaling
    Assert_INTA, Assert_INTB, Assert_INTC, Assert_INTD, Deassert_INTA, Deassert_INTB, Deassert_INTC, Deassert_INTD, 
    // Power Management Messages
    PM_Active_State_Nak, PM_PME, PME_Turn_Off, PME_TO_Ack, 
    PM_Enter_L1, PM_Enter_L23, PM_Active_State_Request_L1, PM_Request_Ack,
    // Error Signaling Messages
    ERR_COR, ERR_NOFATAL, ERR_FATAL,
    // Locked Transactions Support
    Unlock,
    // Slot Power Limit Support
    Set_Slot_Power_limit,
    // Vendor_Defined Messages
    Vendor_Defined_Type_0, Vendor_Defined_Type_1, 
    // Ignored Messages
    Ignored_Message,
    // Last Value for extensions
    LAST_MessageCode_VALUE
  };
  
#ifdef PCIeDEBUG_ON
  static std::string convert_MessageCode_to_string(const MessageCode code) {
    switch (code) {
      case NO_MESSAGE_CODE: return "NO_MESSAGE_CODE";
      case Assert_INTA: return "Assert_INTA";
      case Assert_INTB: return "Assert_INTB";
      case Assert_INTC: return "Assert_INTC";
      case Assert_INTD: return "Assert_INTD";
      case Deassert_INTA: return "Deassert_INTA";
      case Deassert_INTB: return "Deassert_INTB";
      case Deassert_INTC: return "Deassert_INTC";
      case Deassert_INTD: return "Deassert_INTD";
      case PM_Active_State_Nak: return "PM_Active_State_Nak";
      case PM_PME: return "PM_PME";
      case PME_Turn_Off: return "PME_Turn_Off";
      case PME_TO_Ack: return "PME_TO_Ack";
      case PM_Enter_L1: return "PM_Enter_L1";
      case PM_Enter_L23: return "PM_Enter_L23";
      case PM_Active_State_Request_L1: return "PM_Active_State_Request_L1";
      case PM_Request_Ack: return "PM_Request_Ack";
      case ERR_COR: return "ERR_COR";
      case ERR_NOFATAL: return "ERR_NOFATAL";
      case ERR_FATAL: return "ERR_FATAL";
      case Unlock: return "Unlock";
      case Set_Slot_Power_limit: return "Set_Slot_Power_limit";
      case Vendor_Defined_Type_0: return "Vendor_Defined_Type_0";
      case Vendor_Defined_Type_1: return "Vendor_Defined_Type_1";
      case Ignored_Message: return "Ignored_Message";
      case LAST_MessageCode_VALUE: return "LAST_MessageCode_VALUE";
      default: return "UNKNOWN";
    }
  }
#endif

  /// Enumeration for PCIe quark mComplStatus 
  enum CompletionStatus {
    NO_COMPL_STATUS,
    SuccessfulCompletion,
    UnsupportedRequest,
    ConfigurationRequestRetryStatus,
    CompleterAbort,
    // Last Value for extensions
    LAST_CompletionStatus_VALUE
  };
  
  /// Enumeration for PCIe quark mDLLPtype 
  enum DLLPtype {
    InitFlowControlPostedRequest,
    InitFlowControlNonPostedRequest,
    InitFlowControlCompletion,
    InitFlowControl2PostedRequest,
    InitFlowControl2NonPostedRequest,
    InitFlowControl2Completion,
    UpdateFlowControlPostedRequest,
    UpdateFlowControlNonPostedRequest,
    UpdateFlowControlCompletion,
    // Last Value for extensions
    LAST_DLLPtype_VALUE
  };


} // end namespace PCIe
} // end namespace tlm


#endif
