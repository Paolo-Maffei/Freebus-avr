/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
*  /_/   /_/ |_/_____/_____/_____/\____//____/
*
*  Copyright (c) 2014 Matthias Fechner <matthias@fechner.net>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/

/**
* @file   freebus-arm.h
* @author Matthias Fechner
*
* @brief  Hardware specific options for ARM.
* DO NOT INCLUDE THAT FILE DIRECTLY, include fb_hardware.h instead.
*
*
*/
#if defined(_FB_HARDWARE_H)

#ifndef FREEBUS_ARM_H_
#define FREEBUS_ARM_H_

/**************************************************************************
* DEFINITIONS
**************************************************************************/
#define ENABLE_ALL_INTERRUPTS()     cpu_irq_enable()                   /**< global interrupt enable */
#define DISABLE_IRQS    cpu_irq_disable()  /**< Disable IRQs, save status before doing this */
#define ENABLE_IRQS     cpu_irq_enable()         /**< Enable IRQs if they are enabled before DISABLE_IRQS is called */

/** Enable internal hardware watchdog */
#define ENABLE_WATCHDOG(x)          {           \
          wdt_enable(x);                        \
     }


/** Disable internal hardware watchdog */
#define DISABLE_WATCHDOG()          {           \
          wdt_disable();                        \
     }

#define IO_SET(nr, val)  do {if(val) {IO##nr##_PORT |= (1U<<IO##nr##_PIN);} else {IO##nr##_PORT &= ~(1U<<IO##nr##_PIN);} } while (0)
#define IO_GET(nr)  ((IO##nr##_IN>>IO##nr##_PIN) & 0x01)
#define IO_SET_DIR(nr,type) do {IO##nr##_DDR = (IO##nr##_DDR & ~(1U<<IO##nr##_PIN)) | (type<<IO##nr##_PIN); } while(0)

typedef uint8_t prog_uint8_t;

#endif /* FREEBUS_ARM_H_ */
#endif /* _FB_HARDWARE_H */
