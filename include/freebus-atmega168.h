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
* @file   freebus-atmega168.h
* @author Matthias Fechner, Christian Bode
* @date   Mon Jun  2 09:14:22 2008
* 
* @brief  Hardware specific options for the ATmega168P.
* DO NOT INCLUDE THAT FILE DIRECTLY, include fb_hardware.h instead.
* 
* 
*/
#if defined(_FB_HARDWARE_H)
#ifndef _FREEBUS_ATMEGA168_H
#define _FREEBUS_ATMEGA168_H ///< is defined if target system is the ATMega168

/*************************************************************************
* INCLUDES
*************************************************************************/
#include <inttypes.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h> 
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/parity.h>
#include <util/delay.h>
#include <string.h>

/**************************************************************************
* DEFINITIONS
**************************************************************************/
#define ENABLE_ALL_INTERRUPTS()     sei()                   /**< global interrupt enable */ 
#define DISABLE_IRQS    unsigned char _sreg = SREG; cli();  /**< Disable IRQs, save status before doing this */
#define ENABLE_IRQS     { if(_sreg & 0x80) sei(); }         /**< Enable IRQs if they are enabled before DISABLE_IRQS is called */

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
          EIFR |= (1<<INTF0);                   \
          EIMSK |= (1<<INT0);                   \
     }

/** Reload Timer0
* calculate value with: (F_CPU*TIME_TO_WAIT)/prescaler
* substract from 0xFF (or decimal 255), that's the value you want, keep in mind it must be a 8-bit value,
* adapt the prescaler if necessary
* note substract 8�s from value! (that the microcontroller eats every time)
*/
#define RELOAD_TIMER0(value, tccr0) {                                   \
          TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00)); /* stop timer */ \
          TCCR0A = 0;             /* set timer mode */                  \
          TCNT0  = value;         /* set start of timer, run to 0xFF */ \
          TIMSK0 |= (1<<TOIE0);   /* enable overflow interrupt */       \
          TCCR0B = tccr0;         /* clock select */                    \
     }

/** Checkcondition for application timer overrun */
#define TIMER1_OVERRUN              (TIFR1 & (1U<<OCF1A))

/** Clear overrun bit set for application timer */
#define CLEAR_TIMER1_OVERRUN        TIFR1 |= (1U<<OCF1A)

/** Reload the application timer (Timer1) to start from 0 */
#define RELOAD_APPLICATION_TIMER()  {                                   \
          TCCR1A = 0;             /* CTC (Clear Timer on Compate match) */ \
          TCCR1B = (1U<<WGM12)|(1U<<CS11)|(1U<<CS10); /* CTC-mode, prescale to 64 */ \
          OCR1A  = 16249;         /* every 130 ms OCR1A=(delay*F_CPU)/(prescaler)-1 */ \
          TCNT1  = 0;             /* reset timer */                     \
     }

/** 
* Enable PWM, PWM pin (PB3) is set by hardware.
* 
* @param x Duty-cycle (0xF2=6,3%, 0x01=100%)
* 
*/
#define ENABLE_PWM(x)               {                                   \
          TCCR2A = (1<<WGM20)|(1<<COM2A1)|(1<<COM2A0);/* Phase correct PWM and enable OC2a pin */ \
          TCCR2B = (1<<CS21);     /* prescaler 8 */                     \
          TCNT2  = 0;             /* reset timer2 */                    \
          OCR2A  = (x);           /* defines the duty cycle */          \
     }

/** Disable PWM and set PWM pin to high */
#define DISABLE_PWM()               {                                   \
          TCCR2A &= ~((1<<COM2A1)|(1<<COM2A0)); /* disable PWM pin  */  \
          SETPIN_CTRL(ON);                     /* set port to high */   \
     }

/** Enable interrupt for UART0 */
#define ENABLE_UART_TX_IRQ()        {                            \
          UCSR0B |= (1<<TXCIE0);       /* enable transmit IRQ */ \
     }

/** Disable interrupt for UART0 */
#define DISABLE_UART_TX_IRQ()       {                                 \
          UCSR0B &= ~(1<<TXCIE0);      /* disable transmit IRQ */     \
     }
          
/** Store one byte in UART0 send buffer. */
#define UART_SEND_BYTE(tx_char)     {           \
          UDR0 =(uint8_t)(tx_char);             \
     }

/** Enable internal hardware watchdog */
#define ENABLE_WATCHDOG(x)          {           \
          wdt_enable(x);                        \
     }


/** Disable internal hardware watchdog */
#define DISABLE_WATCHDOG()          {           \
          MCUSR = 0;                            \
          wdt_disable();                        \
     }

// map interrupts
/** map uart interrupt*/
#define SIG_UART_TRANS              USART_TX_vect
/** map eeprom ready vector */
#define EE_RDY_vect                 EE_READY_vect

/*********************************/
/* MACROs for port configuration */
/*********************************/
#define IO1_PORT     PORTB      /**< output port (byte) of IO1  */
#define IO1_DDR      DDRB       /**< data direction byte of IO1 */
#define IO1_IN       PINB       /**< input port (byte) of IO1   */
#define IO1_PIN      PB2        /**< bit position of IO1        */

#define IO2_PORT     PORTC      /**< output port (byte) of IO2  */
#define IO2_DDR      DDRC       /**< data direction byte of IO2 */
#define IO2_IN       PINC       /**< input port (byte) of IO2   */
#define IO2_PIN      PC1        /**< bit position of IO2        */

#define IO3_PORT     PORTD      /**< output port (byte) of IO3  */
#define IO3_DDR      DDRD       /**< data direction byte of IO3 */
#define IO3_IN       PIND       /**< input port (byte) of IO3   */
#define IO3_PIN      PD3        /**< bit position of IO3        */

#define IO4_PORT     PORTD      /**< output port (byte) of IO4  */
#define IO4_DDR      DDRD       /**< data direction byte of IO4 */
#define IO4_IN       PIND       /**< input port (byte) of IO4   */
#define IO4_PIN      PD5        /**< bit position of IO4        */

#define IO5_PORT     PORTD      /**< output port (byte) of IO5  */
#define IO5_DDR      DDRD       /**< data direction byte of IO5 */
#define IO5_IN       PIND       /**< input port (byte) of IO5   */
#define IO5_PIN      PD6        /**< bit position of IO5        */

#define IO6_PORT     PORTD      /**< output port (byte) of IO6  */
#define IO6_DDR      DDRD       /**< data direction byte of IO6 */
#define IO6_IN       PIND       /**< input port (byte) of IO6   */
#define IO6_PIN      PD7        /**< bit position of IO6        */

#define IO7_PORT     PORTC      /**< output port (byte) of IO7  */
#define IO7_DDR      DDRC       /**< data direction byte of IO7 */
#define IO7_IN       PINC       /**< input port (byte) of IO7   */
#define IO7_PIN      PC2        /**< bit position of IO7        */

#define IO8_PORT     PORTC      /**< output port (byte) of IO8  */
#define IO8_DDR      DDRC       /**< data direction byte of IO8 */
#define IO8_IN       PINC       /**< input port (byte) of IO8   */
#define IO8_PIN      PC3        /**< bit position of IO8        */

#define PROG_PORT    PORTB      /**< output port (byte) of PROG button  */
#define PROG_DDR     DDRB       /**< data direction byte of PROG button */
#define PROG_IN      PINB       /**< input port (byte) of PROG button   */
#define PROG_PIN     PB0        /**< bit position of PROG button        */

#define CTRL_PORT    PORTB      /**< output port (byte) of CTRL pin     */
#define CTRL_DDR     DDRB       /**< data direction byte of CTRL pin    */
#define CTRL_IN      PINB       /**< input port (byte) of CTRL pin      */
#define CTRL_PIN     PB3        /**< bit position of CTRL pin           */

#define EIBOUT_PORT  PORTD      /**< output port (byte) of EIB-OUT      */
#define EIBOUT_DDR   DDRD       /**< data direction byte of EIB-OUT     */
#define EIBOUT_IN    PIND       /**< input port (byte) of EIB-OUT       */
#define EIBOUT_PIN   PD4        /**< bit position of EIB-OUT            */

#define EIBIN_PORT   PORTD      /**< output port (byte) of EIB-IN       */
#define EIBIN_DDR    DDRD       /**< data direction byte of EIB-IN      */
#define EIBIN_IN     PIND       /**< input port (byte) of EIB-IN        */
#define EIBIN_PIN    PD2        /**< bit position of EIB-IN             */


#define RES1_PORT    PORTB      /**< output port (byte) of IO1  */
#define RES1_DDR     DDRB       /**< data direction byte of IO1 */
#define RES1_IN      PINB       /**< input port (byte) of IO1   */
#define RES1_PIN     PB1        /**< bit position of IO1        */

#define RES2_PORT    PORTC      /**< output port (byte) of IO1  */
#define RES2_DDR     DDRC       /**< data direction byte of IO1 */
#define RES2_IN      PINC       /**< input port (byte) of IO1   */
#define RES2_PIN     PC0        /**< bit position of IO1        */

#define RES3_PORT    PORTD      /**< output port (byte) of IO1  */
#define RES3_DDR     DDRD       /**< data direction byte of IO1 */
#define RES3_IN      PIND       /**< input port (byte) of IO1   */
#define RES3_PIN     PD0        /**< bit position of IO1        */

#define RES4_PORT    PORTD      /**< output port (byte) of IO1  */
#define RES4_DDR     DDRD       /**< data direction byte of IO1 */
#define RES4_IN      PIND       /**< input port (byte) of IO1   */
#define RES4_PIN     PD1        /**< bit position of IO1        */

/**************************************************************************
* DECLARATIONS
**************************************************************************/
 

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

#endif /* _FREEBUS_ATMEGA168_H */
#endif /* _FB_HARDWARE_H */
/*********************************** EOF *********************************/
