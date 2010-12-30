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
* @file   fb_rotenc_lib.c
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for rotation encoder
*/
 
#include <avr/pgmspace.h>
 
  
// Dekodertabelle für wackeligen Rastpunkt
// halbe Auflösung
int8_t table[16] PROGMEM = {0,0,-1,0,0,0,0,1,1,0,0,0,0,-1,0,0};    
 
// Dekodertabelle für normale Drehgeber
// volle Auflösung
//int8_t table[16] PROGMEM = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};    

 

// requires high speed calling interval, not to miss any state changes

int8_t renc_Read( uint8_t portval )    
{
    static int8_t last=0;           
 
    last = (last << 2)  & 0x0F;
    last |= (portval & 0x03);
    
    return pgm_read_byte(&table[last]);
}
 
 
 

