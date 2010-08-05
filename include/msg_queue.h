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
* @file   msg_queue.h
* @author Dirk Opfer, Matthias Fechner, Christian Bode
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

/**************************************************************************
* DEFINITIONS
**************************************************************************/
/** Max length of a message */
#define MSG_MAX_DATA_LENGTH 25

/* Flags for msg */
#define MSG_AUTOFREE     (1<<0)  /**< clear message from queue automatically after successfully handled */
#define MSG_HI_PRIO      (1<<1)  /**< used for eib message priority handling */

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
    enum emsg_state state;               ///< Status of the message, see here emsg_state
    uint8_t	flags;                     ///< Flags for message, e.g. MSG_AUTOFREE or MSG_HI_PRIO
    uint8_t	len;                       ///< Length of the data which is stored
    uint8_t data[MSG_MAX_DATA_LENGTH];   ///< The message itself
    uint8_t repeat;                      ///< Counter which will be used in EIB sending to count amount of repeats (max resend until repeat==0)
    struct msg *next;                    ///< Pointer to next message
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
	free(pmsg);
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

#endif /* _MSG_QUEUE_H */
/*********************************** EOF *********************************/
