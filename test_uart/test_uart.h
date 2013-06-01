#ifndef _FB_TEST_UART_H
#define _FB_TEST_UART_H

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

#endif /* _FB_TEST_UART_H */