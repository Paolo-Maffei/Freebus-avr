/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
 *  /_/   /_/ |_/_____/_____/_____/\____//____/
 *
 *  Copyright (c) 2010 Dirk Opfer <Dirk@do13.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/**
 * @file   fb_pir_app.c
 * @author Dirk Opfer
 * @date
 *
 * @brief
 */
#ifndef _FB_PIR_APP_C
#define _FB_PIR_APP_C


/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_eeprom.h"
#include "msg_queue.h"
#include "fb_hal.h"
#include "fb_prot.h"
#include "timer.h"
#include "adc.h"
#include "fb_app.h"
#include "fb_pir_app.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/

#define NEXT_STATE(x)	app_state = x
#define GET_STATE()		app_state

enum PIR_Objects_e {
	OBJ_MOVEMENT = 0,
	OBJ_LOCK,
	OBJ_MASTER_TRIG,
	OBJ_BRIGHTNESS_CTRL,
	OBJ_BRIGHTNESS,
	OBJ_CONSTLIGHT,
	OBJ_CONSTLIGHT_LOCK,
	OBJ_BRIGHTNESS_LIGHT,
	OBJ_BRIGHTNESS_LIGHT_CTRL,
	OBJ_BRIGHTNESS_VALUE,
	OBJ_INSTALLATION
};

/* Objekte:
Nr. Objektname         Funktion                     Typ                   Flags
0   Bewegung           Schalten aufgrund Bewegung   EIS 1 1 Bit           K     Ü
1   Sperre             Sperre des Bewegungsmelders  EIS 1 1 Bit           K   S
2   Master Trig        Eingang / Ausgang            EIS 1 1 Bit           K L S Ü
3   Helligk. Schw.     abrufen = 01dez (01hex)      DPT. 18.001 1 Byte    K   S
                       speichern = 129dez (81hex)
4   Helligk. Schwelle  Sollwert                     EIS 5 2 Byte          K L S Ü
5   Konstantlicht Reg. Dimmer                       EIS 2 4 Bits 1 Byte   K L   Ü
6   Sperr K.licht Reg. Sperre der Konstantlicht R.  EIS 1 1 Bit           K   S
7   Helligk. Lichreg.  Sollwert                     EIS 5 2 Byte          K L S Ü
8   Helligk. abrf.sp.  abrufen = 01dez (01hex)      DPT. 18.001 1 Byte    K   S
                       speichern = 129dez (81hex)
9   Helligkeitswert    Helligkeitswert              EIS 5 2 Byte          K     Ü
10  Inbetriebnahme     Eingang                      EIS 1 1 Bit           K   S

# nur Sphinx 332
11  Bewegung 2         Schalten aufgrund Bewegung 2 EIS 1 1 Bit           K     Ü
12  Sperre 2           Sperre des Bewegungsms 2     EIS 1 1 Bit           K   S
13  Helligk. Schw.     abrufen = 01dez (01hex)      DPT. 18.001 1 Byte    K   S
                       speichern = 129dez (81hex)
14  Helligk. Schwelle  Sollwert                     EIS 5 2 Byte          K L D Ü

Flag  Name              Bedeutung
K     Kommunikation     Objekt ist kommunikationsfähig
L     Lesen             Objektstatus kann abgefragt werden (ETS / Display usw.)
S     Schreiben         Objekt kann empfangen
Ü     Übertragen        Objekt kann senden

*/

enum states_e {
	IDLE,
	WAIT_OFF_TIMER,
	WAIT_RETRIGGER,
};

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
/* Fixed delay values
   0,5 sec ... 2 sec
   values stored in table a directly converted into ticks
*/

static const uint8_t delay_values[] PROGMEM = 	{M2TICS(500), M2TICS(600), M2TICS(700), M2TICS(800), M2TICS(900), M2TICS(1000),
												 M2TICS(1100), M2TICS(1200), M2TICS(1300), M2TICS(1400), M2TICS(1500), M2TICS(1600),
												 M2TICS(1700), M2TICS(1800), M2TICS(1900), M2TICS(2000)};

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
		{ SOFTWARE_VERSION_NUMBER, 0x12 },    /**< version number                               */
		{ APPLICATION_RUN_STATUS,  0xFF },    /**< Run-Status (00=stop FF=run)                  */
		{ COMMSTAB_ADDRESS,        0xBE },    /**< COMMSTAB Pointer                             */
		{ APPLICATION_PROGRAMM,    0x00 },    /**< Port A Direction Bit Setting???              */

		{ MANUFACTORER_ADR_HIGH,   0x00 },    /**< Herstellercode 0x48 = Theben                 */
		{ MANUFACTORER_ADR_LOW,    0x48 },    /**<                                              */
		{ DEVICE_NUMBER_HIGH,      0x10 },    /**< Sphinx 331                                   */
		{ DEVICE_NUMBER_LOW,       0x7C },    /**<                                              */
		{ 0x10E,                   0x60 },    /**< Routingcounter                               */
		{ 0xFF,                    0xFF }     /**< END-sign; do not change                      */
};

const struct FBAppInfo AppInfo PROGMEM =
{
	.FBApiVersion = 0x01,
	.pParam =  defaultParam,
};


static enum states_e app_state;

struct {
	timer_t timer_on;
	uint8_t oldpinstate;
	uint8_t inpstate;

}app_dat;

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/

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
	/* IO configuration */
	IO_SET_DIR(1, IO_INPUT);

	app_dat.oldpinstate = 1;
	/* Reset state */
	NEXT_STATE(IDLE);

	return 1;
} /* restartApplication() */



/**
 * Function is called periodically if the application is enabled in system_state
 *
 */
void app_loop()
{
	if (TestObject(OBJ_BRIGHTNESS_CTRL)) {
		/* Reset flag to indicate the next external write */
		SetRAMFlags(OBJ_BRIGHTNESS_CTRL, 0);
		/* Access data directly */
		if (userram[3] == 0x1) {
			uint16_t __UNUSED__ adc = readADC(0, 2);
DEBUG_PUTHEX(adc >> 8);
DEBUG_PUTHEX(adc);
DEBUG_NEWLINE();
			TransmitObject(OBJ_BRIGHTNESS);
		}
		else if (userram[3] == 0x81) {
			/* Store actual level */
		}
	}

	if (TestObject(OBJ_LOCK)) {
		/* Reset flag to indicate the next external write */
		SetRAMFlags(OBJ_LOCK, 0);
		/* Access data directly */
		if (userram[1] == 0x1)
		;
		else if (userram[1] == 0x81) {
			;
			/* Store actual level */
		}
	}

	if (TestObject(OBJ_MASTER_TRIG)) {
		/* Reset flag to indicate the next external write */
		SetRAMFlags(OBJ_MASTER_TRIG, 0);
	}

	/* Get actual input value from PIR */
	app_dat.inpstate = IO_GET(1);

	/* Check if value changed ? */
	if (app_dat.inpstate ^ app_dat.oldpinstate) {
		app_dat.oldpinstate = app_dat.inpstate;

		/* Motion detected */
		if (app_dat.inpstate == 1) {

			if (GET_STATE() == IDLE) {
				/* First Motion */

				//0x01FD  Bit 7-7  Param-ID: 188177  Timebase for Off-Delay  Schalten aufgrund Bewegung  Master/Slave  Bewegung Kanal 1
				//	0  Sekunden
				//	1  Minuten

				//0x01FD  Bit 0-6  Param-ID: 188198  Off-Delay  Retrigger  Einschalten  Master/Slave  Bewegung Kanal 1

				uint16_t off_delay = mem_ReadByte(0x1fd) & 0x7f;
				off_delay = SEC2TICS(off_delay);						/* seconds */

				if (mem_ReadByte(0x1fd) & 0x80)
					 off_delay *= 60;					/* minutes */

				if (off_delay) {
					/* Delay given, so start timer */
					alloc_timer(&app_dat.timer_on, off_delay);
					NEXT_STATE(WAIT_OFF_TIMER);

				} else {
					/* No delay, we have to send no off telegram */

					/* Set new timer to prevent false trigger during off switching */
					uint8_t retrigger = mem_ReadByte(0x1f5) >> 4;
					/* means 0.5 ... 2s */
					alloc_timer(&app_dat.timer_on, pgm_read_byte(&delay_values[retrigger]));
					NEXT_STATE(WAIT_RETRIGGER);
				}
				uint8_t status;
				/* set comobj and send */
				status = 1;
				SetAndTransmitObject(OBJ_MOVEMENT, &status, 0);

			} else 	if (GET_STATE() == WAIT_OFF_TIMER) {

				/* First Motion detected during on phase */
				if (0) {
					/* retrigger timer */
					uint16_t off_delay = mem_ReadByte(0x1fd) & 0x7f;
					off_delay *=  100;						/* seconds */

					if (mem_ReadByte(0x1fd) & 0x80)
						 off_delay *= 60;					/* minutes */

					alloc_timer(&app_dat.timer_on, off_delay);
				} else {
					/* no change wait for expired timer */
				}
			}
		}
	}

	if (GET_STATE() == WAIT_OFF_TIMER) {
		/* already trigger wait for the on timer */
		if (check_timeout(&app_dat.timer_on)) {
			/* Timer expired */

			/* Set new timer to prevent false trigger during off switching */
			uint8_t retrigger = mem_ReadByte(0x1f5) >> 4;
			/* means 0.5 ... 2s */
			alloc_timer(&app_dat.timer_on, pgm_read_byte(&delay_values[retrigger]));

			/* set comobj to off and send */
			uint8_t status;
			status = 0;
			SetAndTransmitObject(OBJ_MOVEMENT, &status, 0);
			NEXT_STATE(WAIT_RETRIGGER);
		}
	}

	if (GET_STATE() == WAIT_RETRIGGER) {
		/* last phase, do not detect the switch off as motion */
		if (check_timeout(&app_dat.timer_on)) {
			/* Timer expired, let's wait for a new motion */
			NEXT_STATE(IDLE);
		}
	}
}

#endif /* _FB_PIR_APP_C */
/*********************************** EOF *********************************/
