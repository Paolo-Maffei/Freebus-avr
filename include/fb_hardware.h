/* $Id$ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
*  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb_hardware.h
* @author Matthias Fechner, Christian Bode, Dirk Armbrust
* @date   Mon Jun  2 09:19:42 2008
* 
* @brief  Hardware specific options which includes the platforms (like ATmega8, ATmega168...)
* 
* 
*/
#ifndef _FB_HARDWARE_H
#define _FB_HARDWARE_H

/*************************************************************************
* INCLUDES
*************************************************************************/
#ifdef __AVR__
#define pgm_read_ptr(x)         pgm_read_word(x)
#include "freebus-avr.h"
#endif

#ifdef BOARD301  /* first freebus AVR board rev. 3.01 */

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega88__)
    #include "freebus-atmega168.h"
#elif defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__) 
    #include "freebus-atmega168p.h"
#else
    #error CPU not supported
#endif

#else  /* board with RFM22 */

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega88__)
    #include "fbrf-atmega168.h"
#elif defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)
    #include "fbrf-atmega168p.h"
#elif defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__)
    #include "freebus-atmega644p.h"
#elif defined(__EIBD__)
    #include "fb-eibd.h"
#else
    #error CPU not supported
#endif

#endif  /* board selection*/

#define SETPIN_PROG(val)        if(val)                                 \
                                {                                       \
                                    PROG_PORT |= (1U<<PROG_PIN);        \
                                }                                       \
                                else                                    \
                                {                                       \
                                    PROG_PORT &= ~(1U<<PROG_PIN);       \
                                }
#define GETPIN_PROG()           ((PROG_IN>>PROG_PIN) & 0x01)
#define SET_IO_PROG(type)       PROG_DDR = (PROG_DDR & ~(1U<<PROG_PIN)) | (type<<PROG_PIN);\
                                PROG_PORT = (PROG_PORT & ~(1U<<PROG_PIN)) | ((~type & 0x01)<<PROG_PIN)
                                
#define SETPIN_CTRL(val)        if(val)                                 \
                                {                                       \
                                    CTRL_PORT |= (1U<<CTRL_PIN);        \
                                }                                       \
                                else                                    \
                                {                                       \
                                    CTRL_PORT &= ~(1U<<CTRL_PIN);       \
                                }
#define GETPIN_CTRL()           ((CTRL_IN>>CTRL_PIN) & 0x01)
#define SET_IO_CTRL(type)       CTRL_DDR = (CTRL_DDR & ~(1U<<CTRL_PIN)) | (type<<CTRL_PIN)

#define SETPIN_EIBOUT(val)      if(val)                                 \
                                {                                       \
                                    EIBOUT_PORT |= (1U<<EIBOUT_PIN);    \
                                }                                       \
                                else                                    \
                                {                                       \
                                    EIBOUT_PORT &= ~(1U<<EIBOUT_PIN);   \
                                }
#define SET_IO_EIBOUT(type)     EIBOUT_DDR = (EIBOUT_DDR & ~(1U<<EIBOUT_PIN)) | (type<<EIBOUT_PIN)   

#define GETPIN_EIBIN()          ((EIBIN_IN>>EIBIN_PIN) & 0x01)
#define GETPIN_EIBOUT()         ((EIBOUT_IN>>EIBOUT_PIN) & 0x01)
#define SET_IO_EIBIN(type)      EIBIN_DDR = (EIBIN_DDR & ~(1U<<EIBIN_PIN)) | (type<<EIBIN_PIN);\
                                EIBIN_PORT = (EIBIN_PORT & ~(1U<<EIBIN_PIN)) | ((~type & 0x01)<<EIBIN_PIN)




#define SETPIN_RES1(val)        if(val)                                 \
                                {                                       \
                                    IO1_PORT |= (1U<<IO1_PIN);          \
                                }                                       \
                                else                                    \
                                {                                       \
                                    IO1_PORT &= ~(1U<<IO1_PIN);         \
                                }
#define SETPIN_RES1_ON          RES1_PORT |= (1U<<RES1_PIN);
#define SETPIN_RES1_OFF         RES1_PORT &= ~(1U<<RES1_PIN);
#define GETPIN_RES1()           ((RES1_IN>>RES1_PIN) & 0x01)
#define SET_IO_RES1(type)       RES1_DDR = (RES1_DDR & ~(1U<<RES1_PIN)) | (type<<RES1_PIN);\
                                RES1_PORT = (RES1_PORT & ~(1U<<RES1_PIN)) | ((~type & 0x01)<<RES1_PIN)

#define SETPIN_RES2_ON          RES2_PORT |= (1U<<RES2_PIN);
#define SETPIN_RES2_OFF         RES2_PORT &= ~(1U<<RES2_PIN);
#define GETPIN_RES2()           ((RES2_IN>>RES2_PIN) & 0x01)
#define SET_IO_RES2(type)       RES2_DDR = (RES2_DDR & ~(1U<<RES2_PIN)) | (type<<RES2_PIN);\
                                RES2_PORT = (RES2_PORT & ~(1U<<RES2_PIN)) | ((~type & 0x01)<<RES2_PIN)

#define SETPIN_RES3_ON          RES3_PORT |= (1U<<RES3_PIN);
#define SETPIN_RES3_OFF         RES3_PORT &= ~(1U<<RES3_PIN);
#define GETPIN_RES3()           ((RES3_IN>>RES3_PIN) & 0x01)
#define SET_IO_RES3(type)       RES3_DDR = (RES3_DDR & ~(1U<<RES3_PIN)) | (type<<RES3_PIN);\
                                RES3_PORT = (RES3_PORT & ~(1U<<RES3_PIN)) | ((~type & 0x01)<<RES3_PIN)

#define SETPIN_RES4_ON          RES4_PORT |= (1U<<RES4_PIN);
#define SETPIN_RES4_OFF         RES4_PORT &= ~(1U<<RES4_PIN);
#define GETPIN_RES4()           ((RES4_IN>>RES4_PIN) & 0x01)
#define SET_IO_RES4(type)       RES4_DDR = (RES4_DDR & ~(1U<<RES4_PIN)) | (type<<RES4_PIN);\
                                RES4_PORT = (RES4_PORT & ~(1U<<RES4_PIN)) | ((~type & 0x01)<<RES4_PIN)

#endif /* _FB_HARDWARE_H */
/*********************************** EOF *********************************/
