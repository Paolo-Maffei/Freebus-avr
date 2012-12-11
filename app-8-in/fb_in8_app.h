/* $Id: fb_in8_app.h 624 2008-10-17 08:03:57Z idefix $ */
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
* @file   fb_in8_app.h
* @author Matthias Fechner, Christian Bode
* @date   Sat Jan 05 17:58:58 2008
* 
* @brief  The application for 8 binary inputs
* Manufactorer code is 0x04 = Jung\n
* Device type (2118) 0x???? Ordernumber: 2118.??REG\n
*/
#ifndef _FB_IN8_APP_H
#define _FB_IN8_APP_H

#ifdef _FB_IN8_APP_C
#define APP_EXT
#else
/** Set APP_EXT to extern to make clear it is include from an external source like a lib */
#define APP_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb_prot.h"
#include "fb_app.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/
/* application parameters */
#define PORTFUNCTION_12     0x01CE  ///< @todo add documentation
#define PORTFUNCTION_34     0x01CF  ///< @todo add documentation
#define PORTFUNCTION_56     0x01D0  ///< @todo add documentation
#define PORTFUNCTION_78     0x01D1  ///< @todo add documentation
#define DEBOUNCE_FACTOR     0x01D2  ///< @todo add documentation 
#define POWERONDELAY_FACTOR 0x01D4  ///< @todo add documentation
/* Funktion Schalten */
#define PORTFUNC_BASEADR    0x01D5  ///< @todo add documentation
#define PORTFUNC_EDGEFUNC   0x01D7  ///< @todo add documentation
/* Funktion Jalousie */
#define PORTFUNC_T1_FAKTOR  0x01D6  ///< @todo add documentation
#define PORTFUNC_T2_FAKTOR  0x01D7  ///< @todo add documentation
#define PORTFUNC_JALOMODE   0x01D8  ///< @todo add documentation
#define PORTFUNC_T1_BASIS   0x01F6  ///< @todo add documentation
#define PORTFUNC_T2_BASIS   0x01FA  ///< @todo add documentation

#define POWERONDELAY_BASE   0x01FE  ///< @todo add documentation

#define OBJ_SIZE            8       ///< @todo add documentation


/* EIB Device Parameter Values */

#define EIB_PAR_UP          0U     /**< Jalousie Actor value for UP */
#define EIB_PAR_DOWN        1U     /**< Jalousie Actor value for DOWN */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
{ MANUFACTORER_ADR_HIGH,        0x00 },    /**< Herstellercode 0x0004 = Jung                 */
{ MANUFACTORER_ADR_LOW,         0x04 },    /**< Herstellercode 0x0004 = Jung                 */
{ DEVICE_NUMBER_HIGH,           0x70 },    /**< Ger�te Typ (2118) 7054h                      */
{ DEVICE_NUMBER_LOW,            0x54 },
{ SOFTWARE_VERSION_NUMBER,      0x02 },    /**< Versionsnummer                               */
{ APPLICATION_RUN_STATUS,       0xFF },    /**< Run-Status (00=stop FF=run)                  */
{ COMMSTAB_ADDRESS,             0x84 },    /**< COMMSTAB Pointer                             */
{ APPLICATION_PROGRAMM,         0x00 },    /**< Port A Direction Bit Setting???              */

{ PA_ADDRESS_HIGH,              0x11 },    /**< default address is 1.1.52                    */
{ PA_ADDRESS_LOW,               0x34 },    /**<                                              */

{ 0xFF,                         0xFF }     /**< END-sign; do not change                      */
};

const struct FBAppInfo AppInfo PROGMEM = {
    .FBApiVersion = 0x01,
    .pParam = defaultParam,
};

/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/


#endif /* _FB_IN8_APP_H */
/*********************************** EOF *********************************/
