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
* @file   
* @author Matthias Fechner, Christian Bode
* @date   Sat Jan 05 17:58:58 2008
*
* @brief  
* Manufactorer code is
* Device type
*/
#ifndef _AIN10VNEWLIB_H
#define _AIN10VNEWLIB_H

#ifdef _AIN10VNEWLIB_C
#define APP_EXT
#else
/** Set APP_EXT to extern to make clear it is include from an external source like a lib */
#define APP_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb_lib.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/


/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
{ SOFTWARE_VERSION_NUMBER,      0x01 },    /**< version number                               */
{ RUN_ERROR_STATUS,             0xFF },    /**< Error-Status (FF=no error)                   */
{ APPLICATION_PROGRAMM,         0x00 },    /**< Port A Direction Bit Setting???              */
{ COMMSTAB_ADDRESS,             0x3A },    /**< COMMSTAB Pointer                             */
{ MANUFACTORER_ADR_HIGH,        0x00 },    /**< Herstellercode					             */
{ MANUFACTORER_ADR_LOW,         0x08 },    /**< Herstellercode						         */
{ DEVICE_NUMBER_HIGH,           0xB0 },    /**< device type					                 */
{ DEVICE_NUMBER_LOW,            0x03 },    /**<                                              */
{ 0xFF,                         0xFF }     /**< END-sign; do not change                      */
};

const struct FBAppInfo AppInfo PROGMEM = {
    .FBApiVersion = 0x01,
    .pParam = defaultParam,
};

#define RESOLUTION_FACTOR 4					/**< set internal resolution, 1=low, note maximum variable size */


/**************************************************************************
* DECLARATIONS
**************************************************************************/



/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/


#endif /* _AIN10VNEWLIB_H */
/*********************************** EOF *********************************/
