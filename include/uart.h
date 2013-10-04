/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
 *  /_/   /_/ |_/_____/_____/_____/\____//____/
 *
 *  Freebus Hardware Layer. Uart functions
 *
 *  Copyright (c) 2007 Dirk Opfer <dirk@do13.de>
 *  Copyright (c) 2007 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/**
* @file   include/uart.h
* @author Dirk Opfer, Matthias Fechner
* @date   Fri Oct 26 17:18:15 2007
*
* @brief
*
* Handles communication via uart (polling mode)
*
*/
#if defined(DEBUG_UART) | defined(USE_UART)

#ifndef _UART_H
#define _UART_H

#ifdef _UART_C
#define UART_EXT
#else
#define UART_EXT extern ///< Header file not included from uart.c so we need to define functions as extern.
#endif

#if defined(DEBUG_UART) || defined(USE_UART)
/**
 * Use standard baudrate 38400, 8 character, Parity disabled and one stop bit.
 *
 */
UART_EXT void uart_init(void) XBOOT_SECTION;

/**
* Initialize the uart with the given parameters
*
* @param baud Define the baud rate: 4800, 9600, 19200, 38400, 57600, 115200
* @param charsize Character size: 7, 8
* @param parity Define the parity: N=disabled, E=even, O=odd
* @param stopb 1=One stopbit, 2=two stopbits
* @return void
*/
UART_EXT void uart_open(uint32_t baud, uint8_t charsize, char parity, uint8_t stopb) XBOOT_SECTION;

/**
 * Flush rx buffer
 *
 */
UART_EXT void uart_rx_flush(void);

/**
 * Enable rx and rx irq
 *
 */
UART_EXT void uart_rx_enable(void) XBOOT_SECTION;

/**
 * Disable rx and rx irq
 *
 */
UART_EXT void uart_rx_disable(void);

/**
 * Returns true if rx buffer is empty
 *
 */
UART_EXT uint8_t uart_rx_empty(void);

/**
 * Gets on byte from rx buffer
 *
 */
UART_EXT uint8_t uart_rx_get(void);

/**
 * Returns number of bytes in rx buffer
 *
 */
UART_EXT uint8_t uart_rx_cnt(void);

/**
 * Queue a character and start background sending to rs232.
 *
 * @param c
 */
UART_EXT void uart_putc(unsigned char c);

/**
 * Display string on rs232. The strings are saved on flash and not in sram.
 * See here definition of DEBUG_PUTS on freebus-debug.h.
 *
 * @param s String pointer which points to flash
 */
UART_EXT void uart_puts(const char *s);

/**
 * Queue a character and write it as a hex value to rs232.
 *
 * @param h Character to write as hex value
 */
UART_EXT void uart_hex(unsigned char h);

/**
 * Queue a newline and write it to the rs232.
 *
 */
UART_EXT void uart_newline(void);

/**
 * Write a character to rs232 without using the queueing system.
 *
 * @param c
 */
UART_EXT void uart_putc_blocking(unsigned char c);

/**
 * Write a character as hex to rs232 without using the queueing system.
 *
 * @param h
 */
UART_EXT void uart_hex_blocking(unsigned char c);

/**
 * Write a string to the rs232 without using the queueing system.
 *
 * @param s
 */
UART_EXT void uart_puts_blocking(const char *s);

/**
 * Write a newline to rs232 without using the queueing system.
 *
 */
UART_EXT void uart_newline_blocking(void);

#endif

/**
* Write the given binary data to rs232 using the queueing system.
*
*/
UART_EXT void uart_put(unsigned char* dat, unsigned char len);

#endif /* _UART_H */

#endif /* DEBUG_UART */




