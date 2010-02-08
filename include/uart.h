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
* @file   uart.h
* @author Dirk Opfer, Matthias Fechner
* @date   Fri Oct 26 17:18:15 2007
* 
* @brief
*
* Handles communication via uart (polling mode)
* 
*/
#ifdef DEBUG_UART

#ifndef _UART_H
#define _UART_H

extern void uart_init(void);
extern void uart_putc(unsigned char c);
extern void uart_puts(const char *s);
extern void uart_hex(unsigned char h);
extern void uart_newline(void);
extern void uart_putc_blocking(unsigned char c);
extern void uart_hex_blocking(unsigned char c);
extern void uart_puts_blocking(const char *s);
extern void uart_newline_blocking(void);
#endif /* _UART_H */

#endif /* DEBUG_UART */




