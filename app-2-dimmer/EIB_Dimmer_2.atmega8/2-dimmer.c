/* $Id: fb_2-dimmer_app.c 569 2008-08-25 20:26:36Z seelaus $ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2008 Richard Weissteiner richard@seelaus.at
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *  Fuses für mega 8 low 0xE4 high 0xD9
 */
/**
* @file   2-dimmer.c
* @author 
* @date   Sun 7 9 21:30:01 2008
* 
* @brief  Dimmer mit atmega8 phasenabschnitt
*/

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/twi.h>
#include "uart_printf.c"



static unsigned char zl1=0;			///< Timer Kanal1  
static unsigned char zl2=0;			///< Timer Kanal2  
static unsigned int  zl_50hz=0;		///< Merker für triggereingang  
static unsigned char teiler=0;          ///< @todo add documentation


static unsigned char K1dimmwert_ausgang=0; ///< @todo add documentation
static unsigned char K2dimmwert_ausgang=0; ///< @todo add documentation

#define MAXZEIT 1000	///< maxzeit bis abbruch i2c


/** 
* @todo add documentation
* 
*/
void USART_Init(void)
{
	unsigned int baud=12;			// 8000000/16/BAUD-1     51  == 9600 baud 8 000 000
	UBRRH = (unsigned char)(baud>>8);	// Set baud rate
	UBRRL = (unsigned char)baud;		// Set baud rate
	UCSRB = (1<<RXEN)|(1<<TXEN);		// Enable receiver and transmitter
	//UCSR1C = (1<<UCSZ01)|(1<<UCSZ02);//Set frame format: 8data, 2stop bit
	//UCSR1B=UCSR1B |(1<<UCSZ12);		//9 bit
}

/**
 Public Function: TWIS_ResonseRequired

 Purpose: Get the response type to be performed by slave

 Input Parameter:
  	- uint8_t*	Pointer to response type
	on return:
		TWIS_ReadBytes	--> Read byte(s) from master
		TWIS_WriteBytes	--> Write byte(s) to master

 Return Value: uint8_t
  	Response required
		TRUE: Yes, response required
		FALSE: No response required

*/
unsigned char TWIS_ResonseRequired (unsigned char  *TWI_ResonseType)
{
    *TWI_ResonseType = TWSR;
    return TWCR & (1<<TWINT);
}

/**
 Public Function: TWIS_ReadAck

 Purpose: Read a byte from the master and request next byte

 Input Parameter: None

 Return Value: uint8_t
  	- uint8_t	Read byte

*/
unsigned char TWIS_ReadAck (void)
	{
	unsigned int timeueberlauf=0;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while (!(TWCR & (1<<TWINT)))
		{
		timeueberlauf++;
		if(timeueberlauf>MAXZEIT)
			return 0xff;
		}
	return TWDR;
	}
/**
 Public Function: TWIS_Stop

 Purpose: Stop the TWI Slave Interface

 Input Parameter: None

 Return Value: None

*/
void TWIS_Stop (void)
	{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO)|(1<<TWEA);
	}

/**
 Public Function: TWIS_ReadNack

 Purpose: Read the last byte from the master

 Input Parameter: None

 Return Value: uint8_t
  	- uint8_t	Read byte

*/
uint8_t	TWIS_ReadNack (void)
	{
	unsigned int timeueberlauf=0;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
		{
		timeueberlauf++;
		if(timeueberlauf>MAXZEIT)
			return 0xff;
		}
	return TWDR;
	}


/** 
* @todo add documentation
* 
*/
void nulldurchgang(void)
	{
	zl1=0;
	if(K1dimmwert_ausgang)	//dimmwert grösser 0 Dimmer  ein 
		PORTC|= (1<<PC1); 	//EIN
	zl2=0;
	if(K2dimmwert_ausgang)	//dimmwert grösser 0 Dimmer  ein 
		PORTC|= (1<<PC2); 	//EIN
	return;
	}

/** 
* @todo add documentation
* 
* @return 
*/
ISR(TIMER2_COMP_vect)
{
	if(zl_50hz<1000)
		zl_50hz++;
	if(zl_50hz==30)		//zeitversetzt nulldurchgang steigende flanke
		nulldurchgang();
	if(zl_50hz==565)		//zeitversetzt nulldurchgang fallende flanke ~255*2 wegen teiler 
		nulldurchgang();

	if(teiler<1)			 
		teiler++;
	else
		{
		teiler=0;
		if(zl1>=K1dimmwert_ausgang)				
			PORTC &=~(1<<PC1);	//aus
		else
			zl1++;
		if(zl2>=K2dimmwert_ausgang)				
			PORTC &=~(1<<PC2);	//aus
		else
			zl2++;
		}

	return;
}



/** 
* @todo add documentation
* 
* @return 
*/
ISR(INT1_vect)		//steigende flanke auswertung
{
	zl_50hz=0;
	return;
}


/** 
* The start point of the program, init all libraries, start the bus interface, the application
* and check the status of the program button.
* 
* @return 
*/
int main(void)
{
	unsigned char TWIS_ResonseType=0;
	USART_Init();
	
 	// pin eingang
	MCUCR|= (1<<ISC10)|(1<<ISC11); 	// int1 bei steigender flanke
	GICR	|= (1<<INT1); 	// ext int1 eneble
	DDRD &=~(1<<PD3);       // define PD3 as INPUT
	PORTD|= (1<<PD3);
	
	//2dimmausgänge
	DDRC |= (1<<PC1);       // define as output
	DDRC |= (1<<PC2);       // define as output
	DDRC |= (1<<PC3);       // define as output
	
	

	TCCR2 |= (1<<COM21);
	OCR2 =155; // defines reload Timer 2                       
  	TCCR2 |= (1<<CS20)|(1<<WGM21); //   CS21=clk / 8 , CTC
	TIMSK |=(1<<OCIE2); // enable timer2 Output Compare Match Interrupt

	sei();   

	TWAR = 0xa0;	//Slave adresse adresse 
	TWCR =(1<<TWEN)|(1<<TWEA);	//SLAVE mode

	uart_printf("Programmstart\n");

	while(1)
         {
		PORTC |= (1<<PC3);
		PORTC &=~(1<<PC3);
		if (TWIS_ResonseRequired (&TWIS_ResonseType))
			{
			if(TWIS_ResonseType==0x60)
				{
				TWIS_ReadAck();		//adressbyte wird nicht benötigt	
				K1dimmwert_ausgang =TWIS_ReadAck();		//K1 Dimmwert
				K2dimmwert_ausgang=TWIS_ReadNack(); 	//K2 Dimmwert
				//uart_printf("1=%x 2=%x \n",K1dimmwert_ausgang,K2dimmwert_ausgang);
				TWIS_Stop ();
				}
			}
		}
	return 0;
}
