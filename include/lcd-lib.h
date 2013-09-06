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

#include <util/delay.h>

#ifndef _LCD_LIB_C
#define LCD_EXTERN
#else
#define LCD_EXTERN extern
#endif

#define LCD_DATA_PORT		PORTD         ///< on which port the LCD data pin is connected
#define LCD_CLOCK_PORT		PORTD         ///< on which port the LCD clock port is connected
#define LCD_E_PORT			PORTD         ///< on which port the LCD e pin is connected

#define LCD_PIN_DATA		5             ///< on which port number is the data pin connected
#define LCD_PIN_CLOCK		6             ///< on which port number is the clock pin connected
#define LCD_PIN_E			7             ///< on which port number is the e pin connected

#define LCD_CLEAR_HOME_DELAY_MS	1.64      ///< time in ms we have to wait after a clear or home command
#define LCD_DATA_DELAY_US       40        ///< time in us we have to wait after a normal 

#define LCD_DDADR_LINE1         0x00
#define LCD_DDADR_LINE2         0x40
#define LCD_DDADR_LINE3         0x14
#define LCD_DDADR_LINE4         0x54

// Init command to set mode of display
#define LCD_8_BIT_MODE      0x30          ///< set display to 8-bit mode
#define LCD_4_BIT_MODE      0x20          ///< set display to 4-bit mode
#define LCD_2_OR_4_LINES    0x28          ///< set display to 2/4 line mode
#define LCD_1_LINES         0x20          ///< set display to 1 line mode
#define LCD_5_10_DOTS		0x24          ///< set display to 5x10 dots
#define LCD_5_7_DOTS        0x20          ///< set display to 5x7 dots

// Normal commands
#define LCD_ON				0x0C          ///< switch LCD on
#define LCD_OFF			    0x08          ///< switch LCD off
#define LCD_CURSOR_ON		0x0A          ///< switch cursor on LCD on
#define LCD_CURSOR_OFF		0x08          ///< switch cursor on LCD off
#define LCD_CURSOR_BLINK	0x09          ///< set cursor to blink mode
#define LCD_CURSOR_NOBLINK	0x08          ///< disable blink mode of cursor
#define LCD_SET_DDADR       0x80          ///< set DD address, required to write to a specific position on the display
#define LCD_CLEAR			0x01          ///< clear the LCD
#define LCD_HOME			0x02          ///< jump to home position on LCD

LCD_EXTERN void lcd_init(unsigned char cmd);
LCD_EXTERN void lcd_controller_init(unsigned char cmd);
LCD_EXTERN void lcd_write_global(unsigned char byte, unsigned int);
LCD_EXTERN void lcd_clear(void);
LCD_EXTERN void lcd_home(void);
LCD_EXTERN void lcd_puts(const char *s);
LCD_EXTERN void lcd_putc(char c);
LCD_EXTERN void lcd_newline(void);
LCD_EXTERN void lcd_digit(uint8_t d);
LCD_EXTERN void lcd_hex(uint8_t d);
LCD_EXTERN void lcd_setcursor(uint8_t x, uint8_t y);

#define DDROFPORT(X) (*(&X - 1))            ///< macro used to switch port direction

#endif /* _LCD_LIB_H */

#endif /* DEBUG_LCD */

