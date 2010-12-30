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
* @file   fb_ow_lib.h
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for one-wire usage via DS2482
*/
#ifndef _FB_OW_LIB_H
#define _FB_OW_LIB_H


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include <avr/io.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define OW_FAMILY_TEMP 0x28

#define OW_ERR_SD    -2    // short detected
#define OW_ERR_PPD   -3    // not device presence pulse detected
#define OW_ERR_CRC   -4    // crc error
#define OW_ERR_BUSY  -3    // busy timeout, command did not finish in reasonable time
#define OW_ERR_CFG   -5    // read back configuration byte did not match
#define OW_ERR_LAST  -6    // no more devices found at search
 

/**************************************************************************
* DECLARATIONS
**************************************************************************/

typedef struct {
    uint8_t familyId;
    uint8_t serialNr[6];
    uint8_t crc;
} OW_DEVICE_ID;

typedef struct {
    uint8_t         lastDiscrepancy;
    uint8_t         lastDevice;
    uint8_t         lastFamilyDiscrepancy;
    OW_DEVICE_ID    idDevice;
} OW_SEARCH;


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

uint8_t ow_Init();
uint8_t ow_Reset();
uint8_t ow_Config(uint8_t nConfig);
uint8_t ow_StrongPullup();
uint8_t ow_GetPower(uint8_t *pPwr);
uint8_t ow_AdjustPower();

uint8_t ow_ResetDevices(void);
uint8_t ow_TouchBit(uint8_t nBitIn, uint8_t *pBitOut);
uint8_t ow_ReadBit(uint8_t *pBitOut);
uint8_t ow_WriteBit(uint8_t nBitIn);
uint8_t ow_WriteByte(uint8_t nByte);
uint8_t ow_ReadByte(uint8_t *pByte);
uint8_t ow_WriteBuffer(uint8_t *pByte, uint8_t nLen);
uint8_t ow_ReadBuffer(uint8_t *pByte, uint8_t nLen);

void    ow_SearchInit( OW_SEARCH *pSearch );
uint8_t ow_Search( OW_SEARCH *pSearch );

uint8_t ow_SelectDevice( OW_DEVICE_ID * pId );
uint8_t ow_CRC(uint8_t crc, uint8_t nVal);



#endif /* _FB_OW_LIB_H */
/*********************************** EOF *********************************/
