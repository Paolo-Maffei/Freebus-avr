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
* @brief  Lib to control a 4x20 character display with 3 wires
* the current file works only with 8Bit mode with 3 wires and a bit shifter
* called 74HC164N. It is only included if -DDEBUG_LCD is set at compile time.
* 
*/
// compile only if DEBUG_LCD is defined
#ifdef DEBUG_LCD

#ifndef _LCD_LIB_H
#define _LCD_LIB_H

#include <util/delay.h>

#ifdef _LCD_LIB_C
#define LCD_EXTERN
#else
#define LCD_EXTERN extern  ///< Header file not included from lcd-lib.c so we need to define functions as extern.
#endif

#define LCD_DATA_PORT		PORTD         ///< on which port the LCD data pin is connected
#define LCD_CLOCK_PORT		PORTD         ///< on which port the LCD clock port is connected
#define LCD_E_PORT			PORTD         ///< on which port the LCD e pin is connected

#define LCD_PIN_DATA		5             ///< on which port number is the data pin connected
#define LCD_PIN_CLOCK		6             ///< on which port number is the clock pin connected
#define LCD_PIN_E			7             ///< on which port number is the e pin connected

#define LCD_CLEAR_HOME_DELAY_MS	1.64      ///< time in ms we have to wait after a clear or home command
#define LCD_DATA_DELAY_US       40        ///< time in us we have to wait after a normal 

#define LCD_DDADR_LINE1         0x00      ///< cursor position for line 1
#define LCD_DDADR_LINE2         0x40      ///< cursor position for line 2
#define LCD_DDADR_LINE3         0x14      ///< cursor position for line 3
#define LCD_DDADR_LINE4         0x54      ///< cursor position for line 4

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

/**
* Init the LCD.
*
* @param cmd Command which should be send after initialisation is finished.
*/
LCD_EXTERN void lcd_init(unsigned char cmd);

/**
* Init the controller.
* At 8MHz function needs 65436.75 us to execute.
*
* @param cmd
*/
LCD_EXTERN void lcd_controller_init(unsigned char cmd);

/**
* Function which is called for all write proceses.
*
* @param byte the byte which should be send
* @param command 0 -> send command, 1 -> send text
*/
LCD_EXTERN void lcd_write_global(unsigned char byte, unsigned int command);

/**
 * Clear the LCD display and set the cursor to home position.
 */
LCD_EXTERN void lcd_clear(void);

/**
 * Set the cursor to home position on display.
 */
LCD_EXTERN void lcd_home(void);

/**
* Write a string to the LCD.
*
* @param s
*/
LCD_EXTERN void lcd_puts(const char *s);

/**
* Display a single character on the LCD.
* Function needs 71.38us to execute.
*
* @param c
*/
LCD_EXTERN void lcd_putc(char c);

/**
* Display a newline on the LCD display
* @todo function should be implemented
*
*/
LCD_EXTERN void lcd_newline(void);

/**
* Display a character as digit.
*
* @param d
*/
LCD_EXTERN void lcd_digit(uint8_t d);

/**
* Display a char as hex.
* Function needs 146.13us
*
* @param d
*/
LCD_EXTERN void lcd_hex(uint8_t d);

/**
* Set the cursor to defined position
*
*
* @param x column to write (0-19)
* @param y line to write (0-3)
*/
LCD_EXTERN void lcd_setcursor(uint8_t x, uint8_t y);

#define DDROFPORT(X) (*(&X - 1))            ///< macro used to switch port direction

#endif /* _LCD_LIB_H */

#endif /* DEBUG_LCD */

