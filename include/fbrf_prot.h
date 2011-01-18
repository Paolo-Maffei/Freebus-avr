/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2009 Dirk Armrust <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   fbrf_prot.h
* @author Dirk Armbrust
* @date 
* 
* @brief  Define the KNX-RF protocol specific definitions
*
*/
#ifndef _FBRF_PROT_H
#define _FBRF_PROT_H

#ifdef  _FBRF_PROT_C
#define RFPROT_EXT
#else
#define RFPROT_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/

/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define CRCPOLY 0x3d65 /* KNX-RF Polynom */

#define NODEPARAM_RSSI  RF_NODEPARAMS - BASE_ADDRESS_OFFSET
/** Structure of the KNX-RF blocks for each telegram */

struct rfblock1 {
  uint8_t length;
  uint8_t c;
  uint8_t esc;
  uint8_t ctrl;
  uint8_t sn[6];
  uint8_t crc[2];
} ;

struct rfblock2 {
  uint8_t knx_ctrl;
  uint8_t src[2];
  uint8_t dest[2];
  uint8_t npci;
  uint8_t tpci;
  uint8_t apci;
  uint8_t data[8];
  uint8_t crc[2];
};

struct rfblock3 {
  uint8_t data[8];
  uint8_t crc[2];
};

/* struct to hold source address (pa) and lfn of one telegram */
struct rf_mem{
    uint8_t t;
    uint8_t lfn;
    uint8_t pa[2];
};

#define N_RF_HIST 6   /* number of telegrams in the history */
#define RF_HIST_T 30   /* delete history entry after 30*130ms (3.9 sec) */

/**************************************************************************
* DECLARATIONS
**************************************************************************/

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
RFPROT_EXT void    sendRfTelegram(uint8_t commObjectNumber, uint8_t value, uint8_t offset);
RFPROT_EXT uint8_t rfmsg_len ( uint8_t lengthbyte );
RFPROT_EXT void forward_msg  ( struct msg* rf_msg );
RFPROT_EXT uint8_t fbrf_handlemsg(struct msg* rf_msg);
RFPROT_EXT void rf_hist_periodic(void);


#endif