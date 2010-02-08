/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/*
* @file   1wire.c
* @author Matthias Fechner
* @date   Mon Jan 21 15:11:38 2008
* 
* @brief  Function to access the 1-wire bus
* 
* 
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "1wire.h"
#include "freebus-debug.h"

#define XTAL 8000000L       ///< The clockrate of the microcontroller
#define DEBOUNCE 256L       ///< 

#define W1_PIN    PC3       ///< Pin on which the 1-wire device is connected on the port
#define W1_IN     PINC      ///< From which pin we read data
#define W1_OUT    PORTC     ///< Port on which the w-ire is connected
#define W1_DDR    DDRC      ///< On which port is the 1-wire device connect to change the direction between read and write to port.

#define MATCH_ROM	0x55    ///<
#define SKIP_ROM	0xCC    ///<
#define SEARCH_ROM	0xF0    ///<

#define CONVERT_T	0x44    ///< DS1820 commands
#define READ		0xBE    ///<
#define WRITE		0x4E    ///<
#define EE_WRITE	0x48    ///<
#define EE_RECALL	0xB8    ///<

#define SEARCH_FIRST 0xFF   ///< start new search
#define PRESENCE_ERR 0xFF   ///<
#define DATA_ERR	 0xFE   ///<
#define LAST_DEVICE	 0x00   ///< last device found
//			0x01 ... 0x40: continue searching

uint8_t volatile second;
uint8_t prescaler;

// prototypes
void test1Wire();
void initTimer();
void start_meas(void);
void read_meas(void);
uint8_t w1_reset(void);
uint8_t w1_bit_io(uint8_t b);
uint8_t w1_byte_wr(uint8_t b);
uint8_t w1_byte_rd(void);
uint8_t w1_rom_search(uint8_t diff, uint8_t *id);
void w1_command(uint8_t command, uint8_t *id);


/** 
* Main routine to receive data from the 1-wire device.
* 
*/
void test1Wire()
{
     initTimer();
     DEBUG_PUTS("1-Wire-Reader:");
     DEBUG_NEWLINE();
     w1_reset();
     
     second = 0;
     for(;;)
     {
          if(second==1)
          {
               start_meas();
               second=2;
          }
          if(second==3)
          {
               read_meas();
               second=0;
          }
     }
     
}

/** 
* Interrupt handler which is used to increase the variable second every second.
* 
* @return 
*/
SIGNAL (SIG_OUTPUT_COMPARE1A)
{
  uint8_t tcnt1h = TCNT1H;

  OCR1A += XTAL / DEBOUNCE;		// new compare value

  if( ++prescaler == (uint8_t)DEBOUNCE ){
    prescaler = 0;
    second++;				// exact one second over
#if XTAL % DEBOUNCE			// handle remainder
    OCR1A += XTAL % DEBOUNCE; 		// compare once per second
#endif
  }
  TCNT1H = tcnt1h;			// restore for delay() !
}

/** 
* Initialize timer 1 comparator A.
* 
*/
void initTimer()
{
     TCCR1B = (1<<CS10); // run with prescaler 1 at 8 MHz
     OCR1A = 0;
     TCNT1 = -1;
     second = 0;
     prescaler = 0;
     
     TIMSK = (1<<OCIE1A);
}

     
/** 
* Send command to 1-wire device if everything on the 1-wire bus is ok.
* 
*/
void start_meas(void)
{
     if(W1_IN & (1<<W1_PIN))
     {
          w1_command(CONVERT_T, NULL);
          W1_OUT |= 1<<W1_PIN;
          W1_DDR |= 1<<W1_PIN;
     }else
     {
          DEBUG_PUTS("Short Circuit!");
          DEBUG_NEWLINE();
     }
}

/** 
* Read response from the 1-wire device and handle errors if something wrong happened.
* 
*/
void read_meas(void)
{
     uint8_t id[8], diff;
     uint8_t i;
     uint16_t temp;
     
     for(diff=SEARCH_FIRST; diff != LAST_DEVICE; )
     {
          diff = w1_rom_search(diff, id);
          
          if(diff == PRESENCE_ERR)
          {
               DEBUG_PUTS("No sensor found");
               DEBUG_NEWLINE();
               break;
          }
          if(diff == DATA_ERR)
          {
               DEBUG_PUTS("Bus error");
               DEBUG_NEWLINE();
               break;
          }
          // check if it is a temperature sensor
          if(id[0]==0x28 || id[0]==0x10)
          {
               DEBUG_PUTS("ID: ");
               for(i=0; i<8; i++)
               {
//                    sprintf(s, "%02X ", id[i]);
                    DEBUG_PUTHEX(id[i]);
               }
               
               // read command
               w1_byte_wr(READ);
               temp=w1_byte_rd();  // read low byte
               temp |= (uint8_t)w1_byte_rd() << 8; // read high byte
               if(id[0] == 0x10)   // 9 -> 12 bit
                    temp <<= 3;
//               sprintf(s, " T: %04X = ",temp);
               DEBUG_PUTHEX(temp>>8);
               DEBUG_PUTHEX(temp);
          }
     }
}

/** 
* Reset all device on the 1-wire bus.
* 
* 
* @return 
*/
uint8_t w1_reset(void)
{
     uint8_t err;
     W1_OUT &= ~(1<<W1_PIN);
     W1_DDR |= 1<<W1_PIN;
     delay(480);
     cli();
     W1_DDR &= ~(1<<W1_PIN);
     delay(66);
     err=W1_IN & (1<<W1_PIN);  // no presence detected
     sei();
     delay(480-66);
     if((W1_IN & (1<<W1_PIN))==0)  // short curcuit
          err=1;
     return err;
}

/** 
* Write one bit to the 1-wire device.
* 
* @param b Bit to write.
* 
* @return 
*/
uint8_t w1_bit_io(uint8_t b)
{
     cli();
     W1_DDR |= 1<<W1_PIN;
     delay(1);
     if(b)
          W1_DDR &= ~(1<<W1_PIN);
     delay(15 -1);
     if( (W1_IN & (1<<W1_PIN))==0)
          b=0;
     delay(60-15);
     W1_DDR &= ~(1<<W1_PIN);
     sei();
     return b;
}

     
/** 
* Write one byte to 1-wire device using w1_bit_io dunction for this.
* 
* @param b 
* 
* @return 
*/
uint8_t w1_byte_wr(uint8_t b)
{
     uint8_t i=8, j;
     do
     {
          j=w1_bit_io( b & 1 );
          b>>=1;
          if(j)
               b|=0x80;
     }while(--i);
     return b;
}

/** 
* Read one from 1-wire device one byte.
* 
* 
* @return 
*/
uint8_t w1_byte_rd(void)
{
     return w1_byte_wr(0xFF);
}

/** 
* Search devices on 1-wire bus and store reference to 1-wire device in id.
* 
* @param diff 
* @param id 
* 
* @return 
*/
uint8_t w1_rom_search(uint8_t diff, uint8_t *id)
{
     uint8_t i,j,next_diff;
     uint8_t b;
     if( w1_reset())
          return PRESENCE_ERR; // Error no device found
     w1_byte_wr(SEARCH_ROM);   // Rom search command
     next_diff=LAST_DEVICE;    // unchanged on last device
     i=8*8;                    // 8 bytes
     do
     {
          j=8;                 // 8 bits
          do
          {
               b=w1_bit_io(1); // read bit
               if(w1_bit_io(1)) // read complement bit
               {
                    if(b)       // 11
                         return DATA_ERR;
               }else
               {
                    if(!b)      // 00 = 2 devices
                    {
                         if(diff > i || ((*id & 1) && diff != i))
                         {
                              b=1;
                              next_diff=i;
                         }
                    }
               }
               w1_bit_io(b);    // write bit
               *id>>=1;
               if(b)
                    *id|=0x80;
               i--;
          }while(--j);
          id++;
     }while(i);
     return next_diff;          // to continue search
}

/** 
* Send command to 1-wire device.
* 
* @param command 
* @param id 
*/
void w1_command(uint8_t command, uint8_t *id)
{
     uint8_t i;
     w1_reset();
     if(id)
     {
          w1_byte_wr(MATCH_ROM);   // to a single device
          i=8;
          do
          {
               w1_byte_wr(*id);
               id++;
          }while(i--);
     }else
     {
          w1_byte_wr(SKIP_ROM);   // to all devices
     }
     w1_byte_wr(command);
}

/** 
* Set the 1-wire timing.
* 
* @param standard 1=standard, 0=overdrive
*/
/*          
void setSpeed(int standard)
{
     const uint8_t clockMHZ=8;
     
     // Adjust tick values depending on speed
     if (standard)
     {
          // Standard Speed
          A = 6 * clockMHZ;
          B = 64 * clockMHZ;
          C = 60 * clockMHZ;
          D = 10 * clockMHZ;
          E = 9 * clockMHZ;
          F = 55 * clockMHZ;
          G = 0;
          H = 480 * clockMHZ;
          I = 70 * clockMHZ;
          J = 410 * clockMHZ;
     }
     else
     {
          // Overdrive Speed
          A = 1.5 * clockMHZ;
          B = 7.5 * clockMHZ;
          C = 7.5 * clockMHZ;
          D = 2.5 * clockMHZ;
          E = 0.75 * clockMHZ;
          F = 7 * clockMHZ;
          G = 2.5 * clockMHZ;
          H = 70 * clockMHZ;
          I = 8.5 * clockMHZ;
          J = 40 * clockMHZ;
     }
}
*/
/** 
* Generate a 1-Wire reset
* 
* 
* @return 1=no presence detected, 0=a device detected
*/
/*          
int OWTouchReset(void)
{
     int result;
     tickDelay(G);
     outp(PORTADDRESS,0x00); // Drives DQ low
     tickDelay(H);
     outp(PORTADDRESS,0x01); // Releases the bus
     tickDelay(I);
     result = inp(PORTADDRESS) & 0x01; // Sample for presence pulse from slave
     tickDelay(J); // Complete the reset sequence recovery
     return result; // Return sample presence pulse result
}
*/
/** 
* Send a 1-Wire write bit. Provide 10us recovery time.
* 
* @param bit 
*/
/*          
void OWWriteBit(int bit)
{
     if (bit)
     {
          // Write '1' bit
          outp(PORTADDRESS,0x00); // Drives DQ low
          tickDelay(A);
          outp(PORTADDRESS,0x01); // Releases the bus
          tickDelay(B); // Complete the time slot and 10us recovery
     }
     else
     {
          // Write '0' bit
          outp(PORTADDRESS,0x00); // Drives DQ low
          tickDelay(C);
          outp(PORTADDRESS,0x01); // Releases the bus
          tickDelay(D);
     }
}
*/
/** 
* Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.
* 
* 
* @return 
*/
/*          
int OWReadBit(void)
{
     int result;
     outp(PORTADDRESS,0x00); // Drives DQ low
     tickDelay(A);
     outp(PORTADDRESS,0x01); // Releases the bus
     tickDelay(E);
     result = inp(PORTADDRESS) & 0x01; // Sample the bit value from the slave
     tickDelay(F); // Complete the time slot and 10us recovery
     return result;
}

*/
