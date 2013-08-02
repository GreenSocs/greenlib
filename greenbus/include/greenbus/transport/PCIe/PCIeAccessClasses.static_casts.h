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


#ifndef __PCIeAccessClasses_STATIC_CASTS_h__
#define __PCIeAccessClasses_STATIC_CASTS_h__

// enable in your global setting file!
#ifdef PCIeDEBUG_ON
    #define PCIeACCESSDEBUG(string) { cout << "@" << sc_time_stamp() << " /" << sc_delta_count() << " (PCIe[Master|Router|Slave]Access): " << string << endl << flush; }
#else
    #define PCIeACCESSDEBUG(string)
#endif

namespace tlm {
namespace PCIe { 


  /// Access method which allows a master to generate transactions.
  /**
   * This is the API a sender has to use to access PCIeTransaction objects.
   *
   * This API provides methods which generate the estimated TLP type.
   */
  class PCIeMasterAccess
   : public virtual gp::GenericMasterAccess,
     public virtual PCIeTransaction
  {
  public:
    // TODO: remove that ones which are not allowed to be modified/read by Master
    using PCIeTransaction::setMPCIeCmd;
    using PCIeTransaction::getMPCIeCmd;
    using PCIeTransaction::setMTrafficClass;
    using PCIeTransaction::getMTrafficClass;
    using PCIeTransaction::setMDigestExists;
    using PCIeTransaction::getMDigestExists;
    using PCIeTransaction::setMECRCpoisoned;
    using PCIeTransaction::getMECRCpoisoned;
    using PCIeTransaction::setMIsIDbasedRouting;
    using PCIeTransaction::getMIsIDbasedRouting;
    using PCIeTransaction::setMIsImplicitRouting;
    using PCIeTransaction::getMIsImplicitRouting;
    using PCIeTransaction::setMBusNo;
    using PCIeTransaction::getMBusNo;
    using PCIeTransaction::setMDevNo;
    using PCIeTransaction::getMDevNo;
    using PCIeTransaction::setMFuncNo;
    using PCIeTransaction::getMFuncNo;
    using PCIeTransaction::setMRegNo;
    using PCIeTransaction::getMRegNo;
    using PCIeTransaction::setMRequBusNo;
    using PCIeTransaction::getMRequBusNo;
    using PCIeTransaction::setMRequDevNo;
    using PCIeTransaction::getMRequDevNo;
    using PCIeTransaction::setMRequFuncNo;
    using PCIeTransaction::getMRequFuncNo;
    using PCIeTransaction::setMByteEn;
    using PCIeTransaction::getMByteEn;
    using PCIeTransaction::setMTagID;
    using PCIeTransaction::getMTagID;
    using PCIeTransaction::setMConfigAddr;
    using PCIeTransaction::getMConfigAddr;
    using PCIeTransaction::setMMessageType;
    using PCIeTransaction::getMMessageType;
    using PCIeTransaction::setMMessageCode;
    using PCIeTransaction::getMMessageCode;
    using PCIeTransaction::setMComplStatus;
    using PCIeTransaction::getMComplStatus;
    using PCIeTransaction::setMVC;
    using PCIeTransaction::getMVC;
    using PCIeTransaction::setMDLLPtype;
    using PCIeTransaction::getMDLLPtype;
    using PCIeTransaction::setMVendorID;
    using PCIeTransaction::getMVendorID;
    using PCIeTransaction::setMVendorHeaderData;
    using PCIeTransaction::getMVendorHeaderData;

    // Generic accesses and overloaded generic accesses
    using PCIeTransaction::setMID;
    using PCIeTransaction::setMCmd; // overloaded in PCIeTransaction
    using PCIeTransaction::setMAddr;
    using PCIeTransaction::setMData;
    //using PCIeTransaction::setMDataWidth;  // depricated, see data_width parameter in tlm_port
    //using PCIeTransaction::setSDataWidth;  // depricated, see data_width parameter in tlm_port
    using PCIeTransaction::setMBurstLength;
    using PCIeTransaction::setSBurstLength; // overloaded in PCIeTransaction
    using PCIeTransaction::setMSBytesValid;
    using PCIeTransaction::setSResp;
    using PCIeTransaction::setSData;
    using PCIeTransaction::setTransID;
    using PCIeTransaction::getMAddr;
    using PCIeTransaction::getMID;
    using PCIeTransaction::getMCmd; // overloaded in PCIeTransaction
    using PCIeTransaction::getMData;
    //using PCIeTransaction::getMDataWidth;  // depricated, see data_width parameter in tlm_port
    //using PCIeTransaction::getSDataWidth;  // depricated, see data_width parameter in tlm_port
    using PCIeTransaction::getMBurstLength;
    using PCIeTransaction::getSBurstLength;
    using PCIeTransaction::getMSBytesValid;
    using PCIeTransaction::getSResp;
    using PCIeTransaction::getSData;
    using PCIeTransaction::setMError; // overloaded in PCIeTransaction
    using PCIeTransaction::getMError; // overloaded in PCIeTransaction
    using PCIeTransaction::setSError; // overloaded in PCIeTransaction
    using PCIeTransaction::getSError; // overloaded in PCIeTransaction
    using PCIeTransaction::getTransID;
    using PCIeTransaction::getAttributeIterator;
    using PCIeTransaction::lastAttribute;
    using PCIeTransaction::getExtendedAttributeIterator;
    using PCIeTransaction::lastExtendedAttribute;
    using PCIeTransaction::setPhases;
    using PCIeTransaction::isExtended;
    using PCIeTransaction::getExtendedType;
    using PCIeTransaction::createAttributeIterator;
    using PCIeTransaction::createExtendedAttributeIterator;
    using PCIeTransaction::notifyTargetUpdate;
    using PCIeTransaction::notifyInitiatorUpdate;
    using PCIeTransaction::getTargetUpdateEvent;
    using PCIeTransaction::waitForTargetUpdate;
    using PCIeTransaction::getInitiatorUpdateEvent;
    using PCIeTransaction::waitForInitiatorUpdate;

    using PCIeTransaction::reset;

    // ///////////// Methods for creating TLPs: /////////////////
    
    /// Make Memory Read TLP: read read_data_size bytes from address targetAddr to vector data.
    /**
     * Fill Memory Read Request (MRd) transaction with data, see spec. p. 66
     * Standard fields to be set: (Format), Type, TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: Requester ID, Tag, (Byte Enable), Address
     *
     * Addresses below 4GB are automatically 32-bit addresses, above 64-bit addresses
     * => 3 or 4 DW header.
     *
     * @param targetAddr  Address where to read from (Byte aligned address)
     * @param data        Target vector for data to be written to, scope must be at least until return
     * @param data_size   Amount of data to be read (data vector may be bigger)
     * @param requester_func_no  Requester Function Number (default=0)
     */
    inline void init_Memory_Read( const MAddr targetAddr, const std::vector<gs_uint8> &data, const unsigned int data_size, const gs_uint32 requester_func_no = 0) {
      init_AddressBased_Request(MemRead, targetAddr, data, data_size, requester_func_no);
    }

    /// Convert an existing Memory Read transaction to a Locked Memory Read
    /**
     * Before calling this method, use init_Memory_Read() to generate the transaction.
     */
    inline void lock_Memory_Read() {
      //SC_REPORT_WARNING("PCIeMasterAccess", "Only Host CPU access to Legacy Endpoints and the Root Complex are allowed to initiate Locked Requests!");
      if (this->getMPCIeCmd() == MemRead) {
        this->setMPCIeCmd(MemReadLocked);
      } else {
        SC_REPORT_WARNING("PCIeMasterAccess", "Please use init_Memory_Read() before calling lock_Memory_Read()!");
      }
    }
    
    /// Convenience method to send Unlock message
    inline void unlock() {
      init_Message(Unlock);
    }

    /// Make Memory Write transaction
    /**
     * Fill Memory Write Request (MWr) transaction with data, see spec. p. 66
     * Standard fields to be set: (Format), Type, TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: (Requester ID), Tag, (Byte Enable), Address
     *
     * Addresses below 4GB are automatically 32-bit addresses, above 64-bit addresses
     * => 3 or 4 DW header.
     *
     * @param targetAddr  Target address (of the TLP) where to write to
     * @param data        Data vector, scope must be at least until return
     * @param data_size   Size of data that should be written (data vector may be bigger)
     * @param requester_func_no  Requester Function Number (default=0)
     */
    inline void init_Memory_Write( const MAddr targetAddr, const std::vector<gs_uint8> &data, const unsigned int data_size, const gs_uint32 requester_func_no = 0 ) {
      /*std::vector<gs_uint8> *tdata;
      tdata = new std::vector<gs_uint8>(data_size);
      memcpy(&((*tdata)[0]), &data[0], data_size);*/
      init_AddressBased_Request(MemWrite, targetAddr, data, data_size, requester_func_no);
    }
    
    /// Make IO Read TLP
    /**
     * Fill IO Read Request (IORd) transaction with data, see spec. p. 66,67
     * Standard fields to be set: (Format), Type, TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: (Requester ID), Tag, (Byte Enable), Address
     * 
     * Only 32-bit addresses. (3 DW header)
     *
     * @param targetAddr  Address where to read from (Byte aligned address)
     * @param data        Target vector for data to be written to, scope must be at least until return
     * @param data_size   Amount of data to be read (data vector may be bigger)
     * @param requester_func_no  Requester Function Number (default=0)
     */
    inline void init_IO_Read( const MAddr targetAddr, const std::vector<gs_uint8> &data, const unsigned int data_size, const gs_uint32 requester_func_no = 0 ) {
      CHECK_RULES( sc_assert(data_size <= 4); ); // only one DataWord (4 Bytes)
      CHECK_RULES( sc_assert(targetAddr <= 0xFFFFFFFF); ); // max 32-bit addresses
      init_AddressBased_Request(IORead, targetAddr, data, data_size, requester_func_no);
    }
    
    /// Make IO Write TLP
    /**
     * Fill IO Write Request (IORd) transaction with data, see spec. p. 66,67
     * Standard fields to be set: (Format), Type, TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: (Requester ID), Tag, (Byte Enable), Address
     * 
     * Only 32-bit addresses. (3 DW header)
     *
     * @param targetAddr  Target address (of the TLP) where to write to
     * @param data        Data vector, scope must be at least until return
     * @param data_size   Size of data that should be written (data vector may be bigger)
     * @param requester_func_no  Requester Function Number (default=0)
     */
    inline void init_IO_Write( const MAddr targetAddr, const std::vector<gs_uint8> &data, const unsigned int data_size, const gs_uint32 requester_func_no = 0 ) {
      CHECK_RULES( sc_assert(data_size <= 4); ); // only one DataWord (4 Bytes)
      CHECK_RULES( sc_assert(targetAddr < 0xFFFFFFFF); ); // max 32-bit addresses
      init_AddressBased_Request(IOWrite, targetAddr, data, data_size, requester_func_no);
    }
    

    /// Calculate the base address for ID based addressing giving the ID
    /**
     * Due to performance reasons use this calculation only once for one target function.
     * Store the result in a local variable and use it in the init_xx call.
     *
     * See spec. p. 416.
     *
     * @param busNo  Bus Number 0...255
     * @param devNo  Device Number 0...31
     * @param funcNo Function Number 0...7
     * @return ID address = busNo*1048576 + devNo*32768 + funcNo*4096
     */
/*    unsigned int calculate_target_ID_address_base(unsigned int busNo, unsigned int devNo, unsigned int funcNo) {
      CHECK_RULES( sc_assert(busNo<256 and devNo<32 and funcNo<8 ); );
      // create IDaddress out of numbers:
      unsigned int idAddr;
      //         busNo           devNo         funcNo        registerByteNo
      // idAddr  [72:20]         [19:15]       [14:12]       [11:0]
      idAddr   = busNo*1048576 + devNo*32768 + funcNo*4096;
#ifdef PCIeDEBUG_ON
      PCIeACCESSDEBUG("generated ID addr = " << idAddr );
#endif
#ifdef CHECK_RULES_ENABLE
      // check if calculation was already done
      // TODO: only for the calling object (different calling modules of course have to calculate separately)
      static std::map<unsigned int, unsigned int> idAddressMap;
      if (idAddressMap.find(idAddr)->second > 0) {
        // if yes, warning
        SC_REPORT_WARNING("PCIeMasterAccess", "calculate_target_ID_address_base was used more than once for one id! Please store the result in a local variable in order to raise performance!");
      } else {
        idAddressMap[idAddr] = 1;
      }
#endif
      return idAddr;
    }*/
    
    /// Make Configuration Read TLP
    /**
     * Fill Configuration Read Request (CfgRd0) transaction with data, see spec. p. 66,68
     * Standard fields to be set: (Format), Type, TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: (Requester ID), Tag, (Byte Enable), Bus, Device, Function Numbers, (Ext.) Register Numbers
     * 3 DW Header.
     *
     * This TLP type may not be sent  peer-to-peer  or  Downstream to Upstream. (p. 422)
     *
     * @param busNo  Bus Number 0...255
     * @param devNo  Device Number 0...31
     * @param funcNo Function Number 0...7
     * @param registerByteNo  Register (byte) address inside the function
     * @param data       Vector where the read data is stored
     * @param data_size  Size of the data that have to be read: max. 4 bytes, all bytes have to be in one DW aligned register address (see p. 416f.)
     * @param requester_func_no  Requester Function Number (default=0)
     */
                // @param idAddr     ID address which was generated out if bus, device and function id (see tlm::PCIe::PCIeMasterAccess::calculate_target_ID_address_base) (see spec. p. 614)
    void init_Configuration_Read( const unsigned int busNo, const unsigned int devNo, const unsigned int funcNo,
                                  const unsigned int registerByteNo, 
                                  const std::vector<gs_uint8> &data, const unsigned int data_size,
                                  const gs_uint32 requester_func_no = 0 ) {
      CHECK_RULES( sc_assert(data_size <= 4); ); // only one DataWord (4 Bytes)
      CHECK_RULES( sc_assert(registerByteNo<4096); );
      CHECK_RULES( sc_assert((/*idAddr+*/registerByteNo % 4) + data_size <= 4); ); // test for DW aligned
      // here send type 1, switch has to transform to type 0
      init_IDBased_Request(CnfgReadTy1, busNo, devNo, funcNo, registerByteNo, data, data_size, requester_func_no);
    }

    /// Make Configuration Write TLP
    /**
     * Fill Configuration Write Request (CfgWr0) transaction with data, see spec. p. 66,68
     * Standard fields to be set: (Format), Type, TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: (Requester ID), Tag, (Byte Enable), Bus, Device, Function Numbers, (Ext.) Register Numbers
     * 3 DW Header.
     *
     * This TLP type may not be sent  peer-to-peer  or  Downstream to Upstream. (p. 422)
     *
     * @param busNo  Bus Number 0...255
     * @param devNo  Device Number 0...31
     * @param funcNo Function Number 0...7
     * @param registerByteNo  Register (byte) address inside the function
     * @param data       Vector where the read data is stored
     * @param data_size  Size of the data that have to be written: max. 4 bytes, all bytes have to be in one DW aligned register address (see p. 416f.)
     * @param requester_func_no  Requester Function Number (default=0)
     */
                // @param idAddr     ID address which was generated out if bus, device and function id (see tlm::PCIe::PCIeMasterAccess::calculate_target_ID_address_base) (see spec. p. 614)
    void init_Configuration_Write( const unsigned int busNo, const unsigned int devNo, const unsigned int funcNo,
                                   const unsigned int registerByteNo, 
                                   const std::vector<gs_uint8> &data, const unsigned int data_size,
                                   const gs_uint32 requester_func_no = 0 ) {
      CHECK_RULES( sc_assert(data_size <= 4); ); // only one DataWord (4 Bytes)
      CHECK_RULES( sc_assert(registerByteNo<4096); );
      CHECK_RULES( sc_assert((/*idAddr+*/registerByteNo % 4) + data_size <= 4); ); // test for DW aligned
      // here send type 1, switch has to transform to type 0
      init_IDBased_Request(CnfgWriteTy1, busNo, devNo, funcNo, registerByteNo, data, data_size, requester_func_no);
    }
    
    /// Make Message (without data) TLP
    /**
     * Fill Message Request (Msg) transaction without data, see spec. p. 69
     * Standard fields to be set: (Format), Type (and subtype), TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering=00, Address Type=00, Length
     * Additional fields to be set: (Requester ID), Tag, Message Code
     * 3 DW Header.
     *
     * The Msg TLP type may not contain data.
     *
     * The routing subtype field (mMessageType) is set with the MessageCode information.
     *
     * @param messCode  Message Code of the Message (see enum tlm::PCIe::MessageCode and spec. pp.70-79)
     * @param requester_func_no  Requester Function Number (default=0)
     */
    void init_Message(const MessageCode messCode, const gs_uint32 requester_func_no = 0) {
      CHECK_RULES(if (this->getMRequBusNo() == 256 || this->getMRequDevNo() == 32) SC_REPORT_ERROR("PCIeMasterAccess", "A message Request needs to have set the Requester ID before sending. This is not yet done and should be done by the API after creating the PCIe Transaction."););
      this->setMRequFuncNo(requester_func_no);
      this->setMPCIeCmd(Msg);
      this->setMIsImplicitRouting(true);
      this->setMMessageCode(messCode);
      // default value: this->setMBurstLength(0);
      MessageType messType;
      switch(messCode) {
        // INTx Interrupt Signaling
        case Assert_INTA:
        case Assert_INTB:
        case Assert_INTC:
        case Assert_INTD:
        case Deassert_INTA:
        case Deassert_INTB:
        case Deassert_INTC:
        case Deassert_INTD:
          {
            messType = LocalTerminateAtReceiver;
          }
          break;
         
        // Power Management Messages
        case PM_Active_State_Nak:
          {
            messType = LocalTerminateAtReceiver;
          }
          break;
          
        case PM_PME:
          {
            messType = RoutedToRootComplex;
          }
          break;
          
        case PME_Turn_Off:
          {
            messType = BroadcastFromRootComplex;
          }
          break;
          
        case PME_TO_Ack:
          {
            messType = GatheredAndRoutedToRootComplex;
          }
          break;

        // TODO?: DLLP Messages, see spec.p. 332
        //        PM_Enter_L1, PM_Enter_L23, PM_Active_State_Request_L1, PM_Request_Ack
          
        // Error Signaling Messages
        case ERR_COR:
        case ERR_NOFATAL:
        case ERR_FATAL:
          {
            messType = RoutedToRootComplex;
          }
          break;
          
        // Locked Transactions Support          
        case Unlock:
          {
            messType = BroadcastFromRootComplex;
          }
          break;
          
        // Ignored Messages
        case Ignored_Message:
          {
            messType = LocalTerminateAtReceiver;
          }
          break;

        // others
        default:
          {
            SC_REPORT_WARNING("PCIeMasterAccess", "init_Message called with not supported Message Code.");
            if (messCode == Vendor_Defined_Type_0 or messCode == Vendor_Defined_Type_1)
              SC_REPORT_WARNING("PCIeMasterAccess", "Please don't use this method for creating Vendor_Defined Messages!");
            messType = NO_MESSAGE;
            return;
          }
      }
      this->setMMessageType(messType);
    }


    /// Make Set_Slot_Power_Limit Message (without) TLP
    /**
     * Fill Message Request (MsgD) transaction (type Set_Slot_Power_Limit) with 1 DW data, see spec. p. 76
     * Standard fields to be set: (Format), Type (and subtype), TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering=00, Address Type=00, Length
     * Additional fields to be set: (Requester ID), Tag, Message Code
     * 3 DW Header.
     *
     * @param power_limit_data  Data to be transmitted: 1 DW (4 bytes)
     */
    void init_Set_Slot_Power_Limit_Message(const gs_uint32 &power_limit_data) {
      GSDataType dt(power_limit_data);
      this->setMPCIeCmd(MsgWithData);
      this->setMData(dt);
      this->setMBurstLength(1);

      this->setMMessageCode(Set_Slot_Power_limit);
      this->setMMessageType(LocalTerminateAtReceiver);
    }

    /// Make Vendor_Defined Message TLP (with or without data)
    /**
     * Fill Message Request (Msg or MsgD) transaction, see spec. p. 69, 77f.
     * Standard fields to be set: (Format), Type (and subtype), TrafficClass, TLP Digest exists (TD), ECRC poisoned (EP), Attribute Relaxed Ordering, Address Type, Length
     * Additional fields to be set: (Requester ID), Tag, Message Code,
     *                              Bus, Device, Function No., Vendor ID, additional vendor header data
     * 4 DW Header.
     *
     * If ID based routing without data is needed, set data=0 and data_size=0.
     *
     * - Automatically sends a Msg or a MsgWithData
     *
     * @param routingType  Routing type of the message (allowed: RoutedToRootComplex, RoutedByID, BroadcastFromRootComplex, LocalTerminateAtReceiver)
     * @param silent_discard_at_receiver  Whether use Type 0 (=false) or Type 1 (=true) Vendor_Defined Message (defines mMessageCode)
     * @param vendorID  ID of the vendor (see spec. p. 77, sec. 2.2.8.6)
     * @param vendorHeaderData  Additional header data for vendor definition (see spec. p. 77f., sec. 2.2.8.6)
     * @param data        (optional:) Data vector, scope must be at least until return
     * @param data_size   (optional:) Size of data that should be written (data vector may be bigger)
     * @param requester_func_no  (optional:) Requester Function Number (default=0)
     * @param busNo       (optional:) Bus number needed for ID based routing (routingType=RoutedByID)
     * @param devNo       (optional:) Device number needed for ID based routing (routingType=RoutedByID)
     * @param funcNo      (optional:) Function number needed for ID based routing (routingType=RoutedByID)
     */
    void init_Vendor_Defined_Message(MessageType routingType, const bool silent_discard_at_receiver,
                                     const unsigned int vendorID, const unsigned int vendorHeaderData,
                                     const std::vector<gs_uint8> &data = *(reinterpret_cast<std::vector<gs_uint8>* >(0)), const unsigned int data_size = 0,
                                     const gs_uint32 requester_func_no = 0,
                                     const unsigned int busNo = 0, const unsigned int devNo = 0, const unsigned int funcNo = 0 ) {
      CHECK_RULES(if (this->getMRequBusNo() == 256 || this->getMRequDevNo() == 32) SC_REPORT_ERROR("PCIeMasterAccess", "A message Request needs to have set the Requester ID before sending. This is not yet done and should be done by the API after creating the PCIe Transaction."););
      this->setMRequFuncNo(requester_func_no);
      if (data_size > 0) {
        GSDataType dt(data);
        this->setMPCIeCmd(MsgWithData);
        this->setMData(dt);
        this->setMBurstLength(1);
      } else {
        this->setMPCIeCmd(Msg);
      }
      if (!silent_discard_at_receiver) 
        this->setMMessageCode(Vendor_Defined_Type_0);
      else
        this->setMMessageCode(Vendor_Defined_Type_1);

      this->setMVendorID(vendorID);
      this->setMVendorHeaderData(vendorHeaderData);

      switch(routingType) {
        case RoutedToRootComplex:
          {
            this->setMIsImplicitRouting(true);
          }
          break;
        case RoutedByID:
          {
            this->setMIsIDbasedRouting(true);
            this->setMBusNo(busNo);
            this->setMDevNo(devNo);
            this->setMFuncNo(funcNo);
          }
          break;
        case BroadcastFromRootComplex:
          {
            this->setMIsImplicitRouting(true);
          }
          break;
        case LocalTerminateAtReceiver:
          {
            this->setMIsImplicitRouting(true);
          }
          break;
        default:
          {
            SC_REPORT_WARNING("PCIeMasterAccess", "Message Type not supported for Vendor_Defined Messages!");
            this->setMPCIeCmd(NO_CMD);
            routingType = NO_MESSAGE;
          }
      }
      this->setMMessageType(routingType);
    }
    
    
  protected:
    inline void init_AddressBased_Request(const PCIeCmd cmd, const MAddr targetAddr, const std::vector<gs_uint8> &data, 
                                          const unsigned int data_size, const gs_uint32 requester_func_no) {
      CHECK_RULES(if (this->getMRequBusNo() == 256 || this->getMRequDevNo() == 32) SC_REPORT_ERROR("PCIeMasterAccess", "An address based Request needs to have set the Requester ID before sending. This is not yet done and should be done by the API after creating the PCIe Transaction."););
      this->setMRequFuncNo(requester_func_no);
      GSDataType dt(data);
      this->setMPCIeCmd(cmd);
      this->setMAddr(targetAddr); // 32-bit if below 4GB, 64-bit otherwise
      this->setMData(dt);
      this->setMBurstLength(data_size);
    }
    inline void init_IDBased_Request(const PCIeCmd cmd, const unsigned int busNo, const unsigned int devNo, const unsigned int funcNo,
                                  const unsigned int registerByteNo, const std::vector<gs_uint8> &data, const unsigned int data_size,
                                  const gs_uint32 requester_func_no) {
      CHECK_RULES(if (this->getMRequBusNo() == 256 || this->getMRequDevNo() == 32) SC_REPORT_ERROR("PCIeMasterAccess", "A message Request needs to have set the Requester ID before sending. This is not yet done and should be done by the API after creating the PCIe Transaction."););
      this->setMRequFuncNo(requester_func_no);
      GSDataType dt(data);
      this->setMPCIeCmd(cmd);
      this->setMIsIDbasedRouting(true);
      this->setMBusNo(busNo);
      this->setMDevNo(devNo);
      this->setMFuncNo(funcNo);
      this->setMRegNo(registerByteNo);
      this->setMData(dt);
      this->setMBurstLength(data_size);
    }

  public:
    
    // ///////////// Methods for processing completed TLPs: /////////////////
    
    inline MComplStatus get_Completion_Status() {
      return this->getMComplStatus();
    }
    
    inline MFuncNo get_completers_Function_Number() {
      return this->getMFuncNo();
    }
    
  };

  /// Access method which allows a slave to process transactions.
  /**
   * This is the API a receiver has to use to process PCIeTransaction objects.
   *
   */
  class PCIeSlaveAccess
   : public virtual gp::GenericSlaveAccess,
     public virtual PCIeTransaction
  {
  public :
    // TODO: remove that ones which are not allowed to be modified/read by Slave
    using PCIeTransaction::setMPCIeCmd;
    using PCIeTransaction::getMPCIeCmd;
    using PCIeTransaction::setMTrafficClass;
    using PCIeTransaction::getMTrafficClass;
    using PCIeTransaction::setMDigestExists;
    using PCIeTransaction::getMDigestExists;
    using PCIeTransaction::setMECRCpoisoned;
    using PCIeTransaction::getMECRCpoisoned;
    using PCIeTransaction::setMIsIDbasedRouting;
    using PCIeTransaction::getMIsIDbasedRouting;
    using PCIeTransaction::setMIsImplicitRouting;
    using PCIeTransaction::getMIsImplicitRouting;
    using PCIeTransaction::setMBusNo;
    using PCIeTransaction::getMBusNo;
    using PCIeTransaction::setMDevNo;
    using PCIeTransaction::getMDevNo;
    using PCIeTransaction::setMFuncNo;
    using PCIeTransaction::getMFuncNo;
    using PCIeTransaction::setMRegNo;
    using PCIeTransaction::getMRegNo;
    using PCIeTransaction::setMRequBusNo;
    using PCIeTransaction::getMRequBusNo;
    using PCIeTransaction::setMRequDevNo;
    using PCIeTransaction::getMRequDevNo;
    using PCIeTransaction::setMRequFuncNo;
    using PCIeTransaction::getMRequFuncNo;
    using PCIeTransaction::setMByteEn;
    using PCIeTransaction::getMByteEn;
    using PCIeTransaction::setMTagID;
    using PCIeTransaction::getMTagID;
    using PCIeTransaction::setMConfigAddr;
    using PCIeTransaction::getMConfigAddr;
    using PCIeTransaction::setMMessageType;
    using PCIeTransaction::getMMessageType;
    using PCIeTransaction::setMMessageCode;
    using PCIeTransaction::getMMessageCode;
    using PCIeTransaction::setMComplStatus;
    using PCIeTransaction::getMComplStatus;
    using PCIeTransaction::setMVC;
    using PCIeTransaction::getMVC;
    using PCIeTransaction::setMDLLPtype;
    using PCIeTransaction::getMDLLPtype;
    using PCIeTransaction::setMVendorID;
    using PCIeTransaction::getMVendorID;
    using PCIeTransaction::setMVendorHeaderData;
    using PCIeTransaction::getMVendorHeaderData;

    // Generic accesses and overloaded generic accesses
    using PCIeTransaction::setMID;
    using PCIeTransaction::setMCmd; // overloaded in PCIeTransaction
    using PCIeTransaction::setMAddr;
    using PCIeTransaction::setMData;
    //using PCIeTransaction::setMDataWidth;  // depricated, see data_width parameter in tlm_port
    //using PCIeTransaction::setSDataWidth;  // depricated, see data_width parameter in tlm_port
    using PCIeTransaction::setMBurstLength;
    using PCIeTransaction::setSBurstLength; // overloaded in PCIeTransaction
    using PCIeTransaction::setMSBytesValid;
    using PCIeTransaction::setSResp;
    using PCIeTransaction::setSData;
    using PCIeTransaction::setTransID;
    using PCIeTransaction::getMAddr;
    using PCIeTransaction::getMID;
    using PCIeTransaction::getMCmd; // overloaded in PCIeTransaction
    using PCIeTransaction::getMData;
    //using PCIeTransaction::getMDataWidth;  // depricated, see data_width parameter in tlm_port
    //using PCIeTransaction::getSDataWidth;  // depricated, see data_width parameter in tlm_port
    using PCIeTransaction::getMBurstLength;
    using PCIeTransaction::getSBurstLength;
    using PCIeTransaction::getMSBytesValid;
    using PCIeTransaction::getSResp;
    using PCIeTransaction::getSData;
    using PCIeTransaction::setMError; // overloaded in PCIeTransaction
    using PCIeTransaction::getMError; // overloaded in PCIeTransaction
    using PCIeTransaction::setSError; // overloaded in PCIeTransaction
    using PCIeTransaction::getSError; // overloaded in PCIeTransaction
    using PCIeTransaction::getTransID;
    using PCIeTransaction::getAttributeIterator;
    using PCIeTransaction::lastAttribute;
    using PCIeTransaction::getExtendedAttributeIterator;
    using PCIeTransaction::lastExtendedAttribute;
    using PCIeTransaction::setPhases;
    using PCIeTransaction::isExtended;
    using PCIeTransaction::getExtendedType;
    using PCIeTransaction::createAttributeIterator;
    using PCIeTransaction::createExtendedAttributeIterator;
    using PCIeTransaction::notifyTargetUpdate;
    using PCIeTransaction::notifyInitiatorUpdate;
    using PCIeTransaction::getTargetUpdateEvent;
    using PCIeTransaction::waitForTargetUpdate;
    using PCIeTransaction::getInitiatorUpdateEvent;
    using PCIeTransaction::waitForInitiatorUpdate;

    // ///////////// Methods for processing incoming TLPs: /////////////////

    inline const MPCIeCmd& get_TLP_type() const {
      return this->getMPCIeCmd();
    }
        
    inline const MAddr& get_addr() const {
      sc_assert(!this->getMIsIDbasedRouting());
      return this->getMAddr();
    }

    // ///////////// Methods for answering TLPs: ///////////////////////////
    
    inline void init_Memory_Read_Completion(const CompletionStatus status, const unsigned int func_no = 0) {
      sc_assert(this->getMPCIeCmd() == MemRead or this->getMPCIeCmd() == MemReadLocked);
      // set completer function number
      this->setMFuncNo(func_no);
      // set Completion Status
      this->setMComplStatus(status);
    }

    inline void init_IO_Read_Completion(const CompletionStatus status, const unsigned int func_no = 0) {
      sc_assert(this->getMPCIeCmd() == IORead);
      // set completer function number
      this->setMFuncNo(func_no);
      // set Completion Status
      this->setMComplStatus(status);
    }
    
    inline void init_IO_Write_Completion(const CompletionStatus status, const unsigned int func_no = 0) {
      sc_assert(this->getMPCIeCmd() == IOWrite);
      // set completer function number
      this->setMFuncNo(func_no);
      // set Completion Status
      this->setMComplStatus(status);
    }

    inline void init_Cnfg_Read_Completion(const CompletionStatus status) {
       init_Cnfg_Read_Completion(status, this->getMFuncNo());  }
    inline void init_Cnfg_Read_Completion(CompletionStatus status, unsigned int func_no) {
      sc_assert(this->getMPCIeCmd() == CnfgReadTy0);
      // set completer function number
      this->setMFuncNo(func_no);
      // set Completion Status
      this->setMComplStatus(status);
    }


    inline void init_Cnfg_Write_Completion(const CompletionStatus status) {
       init_Cnfg_Write_Completion(status, this->getMFuncNo());  }
    inline void init_Cnfg_Write_Completion(CompletionStatus status, unsigned int func_no) {
      sc_assert(this->getMPCIeCmd() == CnfgWriteTy0);
      // set completer function number
      this->setMFuncNo(func_no);
      // set Completion Status
      this->setMComplStatus(status);
    }
        
    inline void set_Completion_Status(const MComplStatus st) {
      this->setMComplStatus(st);
    }

    inline void init_Unsupported_Request() {
      set_Completion_Status(UnsupportedRequest);
    }
    
  };

  /// Access method which allows a router to access the quarks needed by routers.
  class PCIeRouterAccess
   : public virtual gp::GenericRouterAccess,
     public virtual PCIeTransaction
  {
  public :
    // TODO: remove that ones which are not allowed to be modified/read by Router
    using PCIeTransaction::setMPCIeCmd;
    using PCIeTransaction::getMPCIeCmd;
    using PCIeTransaction::setMTrafficClass;
    using PCIeTransaction::getMTrafficClass;
    using PCIeTransaction::setMDigestExists;
    using PCIeTransaction::getMDigestExists;
    using PCIeTransaction::setMECRCpoisoned;
    using PCIeTransaction::getMECRCpoisoned;
    using PCIeTransaction::setMIsIDbasedRouting;
    using PCIeTransaction::getMIsIDbasedRouting;
    using PCIeTransaction::setMIsImplicitRouting;
    using PCIeTransaction::getMIsImplicitRouting;
    using PCIeTransaction::setMBusNo;
    using PCIeTransaction::getMBusNo;
    using PCIeTransaction::setMDevNo;
    using PCIeTransaction::getMDevNo;
    using PCIeTransaction::setMFuncNo;
    using PCIeTransaction::getMFuncNo;
    using PCIeTransaction::setMRegNo;
    using PCIeTransaction::getMRegNo;
    using PCIeTransaction::setMRequBusNo;
    using PCIeTransaction::getMRequBusNo;
    using PCIeTransaction::setMRequDevNo;
    using PCIeTransaction::getMRequDevNo;
    using PCIeTransaction::setMRequFuncNo;
    using PCIeTransaction::getMRequFuncNo;
    using PCIeTransaction::setMByteEn;
    using PCIeTransaction::getMByteEn;
    using PCIeTransaction::setMTagID;
    using PCIeTransaction::getMTagID;
    using PCIeTransaction::setMConfigAddr;
    using PCIeTransaction::getMConfigAddr;
    using PCIeTransaction::setMMessageType;
    using PCIeTransaction::getMMessageType;
    using PCIeTransaction::setMMessageCode;
    using PCIeTransaction::getMMessageCode;
    using PCIeTransaction::setMComplStatus;
    using PCIeTransaction::getMComplStatus;
    using PCIeTransaction::setMVC;
    using PCIeTransaction::getMVC;
    using PCIeTransaction::setMDLLPtype;
    using PCIeTransaction::getMDLLPtype;
    using PCIeTransaction::setMVendorID;
    using PCIeTransaction::getMVendorID;
    using PCIeTransaction::setMVendorHeaderData;
    using PCIeTransaction::getMVendorHeaderData;

    // Generic accesses and overloaded generic accesses
    using PCIeTransaction::setMID;
    using PCIeTransaction::setMCmd; // overloaded in PCIeTransaction
    using PCIeTransaction::setMAddr;
    using PCIeTransaction::setMData;
    //using PCIeTransaction::setMDataWidth;  // depricated, see data_width parameter in tlm_port
    //using PCIeTransaction::setSDataWidth;  // depricated, see data_width parameter in tlm_port
    using PCIeTransaction::setMBurstLength;
    using PCIeTransaction::setSBurstLength; // overloaded in PCIeTransaction
    using PCIeTransaction::setMSBytesValid;
    using PCIeTransaction::setSResp;
    using PCIeTransaction::setSData;
    using PCIeTransaction::setTransID;
    using PCIeTransaction::getMAddr;
    using PCIeTransaction::getMID;
    using PCIeTransaction::getMCmd; // overloaded in PCIeTransaction
    using PCIeTransaction::getMData;
    //using PCIeTransaction::getMDataWidth;  // depricated, see data_width parameter in tlm_port
    //using PCIeTransaction::getSDataWidth;  // depricated, see data_width parameter in tlm_port
    using PCIeTransaction::getMBurstLength;
    using PCIeTransaction::getSBurstLength;
    using PCIeTransaction::getMSBytesValid;
    using PCIeTransaction::getSResp;
    using PCIeTransaction::getSData;
    using PCIeTransaction::setMError; // overloaded in PCIeTransaction
    using PCIeTransaction::getMError; // overloaded in PCIeTransaction
    using PCIeTransaction::setSError; // overloaded in PCIeTransaction
    using PCIeTransaction::getSError; // overloaded in PCIeTransaction
    using PCIeTransaction::getTransID;
    using PCIeTransaction::getAttributeIterator;
    using PCIeTransaction::lastAttribute;
    using PCIeTransaction::getExtendedAttributeIterator;
    using PCIeTransaction::lastExtendedAttribute;
    using PCIeTransaction::setPhases;
    using PCIeTransaction::isExtended;
    using PCIeTransaction::getExtendedType;
    using PCIeTransaction::createAttributeIterator;
    using PCIeTransaction::createExtendedAttributeIterator;
    using PCIeTransaction::notifyTargetUpdate;
    using PCIeTransaction::notifyInitiatorUpdate;
    using PCIeTransaction::getTargetUpdateEvent;
    using PCIeTransaction::waitForTargetUpdate;
    using PCIeTransaction::getInitiatorUpdateEvent;
    using PCIeTransaction::waitForInitiatorUpdate;

    // ///////////// Methods for processing incoming TLPs: /////////////////

    inline const MPCIeCmd& get_TLP_type() const {
      return this->getMPCIeCmd();
    }
        
    inline const MAddr& get_addr() const {
      sc_assert(!this->getMIsIDbasedRouting());
      return this->getMAddr();
    }

    // ///////////// Methods for answering TLPs: ///////////////////////////

    inline void set_Completion_Status(const MComplStatus st) {
      this->setMComplStatus(st);
    }

    inline void init_Unsupported_Request() {
      set_Completion_Status(UnsupportedRequest);
    }

    // ///////////// Methods for processing completed TLPs: /////////////////
    
    inline MComplStatus get_Completion_Status() {
      return this->getMComplStatus();
    }
    
  };


} // end namespace PCIe
} // end namespace tlm


#endif
