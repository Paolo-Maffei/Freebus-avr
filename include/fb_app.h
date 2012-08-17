/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
 *  /_/   /_/ |_/_____/_____/_____/\____//____/
 *
 *  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
 *  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/**
* @file   fb_app.h
* @author Matthias Fechner
* @date   Mon Oct 13 23:53:50 2008
*
* @brief  General application definition file.
*
*
*/
#ifndef _FB_APP_H
#define _FB_APP_H

#ifdef _FB_RELAIS_APP_C
#define FBAPP_EXT
#else
/** Set FBAPP_EXT to extern to make clear it is include from an external source like a lib */
#define FBAPP_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/


/**************************************************************************
* DEFINITIONS
**************************************************************************/
FBAPP_EXT const STRUCT_DEFPARAM defaultParam[] PROGMEM;
FBAPP_EXT const struct FBAppInfo AppInfo PROGMEM;


/**************************************************************************
* DECLARATIONS
**************************************************************************/
/* State used for new lib api */
#define SET_STATE(x) app_state |= (x)          ///< Defined the next state for the application
#define UNSET_STATE(x) app_state &= ~(x)  ///< disable state defined in x
#define IN_STATE(x) app_state & (x)      ///< Return true, if state x is active
//#define GET_STATES() app_state             ///< Get the current state the application is in
#define RESET_STATE() app_state = 0          ///< Clear all bits used in the state variable

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
FBAPP_EXT uint8_t restartApplication(void);


#endif /* _FB_APP_H */
/*********************************** EOF *********************************/
