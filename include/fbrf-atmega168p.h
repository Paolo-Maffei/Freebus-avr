/* $Id: freebus-atmega168p.h 1735 2010-02-08 12:59:47Z idefix $ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
*  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   freebus-atmega168P.h
* @author Matthias Fechner, Christian Bode, Tobias Buss, Dirk Armbrust
* @date   Mon Jul 26 20:45:22 2009
* 
* @brief  Hardware specific options for the ATmega168P / ATmega328P, for board with RF.
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
#define ENABLE_ALL_INTERRUPTS()     sei()                   /**< global interrupt enable */ 
#define DISABLE_IRQS    unsigned char _sreg = SREG; cli();  /**< Disable IRQs, save status before doing this */
#define ENABLE_IRQS     { if(_sreg & 0x80) sei(); }         /**< Enable IRQs if they are enabled before DISABLE_IRQS is called */

#define ENABLE_RFM22_INT()          {  \
          EIMSK |= 1<<INT1;            \
     }

/** disable interrupt from INPUT CAPTURE */

#define DISABLE_RX_INT()            {           \
          TIMSK1 &= ~(1<<ICIE1);                  \
     }

/** clear pending INPUT CAPTURE interrupts and enable interrupt */
#define ENABLE_RX_INT()   { \
          TIFR1   = (1<<ICF1);                          \
          TIMSK1 |= (1<<ICIE1);                        \
          TCCR1B |= (1<<ICNC1);  /*noise canceller */ \
     }

/** Init EIB timer. We use Timer 0 in CTC mode.
*   EIB_OUT is generated by Output Compare (pin OC0B)
*/
#define INIT_EIB_TIMER(value) { \
          TCCR0A = (1<<WGM01);                    /*  CTC mode, set OC0B low on match */ \
          OCR0A = value         ;                 /* restart after duration of one bit */ \
    }

#define START_EIB_TIMER(value) { \
          TCNT0  = value             ;            /* set start value */ \
          TIFR0  = (1<<OCF0A)|(1<<OCF0B) ;        /* clear pending interrups */ \
          TIMSK0 = (( 1<<OCIE0A )|( 1<<OCIE0B )); /* enable OCRA, OCRB interrupts */  \
          TCCR0B = (1<<CS01) ;                    /* start the timer, prescaler :8, increment each 1 �sec */ \
    }

#define STOP_EIB_TIMER() { \
          TCCR0B = 0;      \
          TIMSK0 = 0;      \
    }

#define SLOW_EIB_TIMER() \
          TCCR0B |=  (1<<CS00);

#define NORMAL_EIB_TIMER() \
          TCCR0B &= ~(1<<CS00);
 
#define START_EIB_OUT()  { \
          TIFR0   = ( 1<<OCF0B );  /* clear a pending interrupt */ \
          TIMSK0 |= ( 1<<OCIE0B ); /* enable OCRB interrupt */ \
    }

#define STOP_EIB_OUT()   \
          TIMSK0 &= ~( 1<<OCIE0B ); /* disable OCRB interrupt */

#define EIB_OUT_SCHEDULED  OCR0B

#define SCHEDULE_EIB_OUT_ON(time) {     \
          OCR0B = time;                 \
          /* set OC0B (EIB_OUT) on compare match */ \
          TCCR0A |= ((1<<COM0B1)|(1<<COM0B0)); \
          TIMSK0 &= ~( 1<<OCIE0B ); /* disable OCRB interrupt */ \
    }

#define SCHEDULE_EIB_OUT_OFF(time) {    \
          OCR0B = time;                 \
          /* clear OC0B (EIB_OUT) on compare match */ \
          TCCR0A |= (1<<COM0B1);        \
          TCCR0A &= ~(1<<COM0B0);       \
          TIFR0   = ( 1<<OCF0B );  /* clear a pending interrupt */ \
          TIMSK0 |= ( 1<<OCIE0B ); /* enable OCRB interrupt */ \
    }

#define FORCE_EIB_OUT_OFF() {           \
          TCCR0A |= (1<<COM0B1);        \
          TCCR0A &= ~(1<<COM0B0);       \
          TCCR0B |= (1<<FOC0B);         \
          TCCR0B &= ~(1<<FOC0B);        \
    }

/**	with RF, we do't have timer2 for PWM, instead we use timer1.
*	This also applies for new AVR board in TP only mode !!
*	timer1 overflows every 102.4 �sec (10MHz), 128�sec (8MHz).
*	The application may poll APP_TIMER_OVERRUN() for 130ms time frame.
*	Only on old board (rev. 3.01) timer2 is used as app timer */
/** Checkcondition for application timer overrun */
#define TIMER1_OVERRUN              (TIFR1 & (1U<<TOV1))

/** Clear overrun bit set for application timer */
#define CLEAR_TIMER1_OVERRUN        TIFR1 |= (1U<<TOV1)

/** Reload the application timer (Timer1) to start from 0 */
#define RELOAD_APPLICATION_TIMER()  {                                   \
        TCCR1A = (1<<WGM11);                         /* phase correct PWM, MAX=0x1FF */ \
        TCCR1B =  (1U<<CS10);                        /* no prescale , 100�sec per cycle */ \
     }

static uint8_t inline appTimerOverrun (void)
{
    static uint16_t t1cnt;
    if ( t1cnt-- ) return 0 ;
    t1cnt = F_CPU/7692;  //10MHz : 1300 * 100�sec = 130msec
	return 1;
}

#define APP_TIMER_OVERRUN()	\
		appTimerOverrun()

/** 
* Enable PWM, PWM pin (OC1A / PB1) is set by hardware.
* 
* @param x Duty-cycle = x/511 (active low)
* 
*/
#define ENABLE_PWM(x)               {                                   \
        TCCR1A |= ((1<<COM1A1)|(1<<COM1A0));        /* phase correct PWM inverted  */ \
        OCR1A = (2*x);                              /* set duty cycle, active low */ \
     }

/** Disable PWM and set PWM pin to high */
#define DISABLE_PWM()               {                                   \
          TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0)); /* disable PWM pin  */  \
          SETPIN_CTRL(ON);                      /* set port to high */   \
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
/** map uart interrupt*/
#define SIG_UART_TRANS              USART_TX_vect
/** map eeprom ready vector */
#define EE_RDY_vect                 EE_READY_vect

/*********************************/
/* MACROs for port configuration */
/*********************************/
#define IO1_PORT     PORTC      /**< output port (byte) of IO1  */
#define IO1_DDR      DDRC       /**< data direction byte of IO1 */
#define IO1_IN       PINC       /**< input port (byte) of IO1   */
#define IO1_PIN      0        /**< bit position of IO1        */

#define IO2_PORT     PORTC      /**< output port (byte) of IO2  */
#define IO2_DDR      DDRC       /**< data direction byte of IO2 */
#define IO2_IN       PINC       /**< input port (byte) of IO2   */
#define IO2_PIN      1        /**< bit position of IO2        */

#define IO3_PORT     PORTC      /**< output port (byte) of IO3  */
#define IO3_DDR      DDRC       /**< data direction byte of IO3 */
#define IO3_IN       PINC       /**< input port (byte) of IO3   */
#define IO3_PIN      2        /**< bit position of IO3        */

#define IO4_PORT     PORTC      /**< output port (byte) of IO4  */
#define IO4_DDR      DDRC       /**< data direction byte of IO4 */
#define IO4_IN       PINC       /**< input port (byte) of IO4   */
#define IO4_PIN      3       /**< bit position of IO4        */

#define IO5_PORT     PORTC      /**< output port (byte) of IO5  */
#define IO5_DDR      DDRC       /**< data direction byte of IO5 */
#define IO5_IN       PINC       /**< input port (byte) of IO5   */
#define IO5_PIN      4        /**< bit position of IO5        */

#define IO6_PORT     PORTC      /**< output port (byte) of IO6  */
#define IO6_DDR      DDRC       /**< data direction byte of IO6 */
#define IO6_IN       PINC       /**< input port (byte) of IO6   */
#define IO6_PIN      5        /**< bit position of IO6        */

#define IO7_PORT     PORTD      /**< output port (byte) of IO7  */
#define IO7_DDR      DDRD       /**< data direction byte of IO7 */
#define IO7_IN       PIND       /**< input port (byte) of IO7   */
#define IO7_PIN      6        /**< bit position of IO7        */

#define IO8_PORT     PORTD      /**< output port (byte) of IO8  */
#define IO8_DDR      DDRD       /**< data direction byte of IO8 */
#define IO8_IN       PIND       /**< input port (byte) of IO8   */
#define IO8_PIN      7        /**< bit position of IO8        */

#define PROG_PORT    PORTD      /**< output port (byte) of PROG button  */
#define PROG_DDR     DDRD       /**< data direction byte of PROG button */
#define PROG_IN      PIND       /**< input port (byte) of PROG button   */
#define PROG_PIN     4        /**< bit position of PROG button        */

#define CTRL_PORT    PORTB      /**< output port (byte) of CTRL pin     */
#define CTRL_DDR     DDRB       /**< data direction byte of CTRL pin    */
#define CTRL_IN      PINB       /**< input port (byte) of CTRL pin      */
#define CTRL_PIN     1        /**< bit position of CTRL pin           */

#define EIBOUT_PORT  PORTD      /**< output port (byte) of EIB-OUT      */
#define EIBOUT_DDR   DDRD       /**< data direction byte of EIB-OUT     */
#define EIBOUT_IN    PIND       /**< input port (byte) of EIB-OUT       */
#define EIBOUT_PIN   5        /**< bit position of EIB-OUT            */

#define EIBIN_PORT   PORTB      /**< output port (byte) of EIB-IN       */
#define EIBIN_DDR    DDRB       /**< data direction byte of EIB-IN      */
#define EIBIN_IN     PINB       /**< input port (byte) of EIB-IN        */
#define EIBIN_PIN    0        /**< bit position of EIB-IN             */


/**************************************************************************
* DECLARATIONS
**************************************************************************/
 

/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

#endif /* _FREEBUS_ATMEGA168P_H */
#endif /* _FB_HARDWARE_H */
/*********************************** EOF *********************************/
