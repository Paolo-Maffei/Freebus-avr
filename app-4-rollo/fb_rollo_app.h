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
* @brief  The rollo application to switch 4 rollo
* Manufactorer code is 0x04 = Jung\n
* Device type (2038.10) 0x2070 Ordernumber: 2204REG HR\n
*/
#ifndef _FB_ROLLO_APP_H
#define _FB_ROLLO_APP_H

#ifdef _FB_ROLLO_APP_C
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
/** PWM duty cycle. 0 = 0%, 255 = 100% */
#define PWM_SETPOINT    0x55   /* 33% duty cycle */
/** How long we hold the relais at 100% before we enable PWM again */
#define PWM_DELAY_TIME               3*M2TICS(130)      /* 3 * 130ms */

/* application spez. speicheradressen */
#define APP_ROLLO_OPMODE            0x01E4   // Betriebsart: 4 Kanal oder 2x2 Kanal, Bit Xxxx xxxx
#define APP_ROLLO_SAFTY_TIMEOUT     0x01D9   // zyklische Ueberwachung der Sicherheit, Bit xxxx XXXX
                                             // 0000 keine, 0001 1min, 0010 2min, 0011 3min, ...
#define APP_ROLLO_AFTER_PL          0x01D9   // Verhalten nach Buswiederkehr, Bit XXxx xxxx
#define APP_ROLLO_LONGTIME_BASE     0x01FB   // Langzeitbasis, bitcodiert
                                             // 010 8ms, 011 130ms, 100 2,1sec, 101 33sec
                                             // 0x01FB xxxx xXXX  Ausgang 1
                                             //        xxXX Xxxx  Ausgang 2
                                             // 0x01FC xxxx xXXX  Ausgang 3
                                             //        xxXX Xxxx  Ausgang 4
#define APP_ROLLO_LONGTIME_FACTOR   0x01DA   // Langzeitfaktor,   Ausgang 1
                                             // 0x01DB            Ausgang 2
                                             // 0x01DC            Ausgang 3
                                             // 0x01DD            Ausgang 4
#define APP_ROLLO_SHORTTIME_BASE    0x01E8   // Kurzzeitbasis, bitcodiert
                                             // 010 8ms, 011 130ms, 100 2,1sec, 101 33sec
                                             // 0x01E8 xxxx xXXX  Ausgang 1
                                             //        xxXX Xxxx  Ausgang 2
                                             // 0x01E) xxxx xXXX  Ausgang 3
                                             //        xxXX Xxxx  Ausgang 4
#define APP_ROLLO_SHORTTIME_FACTOR  0x01EA   // Langzeitfaktor,   Ausgang 1
                                             // 0x01EB            Ausgang 2
                                             // 0x01EC            Ausgang 3
                                             // 0x01ED            Ausgang 4
#define APP_ROLLO_DIRECTION_DELAY   0x01EE   // Pause bei Richtungswechsel
                                             // 00 0,5sec, 01 1sec, 10 2sec, 11 5sec
                                             //        xxxx xxXX  Ausgang 1
                                             //        xxxx XXxx  Ausgang 2
                                             //        xxXX xxxx  Ausgang 3
                                             //        XXxx xxxx  Ausgang 4
#define APP_ROLLO_LONGTIME_MODE     0x01F2   // Zeit für Langzeitbetrieb, bitcodiert
                                             // 0 zeit+20%, 1 unendlich
                                             //        xxxX xxxx  Ausgang 1
                                             //        xxXx xxxx  Ausgang 2
                                             //        xXxx xxxx  Ausgang 3
                                             //        Xxxx xxxx  Ausgang 4
#define APP_ROLLO_SAFTY_POLARITY    0x01E2   // Polaritaet der Sicherheitsobjekte
                                             //        xXxx xxxx  Sicherheit 1
                                             //        Xxxx xxxx  Sicherheit 2
#define APP_ROLLO_SAFTY_PORT        0x01F1   // Zuordnung der Sicherheitsfunktionen, bitcodiert
                                             //        xxxx xxxX  sicherheit 1, Ausgang 1
                                             //        xxxx xxXx  sicherheit 1, Ausgang 2
                                             //        xxxx xXxx  sicherheit 1, Ausgang 3
                                             //        xxxx Xxxx  sicherheit 1, Ausgang 4
                                             //        xxxX xxxx  sicherheit 2, Ausgang 1
                                             //        xxXx xxxx  sicherheit 2, Ausgang 2
                                             //        xXxx xxxx  sicherheit 2, Ausgang 3
                                             //        Xxxx xxxx  sicherheit 2, Ausgang 4
#define APP_ROLLO_SAFTY_BEGINN      0x01F0   // Verhalten bei Beginn Sicherheit
                                             // 00 none 01 UP 10 DOWN
                                             //        xxxx xxXX  Ausgang 1
                                             //        xxxx XXxx  Ausgang 2
                                             //        xxXX xxxx  Ausgang 3
                                             //        XXxx xxxx  Ausgang 4
#define APP_ROLLO_SAFTY_END         0x01EF   // Verhalten bei Ende Sicherheit
                                             // 00 none 01 UP 10 DOWN
                                             //        xxxx xxXX  Ausgang 1
                                             //        xxxx XXxx  Ausgang 2
                                             //        xxXX xxxx  Ausgang 3
                                             //        XXxx xxxx  Ausgang 4


#define EIS7_UP         0
#define EIS7_DOWN       1
#define PORTMASK_UP     0x01
#define PORTMASK_DOWN   0x02
#define PORTMASK        0x03


/**************************************************************************
* DECLARATIONS
**************************************************************************/
/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
{ SOFTWARE_VERSION_NUMBER,      0x01 },    /**< version number                               */
{ RUN_ERROR_STATUS,             0xFF },    /**< Error-Status (FF=no error)                   */
{ COMMSTAB_ADDRESS,             0x9A },    /**< COMMSTAB Pointer                             */
{ APPLICATION_PROGRAMM,         0x00 },    /**< Port A Direction Bit Setting???              */

{ 0x0000,                       0x00 },    /**< default is off                               */

{ MANUFACTORER_ADR_HIGH,        0x00 },    /**< Herstellercode 0x0004 = Jung                 */
{ MANUFACTORER_ADR_LOW,         0x04 },    /**< Herstellercode 0x0004 = Jung                 */
{ DEVICE_NUMBER_HIGH,           0x20 },    /**< device type (2204 REG HR) 2070h                  */
{ DEVICE_NUMBER_LOW,            0x70 },    /**<                                              */

{ 0xFF,                         0xFF }     /**< END-sign; do not change                      */
};

const struct FBAppInfo AppInfo PROGMEM = {
    .FBApiVersion = 0x01,
    .pParam = defaultParam,
};


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/


#endif /* _FB_RELAIS_APP_H */
/*********************************** EOF *********************************/
