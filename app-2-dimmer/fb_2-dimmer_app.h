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
* @file   fb_2-dimmer_app.h
* @author Uwe S.
* @date   16.05.2013
*
* @brief  Dimmer application based on the Gira Universal Dimmaktor 2fach kompakt REG  8 // 103200
* Manufacturer code is 0x0008 = GIRA
* Device type 0x3015
*/
#ifndef FB_DIMM2_APP_H_
#define FB_DIMM2_APP_H_


#ifdef _FB_APP_C
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
// #define BASIC_BRIGHTNESS_FACTOR              200          /**<  Stufe Grundhelligkeit*Faktor = Dimmwert f�r Helligkeit=1          */

#define APP_BASIC_BRIGHTNESS                 0x01C2     /**<  Grundhelligkeit Bit0-2 Kanal1; Bit4-6 Kanal2                      */
#define APP_LOCK_FUNCTION                    0x01C3     /**<  Sperrfunktion Ein/Aus, Polarit�t                                  */
#define APP_SWITCH_ON_BRIGHTNESS             0x01C4     /**<  Einschalthelligkeit Bit0-3 Kanal1; Bit4-7 Kanal2                  */
#define APP_BASE_DIMMING_STEP                0x01C6     /**<  Bit0-2 Kanal1; Bit4-6 Kanal2                                      */
#define APP_FACTOR_DIMMING_STEP_CH1          0x01C8     /**<  Faktor Dimmschritt Kanal1                                         */
#define APP_FACTOR_DIMMING_STEP_CH2          0x01C9     /**<  Faktor Dimmschritt Kanal2                                         */
#define APP_SWITCH_OFF_BRIGHTNESS_CH1        0x01CB     /**<  Ausschalthelligkeit Kanal1                                        */
#define APP_SWITCH_OFF_BRIGHTNESS_CH2        0x01CC     /**<  Ausschalthelligkeit Kanal2                                        */
#define APP_SWITCH_OFF_FUNCTION              0x01CE     /**<  Ausschaltfunktion ein/aus; Basis; Bit0-3 Kanal1; Bit4-7 Kanal2    */
#define APP_SWITCH_OFF_DELAY_FACTOR_CH1      0x01CF     /**<  Ausschaltfunktion Faktor Verz�gerung Kanal1                       */
#define APP_SWITCH_OFF_DELAY_FACTOR_CH2      0x01D0     /**<  Ausschaltfunktion Faktor Verz�gerung Kanal2                       */
#define APP_SOFT_ON_BASE                     0x01D2     /**<  Soft Ein Basis; Bit0-2 Kanal1; Bit4-6 Kanal2                      */
#define APP_SOFT_ON_FACTOR_CH1               0x01D3     /**<  Soft Ein Faktor Kanal1; 0=Funktion Aus                            */
#define APP_SOFT_ON_FACTOR_CH2               0x01D4     /**<  Soft Ein Faktor Kanal2; 0=Funktion Aus                            */
#define APP_SOFT_OFF_BASE                    0x01D6     /**<  Soft Ein Basis; Bit0-2 Kanal1; Bit4-6 Kanal2                      */
#define APP_SOFT_OFF_FACTOR_CH1              0x01D7     /**<  Soft Ein Faktor Kanal1; 0=Funktion Aus                            */
#define APP_SOFT_OFF_FACTOR_CH2              0x01D8     /**<  Soft Ein Faktor Kanal2; 0=Funktion Aus                            */
#define APP_TIMEDIMM_BASE                    0x01DA     /**<  Zeitdimmerfunktion ein/aus; Basis; Bit0-3 Kanal1; Bit4-7 Kanal2   */
#define APP_TIMEDIMM_FACTOR_CH1              0x01DC     /**<  Zeitdimmerfunktion Faktor Kanal1                                  */
#define APP_TIMEDIMM_FACTOR_CH2              0x01DD     /**<  Zeitdimmerfunktion Faktor Kanal2                                  */
#define APP_BRIGHTNESS_BLOCKING_FUNCTION_CH1 0x01DF     /**<  Helligkeit bei Beginn (Bit0-3); Ende (Bit4-7) der Sperrfunktion   */
#define APP_BRIGHTNESS_BLOCKING_FUNCTION_CH2 0x01E0     /**<  Helligkeit bei Beginn (Bit0-3); Ende (Bit4-7) der Sperrfunktion   */
#define APP_BUS_ON_BRIGHTNESS                0x01E2     /**<  Wert bei Busspannungswiederkehr 0-3 K1; 4-7 K2                    */
#define APP_CHANGE_LIGHT_SCENE               0x01E3     /**<  Lichtszene ver�nderbar Bit4 Kanal1; Bit5 Kanal 2                  */
#define APP_LIGHT_SCENE_CH1                  0x01E7     /**<  Lichtszenen Kanal1                                                */
#define APP_LIGHT_SCENE_CH2                  0x01E8     /**<  Lichtszenen Kanal2                                                */

#define OBJECT_SWITCH                        0          /**<  1Bit Objekt zum Ein/Aus schalten                                  */
#define OBJECT_DIMM                          2          /**<  4Bit Objekt zum Auf/Ab Dimmen                                     */
#define OBJECT_BRIGHTNESS                    4          /**<  1Byte Objekt absolute Helligkeit                                  */
#define OBJECT_RESPONSE_SWITCH               6          /**<  1Bit Objekt zur R�ckmeldung Ein/Aus                               */
#define OBJECT_RESPONSE_BRIGHTNESS           8          /**<  1Byte Objekt zur R�ckmeldung der Helligkeit                       */
#define OBJECT_BLOCKING_FUNCTION             10         /**<  1Bit Objekt zum Sperren des Dimmkanals                            */
#define OBJECT_LIGHT_SCENE                   12         /**<  1Byte Objekt zum Abrufen einer Lichtszene                         */
#define OBJECT_SHORT_CIRCUIT_ALARM           14         /**<  1Bit Objekt zur Meldung eines Kurzschlusses                       */
#define OBJECT_LOAD_FAILURE                  16         /**<  1Bit Objekt zur Meldung eines Lastausfalls                        */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
{ SOFTWARE_VERSION_NUMBER,      0x01 },                 /**< version number                               */
{ RUN_ERROR_STATUS,             0xFF },                 /**< Error-Status (FF=no error)                   */
{ COMMSTAB_ADDRESS,             0x8A },                 /**< COMMSTAB Pointer                             */
{ APPLICATION_PROGRAMM,         0x00 },                 /**< Port A Direction Bit Setting???              */

{ MANUFACTORER_ADR_HIGH,        0x00 },                 /**< Herstellercode 0x0008 = GIRA                 */
{ MANUFACTORER_ADR_LOW,         0x08 },                 /**<                                              */
{ DEVICE_NUMBER_HIGH,           0x30 },                 /**< device type 3015h                            */
{ DEVICE_NUMBER_LOW,            0x15 },                 /**<                                              */

{ 0xFF,                         0xFF }                  /**< END-sign; do not change                      */
};

const struct FBAppInfo AppInfo PROGMEM = {
	.FBApiVersion = 0x01,
	.pParam = defaultParam,
};


/**************************************************************************
* DECLARATIONS
**************************************************************************/
#ifdef OUT0-10V
    /** Parameter Grundhelligkeit für 0-10V, 1-10V Ausgang
	    entspricht : 0,1V; 0,5V; 0,9V; 1,0V; 1,1V; 1,2V; 1,3V; 1,4V */ 
	const uint16_t BasicBrightness[] ={326,1632,2938,3264,3590,3917,4243,4570};
#else
    /** Parameter Grundhelligkeit für PWM8, PWM10, UART
	    entspricht : 1%, 2%, 3%, 4%, 5%, 8%, 10%; 12% */ 
	const uint16_t BasicBrightness[] ={326,653,979,1306,1632,2611,3264,3917};
#endif	


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/


#endif /* FB_DIMM2_APP_H_ */
/*********************************** EOF *********************************/
