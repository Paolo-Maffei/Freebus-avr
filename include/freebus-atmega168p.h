/* $Id$ */
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
*/
/**
* @file   freebus-atmega168p.h
* @author Matthias Fechner, Christian Bode, Tobias Buss
* @date   Mon Jul 26 20:45:22 2009
* 
* @brief  Hardware specific options for the ATmega168P.
* DO NOT INCLUDE THAT FILE DIRECTLY, include fb_hardware.h instead.
* 
* 
*/
#if defined(_FB_HARDWARE_H)
#ifndef _FREEBUS_ATMEGA168P_H
#define _FREEBUS_ATMEGA168P_H ///< is defined if target system is the ATMega168P

/*************************************************************************
* INCLUDES
*************************************************************************/
#include <string.h>

/**************************************************************************
* DEFINITIONS
**************************************************************************/

/** Configure external INT0 to match falling edge and enable the interrupt. */
#define CONFIGURE_INT0()            {           \
          EICRA |= (1<<ISC01);      /* Enable Int0 falling edge */    \
          EICRA &= ~(1<<ISC00);     /* Enable Int0 falling edge */    \
          EIMSK |= (1<<INT0);       /* Enable Int0 */                 \
     }

/** disable interrupt INT0 */
#define DISABLE_RX_INT()            {           \
          EIMSK &= ~(1<<INT0);                  \
     }

/** clear pending interrupts INT0 and enable interrupt */
#define ENABLE_RX_INT()             {           \
          EIFR   = (1<<INTF0);                  \
          EIMSK |= (1<<INT0);                   \
     }

#define INIT_EIB_TIMER(vala,valb) { \
          TCCR0A = (1<<WGM01);     /*  CTC mode  */ \
          OCR0A = vala  ;  /* duration of one bit */ \
          OCR0B = valb  ;  \
     } 

#define START_EIB_TIMER(value) { \
          TCNT0  = value             ;            /* set start value */ \
          TIFR0  = (1<<OCF0A)|(1<<OCF0B) ;        /* clear pending interrups */ \
          TIMSK0 = (( 1<<OCIE0A )|( 1<<OCIE0B )); /* enable OCRA, OCRB interrupts */  \
          TCCR0B = (1<<CS01) ;                    /* start the timer, prescaler :8, increment each 1 usec */ \
    }

#define STOP_EIB_TIMER() { \
          TCCR0B = 0;      \
          TIMSK0 = 0;      \
    }

#define SLOW_EIB_TIMER() \
          TCCR0B |=  (1<<CS00);

#define NORMAL_EIB_TIMER() \
          TCCR0B &= ~(1<<CS00);
 

/** Checkcondition for application timer overrun */
#define TIMER1_OVERRUN              (TIFR1 & (1U<<OCF1A))

/** Clear overrun bit set for application timer */
#define CLEAR_TIMER1_OVERRUN        TIFR1  = (1U<<OCF1A)

/** 
* Enable PWM, PWM pin (PB3) is set by hardware.
* 
* New Frequency to get out of the hearable frequency. (At least at the end of it.)
* freq = F_CPU/(Prescaler*510) = 8000000/(1*510) = 15868 Hz
*
* @param x Duty-cycle (0xF2=6,3%, 0x01=100%)
* 
*/
#define ENABLE_PWM(x)               {                                   \
          TCCR2A = (1<<WGM20)|(1<<COM2A1)|(1<<COM2A0);/* Phase correct PWM and enable OC2a pin */ \
          TCCR2B = (1<<CS20);     /* prescaler 0 */                     \
          TCNT2  = 0;             /* reset timer2 */                    \
          OCR2A  = (x);           /* defines the duty cycle */          \
     }

/** Disable PWM and set PWM pin to high */
#define DISABLE_PWM()               {                                   \
          TCCR2A &= ~((1<<COM2A1)|(1<<COM2A0)); /* disable PWM pin  */  \
          SETPIN_CTRL(ON);                     /* set port to high */   \
     }

/** Execute some EEPROM specific commmands
 * configure eeprom to only write and not delete before, that operation will take 1.8ms
 */
#define CONFIG_EEPROM() {                       \
        EECR |= (1 << EEPM1);                   \
        EECR &= ~(1 << EEPM0);                  \
    }

/** Enable the EEPROM ready interrupt */
#define ENABLE_EEPROM_READY_INT()   {           \
        EECR |= (1U<<EERIE);                    \
    }
    
/** Disable the EEPROM ready interrupt */
#define DISABLE_EEPROM_READY_INT()   {           \
        EECR &= ~(1U<<EERIE);                    \
    }

// map interrupts
/** map eeprom ready vector */
#define EE_RDY_vect                 EE_READY_vect

/*********************************/
/* MACROs for port configuration */
/*********************************/
#define IO1_PORT     PORTB      /**< output port (byte) of IO1  */
#define IO1_DDR      DDRB       /**< data direction byte of IO1 */
#define IO1_IN       PINB       /**< input port (byte) of IO1   */
#define IO1_PIN      2          /**< bit position of IO1        */

#define IO2_PORT     PORTC      /**< output port (byte) of IO2  */
#define IO2_DDR      DDRC       /**< data direction byte of IO2 */
#define IO2_IN       PINC       /**< input port (byte) of IO2   */
#define IO2_PIN      1        	/**< bit position of IO2        */

#define IO3_PORT     PORTD      /**< output port (byte) of IO3  */
#define IO3_DDR      DDRD       /**< data direction byte of IO3 */
#define IO3_IN       PIND       /**< input port (byte) of IO3   */
#define IO3_PIN      3        	/**< bit position of IO3        */

#define IO4_PORT     PORTD      /**< output port (byte) of IO4  */
#define IO4_DDR      DDRD       /**< data direction byte of IO4 */
#define IO4_IN       PIND       /**< input port (byte) of IO4   */
#define IO4_PIN      5	        /**< bit position of IO4        */

#define IO5_PORT     PORTD      /**< output port (byte) of IO5  */
#define IO5_DDR      DDRD       /**< data direction byte of IO5 */
#define IO5_IN       PIND       /**< input port (byte) of IO5   */
#define IO5_PIN      6	        /**< bit position of IO5        */

#define IO6_PORT     PORTD      /**< output port (byte) of IO6  */
#define IO6_DDR      DDRD       /**< data direction byte of IO6 */
#define IO6_IN       PIND       /**< input port (byte) of IO6   */
#define IO6_PIN      7	        /**< bit position of IO6        */

#define IO7_PORT     PORTC      /**< output port (byte) of IO7  */
#define IO7_DDR      DDRC       /**< data direction byte of IO7 */
#define IO7_IN       PINC       /**< input port (byte) of IO7   */
#define IO7_PIN      2	        /**< bit position of IO7        */

#define IO8_PORT     PORTC      /**< output port (byte) of IO8  */
#define IO8_DDR      DDRC       /**< data direction byte of IO8 */
#define IO8_IN       PINC       /**< input port (byte) of IO8   */
#define IO8_PIN      3	        /**< bit position of IO8        */

#define PROG_PORT    PORTB      /**< output port (byte) of PROG button  */
#define PROG_DDR     DDRB       /**< data direction byte of PROG button */
#define PROG_IN      PINB       /**< input port (byte) of PROG button   */
#define PROG_PIN     0	        /**< bit position of PROG button        */

#define CTRL_PORT    PORTB      /**< output port (byte) of CTRL pin     */
#define CTRL_DDR     DDRB       /**< data direction byte of CTRL pin    */
#define CTRL_IN      PINB       /**< input port (byte) of CTRL pin      */
#define CTRL_PIN     3	        /**< bit position of CTRL pin           */

#define EIBOUT_PORT  PORTD      /**< output port (byte) of EIB-OUT      */
#define EIBOUT_DDR   DDRD       /**< data direction byte of EIB-OUT     */
#define EIBOUT_IN    PIND       /**< input port (byte) of EIB-OUT       */
#define EIBOUT_PIN   4	        /**< bit position of EIB-OUT            */

#define EIBIN_PORT   PORTD      /**< output port (byte) of EIB-IN       */
#define EIBIN_DDR    DDRD       /**< data direction byte of EIB-IN      */
#define EIBIN_IN     PIND       /**< input port (byte) of EIB-IN        */
#define EIBIN_PIN    2	        /**< bit position of EIB-IN             */


#define RES1_PORT    PORTB      /**< output port (byte) of IO1  */
#define RES1_DDR     DDRB       /**< data direction byte of IO1 */
#define RES1_IN      PINB       /**< input port (byte) of IO1   */
#define RES1_PIN     1	        /**< bit position of IO1        */

#define RES2_PORT    PORTC      /**< output port (byte) of IO1  */
#define RES2_DDR     DDRC       /**< data direction byte of IO1 */
#define RES2_IN      PINC       /**< input port (byte) of IO1   */
#define RES2_PIN     0	        /**< bit position of IO1        */

#define RES3_PORT    PORTD      /**< output port (byte) of IO1  */
#define RES3_DDR     DDRD       /**< data direction byte of IO1 */
#define RES3_IN      PIND       /**< input port (byte) of IO1   */
#define RES3_PIN     0	        /**< bit position of IO1        */

#define RES4_PORT    PORTD      /**< output port (byte) of IO1  */
#define RES4_DDR     DDRD       /**< data direction byte of IO1 */
#define RES4_IN      PIND       /**< input port (byte) of IO1   */
#define RES4_PIN     1	        /**< bit position of IO1        */

/**************************************************************************
* DECLARATIONS
**************************************************************************/
 

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

#endif /* _FREEBUS_ATMEGA168P_H */
#endif /* _FB_HARDWARE_H */
/*********************************** EOF *********************************/
