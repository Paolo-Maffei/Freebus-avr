/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/**
* @file   delay.h
* @author Matthias Fechner
* @date   Sat Mar 22 00:18:54 2008
* 
* @brief  Functions to delay the execution.
* 
* 
*/

/** 
* delay loop for small accurate delays: 16-bit counter, 4 cycles/loop 
* 
* @param __count 
*/
static inline void _delayFourCycles(unsigned int __count)
{
    if ( __count == 0 )    
        __asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
    else
        __asm__ __volatile__ (
    	    "1: sbiw %0,1" "\n\t"                  
    	    "brne 1b"                              // 4 cycles/loop
    	    : "=w" (__count)
    	    : "0" (__count)
    	   );
}


/**
* delay for a minimum of microseconds
* the number of loops is calculated at compile-time from MCU clock frequency
*/
#define delay(us)  _delayFourCycles( ( ( 1*(F_CPU/4000) )*us)/1000 )

