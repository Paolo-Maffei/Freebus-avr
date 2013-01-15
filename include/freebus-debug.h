/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2007 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/**
* @file   freebus-debug.h
* @author Matthias Fechner
* @date   Sun Nov 18 19:01:35 2007
* 
* @brief  Header file to map debugging between several output devices.
* Possible outputs are lcd, uart and no debugging.
* To select the correct output define the compile variable
* DEBUG_LCD for lcd output
* DEBUG_UART for uart output
* HARDWARETEST for a hardware test
* DEBUG_TIMING to test bus timing (IO1-IO8 is used to set pin high and low to see what is called)
* SENDTESTTEL if prog button is clicked a test telegram will be send
*
* If none of them are defined, debugging messages are disabled.
*
* The following debugging pins can be used, enable them with DEBUG_TIMING:
* - DEBUG_PIN1 = PB2
* - DEBUG_PIN2 = PC1
* - DEBUG_PIN3 = PD3
* - DEBUG_PIN4 = PD5
* - DEBUG_PIN5 = PD6
* - DEBUG_PIN6 = PD7
* - DEBUG_PIN7 = PC2
* - DEBUG_PIN8 = PC3
* 
*/
#ifndef _FREEBUS_DEBUG_H
#define _FREEBUS_DEBUG_H

#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

#ifdef DEBUG_LCD
#include "lcd-lib.h"
/** Init the device for debugging */
#define DEBUG_INIT(x) lcd_init(LCD_CLEAR)
/** Display a character on the device */
#define DEBUG_PUTC(x) lcd_putc(x)
/** Display a character on the device without using the queueing system */
#define DEBUG_PUTC_BLOCKING(x)
/** Display a string on the device */
#define DEBUG_PUTS(x) lcd_puts(x)
/** Display a string on the device without using the queueing system */
#define DEBUG_PUTS_BLOCKING(x)
/** Display a digit on the device */
#define DEBUG_DIGIT(x) lcd_digit(x)
/** Display a character as hex value on the device */
#define DEBUG_PUTHEX(x) lcd_hex(x)
/** Display a character as hex value on the device without using the queueing system */
#define DEBUG_PUTHEX_BLOCKING(x)
/** Display a newline on the device */
#define DEBUG_NEWLINE(x) lcd_putc(' ')
/** Display a newline on the device without using the queueing system */
#define DEBUG_NEWLINE_BLOCKING()
/** Display a space on the device */
#define DEBUG_SPACE() 
/** Display a space on the device without using the queueing system */
#define DEBUG_SPACE_BLOCKING()

#elif defined DEBUG_UART
#include "uart.h"
/** Init the device for debugging */
#define DEBUG_INIT(x) uart_init()
/** Display a character on the device */
#define DEBUG_PUTC(x) uart_putc(x)
/** Display a character on the device without using the queueing system */
#define DEBUG_PUTC_BLOCKING(x) uart_putc_blocking(x)
/** Display a string on the device */
#define DEBUG_PUTS(x) uart_puts(PSTR(x))
/** Display a string on the device without using the queueing system */
#define DEBUG_PUTS_BLOCKING(x) uart_puts_blocking(PSTR(x))
/** Display a character as hex value on the device */
#define DEBUG_PUTHEX(x) uart_hex(x)
/** Display a character as hex value on the device without using the queueing system */
#define DEBUG_PUTHEX_BLOCKING(x) uart_hex_blocking(x)
/** Display a newline on the device */
#define DEBUG_NEWLINE() uart_newline()
/** Display a newline on the device without using the queueing system */
#define DEBUG_NEWLINE_BLOCKING() uart_newline_blocking()
/** Display a space on the device */
#define DEBUG_SPACE() uart_putc(' ')
/** Display a space on the device without using the queueing system */
#define DEBUG_SPACE_BLOCKING() uart_putc_blocking(' ')
#define DEBUG_MSG(descr, msg) do { \
	uint8_t i; \
	DEBUG_PUTS(descr); \
	for (i=0;i<msg->len;i++) \
		DEBUG_PUTHEX(msg->data[i]); \
	DEBUG_NEWLINE(); \
} while (0) 

#else
/** No debugging map to nothing */
#define DEBUG_INIT(x)
/** No debugging map to nothing */
#define DEBUG_PUTC(x)
/** No debugging map to nothing */
#define DEBUG_PUTC_BLOCKING(x)
/** No debugging map to nothing */
#define DEBUG_PUTS(x)
/** No debugging map to nothing */
#define DEBUG_PUTS_BLOCKING(x)
/** No debugging map to nothing */
#define DEBUG_PUTDIGIT(x)
/** No debugging map to nothing */
#define DEBUG_PUTHEX(x)
/** No debugging map to nothing */
#define DEBUG_PUTHEX_BLOCKING(x)
/** No debugging map to nothing */
#define DEBUG_NEWLINE(x)
/** No debugging map to nothing */
#define DEBUG_NEWLINE_BLOCKING()
/** No debugging map to nothing */
#define DEBUG_SPACE() 
/** No debugging map to nothing */
#define DEBUG_SPACE_BLOCKING()
/** No debugging map to nothing */
#define DEBUG_MSG(x,msg)
#endif

#ifdef DEBUG_TIMING
#define DEBUG_PIN1_ON(x)            SETPIN_IO1(1)     /** Set debugging pin 1 (PB2) to high (set if start bit received)*/
#define DEBUG_PIN1_OFF(x)           SETPIN_IO1(0)     /** Set debugging pin 1 (PB2) to low  */
#define DEBUG_PIN2_ON(x)            SETPIN_IO2(1)     /** Set debugging pin 2 (PC1) to high (set while in interrupt handler) */
#define DEBUG_PIN2_OFF(x)           SETPIN_IO2(0)     /** Set debugging pin 2 (PC1) to low  */
#define DEBUG_PIN3_ON(x)            SETPIN_IO3(1)     /** Set debugging pin 3 (PD3) to low  (set while timer-reading bits) */
#define DEBUG_PIN3_OFF(x)           SETPIN_IO3(0)     /** Set debugging pin 3 (PD3) to low  */
#define DEBUG_PIN4_ON(x)            SETPIN_IO4(1)     /** Set debugging pin 4 (PD5) to high (set while read 35us data range) */
#define DEBUG_PIN4_OFF(x)           SETPIN_IO4(0)     /** Set debugging pin 4 (PD5) to low  */
#define DEBUG_PIN5_ON(x)            SETPIN_IO5(1)     /** Set debugging pin 5 (PD6) to high (set while working on telegram) */
#define DEBUG_PIN5_OFF(x)           SETPIN_IO5(0)     /** Set debugging pin 5 (PD6) to low  */
#define DEBUG_PIN6_ON(x)            SETPIN_IO6(1)     /** Set debugging pin 6 (PD7) to high (set while setting up till bus is free again) */
#define DEBUG_PIN6_OFF(x)           SETPIN_IO6(0)     /** Set debugging pin 6 (PD7) to low  */
#define DEBUG_PIN7_ON(x)            SETPIN_IO7(1)     /** Set debugging pin 7 (PC2) to high (received no bit in 5.1ms free bus) */
#define DEBUG_PIN7_OFF(x)           SETPIN_IO7(0)     /** Set debugging pin 7 (PC2) to low  */
#define DEBUG_PIN8_ON(x)            SETPIN_IO8(1)     /** Set debugging pin 8 (PC3) to high */
#define DEBUG_PIN8_OFF(x)           SETPIN_IO8(0)     /** Set debugging pin 8 (PC3) to low  */
#else
#define DEBUG_PIN1_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN1_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN2_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN2_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN3_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN3_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN4_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN4_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN5_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN5_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN6_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN6_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN7_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN7_OFF(x)                             /** No debugging map to nothing */
#define DEBUG_PIN8_ON(x)                              /** No debugging map to nothing */
#define DEBUG_PIN8_OFF(x)                             /** No debugging map to nothing */

#endif

#endif /* _FREEBUS_DEBUG_H */
