/* $Id$ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2007 Dirk Opfer <dirk@do13.de>
*  Copyright (c) 2007 Matthias Fechner <matthias@fechner.net>
*  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   fb_prot.h
* @author Dirk Opfer, Matthias Fechner, Christian Bode, Dirk Armbrust
* @date   Fri Dec 21 18:28:22 2007
* 
* @brief  Define the protocol specific definitions
*        Implementation of the communication protocol
*
*/
#ifndef _FB_PROT_H
#define _FB_PROT_H

#ifdef  _FB_PROT_C
#define PROT_EXT
#else
/** Set PROT_EXT to extern to make clear it is include from an external source like a lib */
#define PROT_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "msg_queue.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/
#define MSG_PRIO_SYSTEM  0                    /**< Mesage priority is system (highest priority, configuration and management) */
#define MSG_PRIO_ALARM   (1<<1)               /**< Mesage priority is alarm (interface time 20-22ms) */
#define MSG_PRIO_HIGH    (1<<0)               /**< Mesage priority is high (interface time > 22ms) */
#define MSG_PRIO_LOW     (1<<1 | 1<<0)        /**< Mesage priority is low (interface time > 25ms) */
#define MSG_MASK         0xC                  /**< Mask for the message priority */

#define T_BROADCAST_DATA       0x8000         /**< A broadcast telegram             */
#define T_BROADCAST_DATA_MASK  0x80FC         /**< Mask for the broadcast telegram  */
#define T_CONNECT_REQ          0x0080         /**< A connect request                */
#define T_CONNECT_REQ_MASK     0x80FF         /**< Mask for the connect request     */
#define T_DISCONNECT_REQ       0x0081         /**< A disconnect request             */
#define T_DISCONNECT_REQ_MASK  0x80FF         /**< Mask for the disconnect request  */

#define PROGRAMMING_MODE       0x81           /**< value for programming mode       */
#define MONITOR_MODE           0x90           /**< value for bus monitor mode       */
#define LINK_MODE              0x12           /**< value for link layer mode        */
#define TRANSPORT_MODE         0x96           /**< value for transport layer mode   */
#define APPLICATION_MODE       0x1E           /**< value for application layer mode */
#define RESET_MODE             0xC0           /**< value for reset                  */


#define BASE_ADDRESS_OFFSET           0x0100  /**< Offset in BCU1 EEPROM                               */
/** @todo delete define */
#define MANUFACTORER_ADR_HIGH         0x0103  ///< High address for manufactorer id                    */
#define MANUFACTORER_ADR_LOW          0x0104  /**< Low address for manufactorer id                     */
#define DEVICE_NUMBER_HIGH            0x0105  /**< High address for device id                          */
#define DEVICE_NUMBER_LOW             0x0106  /**< Low address for device id                           */
#define SOFTWARE_VERSION_NUMBER       0x0107  /**< Version number of the application                   */
#define APPLICATION_PROGRAMM          0x010C  /**< Startadress of the application program              */
/** @todo delete define */
#define PORT_A_DIRECTION_BIT          0x010C  ///< Port A Direction Bit Setting
#define APPLICATION_RUN_STATUS        0x010D  /**< status of the application 0x00=run, 0xFF=stop       */
#define ROUTECOUNT                    0x010E  /**< @todo document variable                             */
#define ASSOCTABPTR                   0x0111  /**< start of associations in eeprom                     */
#define COMMSTAB_ADDRESS              0x0112  /**< COMMSTAB Pointer                                    */
#define COUNT_GRP_ADDRESSES           0x0116  /**< number of group addresses stored in EEPROM          */
#define PA_ADDRESS_HIGH               0x0117  /**< high address for the physical address of the device */
#define PA_ADDRESS_LOW                0x0118  /**< low address for the physical address of the device  */
#define START_ADDRESS_GROUP_ADR       0x0119  /**< address in eeprom where group addresses are stored  */
/** @todo delete define */
#define START_ADDRESS_GROUP_ADDRESSES 0x0119  ///< address in eeprom where group addresses are stored  */
#ifdef  FB_RF
#define RF_RSSI                       RF_NODEPARAMS
#define RF_DOMAINADDRESS              RF_NODEPARAMS+10
#define RF_PHYSICALADRESS_MASK        RF_NODEPARAMS+8
#endif

/** group address rewrite **/

//-----------------------------------------------------------------------------------------------------------
// Ramflags
//-----------------------------------------------------------------------------------------------------------
#define UPDATE                      0x01

#if 0
/* BCUx values. Not used yet */
#define IDLEERROR                   0x01
#define TRANSMITTING                0x02
#define TRANSMIT_REQUEST            0x03
#define DATA_REQUEST                0x04
#define UPDATE                      0x08
#define SET_FLG                     0x80
#define CLR_FLG                     0x00
#endif

//-----------------------------------------------------------------------------------------------------------
// ConfigByte
//-----------------------------------------------------------------------------------------------------------
#define TX_SYSTEM                   0x00
#define TX_URGENT                   0x01
#define TX_NORMAL                   0x02
#define TX_LOW                      0x03
#define COMM_ENABLE                 0x04
#define READ_ENABLE                 0x08
#define WRITE_ENABLE                0x10
#define MEMORY_SEGMENT              0x20
#define TRANSMIT_ENABLE             0x40
#define UPDATE_ON_RESPONSE          0x80

//-----------------------------------------------------------------------------------------------------------
// Typebyte
//-----------------------------------------------------------------------------------------------------------
#define UINT1                       0x00                    // 1 bit
#define UINT2                       0x01                    // 2 bit
#define UINT3                       0x02                    // 3 bit
#define UINT4                       0x03                    // 4 bit
#define UINT5                       0x04                    // 5 bit
#define UINT6                       0x05                    // 6 bit

#define UINT7                       0x06                    // 7 bit
#define UINT8                       0x07                    // 8 bit

#define UINT16                      0x08                    // 16 bit
#define TIME_DATE                   0x09                    // 3byte
#define FLOAT                       0x0A                    // 4byte
#define DATA6                       0x0B                    // 6byte
#define DOUBLE                      0x0C                    // 8byte
#define DATA10                      0x0D                    // 10byte
#define MAXDATA                     0x0E                    // 14byte
#define VARDATA                     0x0F                    // 1-14 bytes (???)

struct FBAppInfo
{
    const uint8_t           FBApiVersion;
    const STRUCT_DEFPARAM   *pParam;

/* Not used yet */
    void (*AppMain)         (void);
    void (*AppSave)         (void);
    void (*AppUnload)       (void);
    uint8_t (*AppReadObject)     (uint8_t objectNr, void* dest, uint8_t len);
    uint8_t (*AppWriteObject)    (uint8_t objectNr, void* src, uint8_t len);
    void *ramflags;
    void *comobjtable;
    void *assoctable;
    void *grpaddr;
}; 

struct assoc_table_s {
	uint8_t count;
	struct {
		uint8_t ganr;
		uint8_t objnr;
	} entry[];
};

struct comm_stab_s {
    uint8_t count;
    uint8_t ramptr;
    struct {
	uint8_t ptr;
	uint8_t flags;
	uint8_t type;
    } object[];
};

/** Structure for the group addresses */
struct grp_addr_s {
	uint8_t   count;                    /**< Number of group addresses stored on controller */
	union addr_u {
		uint16_t gai;
		uint8_t gab[2];
	} addr[];
};

/** Structure of the EIB header for each telegram */
struct fbus_hdr {
        uint8_t ctrl;      /**< control byte l0r1pp00, l=frame length (0=long frame, more then 15 octets, 1=short frame), r=repeat flag, pp=priority (for communication medium TP1) */
        uint8_t src[2];    /**< source adress, 16-bit, hhhhmmmm llllllll, convert it to ascii so you have: h.m.l or e.g. 1.1.28 */
        uint8_t dest[2];   /**< destination adress, 16-bit, hhhhmmmm llllllll if DAF=0, 0hhhhmmm llllllll if DAF=1, DAF can be found in npci */
        uint8_t npci;      /**< dnnnllll, d=destination address flag (DAF, 0==unicast, 1==multicast), n=network control field, l=length */
        uint8_t tpci;      /**< combination of tpci (t) and apci (a), tttt ttaa */
        uint8_t apci;      /**< application layer protocol control information, here every function has its own number */
//  uint8_t cmd[2];        /**< command */
};

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
PROT_EXT uint8_t TestAndCopyObject (uint8_t objectNr, void* dst, uint8_t len);
PROT_EXT void SetAndTransmitObject(uint8_t objectNr, void* src, uint8_t len);
PROT_EXT uint8_t TestObject(uint8_t objectNr);
PROT_EXT void TransmitObject(uint8_t objectNr);
PROT_EXT uint8_t ReadObject(uint8_t objectNr);
PROT_EXT void SetRAMFlags(uint8_t objectNr, uint8_t flags);

PROT_EXT void sendExtTelegram(uint8_t commObjectNumber, uint8_t value, uint8_t offset);
PROT_EXT void sendTelegram(uint8_t commObjectNumber, uint8_t value, uint8_t offset);
PROT_EXT uint8_t findRespondGroupAddressByObjectNumber(uint8_t commObjectNumber, uint8_t offset);
PROT_EXT uint8_t fb_handlemsg(struct msg *rxmsg);
PROT_EXT void fbprot_msg_handler ( void )XBOOT_SECTION;
PROT_EXT void fbprot_forward_msg ( struct msg *rxmsg );
PROT_EXT void fbprot_LibInit(void);
#endif /* _FB_PROT_H */
/*********************************** EOF *********************************/
