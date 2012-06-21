/* $Id$ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
*  /_/   /_/ |_/_____/_____/_____/\____//____/
*
*  Copyright (c) 2012 Dirk Opfer (do13) <dirk@do13.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb-eibd.h
* @author Dirk Opfer
* @date   Mon May  21 09:19:42 2012
*
* @brief  Hardware specific options for eibd under linux
*
*
*
#ifndef _FB_EIBD_H
#define _FB_EIBD_H

#include <stdint.h>
#include <malloc.h>
#define DISABLE_IRQS
#define ENABLE_IRQS
// neither PROGMEM or PSTR are needed for plain gcc, just define them as null
#define PROGMEM
#define PSTR(s) (s)
typedef uint8_t prog_uint8_t;
#define pgm_read_byte(x)	        (*((char *)x))
#define pgm_read_byte_near(x)	(*((char *)x))
#define pgm_read_byte_far(x)	(*((char *)x))
#define pgm_read_word(x)    	(*((short *)x))
#define pgm_read_word_near(x)	(*((short *)x))
#define pgm_read_workd_far(x)	(*((short *)x))
#define pgm_read_ptr(x)			(*((size_t *)x))
#endif