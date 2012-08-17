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
* @file   fb_relais_app.h
* @author Matthias Fechner, Christian Bode
* @date   Sat Jan 05 17:58:58 2008
*
* @brief  The relais application to switch 8 relais
* Manufactorer code is 0x04 = Jung\n
* Device type (2038.10) 0x2060 Ordernumber: 2138.10REG\n
*/
#ifndef _FB_RELAIS_APP_H
#define _FB_RELAIS_APP_H

#ifdef _FB_RELAIS_APP_C
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
/** PWM duty cycle. 0 = 0%, 255 = 100% */
#define PWM_SETPOINT    0x55   /* 33% duty cycle */
/** How long we hold the relais at 100% before we enable PWM again */
#define PWM_DELAY_TIME               3*M2TICS(130)      /* 3 * 130ms */

#define APP_SPECIAL_FUNC_OBJ_1_2     0x01D8
#define APP_SPECIAL_FUNC_OBJ_3_4     0x01D9
#define APP_DELAY_FACTOR_ON          0x01DA
#define APP_DELAY_FACTOR_OFF         0x01E2
#define APP_DELAY_ACTIVE             0x01EA //< 0x00=no timer active, 0x01=timer on object 1 active, 0x03=timer on object 1+2 active
#define APP_DELAY_ACTION             0x01EB

#define APP_SPECIAL_CONNECT          0x01D8 // from 0x01D8-0x01D9
#define APP_SPECIAL_FUNC_MODE        0x01ED
#define APP_SPECIAL_LOGIC_MODE       0x01EE
#define APP_SPECIAL_FUNCTION1        0x01EF
#define APP_SPECIAL_FUNCTION2        0x01F0
#define APP_SPECIAL_POLARITY         0x01F1

#define APP_CLOSER_MODE              0x01F2
#define APP_REPORT_BACK_INVERT       0x01F3
#define APP_RESTORE_AFTER_PL         0x01F6 // 0x01F6-0x01F7 Verhalten nach Spannungswiederkehr
#define APP_RESTORE_AFTER_PL_HIGH    0x01F7
#define APP_DELAY_BASE               0x01F9

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
{ SOFTWARE_VERSION_NUMBER,      0x01 },    /**< version number                               */
{ APPLICATION_RUN_STATUS,       0xFF },    /**< Run-Status (00=stop FF=run)                  */
{ COMMSTAB_ADDRESS,             0x9A },    /**< COMMSTAB Pointer                             */
{ APPLICATION_PROGRAMM,         0x00 },    /**< Port A Direction Bit Setting???              */

{ 0x0000,                       0x00 },    /**< default is off                               */
{ APP_DELAY_ACTIVE,             0x00 },    /**< no timer active                              */
{ APP_CLOSER_MODE,              0x00 },    /**< closer mode for all relais                   */
{ APP_RESTORE_AFTER_PL,         0x55 },    /**< don't save status at power loss (number 1-4) */
{ APP_RESTORE_AFTER_PL_HIGH,    0x55 },    /**< don't save status at power loss (number 5-8) */

{ MANUFACTORER_ADR_HIGH,        0x00 },    /**< Herstellercode 0x0004 = Jung                 */
{ MANUFACTORER_ADR_LOW,         0x04 },    /**< Herstellercode 0x0004 = Jung                 */
{ DEVICE_NUMBER_HIGH,           0x20 },    /**< device type (2038.10) 2060h                  */
{ DEVICE_NUMBER_LOW,            0x60 },    /**<                                              */

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


#endif /* _FB_RELAIS_APP_H */
/*********************************** EOF *********************************/
