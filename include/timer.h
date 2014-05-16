/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
*  /_/   /_/ |_/_____/_____/_____/\____//____/
*
*  Copyright (c) 2010 Dirk Opfer <dirk@do13.de>
*  Copyright (c) 2012 Matthias Fechner <idefix@fechner.net>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   timer.h
* @author Dirk Opfer, Matthias Fechner
* @date
*
* @brief  Generic timerfunctions
*
*
*/
#ifndef _AVR_TIMER_H
#define _AVR_TIMER_H

#ifdef  _AVR_TIMER_C
#define TIMER_EXT
#else
/** Set MSG_EXT to extern to make clear it is include from an external source like a lib */
#define TIMER_EXT    extern  ///< Header file not included from timer.c so we need to define functions as extern.
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb_hardware.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/
#if REVISION==1 || REVISION==2
/**
 * Calculates how many tics you must wait to reach x milliseconds
 *
 * 10 ms -> 1 TICKS
 * 100 ms -> 10 TICKS
 */
#define M2TICS(x) (x / 10)
#else
/**
 * Calculates how many tics we must wait to reach x milliseconds. The value must also a multiplication of 2.
 *
 * 2   ms -> 1 TICK
 * 10  ms -> 5 TICKS
 * 100 ms -> 50 TICKS
 */
static inline uint32_t M2TICS(uint32_t x) {
    if(x%2!=0) {
        // only multiple of 2 are allowed as the minimum step is 2ms
        printf("Wrong timer value, must be dividable by 2\r\n");
        Assert(false);
    }
    return (x/2);
}
#endif

/**
 * Calculates how many tics you must wait to reach x seconds
 *
 * 1 sec -> 1000 msec -> 100 TICKS
 */
#if REVISION==1 || REVISION==2
#define SEC2TICS(x) (x * 100)
#else
static inline uint32_t SEC2TICS(uint32_t x) {
    return(M2TICS(x*1000));
}
#endif

/**
 * Data type which should be used to store timer tics.
 */
typedef uint32_t timer_t;

/**
 * Checks if timer has reached its defined end.
 *
 * @return true if timer value is reached
 */
#define timed_out(a, b) ((int32_t)(a) - (int32_t)(b) > 0)

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
/**
 * Return the actual tick counter value of the generic timer interface.
 *
 * @return Current tick counter value
 */
#if REVISION==1 || REVISION==2
TIMER_EXT timer_t get_ticks(void);
#else
static inline timer_t get_ticks(void) {
    return (ast_read_counter_value(AST));
}
#endif

/**
 * Init the generic timer interface to get the internal tick every 10ms increased.
 *
 * @todo Optimize for other frequencies then 8MHz.
 */
TIMER_EXT void timer_init(void);

/**
 * Return a reference to a generic timer to meassure time.
 * The function check_timeout can be used to see if the end of the timer is reached.
 *
 * @param t Reference to the timer
 * @param ticks How many ticks we wait, use M2TICS to calculate this value
 */
TIMER_EXT void alloc_timer(timer_t *t, timer_t ticks);

/**
 * Check if a given timer has reached its defined end.
 *
 * @param t
 *
 * @return Return true if the timer has reached its defined end
 */
#if REVISION==1 || REVISION==2
TIMER_EXT uint8_t check_timeout(timer_t *t);
#else
static inline uint8_t check_timeout(timer_t *t) {
    return timed_out(get_ticks(), *t);
}
#endif

#endif
