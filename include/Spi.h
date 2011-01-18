/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright 2008 Simon Kueppers <simon.kueppers@web.de> http://klinkerstein.m-faq.de
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   Spi.h
* @author Simon Kueppers, Dirk Armbrust
* @date   Jan 13 2011
* 
* @brief  Defines and function-declarations regarding the Spi Code Module
*         which is used by the rf22 Code Module
* 
* 
*/

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

#define SPI_PORT    PORTB
#define SPI_PIN     PINB
#define SPI_DDR     DDRB
#define SS          PORTB2
#define MISO        PINB4
#define MOSI        PORTB3
#define SCK         PORTB5

void SpiInit( void );
void SpiWriteByte(uint8_t nByte);
void SpiWriteChunk(	const uint8_t* pChunk,
					uint16_t nBytes);
uint8_t SpiReadByte( void );
uint8_t SpiTransByte(uint8_t nByte);
void SpiReadChunk(	uint8_t* pChunk,
					uint16_t nBytes);
void SpiTransBurst( volatile uint8_t* pBuf, uint8_t addr, uint8_t nBytes, void (*callback)(void));
void SpiHoldCallbacks( void );
void SpiFireCallbacks( void );
 
/*
struct SpiBurstStruct {
  uint8_t *ptr;
  uint8_t n;
  uint8_t buf[64];  //must be power of 2
}
*/

#endif /*SPI_H_*/
