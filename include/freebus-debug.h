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
* DEBUG_BUSTIMING to test bus timing
* SENDTESTTEL if prog button is clicked a test telegram will be send
*
* If none of them are defined, debugging messages are disabled.
*
* The following debugging pins can be used:
* - DEBUG_PIN1 = PORTD, PD5
* - DEBUG_PIN2 = PORTD, PD6
* - DEBUG_PIN3 = PORTD, PD7
* - DEBUG_PIN4 = PORTC, PC2
* - DEBUG_PIN5 = PORTC, PC3
* 
*
* @bug If UART_DEBUG is enabled, a delay (about 1 second) by turning on switches occurs!
*/
#ifndef _FREEBUS_DEBUG_H
#define _FREEBUS_DEBUG_H

#include <avr/pgmspace.h>

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
/** Set debugging pin 1 to high */
#define DEBUG_PIN1_ON(x) PORTD |= (1<<5)
/** Set debugging pin 1 to low */
#define DEBUG_PIN1_OFF(x) PORTD &= ~(1<<5)
/** Set debugging pin 2 to high */
#define DEBUG_PIN2_ON(x) PORTD |= (1<<6)
/** Set debugging pin 2 to low */
#define DEBUG_PIN2_OFF(x) PORTD &= ~(1<<6)

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
/** Set debugging pin 1 (PD5) to high */
#define DEBUG_PIN1_ON(x) PORTD |= (1<<5)
/** Set debugging pin 1 (PD5) to low */
#define DEBUG_PIN1_OFF(x) PORTD &= ~(1<<5)
/** Set debugging pin 2 (PD6) to high */
#define DEBUG_PIN2_ON(x) PORTD |= (1<<6)
/** Set debugging pin 2 (PD6) to low */
#define DEBUG_PIN2_OFF(x) PORTD &= ~(1<<6)
/** Set debugging pin 3 (PD7) to high */
#define DEBUG_PIN3_ON(x) PORTD |= (1<<7)
/** Set debugging pin 3 (PD7) to low */
#define DEBUG_PIN3_OFF(x) PORTD &= ~(1<<7)
/** Set debugging pin 4 (PC2) to high */
#define DEBUG_PIN4_ON(x) PORTC |= (1<<2)
/** Set debugging pin 4 (PC2) to low */
#define DEBUG_PIN4_OFF(x) PORTC &= ~(1<<2)
/** Set debugging pin 5 (PC3) to high */
#define DEBUG_PIN5_ON(x) PORTC |= (1<<3)
/** Set debugging pin 5 (PC3) to low */
#define DEBUG_PIN5_OFF(x) PORTC &= ~(1<<3)

#elif defined DEBUG_BUSTIMING
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
#define DEBUG_PIN1_ON(x)            SETPIN_RES1_ON
#define DEBUG_PIN1_OFF(x)           SETPIN_RES1_OFF
#define DEBUG_PIN2_ON(x)            SETPIN_RES2_ON
#define DEBUG_PIN2_OFF(x)           SETPIN_RES2_OFF
#define DEBUG_PIN3_ON(x)            SETPIN_RES3_ON
#define DEBUG_PIN3_OFF(x)           SETPIN_RES3_OFF
/** Set debugging pin 4 (PC2) to high */
#define DEBUG_PIN4_ON(x)            SETPIN_RES4_ON
/** Set debugging pin 4 (PC2) to low */
#define DEBUG_PIN4_OFF(x)           SETPIN_RES4_OFF
/** Set debugging pin 5 (PC3) to high */
#define DEBUG_PIN5_ON(x)
/** Set debugging pin 5 (PC3) to low */
#define DEBUG_PIN5_OFF(x)

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
#define DEBUG_PIN1_ON(x)
/** No debugging map to nothing */
#define DEBUG_PIN1_OFF(x)
/** No debugging map to nothing */
#define DEBUG_PIN2_ON(x)
/** No debugging map to nothing */
#define DEBUG_PIN2_OFF(x)
/** Set debugging pin 3 (PD7) to high */
#define DEBUG_PIN3_ON(x)
/** Set debugging pin 3 (PD7) to low */
#define DEBUG_PIN3_OFF(x)
/** Set debugging pin 4 (PC2) to high */
#define DEBUG_PIN4_ON(x)
/** Set debugging pin 4 (PC2) to low */
#define DEBUG_PIN4_OFF(x)
/** Set debugging pin 5 (PC3) to high */
#define DEBUG_PIN5_ON(x)
/** Set debugging pin 5 (PC3) to low */
#define DEBUG_PIN5_OFF(x)
#endif

#endif /* _FREEBUS_DEBUG_H */
