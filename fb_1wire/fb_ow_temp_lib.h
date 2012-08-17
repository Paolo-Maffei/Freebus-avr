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
#include "fb_ow_lib.h"
#include <avr/io.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/
// constant to convert the fraction bits to cel*(10^-4)
#define OW_TEMP_FRACCONV          625

// undefined bits in LSB if 18B20 != 12bit
#define OW_TEMP_9_BIT_UNDF        ((1<<0)|(1<<1)|(1<<2))
#define OW_TEMP_10_BIT_UNDF       ((1<<0)|(1<<1))
#define OW_TEMP_11_BIT_UNDF       ((1<<0))
#define OW_TEMP_12_BIT_UNDF       0


#define OW_TEMP_CONF_REG          4
#define OW_TEMP_9_BIT             0
#define OW_TEMP_10_BIT            (1<<5)
#define OW_TEMP_11_BIT            (1<<6)
#define OW_TEMP_12_BIT            ((1<<6)|(1<<5))
#define OW_TEMP_RES_MASK          ((1<<6)|(1<<5))



/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

uint8_t   ow_tempConfig(OW_DEVICE_ID *pId, uint8_t nCfg);
uint8_t   ow_tempStartConversion(OW_DEVICE_ID *pId);
uint8_t   ow_tempReadTemperature(OW_DEVICE_ID *pId, int16_t *pTemp);
uint8_t   ow_tempReadTemperature2(OW_DEVICE_ID *pId, int32_t *pTemp);
int32_t   ow_temp_raw_to_maxres( uint8_t familycode, uint8_t sp[] );

uint8_t   ow_tempReadScratchpad( OW_DEVICE_ID *pId, uint8_t *pBuf );


#endif /* _FB_OW_TEMP_LIB_H */
/*********************************** EOF *********************************/
