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

#ifndef __dust_h__
#define __dust_h__

#if defined(__ATTRIBUTES_H__) || defined(__GENERIC_H__) || defined(PAYLOAD_EVENT_QUEUE_H) || defined(__ATTRIBUTES_H__) || defined(__BASICPORTS_H__) || defined(__CLASS_WRAPPER_H__) || defined(_GS_DUST_PORT_H_) || defined(__shipAPI_h__) || defined(__TLM_PORT_H__)
#error The dust include HAS TO be included prior to all other includes, because many includes depend on the definitions made inside the dust include!
#endif

/* Define USE_DUST when not using GSPM */
#ifndef USE_DUST
#define USE_DUST
#endif

/* Define USE_SCV when not using GSPM */
#ifndef USE_SCV
#define USE_SCV
#endif

#include <string>
#include "greenbus/utils/gs_datatypes.h"

//---------------------------------------------------------------------------------
// DUST Makro definitions for "intelligent" design interpretation
//---------------------------------------------------------------------------------

// include dust headers for users convenience
# include "dust/xml_log/static_model/model_builder.h"
# include "dust/xml_log/tr_model/tr_listener.h"
# include "dust/xml_log/static_model/extended_attr.h"


# define DUST_MASTER_PORT(a,b) DUST_ATTRIBUTE(a, "DUST_PORT", b, "Master", "");
# define DUST_SLAVE_PORT(a,b) DUST_ATTRIBUTE(a, "DUST_PORT", b, "Slave", "");
# define DUST_BUS(a)  DUST_ATTRIBUTE(a, "DUST_BUS", "", "", ""); 
# define DUST_MASTER_MODULE(a,b) DUST_ATTRIBUTE(a, "DUST_MODULE", b, "Master", "");
# define DUST_SLAVE_MODULE(a,b) DUST_ATTRIBUTE(a, "DUST_MODULE", b, "Slave", "");
# define DUST_PROTOCOL(a) DUST_ATTRIBUTE(a, "DUST_PROTOCOL", "", "", "");
# define DUST_SCHEDULER(a) DUST_ATTRIBUTE(a, "DUST_SCHEDULER", "", "", "");
# define DUST_PORT(a,b) DUST_ATTRIBUTE(a, "DUST_PORT", b, "", "");
# define DUST_BRIDGE(a) DUST_ATTRIBUTE(a, "DUST_BRIDGE", "", "", "");



//---------------------------------------------------------------------------------
// SCV extensions for the TransactionContainer
//---------------------------------------------------------------------------------

//this one will include scv.h
#ifndef USE_STATIC_CASTS
#include "greenbus/transport/generic.h"
#endif
#include "greenbus/core/attributes.h"



using namespace tlm;

SCV_ENUM_EXTENSIONS(GenericSRespType){
 public:
  SCV_ENUM_CTOR(GenericSRespType) {
    SCV_ENUM(Generic_SRESP_NULL);
    SCV_ENUM(Generic_SRESP_DVA);
    SCV_ENUM(Generic_SRESP_FAIL);
    SCV_ENUM(Generic_SRESP_ERR);
  }
};


SCV_ENUM_EXTENSIONS(GenericMCmdType){
 public:
  SCV_ENUM_CTOR(GenericMCmdType) {
    SCV_ENUM(Generic_MCMD_IDLE);
    SCV_ENUM(Generic_MCMD_WR);
    SCV_ENUM(Generic_MCMD_RD);
    SCV_ENUM(Generic_MCMD_RDEX);
    SCV_ENUM(Generic_MCMD_RDL); 
    SCV_ENUM(Generic_MCMD_WRNP);
    SCV_ENUM(Generic_MCMD_WRC); 
    SCV_ENUM(Generic_MCMD_BCST);   
  }
};
  
SCV_ENUM_EXTENSIONS(GenericMBurstSeqType){
 public:
  SCV_ENUM_CTOR(GenericMBurstSeqType) {
    SCV_ENUM(Generic_MBURSTSEQ_INCR);
    SCV_ENUM(Generic_MBURSTSEQ_DFLT1);
    SCV_ENUM(Generic_MBURSTSEQ_WRAP);
    SCV_ENUM(Generic_MBURSTSEQ_DFLT2);
    SCV_ENUM(Generic_MBURSTSEQ_XOR);
    SCV_ENUM(Generic_MBURSTSEQ_STRM);
    SCV_ENUM(Generic_MBURSTSEQ_UNKN);
    SCV_ENUM(Generic_MBURSTSEQ_RESERVED);    
  }
};

SCV_ENUM_EXTENSIONS(GenericSimulationModeType) {
 public:
  SCV_ENUM_CTOR(GenericSimulationModeType) {
    SCV_ENUM(MODE_BA);
    SCV_ENUM(MODE_CT);
  }
};


SCV_EXTENSIONS(GSDataType){
 public:
  // TODO: add extension for field "void *pointer" (void* is not supported by SCV)
  // TODO: add extension for field "std::vector<gs_uint8> data" (std::vector<gs_uint8> is not supported by SCV)
  scv_extensions<std::string*> name;
  SCV_EXTENSIONS_CTOR(GSDataType){
    SCV_FIELD(name);
  }  
};


#define DUST_ATTRIBUTE_EXTENSION(type)        \
  SCV_EXTENSIONS(AttributeBase<type>){        \
  public:                                     \
    scv_extensions<type> value;               \
    SCV_EXTENSIONS_CTOR(AttributeBase<type>){ \
      SCV_FIELD(value);                       \
    }                                         \
  };                                          \
  
DUST_ATTRIBUTE_EXTENSION(GenericMCmdType);
DUST_ATTRIBUTE_EXTENSION(GenericSRespType);
DUST_ATTRIBUTE_EXTENSION(GenericMBurstSeqType);
DUST_ATTRIBUTE_EXTENSION(gs_uint32);
DUST_ATTRIBUTE_EXTENSION(gs_uint64);
DUST_ATTRIBUTE_EXTENSION(gs_handle);


SCV_EXTENSIONS(GenericTransaction){
public:
  scv_extensions<AttributeBase<gs_uint64> > mAddr;
  scv_extensions<AttributeBase<gs_uint32> > mBurstLength;
  scv_extensions<AttributeBase<gs_uint32> > sBurstLength;
  scv_extensions<AttributeBase<GenericMCmdType> > mCmd;
  scv_extensions<AttributeBase<gs_uint32> > mDataWidth;
  scv_extensions<AttributeBase<gs_uint32> > sDataWidth;
  scv_extensions<AttributeBase<GenericSRespType> > sResp;
  scv_extensions<AttributeBase<gs_uint32> > mError;
  scv_extensions<AttributeBase<gs_uint32> > sError;
  scv_extensions<AttributeBase<gs_handle> > mID;
  scv_extensions<bool> is_extended;
  scv_extensions<GSDataType> msData;
  scv_extensions<std::string*> extendedType;
  SCV_EXTENSIONS_CTOR(GenericTransaction){
    SCV_FIELD(mAddr);
    SCV_FIELD(mBurstLength);
    SCV_FIELD(sBurstLength);
    SCV_FIELD(mCmd);
    SCV_FIELD(mDataWidth);
    SCV_FIELD(sDataWidth);
    SCV_FIELD(sResp);
    SCV_FIELD(mError);
    SCV_FIELD(sError);
    SCV_FIELD(mID);
    SCV_FIELD(is_extended);
    SCV_FIELD(extendedType);
    SCV_FIELD(msData); // must always be the last field (used in peq)
  }
};


SCV_EXTENSIONS(GenericPhase) {
 public:
  scv_extensions<gs_uint32> state;
  scv_extensions<GenericSimulationModeType> sim_mode;
  scv_extensions<gs_uint32> BurstNumber;
  SCV_EXTENSIONS_CTOR(GenericPhase) {
    SCV_FIELD(state);
    SCV_FIELD(sim_mode);
    SCV_FIELD(BurstNumber);
  }
};




#endif /* __dust_h__ */

