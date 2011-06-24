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
*  Copyright (c) 2010 Dirk Armrust <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   msg_queue.h
* @author Dirk Opfer, Matthias Fechner, Christian Bode, Dirk Armbrust
* @date   Sat Apr 05 23:39:28 2008
* 
* @brief  Definitions for the message queue system used for telegrams and communucation like debugging (also rs232 communication).
* 
* 
*/
#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#ifdef  _MSG_QUEUE_C
#define MSG_EXT
#else
/** Set MSG_EXT to extern to make clear it is include from an external source like a lib */
#define MSG_EXT    extern
#endif

/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb_hardware.h"
#include <stdlib.h>
#include <stdbool.h>
#include <fb.h>

/**************************************************************************
* DEFINITIONS
**************************************************************************/
/** Max length of a message */
#define MSG_MAX_DATA_LENGTH 25

/* Flags for msg */
#define MSG_AUTOFREE     (1<<0)  /**< clear message from queue automatically after successfully handled */
#define MSG_HI_PRIO      (1<<1)  /**< used for eib message priority handling */
#ifdef FB_RF
#define MSG_FROM_RF      (1<<2)  /**< is set if message came from rf */
#define MSG_OWN          (1<<3)  /**< is set if message comes from me myself */
#define RF_BLOCK1_LENGTH 12      /* 10 + crc */
#define RF_BLOCK2_LENGTH 18      /* 16 + crc */
#define RF_BLOCK3_LENGTH 10      /*  8 + crc */
#define RF_MAX_DATA_LENGTH  (RF_BLOCK1_LENGTH+RF_BLOCK2_LENGTH+RF_BLOCK3_LENGTH)
#endif

/** Return the number of elements in an array */
#define COUNT_ARRAY_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))

/** Status of the message */
enum emsg_state {
     MSG_FREE = 0x00,   /**< message is not used and can be allocated */
     MSG_ALLOC,         /**< message parameters are initialized and the message container can store data now */
     MSG_SENDING,       /**< message is marked as sending */
     MSG_WAIT_ACK,      /**< message wait for ack */
     MSG_COMPLETE,      /**< transmittion completed */
};

/** Container that store messages */
struct msg {
    struct msg *next;                    ///< Pointer to next message !!!! This has to be the first entry !!!!
    enum emsg_state state;               ///< Status of the message, see here emsg_state
    uint8_t	flags;                     ///< Flags for message, e.g. MSG_AUTOFREE or MSG_HI_PRIO
    uint8_t	len;                       ///< Length of the data which is stored
#ifndef FB_RF
    uint8_t data[MSG_MAX_DATA_LENGTH];   ///< The message itself
#else
    uint8_t block1[RF_BLOCK1_LENGTH];                ///< block1 for rf (10 byte + 2 byte crc )
    uint8_t data[MSG_MAX_DATA_LENGTH]; ///< The tp message itself, max 22 byte + checksum
    uint8_t rfdata[RF_BLOCK2_LENGTH+RF_BLOCK3_LENGTH-MSG_MAX_DATA_LENGTH];
                                       ///< additional space for rf
#endif
    uint8_t repeat;                      ///< Counter which will be used in EIB sending to count amount of repeats (max resend until repeat==0)
};

/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
MSG_EXT void msg_queue_init(void);
MSG_EXT struct msg* AllocMsgI(void);
MSG_EXT void dequeuemsg(struct msg* *ptr);
MSG_EXT void queuemsg(struct msg* *ptr, struct msg *pmsg, void (*trigger)(void));
#ifdef BOOTLOADER
MSG_EXT void freemsg ( struct msg* pmsg );
#endif

/**************************************************************************
* IMPLEMENTATION
**************************************************************************/

/** 
* Free the message and mark it as MSG_FREE.
* 
* @param pmsg Pointer to msg which must be marked as free.
* 
*/
static inline void FreeMsg(struct msg* pmsg)
{
#ifndef BOOTLOADER
    free(pmsg);
#else
    freemsg(pmsg);
#endif
}

/** 
* Allocate a new message. Disables IRQs to prevent interrupt while allocating message here.
* 
* @return
* 
*/
static inline struct msg* AllocMsg(void)
{
	struct msg* ret;

	DISABLE_IRQS;

	ret = AllocMsgI();

	ENABLE_IRQS;

	return ret;
}


/**
* Checks if the given msgqueue is empty
* No need to disable IRQs. Can be called at every context
*
* @param msgqueue to be checked
*
*/
static inline uint8_t isQueueEmpty(struct msg* *ptr)
{
   return ((*ptr) ? FALSE : TRUE);
}
#endif /* _MSG_QUEUE_H */
/*********************************** EOF *********************************/
