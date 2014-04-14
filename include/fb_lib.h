/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
*  /_/   /_/ |_/_____/_____/_____/\____//____/
*
*  Copyright (c) 2013 Matthias Fechner <matthias@fechner.net>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   fb_lib.h
* @author Matthias Fechner
* @date   Mon Apr 29 17:36:22 2013
*
* @brief  Define function from lib
*
*/
#ifndef _FB_LIB_H
#define _FB_LIB_H

#ifdef _FB_PROT_C
#define PROT_EXT
#else
#define PROT_EXT extern
#endif

#ifdef _FB_APP_C
#define FBAPP_EXT
#else
/** Set FBAPP_EXT to extern to make clear it is include from an external source like a lib */
#define FBAPP_EXT    extern
#endif

#ifndef FALSE
#define FALSE (0!=0)         /**< macro which will always return false  */
#endif

#ifndef TRUE
#define TRUE  (0==0)         /**< macro which will always return true   */
#endif

#ifdef BOOTLOADER
#define XBOOT_SECTION __attribute__((section(".xbootloader")))
#else
#define XBOOT_SECTION
#endif

#ifdef __GNUC__
#define __UNUSED__  __attribute__ ((unused))
#else
#define __UNUSED__
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb_hardware.h"
#include "timer.h"
#include "fb_eeprom.h"
#include "freebus-debug.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/
#define MANUFACTORER_ADR_HIGH         0x0103  ///< High address for manufactorer id                    */
#define MANUFACTORER_ADR_LOW          0x0104  /**< Low address for manufacturer id                     */
#define DEVICE_NUMBER_HIGH            0x0105  /**< High address for device id                          */
#define DEVICE_NUMBER_LOW             0x0106  /**< Low address for device id                           */
#define SOFTWARE_VERSION_NUMBER       0x0107  /**< Version number of the application                   */
#define APPLICATION_PROGRAMM          0x010C  /**< Start-address of the application program              */
#define RUN_ERROR_STATUS              0x010D  /**< Error status of the application bit order
                                                   reserved SYS3_ERR SYS2_ERR OBJ_ERR STK_OVL EEPROM_ERR SYS1_ERR SYS0_ERR
                                                   SYS0_ERR Internal system error
                                                   SYS1_ERR Internal system error
                                                   EEPROM_ERR The EEPROM-check detected an error. The EEPROM-data are probably corrupted
                                                   STK_OVL A stack overflow was detected
                                                   OBJ_ERR The AL detected an error in the communication-object or association-table. Probably due to inconsistent EEPROM-data. This error inhibits user-execution
                                                   SYS2_ERR Internal system error
                                                   SYS3_ERR Internal system error. Probably due to inconsistent EEPROM-data */
#define COMMSTAB_ADDRESS              0x0112  /**< COMMSTAB Pointer                                    */

#define IO_INPUT     0U             /**< Portpin-Direction is Input  */
#define IO_OUTPUT    1U             /**< Portpin-Direction is Output */

/*********************************/
/* MACROs for port configuration */
/*********************************/
#define OFF          0U             /**< Global Define: OFF = 0     */
#define ON           1U             /**< Global Define: ON  = 1     */


struct FBAppInfo
{
    const uint8_t           FBApiVersion;                ///< Version of Freeebus API
    const STRUCT_DEFPARAM   *pParam;                     ///< Application parameter, see defaultParam which describes the memory layout of the application

/* Not used yet */
    void (*AppMain)         (void);                      ///< Pointer to AppMain function
    void (*AppSave)         (void);                      ///< Pointer to function in application which must be called if save status is triggered
    void (*AppUnload)       (void);                      ///< Pointer to function which is called if the application will be unloaded
    uint8_t (*AppReadObject)     (uint8_t objectNr, void* dest, uint8_t len); ///< Pointer to function to read an object from the application
    uint8_t (*AppWriteObject)    (uint8_t objectNr, void* src, uint8_t len);  ///< Pointer to function to write an object in the application
    void *ramflags;                                      ///< Pointer to RAM flag table
    void *comobjtable;                                   ///< Pointer to communication object table
    void *assoctable;                                    ///< Pointer to Association table
    void *grpaddr;                                       ///< Pointer to Group address table
};

FBAPP_EXT const STRUCT_DEFPARAM defaultParam[] PROGMEM;
FBAPP_EXT const struct FBAppInfo AppInfo PROGMEM;

/**************************************************************************
* DECLARATIONS
**************************************************************************/
/* State used for new lib api */
#define SET_STATE(x) app_state |= (x)          ///< Defined the next state for the application
#define UNSET_STATE(x) app_state &= ~(x)       ///< disable state defined in x
#define IN_STATE(x) app_state & (x)            ///< Return true, if state x is active
#define RESET_STATE() app_state = 0            ///< Clear all bits used in the state variable

/*************************************************************************
* LIB FUNCTION PROTOTYPES
**************************************************************************/
/**
* Copies "len" bytes from "src" to the object data and transmits the
* object specified by "objectNr".
*
*/
PROT_EXT void SetAndTransmitObject(uint8_t objectNr, void* src, uint8_t len);

/**
* Copies 1 bit to object data and transmits the object specified by objectNr
*
*/
PROT_EXT void SetAndTransmitBit(uint8_t objectNr, uint8_t value);

/**
* Tests if there was an update for the object specified by "objectNr".
*
*/
PROT_EXT uint8_t TestObject(uint8_t objectNr);

/**
* This function tests if there was an update for an object given by "objectNr". If this is
* the case "len" bytes of the object value are copied to "dst".
*
*/
PROT_EXT uint8_t TestAndCopyObject(uint8_t objectNr, void* dst, uint8_t len);

/**
* Set the RAM flags for the object specified by "objectNr".
*
*/
PROT_EXT void SetRAMFlags(uint8_t objectNr, uint8_t flags);

/**
* This function transmits the object specified by "objectNr".
*
*/
PROT_EXT void TransmitObject(uint8_t objectNr);

/**
* Sets the RAM flags to generate a GroupValueRead on the object specified by
* "objectNr".
*
*/
PROT_EXT uint8_t ReadObject(uint8_t objectNr);

/**
 * Function which is defined in each application which is called if the application is restarted.
 * @return
 */
FBAPP_EXT uint8_t restartApplication(void);

#endif
