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
* @file   fb.h
* @author Dirk Opfer
* @date   Sat Apr 05 22:52:06 2008
* 
* @brief  Define general values used in bus communication.
* 
*/

#ifndef _FB_H
#define _FB_H

#define FB_NONE     0x00    /**< Value for nothing  */
#define FB_ACK      0xCC    /**< Value for an ACK   */
#define FB_NACK     0x0C    /**< Value for a NACK   */
#define FB_BUSY     0xC0    /**< Value for BUSY     */


/** Number of group adreses stored in eeprom */ 
#define FB_MAX_GROUP_ADDR   19  


/* bit values for the systemstate register */
#define SS_PROG     (1 << 0) /**< bit for programming mode              */
#define SS_LLM      (1 << 1) /**< bit for link layer mode               */
#define SS_TLE      (1 << 2) /**< bit for transport layer enable        */
#define SS_ALE      (1 << 3) /**< bit for application layer enable      */
#define SS_SE       (1 << 4) /**< bit for serial (PEI) interface enable */
#define SS_UE       (1 << 5) /**< bit for user program enable           */
#define SS_DM       (1 << 6) /**< bit for bootloader-download mode      */

#ifndef FALSE
#define FALSE (0!=0)         /**< macro which will always return false  */
#endif

#ifndef TRUE
#define TRUE  (0==0)         /**< macro which will always return true   */
#endif

#ifdef BOOTLOADER
#define XBOOT_SECTION __attribute__((section(".xbootloader")))
#else
#define XBOOT_SECTION
#endif

#ifdef __GNUC__
#define __UNUSED__  __attribute__ ((unused))
#else
#define __UNUSED__
#endif


#endif /* _FB_H */
/*********************************** EOF *********************************/
