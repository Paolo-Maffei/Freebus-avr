/* $Id$ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
*  /_/   /_/ |_/_____/_____/_____/\____//____/
*
*  Copyright (c) 2012 Dirk Opfer <dirk@do13.net>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   freebus-avr.h
* @author Dirk Opfer
*
* @brief  Hardware specific options for AVR.
* DO NOT INCLUDE THAT FILE DIRECTLY, include fb_hardware.h instead.
*
*
*/
#if defined(_FB_HARDWARE_H)
#ifndef _FREEBUS_AVR_H
#define _FREEBUS_AVR_H

/*************************************************************************
* INCLUDES
*************************************************************************/
#include <inttypes.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h> 
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/parity.h>
#include <util/delay.h>

/**************************************************************************
* DEFINITIONS
**************************************************************************/
#define ENABLE_ALL_INTERRUPTS()     sei()                   /**< global interrupt enable */ 
#define DISABLE_IRQS    unsigned char _sreg = SREG; cli();  /**< Disable IRQs, save status before doing this */
#define ENABLE_IRQS     { if(_sreg & 0x80) sei(); }         /**< Enable IRQs if they are enabled before DISABLE_IRQS is called */

/** Enable internal hardware watchdog */
#define ENABLE_WATCHDOG(x)          {           \
          wdt_enable(x);                        \
     }


/** Disable internal hardware watchdog */
#define DISABLE_WATCHDOG()          {           \
          MCUSR = 0;                            \
          wdt_disable();                        \
     }

#define IO_SET(nr, val)  do {if(val) {IO##nr##_PORT |= (1U<<IO##nr##_PIN);} else {IO##nr##_PORT &= ~(1U<<IO##nr##_PIN);} } while (0)
#define IO_GET(nr)  ((IO##nr##_IN>>IO##nr##_PIN) & 0x01)
#define IO_SET_DIR(nr,type) do {IO##nr##_DDR = (IO##nr##_DDR & ~(1U<<IO##nr##_PIN)) | (type<<IO##nr##_PIN); } while(0)

typedef uint8_t prog_uint8_t;

#endif /* _FREEBUS_AVR_H */
#endif /* _FB_HARDWARE_H */
/*********************************** EOF *********************************/
