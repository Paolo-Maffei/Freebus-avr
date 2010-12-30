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
#ifndef _FB_RTC_LIB_H
#define _FB_RTC_LIB_H


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include <avr/io.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/


/**************************************************************************
* DECLARATIONS
**************************************************************************/

typedef struct {
    uint8_t Seconds:7;
    uint8_t ClockHold:1;
    uint8_t Minutes;
    uint8_t Hours;
    uint8_t DayOfWeek;
    uint8_t Day;
    uint8_t Month;
    uint8_t Year;
} RTC_DATETIME;

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

void        rtc_Init();
uint8_t     rtc_ReadDateTime( RTC_DATETIME * pDateTime );
uint8_t     rtc_WriteDateTime( RTC_DATETIME * pDateTime );

#endif /* _FB_RTC_LIB_H */
/*********************************** EOF *********************************/
