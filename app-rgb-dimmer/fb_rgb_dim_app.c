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
 * @file   fb_rgb_dim_app.c
 * @author Uwe S.
 * @date   18.04.2014
 *
 * @brief  Dimmer application based on the  "Jung Steuereinheit 3fach 2193REG"
 *         to control WS2812 LEDs
 * Manufacturer code is 0x0004 = JUNG
 * Device type is 0x3018 = Ordernumber: 2193REG
 *
 * To enable --
 *
 * This version is designed to be used with the new API.
 */
#ifndef _FB_APP_C
#define _FB_APP_C

/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb_rgb_dim_app.h"
#include "hsv_to_rgb.h"
#include "light_ws2812.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/

/// Bit list of states the program can be in
enum states_e {
	IDLE                    = 0,
	TO_SEND                 = (1<<0),
	DIMMING_TIMER_RUNS      = (1<<1),
	SWITCH_OFF_TIMER_RUNS   = (1<<2),
	PWM_TIMER_RUNS          = (1<<3),
	USE_GAMMA_CORRECTION    = (1<<4),
	HSV_MODE                = (1<<5),
	WAIT_FOR_POWER_ON       = (1<<6),
	POWER_ON_TIMER_RUNS     = (1<<7),
	DIMMER_ON               = (1<<8),
};

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
static const timer_t delay_bases[] PROGMEM = { 1*M2TICS(1), 1*M2TICS(1), 1*M2TICS(10), 1*M2TICS(130), 16*M2TICS(130), 256*M2TICS(130)};
static const uint8_t gamma_correction[] PROGMEM = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,11,11,11,12,12,13,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,21,21,22,22,23,23,24,25,25,26,27,27,28,29,29,30,31,31,32,33,34,34,35,36,37,37,38,39,40,40,41,42,43,44,45,46,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,76,77,78,79,80,81,83,84,85,86,88,89,90,91,93,94,95,96,98,99,100,102,103,104,106,107,109,110,111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};
static const uint8_t switchingOnBrightness[] PROGMEM = {0,1,25,51,76,102,127,153,178,204,229,255} ;
static const uint8_t dimmingLevelBetweenTwoLeds[] PROGMEM = {2,5,10,20,30,40,50,60,70,80,100,150,200,255};	

uint8_t nodeParam[EEPROM_SIZE];                             /**< parameterstructure (RAM)                          */
extern uint8_t userram[USERRAM_SIZE];

static enum states_e app_state;

struct {
uint8_t ledNumber[MAX_NUMBER_OF_LEDS];                      /**< array for dimming functions                       */
uint8_t destinationValue;                                   /**< stop dimming timer at this value                  */
uint8_t lastShutoffValue;                                   /**< stores values for switching on brightness         */
} color[3];                                                 /**< all variables for RED_HUE, GREEN_SAT, BLUE_VAL    */
uint8_t dimmingStepsBetweenLeds;                            /**< for dim each LED individually                     */               
uint8_t dimmingStepsDistanceForEachLed[MAX_NUMBER_OF_LEDS]; /**< array to count the dimming steps between the LEDs */
uint8_t numberOfLeds;
timer_t dimmTimer;                                          /**< stores timer value for dimming timer              */
timer_t dimmTimerReload;                                    /**< value for dimming timer cycle                     */
//timer_t switchOffTimer;                                   /**< stores timer value for switch off function        */
timer_t pwmTimer;                                           /**< stores timer value for relay PWM timer            */
timer_t powerOnTimer;                                       /**< stores timer value for power on timer             */
struct cRGB led[MAX_NUMBER_OF_LEDS];                        /**< array for light_ws2812 function                   */


/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void HandleSwitchingObject(uint8_t switchingObject);
void HandleDimmingObject(uint8_t dimmingObject, uint8_t colorIndex);
void HandleBrightnessObject(uint8_t brightnessObject, uint8_t colorIndex);
void HandleDimIndividuallyObject(uint8_t dimmIndividuallyObject);
void StartDimming(void);
void CheckDimmingTimer(void);
void CheckPowerOnTimer(void);
void CheckSwitchingStatus(void);
void CheckPwmTimer(void);
void FillLedArray(void);
void DirectJumpToOneColor(void);
void FillOffsetArray(uint8_t ledOffset);
void StopTimer(void);
void Dimming(void);
void SetSwitchingOnBrightness(uint8_t index);
void SetOutput(void);


/*************************************************************************
 * Implementation
 **************************************************************************/
/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void) {
	RESET_STATE();
	
	/* RGB or HSV mode */
	if (mem_ReadByte(APP_SOFT_OFF_BASE) & 0x08){
		/* parameter response on bus voltage failure output 1 on --> HSV mode */
		SET_STATE(HSV_MODE);
	}
	
	/* gamma correction yes/no */
	if (mem_ReadByte(APP_SOFT_ON_BASE) & 0x08){
		/* parameter response on bus voltage failure output 2 on --> use gamma correction */
		SET_STATE(USE_GAMMA_CORRECTION);
	}
	
	/* number of connected LEDs */
	/* parameter response on bus voltage failure output 2 on --> use gamma correction */
	numberOfLeds = mem_ReadByte(APP_FACTOR_DIMMING_STEP_CH2);
	
	/* set dimming steps between two Leds */
	HandleDimIndividuallyObject(0);
		
	
	/* Starting with 50% */
	color[RED_HUE].lastShutoffValue = 127;
	color[GREEN_SAT].lastShutoffValue = 127;
	color[BLUE_VAL].lastShutoffValue = 127;
	
	
	/* IO configuration */
	IO_SET_DIR(1,IO_OUTPUT);     /* WS2812 Data output */
	IO_SET_DIR(3,IO_OUTPUT);     /* Relay output       */
	SET_IO_CTRL(1);              /* Relay PWM output   */
	
	SET_STATE(WAIT_FOR_POWER_ON);
				
	return 1;	
}


/**
 * Function os called periodically of the application is enabled in the system_state
 *
 */
void app_loop() {
		
	if (IN_STATE(TO_SEND) && !(IN_STATE(WAIT_FOR_POWER_ON))) {
		SetOutput();
	} else {
		
		uint8_t objectValue;
		
		/*** Switching Object ***/
		if (TestAndCopyObject (OBJECT_SWITCH , &objectValue, 0)) {
			HandleSwitchingObject(objectValue);
		}
	
		/*** Dimming Objects ***/
		if (TestAndCopyObject (OBJECT_DIM_RED_HUE , &objectValue, 0)) {
			HandleDimmingObject(objectValue, RED_HUE);
		}
		if (TestAndCopyObject (OBJECT_DIM_GREEN_SAT , &objectValue, 0)) {
			HandleDimmingObject(objectValue, GREEN_SAT);
		}
		if (TestAndCopyObject (OBJECT_DIM_BLUE_VAL , &objectValue, 0)) {
			HandleDimmingObject(objectValue, BLUE_VAL);
		}
		
		/*** Brightness Objects ***/
		if (TestAndCopyObject(OBJECT_BRIGHTNESS_RED_HUE, &objectValue, 1)) {
			HandleBrightnessObject(objectValue, RED_HUE);
		}			
		if (TestAndCopyObject(OBJECT_BRIGHTNESS_GREEN_SAT, &objectValue, 1)) {
			HandleBrightnessObject(objectValue, GREEN_SAT);
		}
		if (TestAndCopyObject(OBJECT_BRIGHTNESS_BLUE_VAL, &objectValue, 1)) {
			HandleBrightnessObject(objectValue, BLUE_VAL);
		}
		
		/*** Dim each LED individually (Object blocking function ch.1) ***/
		if (TestAndCopyObject (OBJECT_DIM_INDIVIDUALLY , &objectValue, 0)) {
			HandleDimIndividuallyObject(objectValue);
		}
		
		/***                                                           ***/
		CheckDimmingTimer();
		CheckPowerOnTimer();
		CheckSwitchingStatus();
		CheckPwmTimer();	
		
	}
}


void HandleSwitchingObject(uint8_t switchingObject) {
	uint8_t delay_factor;
	uint8_t delayBaseIndex;
	if (switchingObject) {
		/* Switching On */
		StopTimer();
		/* Starting brightness */
		uint8_t indexSwitchingOnBrightness = mem_ReadByte(APP_SWITCH_ON_BRIGHTNESS) & 0x0F;
		// indexSwitchingOnBrightness &= 0b00001111;
		SetSwitchingOnBrightness(indexSwitchingOnBrightness);
		/* Soft-On ? */
		delay_factor = mem_ReadByte(APP_SOFT_ON_FACTOR_CH1);
		if (delay_factor == 0) {
			/* Soft-On no */
			DirectJumpToOneColor();
		} else {
			/* Soft-On yes */
			delayBaseIndex = mem_ReadByte(APP_SOFT_ON_BASE) & 0x07;
			dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
			StartDimming();
			FillOffsetArray(dimmingStepsBetweenLeds);
		}
	} else {
		/* Switching Off */
		StopTimer();
		color[RED_HUE].lastShutoffValue = color[RED_HUE].destinationValue;
		color[GREEN_SAT].lastShutoffValue = color[GREEN_SAT].destinationValue;
		color[BLUE_VAL].lastShutoffValue = color[BLUE_VAL].destinationValue;
		color[RED_HUE].destinationValue = 0;
		color[GREEN_SAT].destinationValue = 0;
		color[BLUE_VAL].destinationValue = 0;
		/* Soft-Off ? */
		delay_factor = mem_ReadByte(APP_SOFT_OFF_FACTOR_CH1);
		if (delay_factor == 0) {
			/* Soft-Off no */
			DirectJumpToOneColor();
		} else {
			/* Soft-Off yes */
			delayBaseIndex = mem_ReadByte(APP_SOFT_OFF_BASE) & 0x07;
			dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
			StartDimming();
			FillOffsetArray(dimmingStepsBetweenLeds);
		}
	}		
}


void HandleDimmingObject(uint8_t dimmingObject, uint8_t colorIndex) {
	uint8_t delayBaseIndex = (mem_ReadByte(APP_BASE_DIMMING_STEP));
	delayBaseIndex &= 0x07;								/* auf Bits 0-3 begrenzen */
	uint8_t delay_factor = mem_ReadByte(APP_FACTOR_DIMMING_STEP_CH1);
	dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
	if (dimmingObject & 0b00000111) {
		/* auf oder abdimmen */
		uint8_t dimmSteps = 255;
		/* Schritte für Dimmbefehle 1,5% - 100% ausrechnen */
		for ( uint8_t i=1; i<(dimmingObject & 0b00000111); i++) {
			dimmSteps >>= 1;
		}
		if (dimmingObject & 0b00001000) {
			/* aufdimmen */
			/* Zielhelligkeit setzen wenn Maximum nicht überschritten wird */
			color[colorIndex].destinationValue = 255-color[colorIndex].destinationValue > dimmSteps ? color[colorIndex].destinationValue + dimmSteps : 255;
			} else {
			/* abdimmen */
			/* Zielhelligkeit setzen wenn 0 nicht unterschritten wird */
			color[colorIndex].destinationValue = color[colorIndex].destinationValue > dimmSteps ? color[colorIndex].destinationValue - dimmSteps : 0;
		}
		StartDimming();
		FillOffsetArray(dimmingStepsBetweenLeds);
		} else {
		/* Stopptelegramm */
		StopTimer();
		//SendBrightness();
	}
}


void HandleBrightnessObject(uint8_t brightnessObject, uint8_t colorIndex) {
	StopTimer();
	color[colorIndex].destinationValue = brightnessObject;
	uint8_t delayBaseIndex = (mem_ReadByte(APP_BASE_DIMMING_STEP));
	/* Response on reception of value */
	if (delayBaseIndex & 0b00001000) {
		/* jump to brightness */
		DirectJumpToOneColor();
	} else {
		/* dim to brightness */
		delayBaseIndex &= 0x07;
		uint8_t delay_factor = mem_ReadByte(APP_FACTOR_DIMMING_STEP_CH1);
		dimmTimerReload = (pgm_read_dword(&delay_bases[delayBaseIndex])) * (uint16_t) delay_factor;
		StartDimming();
		FillOffsetArray(dimmingStepsBetweenLeds);
	}
}


void HandleDimIndividuallyObject(uint8_t dimmIndividuallyObject) {
	/* set dimming steps between two Leds 
	parameter polarity blocking function ch.1 */
	uint8_t polarity = mem_ReadByte(APP_LOCK_FUNCTION) & 0b00001000;
	polarity >>= 3;
	// Test whether object has to be inverted
	if (polarity != dimmIndividuallyObject) {
		// Dim each LED individually, steps = parameter brightness at the beginning of disabling
		dimmingStepsBetweenLeds = pgm_read_byte(&dimmingLevelBetweenTwoLeds[mem_ReadByte(APP_BRIGHTNESS_BLOCKING_FUNCTION_CH1) & 0x0F]);
	} else {
		// Dim all together
		dimmingStepsBetweenLeds = 0;
	}
}	


/**
* Start dimming timer & send object dimming runs
*
* \param  void
*
*
* @return void
*/
void StartDimming(void) {
	SET_STATE(DIMMING_TIMER_RUNS);
	SetAndTransmitBit(OBJECT_DIM_RUNS_RESPONSE, ON);
}


/**
* Check whether the dimming timer has expired 
* Start timer again
* Call Dimming function
*
*
* \param  void
*         
*
* @return void
*/
void CheckDimmingTimer(void) {
	if (IN_STATE(DIMMING_TIMER_RUNS)  &&  !(IN_STATE(WAIT_FOR_POWER_ON)) && check_timeout(&dimmTimer)) {
		alloc_timer(&dimmTimer, dimmTimerReload);
		Dimming();		
	}	
}


/**
* Check whether the Power On timer has expired
* Set status data can be sent
*
*
* \param  void
*
*
* @return void
*/
void CheckPowerOnTimer(void) {
	if (IN_STATE(POWER_ON_TIMER_RUNS)  &&  check_timeout(&powerOnTimer)) {
		UNSET_STATE(POWER_ON_TIMER_RUNS);
		UNSET_STATE(WAIT_FOR_POWER_ON);
	}
}


/**
* Check if we can enable PWM
* if app_state==PWM_TIMER_ACTIVE and pwmTimer is reached enable PWM, else no change
*
*
* \param  void
*
*
* @return void
*/
void CheckPwmTimer(void) {
	if (IN_STATE(PWM_TIMER_RUNS) && check_timeout(&pwmTimer)) {
		ENABLE_PWM(PWM_SETPOINT);
		UNSET_STATE(PWM_TIMER_RUNS);
	}
}


/**
* Stop dimming timer
* Stop switching timer
*
* \param  void
*         
* @return void
*/
void StopTimer(void) {
	UNSET_STATE(DIMMING_TIMER_RUNS);
	UNSET_STATE(SWITCH_OFF_TIMER_RUNS);
}


void Dimming(void) {
	StopTimer();
	for(uint8_t j=0; j<3; j++) {
		// for R,G,B
		// first LED
		if (color[j].destinationValue > color[j].ledNumber[0]) {
			color[j].ledNumber[0]++;
			SET_STATE(DIMMING_TIMER_RUNS);			
		} else if (color[j].destinationValue < color[j].ledNumber[0]) {
	    	color[j].ledNumber[0]--;
			SET_STATE(DIMMING_TIMER_RUNS);
		}
		if (dimmingStepsDistanceForEachLed[0] > 0){
			dimmingStepsDistanceForEachLed[0]--;
			SET_STATE(DIMMING_TIMER_RUNS);
		}
		for (uint8_t i=1; i<numberOfLeds; i++) {
			// next LEDs
			if (dimmingStepsDistanceForEachLed[i-1] == 0) {
				if (dimmingStepsDistanceForEachLed[i] > 0) {
					dimmingStepsDistanceForEachLed[i]--;
					SET_STATE(DIMMING_TIMER_RUNS);
				}
				if (color[j].destinationValue > color[j].ledNumber[i]) {
					color[j].ledNumber[i]++;
					SET_STATE(DIMMING_TIMER_RUNS);
				} else if (color[j].destinationValue < color[j].ledNumber[i]) {
					color[j].ledNumber[i]--;
					SET_STATE(DIMMING_TIMER_RUNS);
				}
			}
		}
	}		
	if (IN_STATE(DIMMING_TIMER_RUNS)) {
		FillLedArray();
	} else {
		/* Dim end */
		SetAndTransmitBit(OBJECT_DIM_RUNS_RESPONSE, OFF);
	}
}


void FillLedArray(void) {
	uint8_t i;
	if (IN_STATE(HSV_MODE)) {
		if (IN_STATE(USE_GAMMA_CORRECTION)) {
			/* HSV mode & gamma correction */
			for (i=0; i<numberOfLeds; i++) {
				/* use gamma correction only for brightness --> V */
				hsv_to_rgb(color[RED_HUE].ledNumber[i], color[GREEN_SAT].ledNumber[i], pgm_read_byte(&gamma_correction[color[BLUE_VAL].ledNumber[i]]), &led[i].r, &led[i].b, &led[i].g);
			}
		} else {
			/* HSV mode & no gamma correction */
			for(i=0; i<numberOfLeds; i++) {
				hsv_to_rgb(color[RED_HUE].ledNumber[i], color[GREEN_SAT].ledNumber[i], color[BLUE_VAL].ledNumber[i], &led[i].r, &led[i].b, &led[i].g);
			}
		}
	} else {
		if (IN_STATE(USE_GAMMA_CORRECTION)) {
			/* RGB mode & gamma correction */
			for (i=0; i<numberOfLeds; i++) {
				led[i].r = pgm_read_byte(&gamma_correction[color[RED_HUE].ledNumber[i]]);
				led[i].g = pgm_read_byte(&gamma_correction[color[GREEN_SAT].ledNumber[i]]);
				led[i].b = pgm_read_byte(&gamma_correction[color[BLUE_VAL].ledNumber[i]]);
			}
		} else {
			/* RGB mode & no gamma correction */
			for(i=0; i<numberOfLeds; i++) {
				led[i].r = color[RED_HUE].ledNumber[i];
				led[i].g = color[GREEN_SAT].ledNumber[i];
				led[i].b = color[BLUE_VAL].ledNumber[i];
			}
		}			
	}		
	SET_STATE(TO_SEND);
}


void DirectJumpToOneColor(void) {
	uint8_t i, r, g, b;
	if (IN_STATE(HSV_MODE)) {
		if (IN_STATE(USE_GAMMA_CORRECTION)) {
			/* HSV mode & gamma correction */
			/* use gamma correction only for brightness --> V */
			hsv_to_rgb(color[RED_HUE].destinationValue, color[GREEN_SAT].destinationValue, pgm_read_byte(&gamma_correction[color[BLUE_VAL].destinationValue]), &r, &b, &g);
			for (i=0; i<numberOfLeds; i++) {
				led[i].r = r;
				led[i].g = g;
				led[i].b = b;
			}
		} else {
			/* HSV mode & no gamma correction */
			hsv_to_rgb(color[RED_HUE].destinationValue, color[GREEN_SAT].destinationValue, color[BLUE_VAL].destinationValue, &r, &b, &g);
			for (i=0; i<numberOfLeds; i++) {
				led[i].r = r;
				led[i].g = g;
				led[i].b = b;
			}
		}
	} else {
		if (IN_STATE(USE_GAMMA_CORRECTION)) {
			/* RGB mode & gamma correction */
			r = pgm_read_byte(&gamma_correction[color[RED_HUE].destinationValue]);
			g = pgm_read_byte(&gamma_correction[color[GREEN_SAT].destinationValue]);
			b = pgm_read_byte(&gamma_correction[color[BLUE_VAL].destinationValue]);
			for (i=0; i<numberOfLeds; i++) {
				led[i].r = r;
				led[i].g = g;
				led[i].b = b;
			}
		} else {
			/* RGB mode & no gamma correction */
			for (i=0; i<numberOfLeds; i++) {
				led[i].r = color[RED_HUE].destinationValue;
				led[i].g = color[GREEN_SAT].destinationValue;
				led[i].b = color[BLUE_VAL].destinationValue;
			}
		}
	}	
	SET_STATE(TO_SEND);
}


void FillOffsetArray(uint8_t ledOffset) {
	for (uint8_t i=0; i<numberOfLeds; i++) {
		dimmingStepsDistanceForEachLed[i] = ledOffset;
	}
}


void SetSwitchingOnBrightness(uint8_t index) {
	if (index<13) {
		// 0%-100%
		color[RED_HUE].destinationValue   = pgm_read_byte(&switchingOnBrightness[index]);
		color[GREEN_SAT].destinationValue = pgm_read_byte(&switchingOnBrightness[index]);
		color[BLUE_VAL].destinationValue  = pgm_read_byte(&switchingOnBrightness[index]);
	} else {
		// value before last shutoff
		color[RED_HUE].destinationValue   = color[RED_HUE].lastShutoffValue;
		color[GREEN_SAT].destinationValue = color[GREEN_SAT].lastShutoffValue;
		color[BLUE_VAL].destinationValue  = color[BLUE_VAL].lastShutoffValue;
	}
}


void CheckSwitchingStatus(void) {
	if (color[RED_HUE].destinationValue | color[GREEN_SAT].destinationValue | color[BLUE_VAL].destinationValue) {
		// is on
		if (!(IN_STATE(DIMMER_ON))) {
			SET_STATE(DIMMER_ON);
			SetAndTransmitBit(OBJECT_SWITCHING_RESPONSE, ON);
			
			/* switch on Relay */
			alloc_timer(&pwmTimer, PWM_DELAY_TIME);
			SET_STATE(PWM_TIMER_RUNS);
			DISABLE_PWM();
			IO_SET(3, ON);
			
			/* wait for power good then send data */
			alloc_timer(&powerOnTimer, POWER_ON_DELAY);
			SET_STATE(POWER_ON_TIMER_RUNS);
			// TODO eventuell Schaltobjekt aktualisieren wenn Dimmer nicht damit eingeschaltet wurde
			//SetAndTransmitBit(OBJECT_SWITCH, 1);
			
		}
	} else {
		// is off
		if (!(IN_STATE(DIMMING_TIMER_RUNS)) && !(IN_STATE(TO_SEND)) && (IN_STATE(DIMMER_ON))) {
			UNSET_STATE(DIMMER_ON);
			SetAndTransmitBit(OBJECT_SWITCHING_RESPONSE, OFF);
			SET_STATE(WAIT_FOR_POWER_ON);
			
			/* switch off Relay */
			IO_SET(3, OFF);
			// TODO eventuell Schaltobjekt aktualisieren wenn Dimmer nicht damit ausgeschaltet wurde
			//SetAndTransmitBit(OBJECT_SWITCH, 0);
		}
	} 
}


void SetOutput(void) {
	ws2812_setleds(led,numberOfLeds);
	UNSET_STATE(TO_SEND);
}


#endif //_FB_APP_C

