/*  */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2010 Kent Filek <kent@filek.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb_rtc_lib.h
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for RTC usage
*/
#ifndef _FB_OW_TEMP_LIB_H
#define _FB_OW_TEMP_LIB_H


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include <avr/io.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define OW_TEMP_RES12 0x00
#define OW_TEMP_RES11 0x20
#define OW_TEMP_RES10 0x40
#define OW_TEMP_RES9  0x60


/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

uint8_t   ow_tempConfig(OW_DEVICE_ID *pId, uint8_t nCfg);
uint8_t   ow_tempStartConversion(OW_DEVICE_ID *pId);
uint8_t   ow_tempReadTemperature(OW_DEVICE_ID *pId, int16_t *pTemp);

uint8_t   ow_tempReadScratchpad( OW_DEVICE_ID *pId, uint8_t *pBuf );


#endif /* _FB_OW_TEMP_LIB_H */
/*********************************** EOF *********************************/
