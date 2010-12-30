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
* @file   fb_rtc_lib.c
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for RTC DS1307 usage
*/
#ifndef _FB_RTC_LIB_C
#define _FB_RTC_LIB_C


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_hal.h"
#include "fb_i2c_lib.h"
#include "fb_rtc_lib.h"


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define I2C_ADDR_RTC  0x68

/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

/*************************************************************************
* GLOBALS
**************************************************************************/


/**************************************************************************
* IMPLEMENTATION
**************************************************************************/

void rtc_Init()
{
    // nothing to do
}

uint8_t     rtc_ReadDateTime( RTC_DATETIME * pDateTime )
{
    uint8_t res;
    uint8_t len = 0;

    do {

        res = i2c_WriteByteReadBuffer( I2C_ADDR_RTC, 0, (uint8_t*)pDateTime, sizeof(RTC_DATETIME), &len ); 
        if ( res ) 
            break;

        if ( len != sizeof(RTC_DATETIME) ) {
            res = -1;
            break;
        }


/*  convert BCD to Dezimal
        pDateTime->Seconds = (pDateTime->Seconds & 0x0f) + ((pDateTime->Seconds&0x70)>>4)*10;
        pDateTime->Minutes = (pDateTime->Minutes & 0x0f) + ((pDateTime->Minutes)>>4)*10;
        pDateTime->Hours   = (pDateTime->Hours & 0x0f) + ((pDateTime->Hours & 0x30)>>4)*10;
        pDateTime->Day     = (pDateTime->Day & 0x0f) + ((pDateTime->Day)>>4)*10;
        pDateTime->Month   = (pDateTime->Month & 0x0f) + ((pDateTime->Month)>>4)*10;
        pDateTime->Year    = (pDateTime->Year & 0x0f) + ((pDateTime->Year)>>4)*10;
*/
    
    } while (0);

    return res;
};


uint8_t     rtc_WriteDateTime( RTC_DATETIME * pDateTime )
{
    uint8_t res;
    uint8_t buf[sizeof(RTC_DATETIME)+1];

    memcpy(buf+1,(void*)pDateTime, sizeof(RTC_DATETIME));
    buf[0] = 0; // register pointer


    res = i2c_WriteBuffer( I2C_ADDR_RTC, buf, sizeof(buf) );


    return res;
};








#endif /* _FB_RTC_LIB_C */
/*********************************** EOF *********************************/
