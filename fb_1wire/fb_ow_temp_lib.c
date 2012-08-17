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
#include "crc8.h"



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


        res = ow_SelectDevice(pId);
        if ( res ) return res;

        res = ow_WriteByte(0xbe); // read scratchpad command
        if ( res ) return res;

        res = ow_ReadBuffer(pBuf,9); // 8 bytes + CRC
        if ( res ) return res;

        uint8_t i;
        uint8_t crc = 0;
        for (i=0;i<9;i++) {
            crc = ow_CRC(crc,pBuf[i]);
        }

        if ( crc ) {
        	return 255;
        }
    	if ( crc8( &pBuf[0], 9 ) ) {
    		return 254;
    	}


    return 0;
}

uint8_t   ow_tempConfig(OW_DEVICE_ID *pId, uint8_t nCfg)
{
    uint8_t res;
    uint8_t buf[9];


        res = ow_SelectDevice(pId);
        if ( res ) return res;

        buf[0] = 0x4e; // write scratchpad command + data for registers 2,3,4
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = nCfg;        

        res = ow_WriteBuffer( buf, 4 ); 
        if ( res ) return res;

        if ( pId != 0 ) { // read back if individual device
            res = ow_tempReadScratchpad(pId, buf);
            if ( res ) return res;

            if ( (buf[3] & 0x60) != (nCfg & 0x60) ) {
                return 255;
            }
        }    
    return 0;
}

uint8_t   ow_tempStartConversion(OW_DEVICE_ID *pId)
{
    uint8_t res;

        res = ow_SelectDevice(pId);
        if ( res ) return res;

        res = ow_AdjustPower();
        if ( res ) return res;

        res = ow_WriteByte(0x44); // CONVERT TEMP Command
        return res;
}

uint8_t   ow_tempReadTemperature(OW_DEVICE_ID *pId, int16_t *pTemp)
{
    uint8_t res;
    uint8_t buf[9];

        //res = ow_SelectDevice(pId);
        //if ( res ) return res;

        res = ow_tempReadScratchpad(pId,buf);
        if ( res ) return res;

        *pTemp = *((int16_t*)buf);

        // TODO: mask any unused LSB according to the resolution configuration in CFG register

    return 0;
}

uint8_t   ow_tempReadTemperature2(OW_DEVICE_ID *pId, int32_t *pTemp)
{
    uint8_t res;
    uint8_t buf[9];
    //res = ow_SelectDevice(pId);
    //if ( res ) return res;
    res = ow_tempReadScratchpad(pId,buf);
    if ( res ) return res;
/*
    DEBUG_PUTS("ScrPad");
    for (uint8_t i= 0;i<9;i++){
    	DEBUG_PUTHEX(buf[i]);
    }
    DEBUG_NEWLINE();
*/

    *pTemp = ow_temp_raw_to_maxres(pId->familyId,buf);
    return 0;
}

int32_t ow_temp_raw_to_maxres( uint8_t familycode, uint8_t sp[] )
{
	uint16_t measure;
	uint8_t  negative;
	int32_t  temperaturevalue;

	measure = sp[0] | (sp[1] << 8);
	//measure = 0xFF5E; // test -10.125
	//measure = 0xFE6F; // test -25.0625

	if( familycode == OW_DS18S20_FAMILY_CODE ) {   // 9 -> 12 bit if 18S20
		/* Extended measurements for DS18S20  */
		measure &= (uint16_t)0xfffe;   // Discard LSB, needed for later extended precicion calc
		measure <<= 3;                 // Convert to 12-bit, now degrees are in 1/16 degrees units
		measure += ( 16 - sp[6] ) - 4; // Add the compensation and remember to subtract 0.25 degree (4/16)
	}

	// check for negative
	if ( measure & 0x8000 )  {
		negative = 1;       // mark negative
		measure ^= 0xffff;  // convert to positive => (2 complement)++
		measure++;
	}
	else {
		negative = 0;
	}

	// clear undefined bits for DS18B20 != 12bit resolution
	if ( familycode == OW_DS18B20_FAMILY_CODE || familycode == OW_DS1822_FAMILY_CODE ) {
		switch( sp[OW_TEMP_CONF_REG] & OW_TEMP_RES_MASK ) {
		case OW_TEMP_9_BIT:
			measure &= ~(OW_TEMP_9_BIT_UNDF);
			break;
		case OW_TEMP_10_BIT:
			measure &= ~(OW_TEMP_10_BIT_UNDF);
			break;
		case OW_TEMP_11_BIT:
			measure &= ~(OW_TEMP_11_BIT_UNDF);
			break;
		default:
			// 12 bit - all bits valid
			break;
		}
	}

	temperaturevalue  = (measure >> 4);
	temperaturevalue *= 10000;
	temperaturevalue +=( measure & 0x000F ) * OW_TEMP_FRACCONV;

	if ( negative ) {
		temperaturevalue = -temperaturevalue;
	}

	return temperaturevalue;
}







#endif /* _FB_OW_TEMP_LIB_C */
/*********************************** EOF *********************************/
