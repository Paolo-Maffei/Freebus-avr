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
* @file   fb_hal.h
* @author Dirk Opfer, Matthias Fechner, Christian Bode
* @date   Tue Jan 08 11:19:00 2008
* 
* @brief  Definitions on Hardware Abstration Layer
* 
* 
*/
#ifndef _FB_HAL_H
#define _FB_HAL_H

#ifdef _FB_HAL_C
#define HAL_EXT
#else
#define HAL_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "msg_queue.h"


/**************************************************************************
* DEFINITIONS
**************************************************************************/
#define FB_FRAME_ERR	(1 << 7) ///< Error error, e.g. stop bit is missing
#define FB_PARITY_ERR	(1 << 6) ///< Parity error in octet
#define FB_COLLISION	(1 << 5) ///< Collision detected

#define FB_CHAR_REC		(1 << 3) ///< In character receive mode
#define FB_BUS_FREE		(1 << 2) ///< EIB bus is free and messages can be send

#define FB_ACK_REC		(1 << 1) ///< In acknowledge receive mode
#define FB_MESSAGE		(1 << 0) ///< Message type is a EIB message

/*********************************/
/* MACROs for port configuration */
/*********************************/
#define OFF          0U             /**< Global Define: OFF = 0     */
#define ON           1U             /**< Global Define: ON  = 1     */

#define IO_INPUT     0U             /**< Portpin-Direction is Input  */
#define IO_OUTPUT    1U             /**< Portpin-Direction is Output */

/* define different panic calls here */
#define PANIC_MEMORY		0x01    /**< Memory error, is set when no memory is left and it is tried to allocate memory */
#define PANIC_RESOURCE		0x02    /**< No more resources are available */
#define PANIC_EEPROM		0x03    /**< Error in eeprom */
#define PANIC_NO_MSG		0x04    /**< No more space for messages is left in allocated message queue */

/**
* define _NOP which calls assembler routine nop.
* Can be used to wait some time and/or to prevent compiler from removing code in optimization
* (empty loops to wait etc.)
*/
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)

/**************************************************************************
* DECLARATIONS
**************************************************************************/
HAL_EXT uint8_t systemState;

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
HAL_EXT uint8_t fbhal_checkProgTaster(void);
HAL_EXT void fbhal_Init(void);
HAL_EXT void fb_send_ack(uint8_t value);
HAL_EXT inline void fb_hal_restart(void);
HAL_EXT uint8_t fb_hal_txqueue_msg(struct msg* tx);
HAL_EXT void sendTestTelegram(void);
HAL_EXT void panic(uint8_t reason);
 

#endif /* _FB_HAL_H */
/*********************************** EOF *********************************/
