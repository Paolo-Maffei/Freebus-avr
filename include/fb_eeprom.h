/* $Id$ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2007 Dirk Opfer <dirk@do13.de>
*  Copyright (c) 2007 Matthias Fechner <matthias@fechner.net>
*  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb_eeprom.h
* @author Dirk Opfer, Matthias Fechner, Christian Bode
* @date   Sat Apr 05 23:37:24 2008
* 
* @brief  Handling the access and mapping to the eeprom.
*
*
*/
#ifndef _FB_EEPROM_H
#define _FB_EEPROM_H

#ifdef _FB_EEPROM_C
#define EEPROM_EXT
#else
/** Set EEPROM_EXT to extern to make clear it is include from an external source like a lib */
#define EEPROM_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/


/**************************************************************************
* DEFINITIONS
**************************************************************************/
/** Speichergroesse des EEPROMs */
#define BASE_ADDRESS_OFFSET           0x0100    /**< Offset in BCU1 EEPROM           */
#ifdef  FB_RF
#define TP_NODEPARAM_SIZE             0x00FF    /**< Size of parameter structur      */
#define NODEPARAM_SIZE  TP_NODEPARAM_SIZE+0x0010  /**< Size of parameter structur + TP parameters  */
#define RF_NODEPARAMS   BASE_ADDRESS_OFFSET+TP_NODEPARAM_SIZE+1 /**< Start of RF parameters */
#define RF_SETXTAL      RF_NODEPARAMS+0x10
#else
#define NODEPARAM_SIZE                0x00FF    /**< Size of parameter structur      */
#endif
#define APPLICATION_MEM_BASE_ADR      0x0000    /**< application memory startaddress */
#define APPLICATION_MEM_SIZE          0x0017    /**< Size of application memory      */
#define SYSTEMSTATE                   0x0060    /**< System Status                   */
#define USERRAM_BASE_ADR              0x0050    /**< User RAM startaddress           */
#define USERRAM_SIZE                  0x0100-0x50   /**< Size of user RAM                */

/** Size of data structur           */
#define EEPROM_SIZE                   NODEPARAM_SIZE + APPLICATION_MEM_SIZE + USERRAM_SIZE
#define MAX_EEPROM_SIZE               EEPROM_SIZE - 1    /**< The maximum size of the eeprom usable */

/** Struktur fuer die Default-Parameter */
typedef struct
{
   uint16_t addr;                               /**< EEPROM-Adresse des Parameters  */
   uint8_t defValue;                            /**< Default des Parameters         */
} STRUCT_DEFPARAM;


/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
EEPROM_EXT uint8_t mem_ReadByte(uint16_t wAdr);
EEPROM_EXT uint16_t mem_Read2Bytes(uint16_t addr);
EEPROM_EXT void mem_WriteBlock(uint16_t addr, uint8_t len, uint8_t *dat);
EEPROM_EXT uint8_t eeprom_busy(void);
EEPROM_EXT void eeprom_ParamSetDefault(const STRUCT_DEFPARAM *pParam );
EEPROM_EXT void eeprom_Init(uint8_t *pNodeParam, uint16_t wParamlen);
EEPROM_EXT uint8_t *eeprom_GetBase(void);

#ifdef DEBUG_UART
// disabled because no memory left in microcontroller
void eeprom_print(void);
#endif

#endif /* _FB_EEPROM_H */
/*********************************** EOF *********************************/
