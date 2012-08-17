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
* @file   fb_i2c_lib.c
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for I2C usage
*/
#ifndef _FB_I2C_LIB_C
#define _FB_I2C_LIB_C


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_hal.h"
#include "fb_i2c_lib.h"


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define I2C_PORT     PORTC      /**< output port (byte) of I2C bus  */
#define I2C_DDR      DDRC       /**< data direction byte of I2C bus */
#define I2C_SDA      4          /**< I2C data      */
#define I2C_SCL      5	        /**< I2C clock     */

#define WAIT_TIMEOUT 65000

#define I2C_WAIT        { uint16_t tmo = WAIT_TIMEOUT; while (!(TWCR & (1<<TWINT)) && --tmo); if (tmo==0) { DEBUG_PUTS(" W-TWCR:"); DEBUG_PUTHEX(TWCR); stat = -1; break; } }
#define I2C_WAIT_STOP   { uint16_t tmo = WAIT_TIMEOUT; while ( (TWCR & (1<<TWSTO)) && --tmo); if (tmo==0) { DEBUG_PUTS(" S-TWCR:"); DEBUG_PUTHEX(TWCR); i2c_Init(); break; } }

#define I2C_XFER        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA)
#define I2C_XFER_NACK   TWCR = (1<<TWINT) | (1<<TWEN)
#define I2C_START       TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN)
#define I2C_STOP        TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN)


#define I2C_SLA(x,a)    { TWDR = (((x)<<1)|(a)); I2C_XFER; }


#define I2C_STATUS      (TWSR & 0xF8)



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

void i2c_Init()
{
    TWCR = 0; // disable TWI - will be enabled at first access - to recover from lockup state

    PRR = PRR & ~PRTWI; // disable power reduction
    TWSR = 0; // pre-scale factor 1
    TWBR = 32; // 100kHz SCL : 8MHz / ( 16 + 2*TWBR*TWSR )

    I2C_DDR  = I2C_DDR & ~((1<<I2C_SDA)|(1<<I2C_SCL)); // set ports to input
    I2C_PORT = I2C_PORT | ((1<<I2C_SDA)|(1<<I2C_SCL)); // enable pullup
}


uint8_t i2c_start(uint8_t nAddress, uint8_t nRW)
{
    uint8_t stat = 0;

    do {

        I2C_WAIT_STOP;

        I2C_START;
        I2C_WAIT;
        stat = I2C_STATUS;
        if ( stat != TW_START ) 
            break;

        I2C_SLA(nAddress,nRW);
        I2C_WAIT;
        stat = I2C_STATUS;
        if ( !((stat == TW_MT_SLA_ACK && nRW == TW_WRITE) || (stat == TW_MR_SLA_ACK && nRW == TW_READ)) ) 
            break;

        stat = 0;

    } while(0);

    return stat;

}

uint8_t i2c_rep_start(uint8_t nAddress, uint8_t nRW)
{
    uint8_t stat = 0;

    do {
        I2C_START;
        I2C_WAIT;
        stat = I2C_STATUS;
        if ( stat != TW_REP_START ) 
            break;

        I2C_SLA(nAddress,nRW);
        I2C_WAIT;
        stat = I2C_STATUS;
        if ( !((stat == TW_MT_SLA_ACK && nRW == TW_WRITE) || (stat == TW_MR_SLA_ACK && nRW == TW_READ)) ) 
            break;

        stat = 0;

    } while(0);

    return stat;

}

uint8_t i2c_write(uint8_t nByte)
{
    uint8_t stat = 0;

    do {

        TWDR = nByte;
        I2C_XFER;
        I2C_WAIT;
        stat = I2C_STATUS;
        if ( stat != TW_MT_DATA_NACK && stat != TW_MT_DATA_ACK ) 
            break;

        stat = 0;

    } while(0);

    return stat;
}

uint8_t i2c_read(uint8_t *pByte, uint8_t nAck)
{
    uint8_t stat = 0;

    do {

        if (nAck) 
            I2C_XFER;
        else
            I2C_XFER_NACK;  // receive byte and stop

        I2C_WAIT;
        stat = I2C_STATUS;
        if ( stat != TW_MR_DATA_ACK && stat != TW_MR_DATA_NACK ) 
            break;

        *pByte = TWDR;

        if ( stat == TW_MR_DATA_ACK || (!nAck && stat == TW_MR_DATA_NACK) )
            stat = 0;

    } while(0);

    return stat;
}


void i2c_stop()
{
    I2C_STOP;
}


uint8_t i2c_WriteByte(uint8_t nAddress, uint8_t nByte)
{
    uint8_t stat = 0;

    do {
        stat = i2c_start(nAddress, TW_WRITE);
        if ( stat ) 
            break;

        stat = i2c_write( nByte );
    
    } while(0);

    i2c_stop();

    return stat;
}

uint8_t i2c_ReadByte(uint8_t nAddress, uint8_t * pByte)
{
    uint8_t stat = 0;

    do {

        stat = i2c_start(nAddress, TW_READ);
        if ( stat ) 
            break;

        stat = i2c_read( pByte, 0 );
    
    } while(0);

    i2c_stop();

    return stat;
}


uint8_t i2c_TransactByte(uint8_t nAddress, uint8_t nByteIn, uint8_t * pByteOut)
{
    uint8_t stat = 0;

    do {

        stat = i2c_start( nAddress, TW_WRITE );
        if ( stat )
            break;

        stat = i2c_write( nByteIn );
        if ( stat ) 
            break;

        stat = i2c_rep_start( nAddress, TW_READ );
        if ( stat )
            break;

        stat = i2c_read( pByteOut,0 );

    } while (0);

    i2c_stop();

    return stat;
}

uint8_t i2c_WriteBuffer(uint8_t nAddress, uint8_t *pByte, uint8_t nLen)
{
    uint8_t stat = 0;

    stat = i2c_start( nAddress, TW_WRITE );
    while ( nLen && !stat) {

        stat = i2c_write( *pByte );
    }
    i2c_stop();

    return stat;

}

uint8_t i2c_ReadBuffer(uint8_t nAddress, uint8_t *pByte, uint8_t nBufSize, uint8_t *pLen)
{
    uint8_t stat = 0;

    *pLen = 0;

    stat = i2c_start( nAddress, TW_READ );

    while (!stat && nBufSize) {
        
        stat = i2c_read( pByte, nBufSize != 1 );

        if ( !stat || stat == TW_MR_DATA_NACK) {
            nBufSize--;
            pByte++;
            (*pLen)++;
            stat = 0;
        }
    }
    i2c_stop();

    return stat;
}

uint8_t i2c_WriteByteReadBuffer(uint8_t nAddress, uint8_t nByteIn, uint8_t *pByteOut, uint8_t nBufSize, uint8_t *pLen)
{
    uint8_t stat = 0;

    *pLen = 0;

    do {
        stat = i2c_start( nAddress, TW_WRITE );
        if ( stat ) 
            break;

        stat = i2c_write( nByteIn );
        if ( stat ) 
            break;

        stat = i2c_rep_start( nAddress, TW_READ );

        while (!stat && nBufSize) {
        
            stat = i2c_read( pByteOut, nBufSize != 1 );

            if ( !stat || stat == TW_MR_DATA_NACK) {
                nBufSize--;
                pByteOut++;
                (*pLen)++;
                stat = 0;
            }
        }
    } while(0);

    i2c_stop();

    return stat;
}

uint8_t i2c_WriteBufferReadByte(uint8_t nAddress, uint8_t *pByteIn, uint8_t nLen, uint8_t *pByteOut)
{
    uint8_t stat = 0;

    do {

        stat = i2c_start( nAddress, TW_WRITE );
        while ( nLen && !stat) {

            stat = i2c_write( *pByteIn );
            nLen--;
            pByteIn++;
        }

        if ( stat )
            break;

        stat = i2c_rep_start( nAddress, TW_READ );
        if ( stat )
            break;

        stat = i2c_read( pByteOut,0 );

    } while(0);

    i2c_stop();

    return stat;
}



#endif /* _FB_I2C_LIB_C */
/*********************************** EOF *********************************/
