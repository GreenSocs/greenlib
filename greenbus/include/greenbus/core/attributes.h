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

#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__

#include "greenbus/core/attributeBase.h"
#include <string.h>
#include <systemc.h>
#include <map>

#ifdef USE_DUST
# include "scv.h"
#endif

namespace tlm {
  
  
//---------------------------------------------------------------------------
/**
 * The generic command set.
 */
//---------------------------------------------------------------------------
enum GenericMCmdType {
  Generic_MCMD_IDLE =0,   //Idle command
  Generic_MCMD_WR     ,   //Write command
  Generic_MCMD_RD     ,   //Read command
  Generic_MCMD_RDEX   ,   //Exclusive read command
  Generic_MCMD_RDL    ,   //Read linked command
  Generic_MCMD_WRNP   ,   //Non-posted write command
  Generic_MCMD_WRC    ,   //Write conditional command
  Generic_MCMD_BCST       //Broadcast command
};


//---------------------------------------------------------------------------
/**
 * The generic response set.
 */
//---------------------------------------------------------------------------
enum GenericSRespType {
  Generic_SRESP_NULL =0,    //Null response
  Generic_SRESP_DVA  ,      //Data valid/accept response
  Generic_SRESP_FAIL ,      //Request failed
  Generic_SRESP_ERR         //Error response
};


//---------------------------------------------------------------------------
/**
 * The generic burst sequence set.
 */
//---------------------------------------------------------------------------
enum GenericMBurstSeqType{
  Generic_MBURSTSEQ_INCR     =0,    //Incrementing
  Generic_MBURSTSEQ_DFLT1    ,      //Custom (packed)
  Generic_MBURSTSEQ_WRAP     ,      //Wrapping
  Generic_MBURSTSEQ_DFLT2    ,      //Custom (not packed)
  Generic_MBURSTSEQ_XOR      ,      //Exclusive OR
  Generic_MBURSTSEQ_STRM     ,      //Streaming
  Generic_MBURSTSEQ_UNKN     ,      //Unknown
  Generic_MBURSTSEQ_RESERVED        //Reserved
};


//---------------------------------------------------------------------------
/**
 * The generic error set. This set is defined in the namespace
 * GenericError to enable extensions with own error classes.
 */
//---------------------------------------------------------------------------
namespace GenericError {
enum GenericErrorType {
  Generic_Error_NoError=0,
  Generic_Error_TimeOut,
  Generic_Error_Collision,
  Generic_Error_AccessDenied,
  Generic_Error_UnknownAddress,
  First_User_Error
};
} // namespace GenericError


//---------------------------------------------------------------------------
/**
 * The simulation modes of the generic protocol
 */
//---------------------------------------------------------------------------
enum GenericSimulationModeType {
  /// BA mode (don't use update_event)
  MODE_BA = 0,
  /// CT mode (fire cycle-count accurate quark update notifications)
  MODE_CT
};


//---------------------------------------------------------------------------
  /**
   * The default data type.
   */
  //---------------------------------------------------------------------------
  // char MData_desc[] ="Transaction Data";
  
  /**
   * This class is a wrapper for data that is to be transferred
   * by a GreenBus transaction.
   * The base type is std::vector<gs_uint8>*. 
   *
   * The data byte array resides either in GSDataTyep
   * or can  be allocated
   * by the initiator of a transaction (usually this 
   * should be done in the UserAPI implementation).
   *
   * Other data types then std::vectory<gs_uint8> 
   * that are to be transferred over GreenBus
   * can be transported by two means, and the chosen method
   * affects the compatibility class of a transaction.
   *
   *  1) Pointer-Mode: the GSDataType can carry a pointer to a complex
   *     data object. This pointer has to be casted
   *     into its original type in the target.
   *     To set the pointer, use the setPointer(void*) method.
   *     To get the pointer, use the getPointer() method.
   *     To find out whether a GSDataType carries a pointer,
   *     use the isPointer() method.
   *     The result will be a very fast simulation, but low
   *     compatibility, since the target port must be aware
   *     of the transmitted object type. Simulation accuracy
   *     is restricted to PV and BA.
   *
   *  2) Full-Data-Mode: in this mode, the GSDataType carries 
   *     a serialized array of bytes which is represented by
   *     the std::vector<gs_uint8>*. 
   *     To set the vector, use the set(std::vector<gs_uint8>*) method.
   *     To get the vector, use the get() method.
   *     To find out whether a GSDataType carries a byte-vector,
   *     use the isPointer() method.
   *     The result will be a slower simulation, but full
   *     compatibility, since the target port does not need
   *     to be aware of the orginal object type of the transmitted
   *     data to interpret them. CT simulation will provide full
   *     cycle-count accurate results, and heterogeneous UserAPIs
   *     can be connected to eachother.
   *
   * When the GSDataType is used with a byte-vector, it fully 
   * supports data introspection with DUST.
   */
class GSDataType : public AttributeRoot {
  
#ifdef USE_DUST
  friend class scv_extensions<GSDataType>;
#endif
  
  friend std::ostream& operator<<(std::ostream&, const GSDataType&);
  friend std::istream& operator>>(std::istream&, GSDataType&);
  
public:
  typedef std::vector<gs_uint8> dtype;
  typedef void ptype;

private:
  dtype *data;
  ptype *pointer;
  std::string *name;
  bool own_vector;
  
public:

  /**
   * Create an empty MasterDataType. Use set method to initialize.
   */
  GSDataType()
    : data(NULL), 
      pointer(NULL),
      name(NULL),
      own_vector(false)
  {}

  /**
   * Create a MasterDataType that holds its own vector, pre-sized to size.
   */
  GSDataType(int size)
    : data(new dtype()), 
      pointer(NULL),
      name(NULL),
      own_vector(true)
  {data->resize(size);}
  
  /**
   * Create a GSDataType which points to a std::vector<gs_uint8>.
   * @param d A data vector.
   */
  GSDataType(const dtype &d) 
    : data(const_cast<dtype*>(&d)), 
      pointer(NULL),
      name(NULL),
      own_vector(false)
  {}

  /**
   * Create a GSDataType which points to a void* pointer.
   * @param p A data pointer.
   */
  GSDataType(const ptype *p) 
    : data(NULL), 
      pointer(const_cast<ptype*>(p)),
      name(NULL),
      own_vector(false)
  {}

  /// destructor
  ~GSDataType() {
    if (name!=NULL)
      delete name;
    if (own_vector)
      delete data;
  }

#ifndef USE_DUST
private: // disabled  
#endif
  /**
   * Copy constructor is disabled to prevent bad access.
   */
  GSDataType(const GSDataType &m) {}
  
  /**
   * Copy operator is disabled to prevent bad access.
   */
  GSDataType &operator= (const GSDataType &m) {
    return *this;
  }

public:  
  /**
   * Set byte vector of this GSDataType.
   * The byte vector is NOT copied. Only a pointer to it is stored.
   * @param d Pointer to byte vector
   */
  void setData(const dtype &d) {
    data=const_cast<dtype*>(&d); 
    pointer=NULL;
  }
  
  /**
   * Create a copy of a GSDataType. Note: does NOT copy the data, only the pointers to it!
   * @param m GSDataType to copy.
   */
  void set(const GSDataType &m) {
    data=m.data;
    pointer=m.pointer;
  }

  /**
   * Set the pointer of this GSDataType.
   */
  void setPointer(const ptype *p) {data=NULL; pointer=const_cast<ptype*>(p);}
  
  /**
   * Test if this GSDataTypes carries a pointer.
   */
  const bool isPointer() const {
    return pointer!=NULL;
  }  
  
  /**
   * Get byte vector carried by this GSDataType.
   * @return Pointer to the data vector.
   */
  dtype &getData() const {
    return *data;
  }
  
  /**
   * Get pointer carried by this GSDataType.
   */
  ptype *getPointer() const {return pointer;}
  
  /**
   * Overwrite this object's data with the data stored in another GSDataType.
   * @param src GSDataType from which the data shall be copied.
   */
  void deepcopyFrom(const GSDataType &src) {
    if (src.isPointer()) {
      pointer=src.pointer;
      data=NULL;
    }
    else {
      pointer=NULL;
      *data=*(src.data);
    }
  }
  
  /**
   * Copy data from this object to another GSDataType's data array.
   * @param dst GSDataType to which the data shall be copied.
   */
  void deepcopyTo(GSDataType &dst) const {
    if (isPointer()) {
      dst.pointer=pointer;
      dst.data=NULL;
    }
    else {
      dst.pointer=NULL;
      *(dst.data)=*data;
    }
  }
  
  /**
   * Operator [] for byte vector access in this GSDataType.
   * Please note that the operation will fail if the GSDataType carries a pointer (see isPointer).
   * Note also that this operation does no range-checking.
   */
  gs_uint8 & operator[] (gs_uint32 i) {
    assert(data!=NULL);
    return (*data)[i];
  }
  
  /**
   * Append one byte to this GSDataType.
   * Note that this method overrides a previously call to setPointer.
   * The method returns false if the data vector in this GSDataType has not been initialized.
   * @param b add this byte to the vector in this GSDataType.
   * @return true on success, false if the data vector in this GSDataType has not been initialized.
   */
  const bool append(const gs_uint8 b) {
    pointer=NULL;
    if (data==NULL)
      return false;
    
    data->push_back(b);
    return true;
  }

  /**
   * Return the size of th ebyte vector of this GSDataType.
   */
  inline virtual const gs_uint32 getSize() const { 
    return data==NULL ? 0 : data->size();
  }

  /**
   * Get the name of this GSDataType.
   * @return A string containing the name.
   */
  virtual const std::string &getName() const { 
    assert(name!=NULL);
    return *name; 
  } 

  /**
   * Serialize to unsigned char array.
   */
  inline virtual const gs_uint8* serialize() {
    return data==NULL ? NULL : &(*data)[0];
  }
  
  /**
   * Set the name of this GSDataType.
   * @param _name The new name for this object.
   */
  virtual void setName(const std::string &_name)
  { 
    if (name!=NULL)
      delete name;
    name=new std::string(_name);
  }    
  
};

//-----------------------------------------------------------------------------
// stream operators for use of GSDataType with gb_param
//-----------------------------------------------------------------------------

inline
std::ostream& operator << (std::ostream &os, const GSDataType &obj) {
  if (obj.data != NULL)
    os << &(*(obj.data))[0];
  return os;
}

inline
std::istream& operator >> (std::istream &is, GSDataType &obj) {  
  if (obj.data != NULL)
    is >> &(*(obj.data))[0];
  return is;
}



  //---------------------------------------------------------------------------
  /** \addtogroup attributes Default attributes
   *  Default attributes that can be used in a user-defined protocol
   *  @{
   */
  //---------------------------------------------------------------------------

#define GSGB_ATTRIBUTE(name,type) typedef tlm::AttributeBase<type> name
#define GSGB_ATTRIBUTESTACK(name,type) typedef tlm::AttributeStackBase<type> name

//Request group
GSGB_ATTRIBUTE (MAddr, gs_uint64);            /// Master address
GSGB_ATTRIBUTE (MAddrSpace, gs_uint64);      /// Master address space
GSGB_ATTRIBUTE (MAtomicLength, gs_uint32);    /// "Length of atomic burst"
GSGB_ATTRIBUTE (MBurstLength, gs_uint32);     /// "Master burst length, set to '0' for no data at all (e.g. interrupt)."
GSGB_ATTRIBUTE (SBurstLength, gs_uint32);     /// "Slave burst length (for a write, this parameter identifies how much data has been accepted by the slave,
GSGB_ATTRIBUTE (MBurstPrecise, bool);         /// "Given burst length is precise (if false, arbitrary length burst is assumed)."
GSGB_ATTRIBUTE (MBurstSeq, GenericMBurstSeqType); /// "Address sequence of burst"
GSGB_ATTRIBUTE (MBurstSingleReq, bool);       /// "Burst uses single request/multiple data protocol"
GSGB_ATTRIBUTE (MByteEn, gs_uint32);          /// "Master byte enable"
GSGB_ATTRIBUTE (MCmd, GenericMCmdType);       /// Master command
GSGB_ATTRIBUTE (MConnID, gs_uint32);          /// "Master connection identifier"
GSGB_ATTRIBUTE (MDataWidth, gs_uint32);     /// depricated, see data_width parameter in tlm_port  /// "Master data bus width: # bytes that are transferred by the initiator port in one cycle."
GSGB_ATTRIBUTE (SDataWidth, gs_uint32);     /// depricated, see data_width parameter in tlm_port  /// "Slave data bus width: # bytes that are transferred by the targer port in one cylce. For busses, usually MDataWidth==SDataWidth holds. For NoCs including multi-bus systems, MDataWidth!=SDataWidth can occur."
GSGB_ATTRIBUTE (MID, gs_handle);              /// "Master identifier"
GSGB_ATTRIBUTE (MReqLast, bool);              /// "Last request in a burst"
GSGB_ATTRIBUTE (MTagID, gs_uint32);           /// "Master tag identifier "
GSGB_ATTRIBUTE (MTagInOrder, bool);           /// "Force tag-in-order"
GSGB_ATTRIBUTE (MThreadID, gs_uint32);        /// "Master thread identifier"
GSGB_ATTRIBUTE (MReqInfo, gs_uint64);         /// "Extra information sent with the request."
GSGB_ATTRIBUTE (TransID, gs_uint64);          /// "Unique transaction ID"

//Data group
typedef GSDataType MSData;                /// Transaction data (resides in master port)
typedef MSData MData;                     /// deprecated
GSGB_ATTRIBUTE (MDataInfo, gs_uint64);        /// "Extra information sent with the write data"
GSGB_ATTRIBUTE (SDataInfo, gs_uint64);        /// "Extra information sent with the read data"
GSGB_ATTRIBUTE (MSBytesValid, gs_uint64);       /// Number of valid bytes in MSData. Points to the first non-valid byte. Is set to MBurstLength upon request.

//Response group
GSGB_ATTRIBUTE (SResp, GenericSRespType);     /// "Slave response"
GSGB_ATTRIBUTE (SThreadID, gs_uint32);        /// "Slave thread identifier"
GSGB_ATTRIBUTE (STagID, gs_uint32);           /// "Slave tag identifier "
GSGB_ATTRIBUTE (STagInOrder, bool);           /// "Force tag-in-order "
GSGB_ATTRIBUTE (SRespInfo, gs_uint64);        /// "Extra information sent out with the response."
GSGB_ATTRIBUTE (SRespLast, bool);             /// "Last response in burst"
                                         ///  for a read, this parameter identifies how much data has been written by the slave."

GSGB_ATTRIBUTE (Error, gs_uint32);            /// "Error identifier"

GSGB_ATTRIBUTESTACK (MAddrSt, gs_uint64);            /// Master address stack
GSGB_ATTRIBUTESTACK (MIDSt, gs_handle);              /// "Master identifier" stack
/** @}
 */

typedef std::multimap<int,AttributeRoot*> attribute_map;
typedef std::multimap<int,AttributeRoot*>::iterator attribute_iterator;


//-----------------------------------------------------------------------------
// stream operators for use of attributes with gb_param.
// We need to define the enum operators first in order to make AttributeBase 
// stream operators work. Hence the AttributeBase stream operators cannot 
// be defined in attributeBase.h
//-----------------------------------------------------------------------------

#define ENUM_OPERATORS(type)                                            \
  inline std::istream& operator >> (std::istream &is, type &v) {        \
    gs_uint32 i;                                                        \
    is >> i;                                                            \
    v = static_cast<type>(i);                                           \
    return is;                                                          \
  }                                                                     \
  inline std::ostream& operator << (std::ostream &os, const type &v) {  \
    os << static_cast<gs_uint32>(v);                                    \
    return os;                                                          \
  }

ENUM_OPERATORS(GenericMCmdType)
ENUM_OPERATORS(GenericSRespType)
ENUM_OPERATORS(GenericMBurstSeqType)

// output stream operator for AttributeBase
template <class T>
inline std::ostream& operator << (std::ostream &os, const AttributeBase<T> &obj) {
  os << "0x" << std::hex << obj.value;
  return os;
}

// input stream operator for AttributeBase
template <class T>
inline std::istream& operator >> (std::istream &is, AttributeBase<T> &obj) {  
  // assume string to be plane number ("42") or hex number ("0x42")
  std::string::size_type idx;
  std::string s;
  std::stringstream ss;
  is >> s;
  idx = s.find("0x");
  if (idx == std::string::npos) 
    ss << s;
  else 
    ss << std::hex << s.substr(idx+2);
  ss >> obj.value;
  return is;
}

// output stream operator for AttributeStackBase
template <class T>
inline std::ostream& operator << (std::ostream &os, const AttributeStackBase<T> &obj) {
  os << "0x" << std::hex << *(obj.recent);
  return os;
}

// input stream operator for AttributeStackBase
template <class T>
inline std::istream& operator >> (std::istream &is, AttributeStackBase<T> &obj) {  
  // assume string to be plane number ("42") or hex number ("0x42")
  std::string::size_type idx;
  std::string s;
  std::stringstream ss;
  is >> s;
  idx = s.find("0x");
  if (idx == std::string::npos) 
    ss << s;
  else 
    ss << std::hex << s.substr(idx+2);
  ss >> *(obj.recent);
  return is;
}

  
} // end of namespace tlm


#endif
