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
 * @file   fb_2-dimmer_app.c
 * @author Uwe S.
 * @date   16.05.2013
 *
 * @brief  Dimmer application based on the Gira Universal Dimmaktor 2fach kompakt REG  8 // 103200
 * Manufacturer code is 0x0008 = GIRA
 * Device type 0x3015
 *
 * To enable ...
 *
 * This version is designed to be used with the new API.
 */
#ifndef _FB_APP_C
#define _FB_APP_C

/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb_2-dimmer_app.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/
#define TIMER_UP_RUN 0                 /** Statusbit Timer aufdimmen läuft */
#define TIMER_DOWN_RUN 1               /** Statusbit Timer aufdimmen läuft */
#define TIMER_SWITCH_RUN 2            /** Statusbit Timer aufdimmen läuft */
#define CHANNEL_BLOCKED 3             /** Statusbit Timer aufdimmen läuft */

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
static const timer_t delay_bases[] PROGMEM = { 1*M2TICS(1), 1*M2TICS(1), 1*M2TICS(10), 1*M2TICS(130), 16*M2TICS(130), 256*M2TICS(130)};

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

uint8_t chNr;                      /**< current channel 0=ch1; 1=ch2    */
struct {
    timer_t dimmTimer;			   /**< stores timer value for dimming timer   */
	timer_t dimmTimerReload;       /**< value for dimming timer cycle          */
	timer_t switchTimer;           /**< stores timer value for switch off function  */
	uint16_t dimmValue;			   /**< dimming value (0*128) - (255*128)    */
	uint16_t dimmValueOld;         /**< dimming value from last cycle               */
	uint16_t outputValue;
	uint16_t destinationValue;     /**< stop dimm timer at this value               */
	uint8_t status;                /**< bitfield for status bits   */
	uint8_t switchValue;
	
} channel[2];                       /**< all variables for current channel 0=ch1; 1=ch2    */


/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void SetBusOnValue(void);
void SetOutput (uint16_t outputValue);
void HandleSwitchObject (void);
void HandleDimmValues(void);
void CheckDimmTimers(void);
void CheckSwitchTimers(void);
void StopTimer(void);
void HandleSwitchOffFunction(void);
uint16_t GetBrightnessFromList (uint8_t brightnessList);
uint8_t SelectBits(uint8_t parameterByte);
void SendBrightness(void);



/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void) {
	#ifdef PWM8
		//Timer2 init
		DDRB |= (1<<PB3);                                     /* PB3 = OC2A = PIN 17 als Ausgang  */
		DDRD |= (1<<PD3);                                     /* PD3 = OC2B = PIN 5 als Ausgang  */
		TCCR2A = (1<<COM2A1) | (1<<COM2B1) | (1<<WGM20);      /* PWM Phase correct, Clear upcounting, Set downcounting    */
		TCCR2B = (1<<CS21) | (1<<CS20);                       /* Prescaler /8    */
	#endif
	
	#ifdef PWM10
		//Timer1
		DDRB |= (1<<PB2) | (1<<PB1);                                      /* PB1 = OC1A = PIN 15  ;  PB2 = OC1B = PIN 16 als Ausgang  */
		TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM11) | (1<<WGM10) ;    /* PWM Phase correct 10bit, Clear upcounting, Set downcounting   */
		TCCR1B = (1<<CS10) ;                                              /* no Prescaler  */
	#endif	
	
	#ifdef USE_UART
		/*UART 38400, 8, N, 1 */
		uart_init();
	#endif
	
		
	/* Helligkeit bei Busspannungswiederkehr setzen */
	/* Kanal 1 */
	chNr = 0;
	SetBusOnValue();
	/* Kanal 2 */
	chNr = 1;
	SetBusOnValue();
	return 1;	
}


/**
 * Function os called periodically of the application is enabled in the system_state
 *
 */
void app_loop() {
	chNr ^= (1<<0);            /* switch channel=0  <>  channel=1     */
	
	uint8_t objectValue;
	
	/* Ein/Aus Objekt bearbeiten */
	if (TestAndCopyObject (OBJECT_SWITCH + chNr , &objectValue, 0)){
		if (objectValue != channel[chNr].switchValue  &&  !(channel[chNr].status & (1<<CHANNEL_BLOCKED))){
			/* Schaltzustand hat sich geändert und Sperrfunktion nicht aktiv */
			StopTimer();
			channel[chNr].switchValue = objectValue;
			HandleSwitchObject();
		}			
	}
	
	/* Dimmobjekt bearbeiten */
	if (TestAndCopyObject (OBJECT_DIMM + chNr, &objectValue, 0)){
		objectValue &= 0b00001111 ;                                 /* Wertbereich von 0x00 bis 0x0F */
		uint8_t delayBaseIndex = SelectBits(mem_ReadByte(APP_BASE_DIMMING_STEP));
		delayBaseIndex &= 0b00000111;								/* auf Bits 0-3 begrenzen */
		uint8_t delay_factor = mem_ReadByte(APP_FACTOR_DIMMING_STEP_CH1 + chNr);
		channel[chNr].dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
	if (objectValue & 0b00000111){
		/* auf oder abdimmen */
		uint16_t dimmSteps = 255;
		/* Schritte für Dimmbefehle 1,5% - 100% ausrechnen */
		for ( uint8_t i=1; i<(objectValue & 0b00000111); i++){
			dimmSteps >>= 1;
		}
		dimmSteps *= 128;
		if (objectValue & 0b00001000){
			/* aufdimmen */
			channel[chNr].status |= (1<<TIMER_UP_RUN);  /*  Timer Richtung heller starten */
			/* Zielhelligkeit setzen wenn Maximum nicht überschritten wird */
			channel[chNr].destinationValue = 255*128-channel[chNr].dimmValue > dimmSteps ? channel[chNr].dimmValue + dimmSteps : 255*128;
		} else {
			/* abdimmen */
			channel[chNr].status |= ( 1 << TIMER_DOWN_RUN );  /*  Timer Richtung dunkler starten */
			/* Zielhelligkeit setzen wenn 0 nicht unterschritten wird */
			channel[chNr].destinationValue = channel[chNr].dimmValue > dimmSteps ? channel[chNr].dimmValue - dimmSteps : 0;
		}
		} else {
			/* Stopptelegramm */
			StopTimer();
			SendBrightness();
		}
	}
	
	/* Helligkeitsobjekt bearbeiten */
	if (TestAndCopyObject (OBJECT_BRIGHTNESS + chNr, &objectValue, 1)){
		StopTimer();
		
		uint8_t delayBaseIndex = SelectBits(mem_ReadByte(APP_BASE_DIMMING_STEP));
		if (delayBaseIndex & 0b00001000) {
			/* Wert anspringen */
			channel[chNr].dimmValue = objectValue * 128;		 
		} else {
			/* Wert andimmen */
			delayBaseIndex &= 0b00000111;									/* auf Bits 0-3 begrenzen */
			uint8_t delay_factor = mem_ReadByte(APP_FACTOR_DIMMING_STEP_CH1 + chNr);
			channel[chNr].dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
			if ((objectValue * 128) > channel[chNr].dimmValue){
				/* Starte DimmTimer aufdimmen */
				channel[chNr].status |= (1<<TIMER_UP_RUN);
			} else {
				/* Starte DimmTimer abdimmen */
				channel[chNr].status |= (1<<TIMER_DOWN_RUN);
			}
			channel[chNr].destinationValue = objectValue * 128;
		}
	}
	
	/* Sperrobjekt bearbeiten */
	if (TestAndCopyObject (OBJECT_BLOCKING_FUNCTION + chNr , &objectValue, 0)){
		StopTimer();
		uint8_t blockingBrightness = mem_ReadByte(APP_BRIGHTNESS_BLOCKING_FUNCTION_CH1 + chNr);
		uint8_t polarity = mem_ReadByte(APP_LOCK_FUNCTION);
		if (chNr) {
			/* Polarität Sperrobjekt Kanal 2 */
			polarity &= 0b00010000;
			polarity >>= 4;
		} else {
			/* Polarität Sperrobjekt Kanal 1 */
			polarity &= 0b00001000;
			polarity >>= 3;
		}
		if (objectValue != polarity){
			/* Beginn einer Sperrung */
			blockingBrightness &= 0b00001111;
			channel[chNr].dimmValue = GetBrightnessFromList(blockingBrightness);
			/* Helligkeit zu Beginn der Sperrung setzen */
			HandleDimmValues();
			/* Sperrflag setzen */
			channel[chNr].status |= (1<<CHANNEL_BLOCKED);
		} else {
			/* Ende einer Sperrung */
			blockingBrightness &= 0b11110000;
			blockingBrightness >>=4;
			channel[chNr].dimmValue = GetBrightnessFromList(blockingBrightness);
			/* Sperrflag löschen */
			channel[chNr].status &= ~(1<<CHANNEL_BLOCKED);
		}
		
		
	}
	
	if (!(channel[chNr].status & (1<<CHANNEL_BLOCKED))){
		HandleDimmValues();
	}		
	CheckDimmTimers();
	HandleSwitchOffFunction();
	CheckSwitchTimers();
	
}


/**
* Timer für Ausschaltfunktion starten wenn parametriert
* 
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void HandleSwitchOffFunction(void){
	if (channel[chNr].dimmValue < (mem_ReadByte(APP_SWITCH_OFF_BRIGHTNESS_CH1 + chNr) * 128)) {
		/* aktuelle Helligkeit ist kleiner als Ausschalthelligkeit */
		uint8_t delayBaseIndex = SelectBits(mem_ReadByte(APP_SWITCH_OFF_FUNCTION));
		if ((delayBaseIndex & 0b00001000) == 0b00001000  &&  (channel[chNr].status & 0b00000111) == 0){	// to do Bitnummern einsetzen
			/* Ausschaltfunktion ein und es läuft kein Timer */
			delayBaseIndex &= 0b00000111;									/* auf Bits 0-3 begrenzen */
			uint8_t delay_factor = mem_ReadByte(APP_SWITCH_OFF_DELAY_FACTOR_CH1 + chNr);
			alloc_timer(&channel[chNr].switchTimer,(pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor);
			channel[chNr].status |= (1<<TIMER_SWITCH_RUN);
		}
	}
}


/**
* Prüfen ob ein Timer für die Ausschaltfunktion oder
* die Zeitdimmerfunktion abgelaufen ist
* 
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void CheckSwitchTimers(void){
	if (channel[chNr].status & 0b00000100  &&  check_timeout(&channel[chNr].switchTimer)){	// to do Bitnamen einsetzen
		StopTimer();
		channel[chNr].switchValue = 0;
		HandleSwitchObject ();
	}
}


/**
* prüfen ob ein Dimm Timer abgelaufen ist --> eine Dimmstufe rauf oder runter
* Timer neu starten wenn Zielhelligkeit noch nicht erreicht
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void CheckDimmTimers(void){
	if (channel[chNr].status & 0b00000001  &&  check_timeout(&channel[chNr].dimmTimer)){	// to do Bitnamen einsetzen
		/* aufwärts dimmen */
		channel[chNr].dimmValue += 128;
		if (channel[chNr].dimmValue < channel[chNr].destinationValue){
			/* Zielhelligkeit noch noch nicht erreicht */
			alloc_timer(&channel[chNr].dimmTimer, channel[chNr].dimmTimerReload);
		} else {
			/* dimmen stop */
			StopTimer();
		}
	}
	if (channel[chNr].status & 0b00000010  &&  check_timeout(&channel[chNr].dimmTimer)){	// to do Bitnamen einsetzen
		/* abwärts dimmen */
		channel[chNr].dimmValue -= 128;
		if (channel[chNr].dimmValue > channel[chNr].destinationValue){
			/* Zielhelligkeit noch noch nicht erreicht */
			alloc_timer(&channel[chNr].dimmTimer, channel[chNr].dimmTimerReload);
		} else {
			/* dimmen stop */
			StopTimer();
		}
	}
}


/**
* prüfen ob sich ein Dimmwert geändert hat
*	
*	SetDimmvalue aufrufen
*   prüfen ob der Dimmwert > oder = null geworden ist
*      Schaltausgänge und Rückmeldung bearbeiten
*   Rückmeldung Helligkeitswert
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void HandleDimmValues(void){
	/* prüfen ob sich ein Dimmwert geändert hat */
	if (channel[chNr].dimmValue != channel[chNr].dimmValueOld){
		channel[chNr].outputValue = channel[chNr].dimmValue;
		SetOutput(channel[chNr].outputValue);
		/* Dimmwert war 0 -> Kanal wurde gerade eingeschaltet */
		if (channel[chNr].dimmValueOld == 0){
			/* Rückmeldung Schalten (Obj. 6+7) */
			SetAndTransmitBit(OBJECT_RESPONSE_SWITCH + chNr, 1);
			channel[chNr].switchValue = 1;
		}
		/* Dimmwert ist 0 -> Kanal wurde gerade ausgeschaltet */
		if (channel[chNr].dimmValue == 0){
			/* Rückmeldung Schalten (Obj. 6+7) */
			SetAndTransmitBit(OBJECT_RESPONSE_SWITCH + chNr, 0);
			channel[chNr].switchValue = 0;
		}
		/* Dimming Timer läuft nicht, Rückmeldung Helligkeit senden */
		if ((channel[chNr].status & 0b00000011) == 0){	// to do Bitnamen einsetzen
			SendBrightness();
		}
		
		channel[chNr].dimmValueOld = channel[chNr].dimmValue;
	}		
}


/**
* Ein/Ausschaltobjekt bearbeiten
* Soft Ein und Soft Aus Timer starten
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void HandleSwitchObject (void){
	if (channel[chNr].switchValue == 1){
		/* Einschalthelligkeit */
		uint8_t switchOnBrightness = SelectBits(mem_ReadByte(APP_SWITCH_ON_BRIGHTNESS));
		switchOnBrightness &= 0b00001111;
		/* Soft Ein? */
		uint8_t delay_factor = mem_ReadByte(APP_SOFT_ON_FACTOR_CH1 + chNr);
		if (delay_factor == 0) {
			channel[chNr].dimmValue = GetBrightnessFromList (switchOnBrightness);
		} else {
			/* Soft Ein aktiv */
			uint8_t delayBaseIndex = SelectBits(mem_ReadByte(APP_SOFT_ON_BASE));
			delayBaseIndex &= 0b00000111;									/* auf Bits 0-3 begrenzen */
			channel[chNr].dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
			channel[chNr].status |= (1<<TIMER_UP_RUN);
			channel[chNr].destinationValue = GetBrightnessFromList (switchOnBrightness);
		}	
	} else {
		/* Soft Aus? */
		uint8_t delay_factor = mem_ReadByte(APP_SOFT_OFF_FACTOR_CH1 + chNr);
		if (delay_factor == 0) {
			channel[chNr].dimmValue = 0;
		} else {
			/* Soft Aus aktiv */
			uint8_t delayBaseIndex = SelectBits(mem_ReadByte(APP_SOFT_OFF_BASE));
			delayBaseIndex &= 0b00000111;									/* auf Bits 0-3 begrenzen */
			channel[chNr].dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
			channel[chNr].status |= (1<<TIMER_DOWN_RUN);
			channel[chNr].destinationValue = 0;
		}
	}	
}


/**
* Ausgänge je nach Compileroption einstellen
*
*
* \param  dimmValue (0*128) - (255*128)
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void SetOutput(uint16_t outputValue){
	// todo Grundhelligkeit mit einrechnen, funktioniert so noch nicht
	//if (dimmValue > 0){
		//uint16_t basicBrightness = SelectBits(mem_ReadByte(APP_BASIC_BRIGHTNESS));
		//basicBrightness = (basicBrightness &= 0b00000111) * BASIC_BRIGHTNESS_FACTOR;
		//uint32_t dimmvalue32 = ((32747 - basicBrightness) * dimmValue / 32640;
		//
		//dimmValue = offset * dimmValue + basicBrightness; 
	//}	
	#ifdef PWM8
		if (chNr) {
			OCR2B = outputValue/128;
		}
		else{
			OCR2A = outputValue/128;
		}
	#endif
	
	#ifdef PWM10
		if (chNr) {
			OCR1B = outputValue/32;
		}
		else{
			OCR1A = outputValue/32;
		}
	#endif
		
	#ifdef USE_UART
		outputValue /= 32;               /* auf 10bit umrechnen                     */
		outputValue |= (chNr<<15);       /* Bit15=0 --> Kanal1; Bit15=1 --> Kanal2  */
		uart_hex(outputValue >> 8);      /* highbyte                                */
		uart_hex(outputValue & 0xFF);    /* lowbyte                                 */
		uart_putc(13);                 /* CR                                      */
		uart_putc(10);                 /* LF                                      */
	#endif	
}


/**
* Parametrierte Helligkeit bei Busspannungswiederkehr setzen
*
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void SetBusOnValue(void){
	uint8_t busOnBrightness = SelectBits(mem_ReadByte(APP_BUS_ON_BRIGHTNESS));
	busOnBrightness &= 0b00001111;
	channel[chNr].dimmValue = GetBrightnessFromList (busOnBrightness);
	if (channel[chNr].dimmValue == 0){
		channel[chNr].dimmValueOld = 128;
	} else {
		channel[chNr].dimmValueOld = 0;
	}
}


/**
* Dimm Timer Stoppen
* Switch Timer Stoppen
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void StopTimer(void){
	channel[chNr].status &= ~((1<<TIMER_UP_RUN) | (1<<TIMER_DOWN_RUN) | (1<<TIMER_SWITCH_RUN));
}


/**
* Helligkeit nach Auswahlliste einstellen
*
*
* \param  Index in der Auswahlliste
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return Helligkeitswert 0 - 255*128
*/
uint16_t GetBrightnessFromList (uint8_t brightnessList){
	switch(brightnessList) {
		case 0x0: return 0; break;                           /* Aus                                    */
		case 0x1: return 1*128; break;                       /* Grundhelligkeit                        */
		case 0x2: return 25*128; break;                      /* 10 %                                   */
		case 0x3: return 51*128; break;                      /* 20 %                                   */
		case 0x4: return 76*128; break;                      /* 30 %                                   */
		case 0x5: return 102*128; break;                     /* 40 %                                   */
		case 0x6: return 127*128; break;                     /* 50 %                                   */
		case 0x7: return 153*128; break;                     /* 60 %                                   */
		case 0x8: return 178*128; break;                     /* 70 %                                   */
		case 0x9: return 204*128; break;                     /* 80 %                                   */
		case 0xA: return 229*128; break;                     /* 90 %                                   */
		case 0xB: return 255*128; break;                     /* maximale Helligkeit                    */
		case 0xC: return channel[chNr].outputValue; break;   /* aktuelle Helligkeit / keine Änderung   */
		//case 0xD                                             /* Helligkeit vor dem letzten Ausschalten */
		case 0xE: return channel[chNr].dimmValue; break;     /* nachgeführte Helligkeit                */
		default: return 0;			//todo:  Helligkeitswert bei Busspannungsausfall
	}
}


/**
* Wenn Parameter für beide Kanäle in einem Byte abgelegt sind 
* Bit 0-3 Kanal 1
* Bit 4-7 Kanal 2
* schiebt die Funktion die entspr. Bits auf 0-3
*
* \param  Parameterbyte
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return Byte mit Bits auf 0-3; das Ergebnis muss noch maskiert werden
*/
uint8_t SelectBits(uint8_t parameterByte){
	if (chNr){
		return parameterByte >>= 4; 
	} else {
		return parameterByte;
	}
}


/**
* Rückmeldung Helligkeitsobjekt senden
*
*
* \param  void
*         Ausführung für aktuellen Kanal chNr 0=Kanal1, 1=Kanal2
*
* @return void
*/
void SendBrightness(void){
	uint8_t dimmValue = channel[chNr].dimmValue / 128;
	SetAndTransmitObject(OBJECT_RESPONSE_BRIGHTNESS + chNr, &dimmValue, 1);
}


#endif /* _FB_APP_C */
