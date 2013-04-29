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
#define UART_EXT extern
#endif

UART_EXT void uart_init(void);
UART_EXT void uart_open(uint32_t baud, uint8_t charsize, char parity, uint8_t stopb);
UART_EXT void uart_rx_flush(void);
UART_EXT void uart_rx_enable(void);
UART_EXT void uart_rx_disable(void);
UART_EXT uint8_t uart_rx_empty(void);
UART_EXT uint8_t uart_rx_get(void);
UART_EXT uint8_t uart_rx_cnt(void);
UART_EXT void uart_putc(unsigned char c);
UART_EXT void uart_puts(const char *s);
UART_EXT void uart_hex(unsigned char h);
UART_EXT void uart_newline(void);
UART_EXT void uart_putc_blocking(unsigned char c);
UART_EXT void uart_hex_blocking(unsigned char c);
UART_EXT void uart_puts_blocking(const char *s);
UART_EXT void uart_newline_blocking(void);
UART_EXT void uart_put(unsigned char* dat, unsigned char len);
#endif /* _UART_H */

#endif /* DEBUG_UART */




