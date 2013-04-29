/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
 *  /_/   /_/ |_/_____/_____/_____/\____//____/
 *
 *  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
 *  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/**
 * @file	fb_ain10v.c
 * @author	Gerald Eichler, Uwe S.
 * @date	30.03.2013
 *
 * @brief
 * to use the 09609110.vd1 Analog-Sensorschnittstelle 4-fach
 * at the moment only for 0-10V !!
 *
 * This version is designed to be used with the new API.
 */
#ifndef _AIN10VNEWLIB_C
#define _AIN10VNEWLIB_C

/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb_ain10v.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
static const timer_t delay_bases[] PROGMEM   ={ (timer_t)1*M2TICS(130),          // 130 msec
                                                (timer_t)2*M2TICS(130),          // 260 msec
                                                (timer_t)4*M2TICS(130),          // 520 msec
                                                (timer_t)8*M2TICS(130),          // 1 sec
                                                (timer_t)16*M2TICS(130),         // 2,1 sec
                                                (timer_t)32*M2TICS(130),         // 4,2 sec
                                                (timer_t)64*M2TICS(130),         // 8.4 sec
                                                (timer_t)128*M2TICS(130),        // 17 sec
                                                (timer_t)256*M2TICS(130),        // 34 sec
                                                (timer_t)512*M2TICS(130),        // 1.1 min
                                                (timer_t)1024*M2TICS(130),       // 2.2 min
                                                (timer_t)2048*M2TICS(130),       // 4.5 min
                                                (timer_t)4096*M2TICS(130),       // 9 min
                                                (timer_t)8192*M2TICS(130),       // 18 min
                                                (timer_t)16384*M2TICS(130),      // 35 min
                                                (timer_t)32768*M2TICS(130)};     // 1.2 h

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

struct {
    timer_t cyclTimerMeasurement[4];							/// stores timer value for cyclic sending Meßwerte
    uint8_t runningCyclTimerMeasurement[4];						/// bitfield for timer active flags
	timer_t cyclTimerLimit[4];									/// stores timer value for cyclic sending Grenzwerte
	uint8_t runningCyclTimerLimit[4];							/// bitfield for timer active flags
	timer_t cyclTimerDifference[4];								/// stores timer value for cyclic sending Meßwertdifferenz
	uint8_t runningCyclTimerDifference[4];						/// bitfield for timer active flags
} app_dat;

uint32_t adcAverage;											// Zwischenspeicher für Mittelwertbildung
uint16_t adcAverageCount;										// Zähler für Mittelwertbildung
uint16_t channelValue[4],lastLimitValue[4],lastsentValue[4];
uint8_t firstLimitCheck;
uint8_t limitReaction[4];
uint8_t delay_base;
unsigned char channelIndex;										// Kanalnummer für A/D Wandlung
uint8_t firstConversionStatus = 0;								//0=Start, 4=alle Kanäle haben einen Messwert


/**************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/

void configureAdc(void);
inline void doAdcMeasurement(uint8_t channel);
void handleResults(uint8_t channel);
void setPowerOnTimer(uint8_t channel);
void checkCyclicTimers(uint8_t channel);
void checkMeasurementDifference(uint8_t channel);
uint8_t checkLimits(uint8_t channel);
void sendLimits(uint8_t channel);
void sendValue(uint8_t channel);
unsigned char sendDPT9 (uint8_t objno, int32_t value);

/**************************************************************************
 * IMPLEMENTATION
 **************************************************************************/

/**
* Function is called when microcontroller gets power or if the application must be restarted.
* It restores data like in the parameters defined.
*
* @return FB_ACK or FB_NACK
*/
uint8_t restartApplication(void)
{
	delay_base = mem_ReadByte(0x0160);
	delay_base &= ~0xF0;
	configureAdc();
	channelIndex = 0;
	doAdcMeasurement(channelIndex); // start new measurement
	return FB_ACK;
}


/**
* Function os called periodically of the application is enabled in the system_state
*
*/
void app_loop()
{
	/* calm the watchdog */
	//wdt_reset();
	if (!(ADCSRA & (1<<ADSC)))											// neuer Wert vom A/D Wandler
	{
		adcAverage += ADCW;												// Meßwert für Mittelwertbildung aufsummieren
		adcAverageCount++;
		if (adcAverageCount == 1000)									// nach 1000 Messungen
		{
			channelValue[channelIndex] = adcAverage/adcAverageCount;	// Mittelwert bilden
			adcAverage=0;
			adcAverageCount=0;
			if (firstConversionStatus <= 3)	setPowerOnTimer(channelIndex);			// Meßwert erstmals vorhanden				
			handleResults(channelIndex);
			if (firstConversionStatus < 9)	firstConversionStatus++;
			channelIndex++;												//
			channelIndex = channelIndex & 0x03;							// only from 0 to 3 !!
		}
		doAdcMeasurement(channelIndex);									// start new measurement
	}
}


/**
* Meßwerte weiterverarbeiten
* Funktion wird ca. alle 85ms aufgerufen
*
*/
void handleResults(uint8_t channel)
{
	uint8_t inputActive_help=(mem_ReadByte(0x16B+(channel>>1)))>>(4*(!(channel&0x01)))&0x0F;
	if (!((inputActive_help & 0b00000111) == 0b00000111))		// Eingang ist nicht auf "keine Funktion" parametriert
	{
		if (firstConversionStatus <= 3)							// senden der Grenzwerte bei Start ausführen
		{
			lastsentValue[channel] = channelValue[channel];
			checkLimits(channel);
			if (mem_ReadByte(0x0171+channel) & 0b10000000)		// senden der GW bei Start ist parametriert
			{
				sendLimits(channel);
			}
		}
		else
		{
			checkMeasurementDifference(channel);
			if (checkLimits(channel))	sendLimits(channel);
		}
		checkCyclicTimers(channel);
	}
}


/**
* Prüfen von Grenzwertüber- bzw. unterschreitung
*	überprüft die Grenzwerte
*	schreibt den Wert des zu sendenden Telegramms in limitReaction[channel]
*
* \param  Eingang
*
* @return wahr: es wurde ein GW über-, unterschritten
*/
uint8_t checkLimits(uint8_t channel)
{
	uint16_t schwelle1, schwelle2;
	schwelle1=(uint16_t)(1024*(uint32_t)(mem_ReadByte(0x171+channel)&0x7F)/100);
	schwelle2=(uint16_t)(1024*(uint32_t)(mem_ReadByte(0x175+channel)&0x7F)/100);
	uint8_t reaction=mem_ReadByte(0x16D+channel);
	
	//steigend
	if ((lastLimitValue[channel]<schwelle2 || firstLimitCheck) && channelValue[channel]>schwelle2)	// GW 2 überschritten
	{
		if (reaction&0x0C)
		{
			limitReaction[channel]=(reaction>>2)&0x01;
			lastLimitValue[channel]=channelValue[channel];
			return 1;
		}
	}
	if ((lastLimitValue[channel]<schwelle1 || firstLimitCheck) && channelValue[channel]>schwelle1)	// GW 1 überschritten
	{
		if (reaction&0xC0)
		{
			limitReaction[channel]=(reaction>>6)&0x01;
			lastLimitValue[channel]=channelValue[channel];
			return 1;
		}
	}
	//fallend
	if ((lastLimitValue[channel]>schwelle1 || firstLimitCheck) && channelValue[channel]<schwelle1)	// GW 1 unterschritten
	{
		if (reaction&0x30)
		{
			limitReaction[channel]=(reaction>>4)&0x01;
			lastLimitValue[channel]=channelValue[channel];
			return 1;
		}
	}
	if ((lastLimitValue[channel]>schwelle2 || firstLimitCheck) && channelValue[channel]<schwelle2)	// GW 2 unterschritten
	{
		if (reaction&0x03)
		{
			limitReaction[channel]=reaction&0x01;
			lastLimitValue[channel]=channelValue[channel];
			return 1;
		}
	}
	return 0;
}


/**
* Sendet ein Telegramm für Grenzwert Über-, Unterschreitung
* 
* \param  Eingang
*
* @return void
*/
void sendLimits(uint8_t channel)
{
	SetAndTransmitBit(channel*2+1,limitReaction[channel]);
	app_dat.runningCyclTimerLimit[channel] = 0;
}


/**
* Prüfen ob ein Timer abgelaufen ist
* zykl. Timer neu starten
*
* \param  Eingang
*
* @return void
*/
void checkCyclicTimers(uint8_t channel)
{
	uint8_t delay_factor;
	delay_factor = mem_ReadByte(0x0161+channel);
	// prüfen ob ein Timer "Meßwert zyklisch senden" abgelaufen ist
	if (app_dat.runningCyclTimerMeasurement[channel]  &&  check_timeout(&app_dat.cyclTimerMeasurement[channel]))
	{
		sendValue(channel);
		app_dat.runningCyclTimerMeasurement[channel] = 0;
		app_dat.runningCyclTimerDifference[channel] = 0;
	}
	// prüfen ob ein Timer "Meßwertdifferenz" abgelaufen ist
	if (app_dat.runningCyclTimerDifference[channel]  &&  check_timeout(&app_dat.cyclTimerDifference[channel]))
	{
		sendValue(channel);
		app_dat.runningCyclTimerMeasurement[channel] = 0;
		app_dat.runningCyclTimerDifference[channel] = 0;
	}
	// Timer "Meßwert zyklisch senden" neu starten wenn parametriert
	if (app_dat.runningCyclTimerMeasurement[channel] == 0  &&  delay_factor & 0b10000000 )  //Bit7 == Meßwert zykl. senden
	{
		// (delay_factor & 0b01111111) == Bits 0-6 auswerten Delayfaktor kann 5-127 sein
		alloc_timer(&app_dat.cyclTimerMeasurement[channel],(pgm_read_dword(&delay_bases[delay_base]))*(delay_factor & 0b01111111)); 
		app_dat.runningCyclTimerMeasurement[channel] = 1;
	}
	// prüfen ob ein Timer "Grenzwert zyklisch senden" abgelaufen ist
	if (app_dat.runningCyclTimerLimit[channel]  &&  check_timeout(&app_dat.cyclTimerLimit[channel]))
	{
		sendLimits(channel);
		app_dat.runningCyclTimerLimit[channel] = 0;
	}
	// Timer "Grenzwert zyklisch senden" neu starten wenn parametriert
	if (app_dat.runningCyclTimerLimit[channel] == 0  &&  mem_ReadByte(0x0175+channel) & 0b10000000 )
	{
		// (delay_factor & 0b01111111) == Bits 0-6 auswerten Delayfaktor kann 5-127 sein
		alloc_timer(&app_dat.cyclTimerLimit[channel],(pgm_read_dword(&delay_bases[delay_base]))*(delay_factor & 0b01111111)); 
		app_dat.runningCyclTimerLimit[channel] = 1;
	}	
}


/**
* Funktion startet die Timer für das Senden nach Busspannungswiederkehr
*
* \param  Eingang
*
* @return void
*/
void setPowerOnTimer(uint8_t channel)
{
	// Verhalten bei Busspannungswiederkehr Meßwerte
	if ((mem_ReadByte(0x0179)<<(channel*2)) & 0b10000000)										//sofort senden
	{
		alloc_timer(&app_dat.cyclTimerMeasurement[channel],pgm_read_dword(&delay_bases[0]));	// Timer auf 130ms stellen = sofort senden
		app_dat.runningCyclTimerMeasurement[channel] = 1;
	}
	
	if ((mem_ReadByte(0x0179)<<(channel*2)) & 0b01000000)										//verzögert senden
	{
		alloc_timer(&app_dat.cyclTimerMeasurement[channel],mem_ReadByte(0x01A0) * pgm_read_dword(&delay_bases[3]));	// in sec. 5-255 * delay_bases[3](1sek.)
		app_dat.runningCyclTimerMeasurement[channel] = 1;
	}
	
	// Verhalten bei Busspannungswiederkehr Grenzwerte
	if ((mem_ReadByte(0x0171)+channel) & 0b10000000)											// sofort senden (Grenzwerte werden nicht verzögert gesendet)
	{
		alloc_timer(&app_dat.cyclTimerLimit[channel],pgm_read_dword(&delay_bases[0]));			// Timer auf 130ms stellen = sofort senden
		app_dat.runningCyclTimerLimit[channel] = 1;
	}
	
}


/**
* Prüfen der Messwertdifferenz
*	überprüft die Messwertdifferenz
*	setzt den zykl.Timer des Kanals auf die Verzögerungszeit
*	löscht den Timer wenn Meßwertdiff. zu kurz war
*
* \param  Eingang
*
* @return void
*/
void checkMeasurementDifference (uint8_t channel)
{
	unsigned int mess_diff;
	int mess_change;
	uint8_t zyk_val;
	uint8_t zykval_help;
	
	if (mem_ReadByte(0x165+channel)&0x80)													//Senden bei Messwertdifferenz
	{
		mess_diff=(uint16_t)(1024*(uint32_t)(mem_ReadByte(0x165+channel)&0x7F)/100);
		if (channelValue[channel] <= lastsentValue[channel])
			mess_change=lastsentValue[channel] - channelValue[channel];
		else
			mess_change=channelValue[channel] - lastsentValue[channel];
		
		if (mess_change > mess_diff)														
		{
			if (app_dat.runningCyclTimerDifference[channel] == 0)
			{
				zykval_help=(mem_ReadByte(0x169+(channel>>1)))>>(4*(!(channel&0x01)))&0x0F;		//Sendeverzögerung bei Messwertdiff.
				if (zykval_help<=5)
				{
					zyk_val=zykval_help;				// 0-5 sek.
				}
				else if (zykval_help<=10)
				{
					zyk_val=(zykval_help-5)*10;			// 10-50 sek.
				}
				else
				{
					zyk_val=(zykval_help-10)*60;		// 1-5 min.
				}
				if (zyk_val == 0)													// sofort senden
				{
					alloc_timer(&app_dat.cyclTimerDifference[channel],pgm_read_dword(&delay_bases[0]));
					app_dat.runningCyclTimerDifference[channel] = 1;
				}
				else																// mit Verzögerung senden
				{
					alloc_timer(&app_dat.cyclTimerDifference[channel],pgm_read_dword(&delay_bases[3])*zyk_val);
					app_dat.runningCyclTimerDifference[channel] = 1;
				}
			}
		}
		else
		{
			app_dat.runningCyclTimerDifference[channel]=0;		// Meßwertdifferenz war kürzer als Sendeverzögerung --> Timer löschen
		}
		
	}
}


/**
* bringt den Meßwert ins Sendeformat 8 Bit oder 16 Bit
* sendet Telegramm
* 
*
* \param  Eingang
*
* @return void
*/
void sendValue(uint8_t channel)
{
	uint8_t i;
	uint8_t mul = mem_ReadByte(0x0184 + (channel*9));
	int32_t value = (int32_t)channelValue[channel];

	int16_t min = (mem_ReadByte(0x017C + 2 + (channel*9)) << 8) + mem_ReadByte(0x017C + 3 + (channel*9));
	int16_t max = (mem_ReadByte(0x0180 + 2 + (channel*9)) << 8) + mem_ReadByte(0x0180 + 3 + (channel*9));
	value = (((int32_t)max-(int32_t)min)*value)/1023;
	value += (int32_t)min;
	lastsentValue[channel]=channelValue[channel];
	
	if ((mem_ReadByte(0x01A4)>>4)&(1<<channel))			// Sendeformat 8Bit
	{
		SetAndTransmitObject(channel*2, &value, 1);		//channel*2=Objektnummer
		return;
	}
	else												// Sendeformat 16 Bit
	{
		for (i = 0; i < mul; i++)						// mit parametriertem Faktor multiplizieren
		{
			value = value*10;
		}
		sendDPT9(channel*2, value);						// Datenpunkttyp9 Gleitkommawert senden
		return;
	}
}


/**
* Datenpunkttyp 9 Gleitkommawert senden
* bringt den Wert ins Sendeformat 16 Bit DPT9 (EIS5) Gleitkomma-Wert
* sendet Telegramm
*
*
* \param  Objektnummer, Wert (wird beim Empfänger mit 0,01 multipliziert)
*
* @return wahr: Wert im gültigen Bereich und gesendet; falsch: invalid Data gesendet
*/
unsigned char sendDPT9(uint8_t objno, int32_t value)
{
	uint8_t data[2];
	uint8_t isNegative=0, exponent;
	uint16_t factor;
	if (value & 0x80000000)						// wenn erstes Bit gesetzt, ist der Wert negativ
	{
		isNegative=1;							// Vorzeichen merken
		value--;								//
		value ^= 0xFFFFFFFF;					// in pos. Wandeln (Zweierkomplement auflösen)
	}
	if (value>67076095)							// Maximalwert für ein EIS5 Telegramm
	{
		data[0]=0x7F;							// 0x7FFF -> invalid Data
		data[1]=0xFF;
		SetAndTransmitObject(objno, &data[0], 2);
		return 0;
	}
	for (factor = 1 ; value>2047; factor *= 2)	value /= 2;		// kleinsten möglichen Teiler suchen (1,2,4,8...32767)
	
	for (exponent=0; !(factor & 1); exponent++)	factor /= 2;	// Teiler in Exponent wandeln (0,1,2...15)
	
	if (isNegative)
	{
		value ^= 0xFFFFFFFF;									// für negative Zahlen Mantisse wieder in Zweierkomplement umwandeln
		value++;												//
	}
	value &= 0b1000011111111111;								// Bits für Exponenten löschen
	value |= (exponent<<11);									// Exponent einfügen
	data[0]=value>>8;
	data[1]=value;
	SetAndTransmitObject(objno, &data[0], 2);
	return 1;
}


void configureAdc(void)
{
	// set reference to Avcc, right adjusted result
	ADMUX = (1<<REFS0);

	// disable DIO of pin
	DIDR0 |= (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D) | (1<<ADC3D);

	// set ADC prescaler to 64, enable ADC interrupt, enable ADC
	ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADEN);
}


inline void doAdcMeasurement(uint8_t channel)
{
	ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0));
	ADMUX |= channel;		//set channelindex !!
	ADCSRA |= (1<<ADSC);	//start measurement
}


#endif /* _AIN10VNEWLIB_C */
/*********************************** EOF *********************************/
