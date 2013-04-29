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
* @file   fb_in8_app.h
* @author Matthias Fechner, Christian Bode
* @date   Sat Jan 05 17:58:58 2008
*
* @brief  The application for 8 binary inputs
* Manufactorer code is 0x04 = Jung\n
* Device type (2118) 0x7054 Ordernumber: 2118 REG\n
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
#include "fb_lib.h"
#include "fb_app.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/
/* application spez. speicheradressen */
#define APP_MSG_RATE_LIM            0x01D3   // Telegrammratenbegrenzung, <30 ist inaktive
#define APP_POWERONDELAY_BASE       0x01FE   // Verzoegerung bei PowerON, Bit XXXX xxxx
#define APP_POWERONDELAY_FACTOR     0x01D4   // Verzoegerung bei PowerON
#define APP_DEBOUNCE_FACTOR         0x01D2   // Entprellzeit, Faktor * 0,5 ms

#define APP_PORTFUNCTION            0x01CE   // port function,
                                             // 0 ohne Funktion, 1 schalten, 2 dimmen, 3 Jalousie
                                             // 4 Wertgeber, 9 Impulszaehler, 10 Schaltzaehler
                                             // 0x01CE xxxx XXXX  Eingang 1
                                             //        XXXX xxxx  Eingang 2
                                             // 0x01CF xxxx XXXX  Eingang 3
                                             //        XXXX xxxx  Eingang 4
                                             // 0x01D0 xxxx XXXX  Eingang 5
                                             //        XXXX xxxx  Eingang 6
                                             // 0x01D1 xxxx XXXX  Eingang 7
                                             //        XXXX xxxx  Eingang 8

                                             // Aufbau fuer die Port 1 ... 8 ist identsich
                                             // 0x01D5 Basisadresse Eingang 1
                                             // 0x01D9 Basisadresse Eingang 2
                                             // 0x01DD Basisadresse Eingang 3
                                             // 0x01E1 Basisadresse Eingang 4
                                             // 0x01E5 Basisadresse Eingang 5
                                             // 0x01E9 Basisadresse Eingang 6
                                             // 0x01ED Basisadresse Eingang 7
                                             // 0x01F1 Basisadresse Eingang 8
/* Funktion Schalten */
#define APP_PORTFUNC_CONFIG         0x01D5   // Verhalten bei Power ON, Bit XXxx xxxx
                                             // 0 keine, 1 EIN, 2 Aus, 3 Portwert
                                             // Verhalten bei Beginn einer Sperrung, Bit xxXX xxxx
                                             // 00 keine, 01 EIN, 10 Aus, 11 UM
                                             // zyklisch senden, Bit xxxx XXxx
                                             // 0 keine, 1 nur EIN, 2 nur Aus, 3 EIn & AUS
                                             // Sperren freigeben, Bit xxxx xxXX
                                             // 00 inaktiv, 01 sperren bei 1, 10 sperren bei 0
#define APP_PORTFUNC_CYCLIC_FACTOR  0x01D6   // Zeitfaktor für zyklisches senden, Bit xXXX XXXX
#define APP_PORTFUNC_EDGEFUNC       0x01D7   // edge Function
                                             //        xxxx xxXX  falling edge object x.1
                                             //        xxxx XXxx  rising edge object x.1
                                             //        xxXX xxxx  falling edge object x.2
                                             //        XXxx xxxx  rising edge object x.2
#define APP_PORTFUNC_CONFIG2        0x01D8   // Verhalten bei Ende einer Sperrung, Bit xxxx xxXX
                                             // 00 keine, 01 EIN, 10 Aus, 11 aktueller Wert
#define APP_PORTFUNC_CYCLIC_BASE1   0x01F6   // Zeitbasis für zyklisches senden
                                             // 0x01F6 XXXX xxxx  object 1.1
                                             // 0x01F7 xxxx XXXX  object 2.1
                                             //        XXXX xxxx  object 3.1
                                             // 0x01F8 xxxx XXXX  object 4.1
                                             //        XXXX xxxx  object 5.1
                                             // 0x01F9 xxxx XXXX  object 6.1
                                             //        XXXX xxxx  object 7.1
                                             // 0x01FA xxxx XXXX  object 8.1
#define APP_PORTFUNC_CYCLIC_BASE2   0x01FA   // Zeitbasis für zyklisches senden
                                             // 0x01FA XXXX xxxx  object 1.2
                                             // 0x01FB xxxx XXXX  object 2.2
                                             //        XXXX xxxx  object 3.2
                                             // 0x01FC xxxx XXXX  object 4.2
                                             //        XXXX xxxx  object 5.2
                                             // 0x01FD xxxx XXXX  object 6.2
                                             //        XXXX xxxx  object 7.2
                                             // 0x01FE xxxx XXXX  object 8.2

/* Funktion Jalousie */
#define APP_PORTFUNC_JALOCONFIG     0x01D5   // Verhalten bei Power ON, Bit XXxx xxxx
                                             // 00 keine, 01 AB, 10 Auf
                                             // Verhalten bei Beginn einer Sperrung, Bit xxXX xxxx
                                             // 00 keine, 01 AB, 10 AUF, 11 UM
                                             // Sperren freigeben, Bit xxxx xxXX
                                             // 00 inaktiv, 01 sperren bei 1, 10 sperren bei 0
#define APP_PORTFUNC_T1_FAKTOR      0x01D6   // Faktor fuer T1, Bit xXXX XXXX
#define APP_PORTFUNC_T2_FAKTOR      0x01D7   // faktor fuer T2, Bit xXXX XXXX
#define APP_PORTFUNC_JALOCONFIG2    0x01D8   // Bedienkonzept, Bit xxxx Xxxx
                                             // 0 kurz-lang-kurz, 1 lang-kurz
                                             // Reaktion auf steigende Flanke, Bit xxXX xxxx
                                             // 00 keine, 01 AUF, 10 AB, 11 UM
#define APP_PORTFUNC_T1_BASIS       0x01F6   // Zeitbasis fuer T1
                                             // 0x01F6 XXXX xxxx  Eingang 1
                                             // 0x01F7 xxxx XXXX  Eingang 2
                                             //        XXXX xxxx  Eingang 3
                                             // 0x01F8 xxxx XXXX  Eingang 4
                                             //        XXXX xxxx  Eingang 5
                                             // 0x01F9 xxxx XXXX  Eingang 6
                                             //        XXXX xxxx  Eingang 7
                                             // 0x01FA xxxx XXXX  Eingang 8
#define APP_PORTFUNC_T2_BASIS       0x01FA   //  Zeitbasis fuer T2
                                             // 0x01FA XXXX xxxx  Eingang 1
                                             // 0x01FB xxxx XXXX  Eingang 2
                                             //        XXXX xxxx  Eingang 3
                                             // 0x01FC xxxx XXXX  Eingang 4
                                             //        XXXX xxxx  Eingang 5
                                             // 0x01FD xxxx XXXX  Eingang 6
                                             //        XXXX xxxx  Eingang 7
                                             // 0x01FE xxxx XXXX  Eingang 8


#define MOVE_UP         0x01
#define MOVE_DOWN       0x02
#define MOVE_MASK       MOVE_UP | MOVE_DOWN
#define STEP            0x04


/**************************************************************************
* DECLARATIONS
**************************************************************************/
/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
   { SOFTWARE_VERSION_NUMBER, 0x02 },    /**< Versionsnummer                               */
   { RUN_ERROR_STATUS,        0xFF },    /**< Error-Status (FF=no error)                   */
   { COMMSTAB_ADDRESS,        0x84 },    /**< COMMSTAB Pointer                             */
   { APPLICATION_PROGRAMM,    0x00 },    /**< Port A Direction Bit Setting???              */

   { 0x0000,                  0x00 },    /**< default is off                               */

   { MANUFACTORER_ADR_HIGH,   0x00 },    /**< Herstellercode 0x0004 = Jung                 */
   { MANUFACTORER_ADR_LOW,    0x04 },    /**< Herstellercode 0x0004 = Jung                 */
   { DEVICE_NUMBER_HIGH,      0x70 },    /**< device type (2118) 7054h                      */
   { DEVICE_NUMBER_LOW,       0x54 },

   { 0xFF,                    0xFF }     /**< END-sign; do not change                      */
};

const struct FBAppInfo AppInfo PROGMEM = {
    .FBApiVersion = 0x01,
    .pParam = defaultParam,
};


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/


#endif /* _FB_IN8_APP_H */
/*********************************** EOF *********************************/
