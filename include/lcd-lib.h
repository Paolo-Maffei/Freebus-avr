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
* @file   lcd-lib.h
* @author Matthias Fechner <idefix@fechner.net>
* @date   Mon Nov 26 22:41:50 2007
* 
* @brief  
* 
* 
*/
// compile only if DEBUG_LCD is defined
#ifdef DEBUG_LCD

#ifndef _LCD_LIB_H
#define _LCD_LIB_H

#include "delay.h"

#define LCD_DATA_PORT		PORTD         ///< on which port the LCD data pin is connected
#define LCD_CLOCK_PORT		PORTD         ///< on which port the LCD clock port is connected
#define LCD_E_PORT			PORTD         ///< on which port the LCD e pin is connected

#define LCD_PIN_DATA		5             ///< on which port number is the data pin connected
#define LCD_PIN_CLOCK		6             ///< on which port number is the clock pin connected
#define LCD_PIN_E			7             ///< on which port number is the e pin connected

#define LCD_ON				0x0C          ///< switch LCD on
#define LCD_OFF			    0x08          ///< switch LCD off
#define LCD_CURSOR_ON		0x0A          ///< switch cursor on LCD on
#define LCD_CURSOR_OFF		0x08          ///< switch cursor on LCD off
#define LCD_CURSOR_BLINK	0x09          ///< set cursor to blink mode
#define LCD_CURSOR_NOBLINK	0x08          ///< disable blink mode of cursor

#define LCD_CLEAR			0x01          ///< clear the LCD
#define LCD_HOME			0x02          ///< jump to home position on LCD

extern void lcd_init(unsigned char cmd);
extern void lcd_controller_init(unsigned char cmd);
extern void lcd_write_global(unsigned char byte, unsigned int);
void lcd_puts(const char *s);
void lcd_putc(char c);
void lcd_newline(void);
void lcd_digit(uint8_t d);
void lcd_hex(uint8_t d);

#define DDROFPORT(X) (*(&X - 1))            ///< macro used to switch port direction

#endif /* _LCD_LIB_H */

#endif /* DEBUG_LCD */

