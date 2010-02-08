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
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   fb_prot.h
* @author Dirk Opfer, Matthias Fechner, Christian Bode
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
#define MANUFACTORER_ADR_LOW          0x0104  ///< Low address for manufactorer id
#define MANUFACTORER_ADR              0x0104  /**< Low address for manufactorer id                     */
#define DEVICE_NUMBER_HIGH            0x0105  /**< High address for device id                          */
#define DEVICE_NUMBER_LOW             0x0106  /**< Low address for device id                           */
#define SOFTWARE_VERSION_NUMBER       0x0107  /**< Version number of the application                   */
#define APPLICATION_PROGRAMM          0x010C  /**<                                                     */
/** @todo delete define */
#define PORT_A_DIRECTION_BIT          0x010C  ///< Port A Direction Bit Setting
#define APPLICATION_RUN_STATUS        0x010D  /**< status of the application 0x00=run, 0xFF=stop       */
#define ROUTECOUNT                    0x010E  /**<                                                     */
#define ASSOCTABPTR                   0x0111  /**< start of associations in eeprom                     */
#define COMMSTAB_ADDRESS              0x0112  /**< COMMSTAB Pointer                                    */
#define COUNT_GRP_ADDRESSES           0x0116  /**< number of group addresses stored in EEPROM          */
#define PA_ADDRESS_HIGH               0x0117  /**< high address for the physical address of the device */
#define PA_ADDRESS_LOW                0x0118  /**< low address for the physical address of the device  */
#define START_ADDRESS_GROUP_ADR       0x0119  /**< address in eeprom where group addresses are stored  */
/** @todo delete define */
#define START_ADDRESS_GROUP_ADDRESSES 0x0119  ///< address in eeprom where group addresses are stored  */


/** Structure for the group addresses */
struct grp_addr_s {
          uint8_t   count;                    /**< Number of group addresses stored on controller */
          uint16_t ga[FB_MAX_GROUP_ADDR];     /**< The group address (2x8 Bit) */
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

/**************************************************************************
* DECLARATIONS
**************************************************************************/
PROT_EXT uint16_t pa;

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
PROT_EXT void sendExtTelegram(uint8_t commObjectNumber, uint8_t value, uint8_t offset);
PROT_EXT void sendTelegram(uint8_t commObjectNumber, uint8_t value, uint8_t offset);
PROT_EXT uint8_t findRespondGroupAddressByObjectNumber(uint8_t commObjectNumber, uint8_t offset);
PROT_EXT uint8_t fb_handlemsg(struct msg *rxmsg);
PROT_EXT void fbprot_Init(const STRUCT_DEFPARAM *pParam);

#endif /* _FB_PROT_H */
/*********************************** EOF *********************************/
