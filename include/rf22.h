/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*/
/**
* @file   rf22.h
* @author Dirk Armbrust
* @date   Jan 13 2011
* 
* @brief  Definitions on RFM22 physical interface
* 
* 
*/
#ifndef _RF22_H_
#define _RF22_H_

#ifdef _RF22_C
#define RF22_EXT
#else
#define RF22_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "Spi.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define  RFM22_NIRQ_PORT  PIND
#define  RFM22_NIRQ_PIN   PD2

/* RFM22 register definitions */
#define RXFFSIZE   64    /* rx fifo size */
#define TXFFSIZE   64    /* tx fif0 size */
#define TXAETHR    TXFFSIZE / 2  /*signal "tx fifo almost empty" when it is half empty*/
#define RXAFTHR    8             /*signal "rx fifo almost full" when it contains 8 chips */

/* register 00, device type */
#define DT         0  /* DT[4:0] */

/* register 01, device version */
#define VC         0  /* VC[4:0] */

/* register 02, device status */
#define CPS        0  /* CPS[1:0] */
#define HEADERR    4
#define RXFFEM     5
#define FFUNFL     6
#define FFOVL      7

/* register 03, interrupt status 1 */
#define ICRCERROR  0
#define IPKVALID   1
#define IPKSENT    2
#define IEXT       3
#define IRXFFAFULL 4
#define ITXFFAEM   5
#define ITXFFAFULL 6
#define IFFERR     7

/* register 04, interrupt status 2 */
#define IPOR       0
#define ICHIPRDY   1
#define ILBD       2
#define IWUT       3
#define IRSSI      4
#define IPREAINVAL 5
#define IPREAVAL   6
#define ISWDET     7

/* register 05, interrupt enable 1 */
#define ENCRCERROR  0
#define ENPKVALID   1
#define ENPKSENT    2
#define ENEXT       3
#define ENRXFFAFULL 4
#define ENTXFFAEM   5
#define ENTXFFAFULL 6
#define ENFFERR     7

/* register 06, interrupt enable 2 */
#define ENPOR       0
#define ENCHIPRDY   1
#define ENLBD       2
#define ENWUT       3
#define ENRSSI      4
#define ENPREAINVAL 5
#define ENPREAVAL   6
#define ENSWDET     7

/* register 07, operating and function control 1 */
#define XTON        0
#define PLLON       1
#define RXON        2
#define TXON        3
#define X32KSEL     4
#define ENWT        5
#define ENLBD7      6
#define SWRES       7

/* register 08, operating and function control 2 */
#define FFCLRTX     0
#define FFCLRRX     1
#define ENLDM       2
#define AUTOTX      3
#define RXMPK       4
#define ANTDIV      5  /* ANTDIV[2:0] */

/* register 09, crystal load capacitance */
#define XLC         0  /* XLC[6..0] */
#define XTALSHFT    7

/* register 0A, microcontroller clock output */
#define MCLK        0  /* MCLK[2:0] */
#define ENLFC       3
#define CLKT        4  /* CLKT[1:0] */

/* register 0B, GPIO0 configuration */
#define GPOI0       0  /* GPIO[4:0] */
#define PUP0        5
#define GPIO0DRV    6  /* GPIODRV[1:0] */

/* register 0C, GPIO1 configuration */
#define GPOI1       0  /* GPIO[4:0] */
#define PUP1        5
#define GPIO1DRV    6  /* GPIODRV[1:0] */

/* register 0D, GPIO2 configuration */
#define GPOI2       0  /* GPIO[4:0] */
#define PUP2        5
#define GPIO2DRV    6  /* GPIODRV[1:0] */

/* register 0E, IO port configuration */
#define DIO0        0
#define DIO1        1
#define DIO2        2
#define ITSD0       3
#define EXTITST     4  /* EXTITST [2:0] */

/* register 0F, IO ADC configuration */
#define ADCGAIN     0  /* ADCGAIN[1:0] */
#define ADCREF      2  /* ADCREF[1:0] */
#define ADCSEL      4  /* ADCSEL[2:0] */
#define ADCSTART    7

/* register 30, data access control */
#define CRC         0  /* CRC[1:0] */
#define ENCRC       2
#define ENPACTX     3
#define SKIP2PH     4
#define CRCDONLY    5
#define LSBFRST     6
#define ENPACRX     7

#define rf22_write(addr,val) { \
rf22_cmd ( addr|0x80, val); }

#define rf22_read(addr)  \
rf22_cmd ( addr, 0)

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
RF22_EXT void     rf22_init    (void);
RF22_EXT uint8_t  rf22_cmd     (uint8_t  addr, uint8_t val);
RF22_EXT void     rf22_cmd_P   (prog_uint8_t* pgmp);
RF22_EXT uint8_t  rf22_setxtal (uint8_t newval);
RF22_EXT uint8_t  rf22_getxtal (void);

#endif /* _RF22_H_ */