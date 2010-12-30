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
* @file   fb_ow_temp_lib.c
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for DS18B20 usage
*/
#ifndef _FB_OW_TEMP_LIB_C
#define _FB_OW_TEMP_LIB_C


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_hal.h"
#include "fb_ow_lib.h"
#include "fb_ow_temp_lib.h"


/**************************************************************************
* DEFINITIONS
**************************************************************************/


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


uint8_t ow_tempReadScratchpad( OW_DEVICE_ID *pId, uint8_t *pBuf )
{
    uint8_t res;

    do {

        res = ow_SelectDevice(pId);
        if ( res )
            break;

        res = ow_WriteByte(0xbe); // read scratchpad command
        if (res)
            break;

        res = ow_ReadBuffer(pBuf,9); // 8 bytes + CRC
        if (res)
            break;

        uint8_t i;
        uint8_t crc = 0;
        for (i=0;i<9;i++) {
            crc = ow_CRC(crc,pBuf[i]);
        }

        if ( crc ) {
            res = -2;
        }

    } while ( FALSE );

    return res;
}

uint8_t   ow_tempConfig(OW_DEVICE_ID *pId, uint8_t nCfg)
{
    uint8_t res;
    uint8_t buf[9];

    do {

        res = ow_SelectDevice(pId);
        if ( res )
            break;

        buf[0] = 0x4e; // write scratchpad command + data for registers 2,3,4
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = nCfg;        

        res = ow_WriteBuffer( buf, 4 ); 
        if ( res )
            break;

        if ( pId != 0 ) { // read back if individual device
            res = ow_tempReadScratchpad(pId, buf);
            if ( res )
                break;

            if ( (buf[3] & 0x60) != (nCfg & 0x60) ) {
                res = -3;
            }
        }    
    } while(FALSE);

    return res;
}

uint8_t   ow_tempStartConversion(OW_DEVICE_ID *pId)
{
    uint8_t res;

    do {

        res = ow_SelectDevice(pId);
        if ( res )
            break;

        res = ow_AdjustPower();
        if ( res )
            break;

        res = ow_WriteByte(0x44); // CONVERT TEMP Command

    } while(FALSE);

    return res;
}

uint8_t   ow_tempReadTemperature(OW_DEVICE_ID *pId, int16_t *pTemp)
{
    uint8_t res;
    uint8_t buf[9];

    do {

        res = ow_SelectDevice(pId);
        if ( res )
            break;

        res = ow_tempReadScratchpad(pId,buf);
        if ( res )
            break;

        *pTemp = *((int16_t*)buf);

        // TODO: mask any unused LSB according to the resolution configuration in CFG register


    } while(FALSE);

    return res;
}








#endif /* _FB_OW_TEMP_LIB_C */
/*********************************** EOF *********************************/
