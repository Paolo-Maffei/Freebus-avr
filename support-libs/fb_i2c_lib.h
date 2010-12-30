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
* @file   fb_i2c_lib.h
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for I2C usage
*/
#ifndef _FB_I2C_LIB_H
#define _FB_I2C_LIB_H


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include <avr/io.h>
#include <util/twi.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/


/**************************************************************************
* DECLARATIONS
**************************************************************************/



/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

void        i2c_Init();
uint8_t     i2c_WriteByte(uint8_t nAddress, uint8_t nByte);
uint8_t     i2c_ReadByte(uint8_t nAddress, uint8_t * pByte);
uint8_t     i2c_WriteBuffer(uint8_t nAddress, uint8_t *pByte, uint8_t nLen);
uint8_t     i2c_ReadBuffer(uint8_t nAddress, uint8_t *pByte, uint8_t nBufSize, uint8_t *pLen);

uint8_t     i2c_TransactByte(uint8_t nAddress, uint8_t nByteIn, uint8_t * pByteOut);
uint8_t     i2c_WriteByteReadBuffer(uint8_t nAddress, uint8_t nByteIn, uint8_t *pByteOut, uint8_t nBufSize, uint8_t *pLen);
uint8_t     i2c_WriteBufferReadByte(uint8_t nAddress, uint8_t *pByteIn, uint8_t nLen, uint8_t *pByteOut);

uint8_t     i2c_start(uint8_t nAddress, uint8_t nRW);
uint8_t     i2c_rep_start(uint8_t nAddress, uint8_t nRW);
uint8_t     i2c_write(uint8_t nByte);
uint8_t     i2c_read(uint8_t *pByte, uint8_t nAck);
void        i2c_stop();

#endif /* _FB_I2C_LIB_H */
/*********************************** EOF *********************************/
