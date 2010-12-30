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
* @file   fb_ow_lib.c
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for one-wire usage via DS2482
*/
#ifndef _FB_OW_LIB_C
#define _FB_OW_LIB_C


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_hal.h"
#include "fb_i2c_lib.h"
#include "fb_ow_lib.h"


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define I2C_ADDR_OW  0x18

#define OW_STAT_1WB  0x01
#define OW_STAT_PPD  0x02
#define OW_STAT_SD   0x04
#define OW_STAT_LL   0x08
#define OW_STAT_RST  0x10
#define OW_STAT_SBR  0x20
#define OW_STAT_TSB  0x40
#define OW_STAT_DIR  0x80

#define OW_CONF_APU  0x01
#define OW_CONF_SPU  0x04
#define OW_CONF_1WS  0x08

/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

/*************************************************************************
* GLOBALS
**************************************************************************/

static uint8_t g_Pwr = -1;

/**************************************************************************
* IMPLEMENTATION
**************************************************************************/


//--------------------------------------------------------------------------
// Perform a device reset on the DS2482
//
//
uint8_t ow_Reset()
{
    uint8_t stat;
    uint8_t res;

   // Device Reset
   //   S AD,0 [A] DRST [A] Sr AD,1 [A] [SS] A\ P
   //  [] indicates from slave
   //  SS status byte to read to verify state

    res = i2c_TransactByte( I2C_ADDR_OW, 0xf0, &stat );
    if ( res ) 
        return res;

    if ( (stat & ~OW_STAT_LL) == OW_STAT_RST )
        return 0;
    
    return stat | OW_STAT_RST; // set reset bit to make it non-null in case the register was 0
}

//--------------------------------------------------------------------------
// Write the configuration register in the DS2482. The configuration
// options are provided in the lower nibble of the provided config byte.
// The uppper nibble in bitwise inverted when written to the DS2482.
//
//
uint8_t ow_Config(uint8_t nConfig)
{
    uint8_t cfg[2];
    uint8_t res;
    uint8_t stat;

   // Write configuration (Case A)
   //   S AD,0 [A] WCFG [A] CF [A] Sr AD,1 [A] [CF] A\ P
   //  [] indicates from slave
   //  CF configuration byte to write

    cfg[0] = 0xd2; // write config command;
    cfg[1] = (nConfig & 0x0f) | ((nConfig<<4) ^ 0xf0);

    res = i2c_WriteBufferReadByte( I2C_ADDR_OW, cfg, 2, &stat );

/*
    DEBUG_PUTS_BLOCKING(" Config(");
    DEBUG_PUTHEX_BLOCKING(res);
    DEBUG_PUTHEX_BLOCKING(cfg[1]);
    DEBUG_PUTHEX_BLOCKING(stat);
    DEBUG_PUTS_BLOCKING(") ");
*/

    if ( !res && (nConfig & 0x0f) != stat ) {
        DEBUG_PUTS_BLOCKING(" Config:");
        DEBUG_PUTHEX_BLOCKING(stat);
        ow_Reset();
        res = OW_ERR_CFG;
    }

    return res;
}

//--------------------------------------------------------------------------
// Wait until Bus returns from Busy State.
//

uint8_t ow_Wait(uint8_t *pStat)
{
    uint8_t res;
    uint16_t cnt = 1000;

    do {
        res = i2c_read( pStat, TRUE );
    } while ( !res && (*pStat & OW_STAT_1WB) && --cnt );
    i2c_read(pStat,FALSE); // do a read with NACK, else the slave may block the bus with a 0 bit

    if ( !res && !cnt)
        res = OW_ERR_BUSY;

    return res;
}


//--------------------------------------------------------------------------
// Reset all of the devices on the 1-Wire Net and return the result.
//
// Returns: 0:  presence pulse(s) detected, device(s) reset
//          else: no presence pulses detected
//
uint8_t ow_ResetDevices(void)
{
    uint8_t res;
    uint8_t stat;

    do {
        res = i2c_start(I2C_ADDR_OW, TW_WRITE);
        if ( res ) 
            break;

        res = i2c_write( 0xb4 ); // reset command
        if ( res ) 
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_READ);
        if ( res ) 
            break;
        
        res = ow_Wait(&stat);


    } while (FALSE);

    i2c_stop();

    if ( res )
        ow_Reset();
    else if ( stat & OW_STAT_SD ) 
        res = OW_ERR_SD;
    else if ( !(stat & OW_STAT_PPD) )
        res = OW_ERR_PPD;


    return res;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and return the
// result 1 bit read from the 1-Wire Net. The parameter 'sendbit'
// least significant bit is used and the least significant bit
// of the result is the return bit.
//
// 'sendbit' - the least significant bit is the bit to send
//
//
uint8_t ow_TouchBit(uint8_t nBitIn, uint8_t *pBitOut)
{
    uint8_t res;
    uint8_t stat;

    do {
        res = i2c_start(I2C_ADDR_OW, TW_WRITE);
        if ( res ) 
            break;

        res = i2c_write( 0x87 ); // 1w bit command
        if ( res ) 
            break;

        res = i2c_write( nBitIn ? 0x80 : 0x00 ); // data
        if ( res ) 
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_READ);
        if ( res ) 
            break;
        
        res = ow_Wait(&stat);


    } while (0);

    i2c_stop();
    
    if ( res )
        ow_Reset();
    else if ( pBitOut != 0 ) {
        *pBitOut = (stat & OW_STAT_SBR) ? 1 : 0;
    }

    return res;
}

uint8_t ow_ReadBit(uint8_t *pBitOut)
{
    return ow_TouchBit(1,pBitOut);
}

uint8_t ow_WriteBit(uint8_t nBitIn)
{
    return ow_TouchBit(nBitIn,0);
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net are the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
//
uint8_t ow_WriteByte(uint8_t nByte)
{
    uint8_t res;
    uint8_t stat;

    do {
        res = i2c_start(I2C_ADDR_OW, TW_WRITE);
        if ( res ) 
            break;

        res = i2c_write( 0xa5 ); // write byte command
        if ( res ) 
            break;

        res = i2c_write( nByte ); // data
        if ( res ) 
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_READ);
        if ( res ) 
            break;
        
        res = ow_Wait(&stat);


    } while (0);

    i2c_stop();
    
    if ( res )
        ow_Reset();


    return res;
}


//--------------------------------------------------------------------------
// Send 8 bits of read communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.
//
//
uint8_t ow_ReadByte(uint8_t *pByte)
{
    uint8_t res;
    uint8_t stat;

    do {
        res = i2c_start(I2C_ADDR_OW, TW_WRITE);
        if ( res ) 
            break;

        res = i2c_write( 0x96 ); // read byte command
        if ( res ) 
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_READ);
        if ( res ) 
            break;
        
        res = ow_Wait(&stat);
        if ( res )
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_WRITE);
        if ( res ) 
            break;

        res = i2c_write( 0xe1 ); // set read pointer
        if ( res ) 
            break;

        res = i2c_write( 0xe1 ); // read data register
        if ( res ) 
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_READ);
        if ( res ) 
            break;

        res = i2c_read( pByte, FALSE ); 

    } while (0);

    i2c_stop();
    
    if ( res )
        ow_Reset();

    return res;
}

//--------------------------------------------------------------------------
// Use the DS2482 help command '1-Wire triplet' to perform one bit of a
//1-Wire search.
//This command does two read bits and one write bit. The write bit
// is either the default direction (all device have same bit) or in case of
// a discrepancy, the 'search_direction' parameter is used.
//
//
uint8_t ow_SearchTriplet(uint8_t nDir, uint8_t *pStat)
{
    uint8_t res;

    do {
        res = i2c_start(I2C_ADDR_OW, TW_WRITE);
        if ( res ) 
            break;

        res = i2c_write( 0x78 ); // triplet command
        if ( res ) 
            break;

        res = i2c_write( nDir ? 0x80 : 0x00 ); // data
        if ( res ) 
            break;

        res = i2c_rep_start(I2C_ADDR_OW, TW_READ);
        if ( res ) 
            break;
        
        res = ow_Wait(pStat);

    } while (0);

    i2c_stop();
    
    if ( res )
        ow_Reset();

    return res;
}

//--------------------------------------------------------------------------
// CRC table
// 
// 
//

static const uint8_t g_crcLookup[256] PROGMEM = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
 };


//--------------------------------------------------------------------------
// The 'OWSearch' function does a general search. This function
// continues from the previous search state. The search state
// can be reset by using the 'OWFirst' function.
// This function contains one parameter 'alarm_only'.
// When 'alarm_only' is TRUE (1) the find alarm command
// 0xEC is sent instead of the normal search command 0xF0.
// Using the find alarm command 0xEC will limit the search to only
// 1-Wire devices that are in an 'alarm' state.
//
//
void ow_SearchInit( OW_SEARCH *pSearch )
{
    pSearch->lastDiscrepancy = 0;
    pSearch->lastFamilyDiscrepancy = 0;
    pSearch->lastDevice = FALSE;
};

uint8_t ow_Search( OW_SEARCH *pSearch )
{
#define OW_SRCH_IDBYTE      ((currentBit-1)>>3)
#define OW_SRCH_BITMASK     (1<<((currentBit-1)&0x07))
    
    uint8_t currentBit = 1;
    uint8_t nDir;
    uint8_t stat;
    uint8_t idBit;
    uint8_t lastZeroBit = 0;
    uint8_t crc = 0;

    uint8_t res;    

    do {

        if ( pSearch->lastDevice ) {
            res = OW_ERR_LAST;
            break;
        }

        res = ow_ResetDevices();
        if ( res )
            break;


        res = ow_WriteByte( 0xF0 ); // issue search command
        if ( res )
            break;


        // loop to do the search
        do
        {
            // if this discrepancy is before the Last Discrepancy
            // on a previous next then pick the same as last time
            if (currentBit < pSearch->lastDiscrepancy)
            {
                nDir = ((uint8_t*)&pSearch->idDevice)[OW_SRCH_IDBYTE] & OW_SRCH_BITMASK;
            }
            else
            {
            // if equal to last pick 1, if not then pick 0
                nDir = pSearch->lastDiscrepancy == currentBit;
            }


            res = ow_SearchTriplet(nDir,&stat);
            if ( res )
                break;

            nDir    = stat & OW_STAT_DIR;
            idBit   = stat & (OW_STAT_SBR | OW_STAT_TSB);

#if 0
            DEBUG_PUTS_BLOCKING("stat(");
            DEBUG_PUTHEX_BLOCKING(stat);
            DEBUG_PUTS_BLOCKING("/");
            DEBUG_PUTHEX_BLOCKING(nDir);
            DEBUG_PUTS_BLOCKING("/");
            DEBUG_PUTHEX_BLOCKING(currentBit);
            DEBUG_PUTS_BLOCKING("/");
            DEBUG_PUTHEX_BLOCKING(OW_SRCH_IDBYTE);
            DEBUG_PUTS_BLOCKING(")");
#endif
            if ( idBit == (OW_STAT_SBR | OW_STAT_TSB) ) {
                res = OW_ERR_LAST;
                break; // no devices
            }

            if ( idBit == 0 && !nDir ) { // discrepancy
                lastZeroBit = currentBit;
                if (lastZeroBit < 9) // bit is within family range
                    pSearch->lastFamilyDiscrepancy = currentBit;
                
            }

            // set the bit as found by search
            if (nDir) 
                ((uint8_t*)&pSearch->idDevice)[OW_SRCH_IDBYTE] |= OW_SRCH_BITMASK;    
            else
                ((uint8_t*)&pSearch->idDevice)[OW_SRCH_IDBYTE] &= ~OW_SRCH_BITMASK;    

            // calculate crc when byte full
            if ( OW_SRCH_BITMASK == 0x80 ) {
                crc = ow_CRC(crc, ((uint8_t*)&pSearch->idDevice)[OW_SRCH_IDBYTE]);
            }

            // check next bit
            currentBit++;

        } while(OW_SRCH_IDBYTE < 8);  // loop until through all ROM bytes 0-7


        if ( res )
            break;

        if ( crc != 0 ) {
            DEBUG_PUTS_BLOCKING("crc:");
            DEBUG_PUTHEX_BLOCKING(crc);
            res = OW_ERR_CRC;
            break;
        }

        pSearch->lastDiscrepancy = lastZeroBit;

        if ( pSearch->lastDiscrepancy == 0 )
            pSearch->lastDevice = TRUE;

    } while(FALSE);

    return res;
}



uint8_t ow_Init()
{
    uint8_t res = ow_Reset();

    if ( !res )
        res = ow_Config( OW_CONF_APU );

    g_Pwr = -1;

    return res;
}

uint8_t ow_StrongPullup()
{
    return ow_Config( OW_CONF_APU | OW_CONF_SPU );
}

uint8_t   ow_SelectDevice(OW_DEVICE_ID *pId)
{
    uint8_t res;

//    DEBUG_PUTS_BLOCKING(" Select:");
    do {

        res = ow_ResetDevices();
        if ( res )
            break;

        if ( pId == 0 ) {
//            DEBUG_PUTC_BLOCKING('a');
            res = ow_WriteByte( 0xcc ); // skip ROM selection
        }
        else {
//            DEBUG_PUTC_BLOCKING('b');
            res = ow_WriteByte( 0x55 ); // select individual device

            if ( res ) 
                break;

//            DEBUG_PUTC_BLOCKING('c');
            res = ow_WriteBuffer((uint8_t*)pId,sizeof(OW_DEVICE_ID));
        }

    } while (FALSE);

//    DEBUG_PUTHEX_BLOCKING(res);

    return res;
}

uint8_t   ow_GetPower(uint8_t *pPwr)
{
    uint8_t res;

    do {

        res = ow_SelectDevice(0);
        if ( res )
            break;

//        DEBUG_PUTC_BLOCKING('x');
        res = ow_WriteByte( 0xb4 ); // read power supply
        if ( res )
            break;

//        DEBUG_PUTC_BLOCKING('y');
        res = ow_ReadBit(pPwr);
    
    } while(FALSE);

    return res;
}

uint8_t ow_AdjustPower()
{
    uint8_t res = 0;
    
//    DEBUG_PUTS_BLOCKING(" Adjust:");
    if ( g_Pwr == -1 ) {
//        DEBUG_PUTC_BLOCKING('a');
        uint8_t pwr;
        res = ow_GetPower( &pwr );
        if ( res ) 
            g_Pwr = pwr ? 0 : 1; // invert
    }

    if ( g_Pwr ) { 
//        DEBUG_PUTC_BLOCKING('b');
        res = ow_StrongPullup();
    }
//    DEBUG_PUTHEX_BLOCKING(res);

    return res;
}


uint8_t ow_WriteBuffer(uint8_t *pByte, uint8_t nLen)
{
    uint8_t res = 0;
    uint8_t i = 0;
    while ( !res && i<nLen) {
        res = ow_WriteByte( pByte[i] );
        i++;
    }


    return res;
};

uint8_t ow_ReadBuffer(uint8_t *pByte, uint8_t nLen)
{
    uint8_t res = 0;
    uint8_t i = 0;
    while ( !res && i<nLen) {
        res = ow_ReadByte( &pByte[i] );
        i++;
    }


    return res;
};

uint8_t ow_CRC(uint8_t crc, uint8_t nVal)
{
    return pgm_read_byte(&g_crcLookup[crc ^ nVal]);
}

#endif /* _FB_OW_LIB_C */
/*********************************** EOF *********************************/
