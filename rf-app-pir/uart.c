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
 * @file
 * @author Dirk Opfer, Matthias Fechner
 * @date   Fri Oct 26 17:18:15 2007
 * 
 * @brief
 *
 * Handles communication via uart (polling mode). Is only included if compiled with -DDEBUG_UART.
 * 
 */

#include <msg_queue.h>
#include <freebus-debug.h>
#include <fb_hardware.h>
#if defined(DEBUG_UART) || defined(USE_UART)
#ifdef FB_RF
#define FF_CLOCK 10000000L    ///< use here own variable because avr studio always set F_CPU to 8000000UL so calculation will fail
#else
#define FF_CLOCK 8000000L
#endif
#define BAUD 38400L          ///< baud rate, the L at the end is important, DO NOT use UL!

#include <util/setbaud.h>

/** used to display hexvalues */
static const uint8_t pgmhex[] PROGMEM = {'0', '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

#define DEBUG_MSG_MAX_LEN MSG_MAX_DATA_LENGTH-2      ///< max length a debug message can have

/** Data is written in background to not block other operations,
 * data and control bytes are stored in that structure */
struct uart_cmds {
    uint8_t count;      ///< number of elements in data
    uint8_t offset;     ///< position of element write next
    uint8_t data[DEBUG_MSG_MAX_LEN];   ///< data
};

static struct msg *uart_msg;    ///< queue for debug messages
static uint8_t AppendToQueue(struct msg *current, uint8_t character);

/** 
 * Set the baudrate and configure the RS232 interface.
 * 
 */
void uart_init(void)
{
    // UART
#if defined(__AVR_ATmega8__)
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;			// setup baud rate

    //     UCSRB |= (1<<TXCIE);               // enable transmit IRQ
	UCSRB |= _BV(TXEN);                // enable UART TX
	UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);    // Asynchron 8N1 
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega328P__)
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;           // setup baud rate

	UCSR0B = (1<<TXEN0);                // enable UART TX
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);    // Asynchron 8N1 
#endif
    // configure debug pins
    PORTD |= ((1<<5) | (1<<6));    // enable pull-up resistor
    DDRD |= ((1<<5) | (1<<6));     // define as output pins
    //     DEBUG_PIN1_OFF();
    //     DEBUG_PIN2_OFF();
	return;
}

/** 
 * ISR is called if rs232 is empty and can store the next byte.
 * 
 * @return 
 */
ISR(SIG_UART_TRANS)
{
    struct uart_cmds *udat = (struct uart_cmds*) uart_msg->data;
    if(udat->count == udat->offset) {
        // finished
        dequeuemsg(&uart_msg);
        if(!uart_msg) {
            DISABLE_UART_TX_IRQ();
            return;
        }
        udat = (struct uart_cmds*) uart_msg->data;
    }
     
    // now send one byte at every ISR
    UART_SEND_BYTE(udat->data[udat->offset]);

    udat->offset++;
    return;
}

/** 
 * Start the send progress and write all queued data to rs232 interface.
 * Trigger here the first byte and enable ISQ for sending.
 * 
 */
static inline void trigger_uart(void)
{
    if(uart_msg) {
        if(uart_msg->state != MSG_SENDING) {
            /* trigger uart interrupt */
            ENABLE_UART_TX_IRQ();
            uart_msg->state = MSG_SENDING;
            struct uart_cmds *udat = (struct uart_cmds*) uart_msg->data;
            // move first byte to uart transmit buffer
            UART_SEND_BYTE(udat->data[0]);
             
            udat->offset++;
        }
    }
}

/**
 * Write a character to rs232 without using the queueing system.
 * 
 * @param c 
 */
void uart_putc_blocking(unsigned char c)
{
#if defined(__AVR_ATmega8__)
    while (!(UCSRA & _BV(UDRE)))  // warten bis Senden moeglich
    	{
    	}
    UDR = c;                    // schreibt das Zeichen in c auf die Schnittstelle
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega168P__)
    while (!(UCSR0A & _BV(UDRE0)))  // warten bis Senden moeglich
    	{
    	}
    UDR0 = c;                    // schreibt das Zeichen in c auf die Schnittstelle
#endif
}

/** 
 * Write a string to the rs232 without using the queueing system.
 * 
 * @param s 
 */
void uart_puts_blocking(const char *s)
{
    register char c;
    while((c=pgm_read_byte(s++)))
        uart_putc_blocking(c);
}

/** 
 * Write a character as hex to rs232 without using the queueing system.
 * 
 * @param h 
 */
void uart_hex_blocking(unsigned char h)
{
    uart_putc_blocking(pgm_read_byte(&pgmhex[h>>4]));
	uart_putc_blocking(pgm_read_byte(&pgmhex[h&0x0f]));
    return;
}

/** 
 * Write a newline to rs232 without using the queueing system.
 * 
 */
void uart_newline_blocking(void)
{
    uart_putc_blocking('\n');
	uart_putc_blocking('\r');
    return;
}

/** 
 * Queue a character and start background sending to rs232.
 * 
 * @param c 
 */
void uart_putc(unsigned char c)
{
    DISABLE_IRQS;
    if(!AppendToQueue(uart_msg, c)) {
        ENABLE_IRQS;
        struct msg *uart=AllocMsg();
        struct uart_cmds *udat = (struct uart_cmds*) uart->data;
          
        udat->count = 1;
        udat->offset=0;
        memcpy(udat->data, &c, 1);
          
        queuemsg(&uart_msg, uart, trigger_uart);
    } else {
        ENABLE_IRQS;
    }
     
	return;
}

/** 
 * Display string on rs232. The strings are saved on flash and not in sram.
 * See here definition of DEBUG_PUTS on freebus-debug.h.
 * 
 * @param s String pointer which points to flash
 */
void uart_puts(const char *s)
{
    char c;
    while((c = pgm_read_byte(s++))) {
        uart_putc(c);
    }
    return;
}

/** 
 * Queue a character and write it as a hex value to rs232.
 * 
 * @param h Character to write as hex value
 */
void uart_hex(unsigned char h)
{
    uart_putc(pgm_read_byte(&pgmhex[h>>4]));
	uart_putc(pgm_read_byte(&pgmhex[h&0x0f]));
	return;
}

/** 
 * Queue a newline and write it to the rs232.
 * 
 */
void uart_newline(void)
{
    uart_putc('\n');
    uart_putc('\r');
    return;
}


/** 
* Write the given binary data to rs232 using the queueing system.
* 
*/
void uart_put(unsigned char* dat, unsigned char len)
{

     struct msg *uart=AllocMsg();
     struct uart_cmds *udat = (struct uart_cmds*) uart->data;

     udat->count = len;
     udat->offset=0;
     memcpy(udat->data, dat, len);

     queuemsg(&uart_msg, uart, trigger_uart);
}

/** 
 * Take the given char and append it to a debug message queue (return 1).
 * If 0 is return new queue is existing or it is full, you must create the queue outside of this funtion.
 * It is important that IRQs are disabled and enabled after the function to protect the message queue of unwanted touches while be in the function!
 *
 * That function was written to use the memory for debugging message more efficient.
 * 
 * @param current Pointer to the message queue
 * @param character Pointer to characters to store in the message queue
 * 
 * @return 
 */
static uint8_t AppendToQueue(struct msg *current, uint8_t character)
{
    // go through all mesages stored in uart_msg
    if(current) {
        struct uart_cmds *udat = (struct uart_cmds*) current->data;
        // check if enough space is left to append message and if the message is not finished
        if(DEBUG_MSG_MAX_LEN - udat->count >= 1 && udat->count >= udat->offset) {
            // if yes append character there and increase count
            udat->data[udat->count] = character;
            udat->count++;
            return 1;
        } else {
            return(AppendToQueue(current->next, character));
        }
    }
    return 0;
}
#endif
