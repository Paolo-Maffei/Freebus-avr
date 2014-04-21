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
* @file   fb_rgb_dim_app.h
* @author Uwe S.
* @date   18.04.2014
*
* @brief  Dimmer application based on the  "Jung Steuereinheit 3fach 2193REG"
*         to control WS2812 LEDs
* Manufacturer code is 0x0004 = JUNG
* Device type is 0x3018 = Ordernumber: 2193REG
*
* To enable --
*
* This version is designed to be used with the new API.
*/

#ifndef FB_RGB_DIM_APP_H_
#define FB_RGB_DIM_APP_H_


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
// parameters
//#define APP_BASIC_BRIGHTNESS                 0x01C2     /**<  Grundhelligkeit Bit0-2 Kanal1; Bit4-6 Kanal2                      */
#define APP_LOCK_FUNCTION                    0x01C3     /**<  Sperrfunktion Ein/Aus, Polarit?t                                  */
#define APP_SWITCH_ON_BRIGHTNESS             0x01C4     /**<  Einschalthelligkeit Bit0-3 Kanal1; Bit4-7 Kanal2                  */
#define APP_BASE_DIMMING_STEP                0x01C6     /**<  Bit0-2 Kanal1; Bit4-6 Kanal2                                      */
#define APP_FACTOR_DIMMING_STEP_CH1          0x01C8     /**<  Faktor Dimmschritt Kanal1                                         */
#define APP_FACTOR_DIMMING_STEP_CH2          0x01C9     /**<  Faktor Dimmschritt Kanal2 , used for Number of LEDs                                        */
//#define APP_SWITCH_OFF_BRIGHTNESS_CH1        0x01CB     /**<  Ausschalthelligkeit Kanal1                                        */
//#define APP_SWITCH_OFF_BRIGHTNESS_CH2        0x01CC     /**<  Ausschalthelligkeit Kanal2                                        */
//#define APP_SWITCH_OFF_FUNCTION              0x01CE     /**<  Ausschaltfunktion ein/aus; Basis; Bit0-3 Kanal1; Bit4-7 Kanal2    */
//#define APP_SWITCH_OFF_DELAY_FACTOR_CH1      0x01CF     /**<  Ausschaltfunktion Faktor Verz?gerung Kanal1                       */
//#define APP_SWITCH_OFF_DELAY_FACTOR_CH2      0x01D0     /**<  Ausschaltfunktion Faktor Verz?gerung Kanal2                       */
#define APP_SOFT_ON_BASE                     0x01D2     /**<  Soft Ein Basis; Bit0-2 Kanal1; Bit4-6 Kanal2                      */
#define APP_SOFT_ON_FACTOR_CH1               0x01D3     /**<  Soft Ein Faktor Kanal1; 0=Funktion Aus                            */
//#define APP_SOFT_ON_FACTOR_CH2               0x01D4     /**<  Soft Ein Faktor Kanal2; 0=Funktion Aus                            */
#define APP_SOFT_OFF_BASE                    0x01D6     /**<  Soft Ein Basis; Bit0-2 Kanal1; Bit4-6 Kanal2                      */
#define APP_SOFT_OFF_FACTOR_CH1              0x01D7     /**<  Soft Ein Faktor Kanal1; 0=Funktion Aus                            */
//#define APP_SOFT_OFF_FACTOR_CH2              0x01D8     /**<  Soft Ein Faktor Kanal2; 0=Funktion Aus                            */
//#define APP_TIMEDIMM_BASE                    0x01DA     /**<  Zeitdimmerfunktion ein/aus; Basis; Bit0-3 Kanal1; Bit4-7 Kanal2   */
//#define APP_TIMEDIMM_FACTOR_CH1              0x01DC     /**<  Zeitdimmerfunktion Faktor Kanal1                                  */
//#define APP_TIMEDIMM_FACTOR_CH2              0x01DD     /**<  Zeitdimmerfunktion Faktor Kanal2                                  */
#define APP_BRIGHTNESS_BLOCKING_FUNCTION_CH1 0x01DF     /**<  Helligkeit bei Beginn (Bit0-3); Ende (Bit4-7) der Sperrfunktion   */
//#define APP_BRIGHTNESS_BLOCKING_FUNCTION_CH2 0x01E0     /**<  Helligkeit bei Beginn (Bit0-3); Ende (Bit4-7) der Sperrfunktion   */
//#define APP_BUS_ON_BRIGHTNESS                0x01E2     /**<  Wert bei Busspannungswiederkehr 0-3 K1; 4-7 K2                    */
//#define APP_CHANGE_LIGHT_SCENE               0x01E3     /**<  Lichtszene ver?nderbar Bit4 Kanal1; Bit5 Kanal 2                  */
//#define APP_LIGHT_SCENE_CH1                  0x01E7     /**<  Lichtszenen Kanal1                                                */
//#define APP_LIGHT_SCENE_CH2                  0x01E8     /**<  Lichtszenen Kanal2                                                */        
                                 
// used communication objects
#define OBJECT_SWITCH                         0          /**<  1Bit On/Off                                                      */
#define OBJECT_DIM_RED_HUE                    3          /**<  4Bit Up/Down                                                     */
#define OBJECT_DIM_GREEN_SAT                  4          /**<  4Bit Up/Down                                                     */
#define OBJECT_DIM_BLUE_VAL                   5          /**<  4Bit Up/Down                                                     */
#define OBJECT_BRIGHTNESS_RED_HUE             6          /**<  1Byte                                                            */
#define OBJECT_BRIGHTNESS_GREEN_SAT           7          /**<  1Byte                                                            */
#define OBJECT_BRIGHTNESS_BLUE_VAL            8          /**<  1Byte                                                            */
#define OBJECT_SWITCHING_RESPONSE             9          /**<  1Bit Objekt zur Rückmeldung Ein/Aus                              */
#define OBJECT_DIM_RUNS_RESPONSE             10          /**<  1Bit Objekt zur Rückmeldung Dimmen läuft                         */
#define OBJECT_DIM_INDIVIDUALLY              12          /**<  1Bit On/Off                                                      */


/** PWM duty cycle. 0 = 0%, 100 = 100% */
#define PWM_SETPOINT    33                               /* 33% duty cycle                                                      */

/** How long we hold the relais at 100% before we enable PWM again */
#define PWM_DELAY_TIME               3*M2TICS(130)       /* 3 * 130ms                                                           */

/** Time between "relay on" and "send data to LEDs" */
#define POWER_ON_DELAY               3*M2TICS(130)       /* 3 * 130ms                                                           */

#define RELAY                        3                   /* Relay connectet to IO3                                              */

#define MAX_NUMBER_OF_LEDS           150

#define RED_HUE                      0
#define GREEN_SAT                    1
#define BLUE_VAL                     2

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
{ SOFTWARE_VERSION_NUMBER,      0x01 },                 /**< version number                               */
{ RUN_ERROR_STATUS,             0xFF },                 /**< Error-Status (FF=no error)                   */
{ COMMSTAB_ADDRESS,             0x8A },                 /**< COMMSTAB Pointer                             */
{ APPLICATION_PROGRAMM,         0x00 },                 /**< Port A Direction Bit Setting???              */

{ MANUFACTORER_ADR_HIGH,        0x00 },                 /**< manufacturer 0x0004 = JUNG                   */
{ MANUFACTORER_ADR_LOW,         0x04 },                 /**<                                              */
{ DEVICE_NUMBER_HIGH,           0x30 },                 /**< device type 3018h                            */
{ DEVICE_NUMBER_LOW,            0x18 },                 /**<                                              */

{ 0xFF,                         0xFF }                  /**< END-sign; do not change                      */
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




#endif /* FB_RGB_DIM_APP_H_ */