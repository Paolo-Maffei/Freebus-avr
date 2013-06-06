/* $Id$ */
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
#define TIMER_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define M2TICS(x) (x / 10)
#define SEC2TICS(x) (x * 100)

// 10 ms -> 1 TICKS
// 100 ms -> 10 TICKS
// 1 sec -> 1000 msec -> 100 TICKS

typedef uint32_t timer_t;
#define timed_out(a, b) ((int32_t)(a) - (int32_t)(b) > 0)

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

TIMER_EXT timer_t get_ticks(void);
TIMER_EXT void timer_init(void);
TIMER_EXT void timer_close(void);
TIMER_EXT void alloc_timer(timer_t *t, timer_t ticks);
TIMER_EXT uint8_t check_timeout(timer_t *t);

#endif
