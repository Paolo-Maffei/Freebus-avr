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
* @file   fb_rotenc_lib.h
* @author Kent Filek
* @date   Tue Aug 31 17:00:00
* 
* @brief  library functions for rotation encoder
*/
#ifndef _FB_ROTENC_LIB_H
#define _FB_ROTENC_LIB_H


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include <avr/io.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/


/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

uint8_t     renc_Read( uint8_t portval );


#endif /* _FB_ROTENC_LIB_H */
/*********************************** EOF *********************************/
