/* $Id$ */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
*  /_/   /_/ |_/_____/_____/_____/\____//____/
*
*  Copyright (c) 2012 Dirk Opfer <dirk@do13.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*
* @brief  Hardware specific options for the ATmega644p.
* DO NOT INCLUDE THAT FILE DIRECTLY, include fb_hardware.h instead.
*
*
*/

#if defined(_FB_HARDWARE_H)
#ifndef _FREEBUS_ATMEGA644P_H
#define _FREEBUS_ATMEGA644P_H ///< is defined if target system is the ATMega644P

/*************************************************************************
* INCLUDES
*************************************************************************/
#include <string.h>

/**************************************************************************
* DEFINITIONS
**************************************************************************/

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

/* TPUART specific, use TPUART connected via USART1*/
#define TPUART_RX_VEC		USART1_RX_vect
#define TPUART_UCSRA		UCSR1A
#define TPUART_FE			FE1
#define TPUART_DOR			DOR1
#define TPUART_TXC			TXC1

#define TPUART_UCSRB		UCSR1B
#define TPUART_UDR			UDR1
#define TPUART_UBRRL		UBRR1L
#define TPUART_UBRRH		UBRR1H
#define TPUART_TXT_DDR		TXD1_BIT
#define TPUART_TXD_PORT		TXD1_PORT
#define TPUART_RXEN			RXEN1
#define TPUART_RXCIE		RXCIE1
#define TPUART_TXEN			TXEN1
#define TPUART_TXCIE		TXCIE1
#define TPUART_UCSRC		UCSR1C
#define TPUART_UDRE			UDRE1
#define TPUART_U2X			U2X1

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

#endif /* _FREEBUS_ATMEGA644P_H */
#endif /* _FB_HARDWARE_H */
/*********************************** EOF *********************************/
