/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   fbrf_hal.h
* @author Dirk Armbrust
* @date   Jan 13 2011
* 
* @brief  Definitions on KNX-RF compatible Hardware Abstration Layer
* 
* 
*/
#ifndef FBRF_HAL_H_
#define FBRF_HAL_H_

#ifndef F_CPU
#error "F_CPU not defined. define it in Make.config"
#endif

#ifdef _FBRF_HAL_C
#define RFHAL_EXT
#else
#define RFHAL_EXT    extern
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

#define RF_TX         /* tx and rx enabled. todo: support for rx only and tx only */
#define RF_RX

#define RFM22_INT_vect    INT1_vect
#define FBRF_TIMER_vect  TIMER2_OVF_vect

/* Antenna switch is controlled by GPIO0 / GPIO1 of RFM22,
   so corresponding macros are empty */
#define  RX_ANT_ON()
#define  RX_ANT_OFF()
#define  TX_ANT_ON()
#define  TX_ANT_OFF()

/* if you don't have RX / TX led, then don't define the following */
/* we can't use IO6 and IO7 for LEDs !! */

#define  RX_LED_PORT   rfleds
/* #define  RX_LED_DDR    DDRD */
#define  RX_LED_PIN    0
#define  TX_LED_PORT   rfleds
/* #define  TX_LED_DDR    DDRD */
#define  TX_LED_PIN    1

#ifdef FB_RF
#define  FBRFHAL_POLLING() {	\
	static uint8_t pollcnt;			\
	if ( (pollcnt--) == 0){		\
	fbrfhal_polling();			\
	pollcnt = 100;          /* 10msec pollrate */ \
	}\
 }
#define FBRFHAL_INIT()  fbrfhal_init();
#else
#define  FBRFHAL_POLLING()
#define FBRFHAL_INIT()
#endif


/**************************************************************************
* DECLARATIONS
**************************************************************************/
RFHAL_EXT uint8_t rfleds;      /**< state of RF RX/TX LEDs */

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
RFHAL_EXT uint8_t fbrf_hal_txqueue_msg(struct msg* rftx);
RFHAL_EXT uint8_t fbrfrx_init( void (*pf)(struct msg* rfrxmsg_pointer));
RFHAL_EXT void    fbrfhal_init(void) XBOOT_SECTION;
RFHAL_EXT uint8_t fbrfhal_polling(void);

#endif