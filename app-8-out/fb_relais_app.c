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
 * @file   fb_relais_app.c
 * @author Matthias Fechner, Dirk Armbrust
 * @date   Sat Jan 05 17:44:47 2008
 *
 * @brief  The relays application to switch 8 relays
 * Manufacturer code is 0x04 = Jung\n
 * Device type (2038.10) 0x2060 Ordernumber: 2138.10REG\n
 *
 * To enable IO test compile with -DIO_TEST
 *
 * This version is designed to be used with the new API.
 */
#ifndef _FB_RELAIS_APP_C
#define _FB_RELAIS_APP_C

/*************************************************************************
 * INCLUDES
 *************************************************************************/
//#include "1wire.h"
#include "fb_relais_app.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/

/* Objects for the 8-out */
enum EIGHT_OUT_Objects_e {
    OBJ_OUT0 = 0,
    OBJ_OUT1,
    OBJ_OUT2,
    OBJ_OUT3,
    OBJ_OUT4,
    OBJ_OUT5,
    OBJ_OUT6,
    OBJ_OUT7,
    OBJ_OUT8, // start of special objects used to lock or combine objects
    OBJ_OUT9,
    OBJ_OUT10,
    OBJ_OUT11,
    OBJ_RESP1,	/* Feedback Ch 1 */
    OBJ_RESP2,	/* Feedback Ch 2 */
    OBJ_RESP3,	/* Feedback Ch 3 */
    OBJ_RESP4,	/* Feedback Ch 4 */
    OBJ_RESP5,	/* Feedback Ch 5 */
    OBJ_RESP6,	/* Feedback Ch 6 */
    OBJ_RESP7,	/* Feedback Ch 7 */
    OBJ_RESP8,	/* Feedback Ch 8 */
};

/* Objekte:
Nr. Objectname        Funktion     Typ             Flags
0   Ausgang 1         Schalten     EIS 1 1 Bit     K   S
1   Ausgang 2         Schalten     EIS 1 1 Bit     K   S
2   Ausgang 3         Schalten     EIS 1 1 Bit     K   S
3   Ausgang 4         Schalten     EIS 1 1 Bit     K   S
4   Ausgang 5         Schalten     EIS 1 1 Bit     K   S
5   Ausgang 6         Schalten     EIS 1 1 Bit     K   S
6   Ausgang 7         Schalten     EIS 1 1 Bit     K   S
7   Ausgang 8         Schalten     EIS 1 1 Bit     K   S
8
9
10
11
12  Rueckm. 1         Rueckmeldung  EIS 1 1 Bit     K     Ü
13  Rueckm. 2         Rueckmeldung  EIS 1 1 Bit     K     Ü
14  Rueckm. 3         Rueckmeldung  EIS 1 1 Bit     K     Ü
15  Rueckm. 4         Rueckmeldung  EIS 1 1 Bit     K     Ü
16  Rueckm. 5         Rueckmeldung  EIS 1 1 Bit     K     Ü
17  Rueckm. 6         Rueckmeldung  EIS 1 1 Bit     K     Ü
18  Rueckm. 7         Rueckmeldung  EIS 1 1 Bit     K     Ü
19  Rueckm. 8         Rueckmeldung  EIS 1 1 Bit     K     Ü

Flag  Name            Bedeutung
K     Kummunikation   Objekt ist kommunikationsfähig
L     Lesen           Objektstatus kann abgefragt werden (ETS / Display usw.)
S     Schreiben       Objekt kann empfangen
Ü     Übertragen      Objekt kann senden

*/

/// Bit list of states the program can be in
enum states_e {
    IDLE = 0,
    INIT_TIMER = (1),
    TIMER_ACTIVE = (1<<1),
    PWM_TIMER_ACTIVE = (1<<2),
};

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
static const timer_t delay_bases[] PROGMEM = { 1*M2TICS(130), 2*M2TICS(130), 4*M2TICS(130), 8*M2TICS(130), 16*M2TICS(130), 32*M2TICS(130), 64*M2TICS(130),
                                               128*M2TICS(130), 256*M2TICS(130), 512*M2TICS(130), 1024*M2TICS(130), 2048*M2TICS(130),
                                               (timer_t) 4096*M2TICS(130), (timer_t) 8192*M2TICS(130),
                                               (timer_t) 16384*M2TICS(130), (timer_t) 32768*M2TICS(130)};

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

static enum states_e app_state;

struct {
    uint8_t portValue;          /**< defines the port status. LSB IO0 and MSB IO8, ports with delay can be set to 1 here
                                             but will be switched delayed depending on the delay */
    uint8_t oldValue;           /// hold the old value to check if we must enable a PWM or not (enable PWM only if switching from low -> high
    timer_t timer[8];
    timer_t pwmTimer;           /// stores a reference to the generic timer
	uint8_t runningTimer;
    uint16_t objectStates;      /**< store logic state of objects, 1 bit each, 8 "real" + 4 sf*/
    uint8_t blockedStates;      /**< 1 bit per object to mark it "blocked" */
	uint8_t buttonWasEvaluated; /**< 1 bit per object to mark hand actuation pushbutton was pressed */
} app_dat;
uint8_t handActuationCounter;	/* */

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void switchObjects(void);
void switchPorts(uint8_t port, uint8_t oldPort);
uint8_t checkHandActuation(uint8_t commObjectNumber);
uint8_t getIO(uint8_t inputNr);
void setIO(uint8_t ioNr, uint8_t val);


#ifdef HARDWARETEST
/** test function: processor and hardware */
void hardwaretest(void);
#endif
#ifdef IO_TEST
void io_test(void);
#endif

/**
 * Funktion:     handleTimers()
 * Parameter: commObjectNumber  0 ... 7
 *                      value   0 ... 256
**/
void handleTimers( uint8_t commObjectNumber, uint8_t value ) {
    // Get delay base
    uint8_t delayBaseIndex=mem_ReadByte(APP_DELAY_BASE+((commObjectNumber+1)>>1));

    if((commObjectNumber & 0x01) == 0x01) {
        delayBaseIndex&=0x0F;
    } else {
        delayBaseIndex = (delayBaseIndex>>4) & 0x0F;
    }
    timer_t delayBase = pgm_read_dword(&delay_bases[delayBaseIndex]);

    // Set some variables to make next commands better readable
    uint8_t timerActive = mem_ReadByte(APP_DELAY_ACTIVE) & (1<<commObjectNumber);
    uint8_t timerOffActive = mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber);
    uint8_t timerOnActive = mem_ReadByte(APP_DELAY_FACTOR_ON+commObjectNumber);

    if( !timerActive ) {
        // Check for delay factor for off
        if((app_dat.portValue & (1<<commObjectNumber)) && timerOffActive && (value == 0)) {
            DEBUG_PUTS("TIMER_OFF ");
            alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOffActive);
            app_dat.runningTimer |= 1<<commObjectNumber;
            SET_STATE(TIMER_ACTIVE);
        }

        // Check for delay factor for on
        if(((app_dat.portValue & (1<<commObjectNumber)) == 0x00) && timerOnActive && (value == 1)) {
            DEBUG_PUTS("TIMER_ON ");
            alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOnActive);
            app_dat.runningTimer |= 1<<commObjectNumber;
            SET_STATE(TIMER_ACTIVE);
        }
    } else {
        if(!timerOnActive)  {
            // Check for a timer function without delay factor for on
            if(((app_dat.portValue & (1<<commObjectNumber)) == 0x00) && (value == 1)) {
                DEBUG_PUTS("TIMER ");
                app_dat.portValue |= (1<<commObjectNumber);
                alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOffActive);
                app_dat.runningTimer |= 1<<commObjectNumber;
                SET_STATE(TIMER_ACTIVE);
            }
        } else {
            // Check for a timer function with delay factor for on
            if(((app_dat.portValue & (1<<commObjectNumber)) == 0x00) && (value == 1)) {
                DEBUG_PUTS("TF_TIMER_ON ");
                alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOnActive);
                app_dat.runningTimer |= 1<<commObjectNumber;
                SET_STATE(TIMER_ACTIVE);
            }

            // Check for delay factor for off
            if((app_dat.portValue & (1<<commObjectNumber)) && value == 1) {
                DEBUG_PUTS("TF_TIMER_OFF ");
                alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOffActive);
                app_dat.runningTimer |= 1<<commObjectNumber;
                SET_STATE(TIMER_ACTIVE);
            }
        }

        // check how to handle off telegram while in timer modus
        if((app_dat.portValue & (1<<commObjectNumber)) && value == 0) {
            DEBUG_PUTS("TK ");
            // only switch off if on APP_DELAY_ACTION the value is equal zero
            if(!(mem_ReadByte(APP_DELAY_ACTION) & (1<<commObjectNumber))) {
                DEBUG_PUTS("TIMER_DISABLE ");
                if(app_dat.runningTimer & (1<<commObjectNumber)) {
                    app_dat.runningTimer &= ~(1<<commObjectNumber);
                    app_dat.portValue &= ~(1<<commObjectNumber);
                }
            }
        }
    }
}

void handleLogicFunction( uint8_t commObjectNumber, uint8_t *value ) {
    uint8_t specialFunc;                                // special function number (0: no sf)
    uint8_t specialFuncTyp;                             // special function type
    uint8_t logicFuncTyp;                               // type of logic function ( 1: or, 2: and)
    uint8_t logicState;                                 // state of logic function
    uint8_t sfOut;                                      // output belonging to sf
    uint8_t sfMask;                                     // special function bit mask (1 of 4)

    if(*value) {
        app_dat.objectStates |= (1<<commObjectNumber);
    } else {
        app_dat.objectStates &= ~(1<<commObjectNumber);
    }
    if(commObjectNumber >= 8) {
        /** if a special function is addressed (and changed in most cases),
        * then the "real" object belonging to that sf. has to be evaluated again
        * taking into account the changed logic and blocking states. */
        /* determine the output belonging to that sf */
        sfOut = mem_ReadByte(APP_SPECIAL_CONNECT+((commObjectNumber-8)>>1))>>(((commObjectNumber-8)&1)*4) & 0x0F;
        /* get associated object no. and state of that object*/
        if (sfOut) {
            if (sfOut > 8) {
                return;
            }
            commObjectNumber =  sfOut-1;
            *value = (app_dat.objectStates>>(sfOut-1))&1;
        } else {
            return;
        }
        /* do new evaluation of that object */
    }

    /** logic function */
    /* check if we have a special function for this object */
    specialFunc  = 0;
    logicFuncTyp = 0;
    for (specialFunc=0; specialFunc < 4; specialFunc++) {
        sfMask = 1<<specialFunc;
        sfOut = mem_ReadByte(APP_SPECIAL_FUNC_OBJ_1_2 + (specialFunc>>1))>> ((specialFunc&1)*4) & 0x0F;
        if (sfOut == (commObjectNumber+1)) {
            /* we have a special function, see which type it is */
            specialFuncTyp = (mem_ReadByte(APP_SPECIAL_FUNC_MODE))>>(specialFunc*2)&0x03;
            /* get the logic state from the special function object */
            logicState = ((app_dat.objectStates>>specialFunc)>>8)&0x01;
            if (specialFuncTyp == 0) {
                /* logic function */
                logicFuncTyp = (mem_ReadByte(APP_SPECIAL_LOGIC_MODE))>>(specialFunc*2)&0x03;
                if (logicFuncTyp == 1) {  // or
                    *value |= logicState;
                }
                if (logicFuncTyp == 2) {  // and
                    *value &= logicState;
                }
            }

            if (specialFuncTyp == 1) {
                /* blocking function */
                if (((app_dat.objectStates>>8) ^ mem_ReadByte(APP_SPECIAL_POLARITY)) & sfMask) {
                    /* start blocking */
                    if (app_dat.blockedStates & sfMask) {
                        return; // we are blocked, do nothing
                    }
                    app_dat.blockedStates |= sfMask;
                    *value = (mem_ReadByte(APP_SPECIAL_FUNCTION1 + (specialFunc>>1)))>>((specialFunc&1)*4)&0x03;
                    if (*value == 0) {
                        return;
                    }
                    if (*value == 1) {
                        app_dat.portValue &= ~(1<<commObjectNumber);
                        SetAndTransmitBit(commObjectNumber, 0);
                    }
                    if (*value == 2) {
                        app_dat.portValue |= (1<<commObjectNumber);
                        SetAndTransmitBit(commObjectNumber, 1);
                    }
                    switchObjects();
                    return;
                } else {
                    /* end blocking */
                    if (app_dat.blockedStates & sfMask ) {  // we have to unblock
                        app_dat.blockedStates &= ~sfMask;
                        /* action at end of blocking, 0: nothing, 1: off, 2: on */
                        *value = (mem_ReadByte(APP_SPECIAL_FUNCTION1 + (specialFunc>>1)))>>((specialFunc&1)*4+2)&0x03;
                        if (*value == 0) {
                            return;
                        }
                        *value--;
                        /* we are unblocked, continue as normal */
                    }
                }
            }
        }
    }
}

/**
 * Function os called periodically of the application is enabled in the system_state
 *
 */
void app_loop() {
    uint8_t commObjectNumber;
    uint8_t value;
    uint8_t needToSwitch=0;

	#ifdef HAND
	// manual Operation is enabled
	// check one butten every app_loop() passing through
	if (handActuationCounter <= OBJ_OUT7  &&  checkHandActuation(handActuationCounter)){
		needToSwitch=1;
	}
	handActuationCounter++;     //count to 255 for debounce buttons, ca. 30ms
	#endif

    // Iterate over all objects and check if the status has changed
    for(commObjectNumber=OBJ_OUT0; commObjectNumber<=OBJ_OUT11; commObjectNumber++) {
        // check if an object has changed its status
        if(TestObject(commObjectNumber)) {
            DEBUG_NEWLINE();
            DEBUG_PUTS("OBJ_");
            DEBUG_PUTHEX(commObjectNumber);
            DEBUG_SPACE();

            // reset object status flag
            SetRAMFlags(commObjectNumber, 0);
            // get value of object (0=off, 1=on)
            value = userram[commObjectNumber + 4];
            DEBUG_PUTHEX(value);
            DEBUG_SPACE();

            // handle the logic part
            handleLogicFunction(commObjectNumber, &value);
            DEBUG_PUTHEX(value);
            DEBUG_SPACE();

            // check if we have a delayed action for this object, only Outputs
            if(commObjectNumber >= OBJ_OUT0 && commObjectNumber <= OBJ_OUT7) {
                handleTimers(commObjectNumber, value);
            }

            if( ! (app_dat.runningTimer & 1<<commObjectNumber)) {
				if (value == 0x01) {
                DEBUG_PUTS("ON ");
                    app_dat.portValue |= 1<<commObjectNumber;
            } else {
                DEBUG_PUTS("OFF ");
                    app_dat.portValue &= ~(1<<commObjectNumber);
                }
            }
            needToSwitch=1;
        }
    }

    // check if we can enable PWM
    // if app_state==PWM_TIMER_ACTIVE and pwmTimer is reached enable PWM, else no change
    if(IN_STATE(PWM_TIMER_ACTIVE) && check_timeout(&app_dat.pwmTimer)) {
        DEBUG_PUTS("ENABLE PWM");
        DEBUG_NEWLINE();
        ENABLE_PWM(PWM_SETPOINT);
        UNSET_STATE(PWM_TIMER_ACTIVE);
    }

    if(IN_STATE(TIMER_ACTIVE)) {
        // action for timer
        for(commObjectNumber=0; commObjectNumber<8; commObjectNumber++) {
			if(app_dat.runningTimer & 1<<commObjectNumber) {
				//DEBUG_PUTS("CTIMEOUT ");
				if(app_dat.runningTimer & 1<<commObjectNumber && check_timeout(&app_dat.timer[commObjectNumber])) {
                    DEBUG_PUTS("TIMEOUT ");
					uint8_t j = 1<<commObjectNumber;
					app_dat.runningTimer &= ~(1<<commObjectNumber);
					app_dat.portValue ^= j;

                    uint8_t value=(app_dat.portValue >> commObjectNumber) & 0x1;

                    handleTimers(commObjectNumber, value);

//                    SetAndTransmitBit(commObjectNumber, value);
                    needToSwitch=1;
                }
			}
		}
		if(app_dat.runningTimer == 0x0) {
            UNSET_STATE(TIMER_ACTIVE);
		}
    }

    if(needToSwitch) {
        switchObjects();
    }
}

/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void) {
    uint8_t i, temp, portOperationMode;
    uint16_t initialPortValue;

    /* reset global timer values */

    /* IO configuration */
    IO_SET_DIR(1,IO_OUTPUT);
    IO_SET_DIR(2,IO_OUTPUT);
    IO_SET_DIR(3,IO_OUTPUT);
    IO_SET_DIR(4,IO_OUTPUT);
    IO_SET_DIR(5,IO_OUTPUT);
    IO_SET_DIR(6,IO_OUTPUT);
    IO_SET_DIR(7,IO_OUTPUT);
    IO_SET_DIR(8,IO_OUTPUT);

#ifdef HAND	
	HAND_PORT |= (1<<HAND_PIN);		//set pullup for manual operation button input
#endif
	
#ifdef BOARD301
#if (HARDWARETEST != 1)
    SET_IO_RES1(IO_OUTPUT);
    SET_IO_RES2(IO_OUTPUT);
    SET_IO_RES3(IO_OUTPUT);
    SET_IO_RES4(IO_OUTPUT);
#else
    /* Port configuration for hardwaretest */
    SET_IO_RES1(IO_OUTPUT);
    SET_IO_RES2(IO_OUTPUT);
    SET_IO_RES3(IO_OUTPUT);
    SET_IO_RES4(IO_OUTPUT);
#endif
#endif

    /* CTRL-Port */
    SET_IO_CTRL(IO_OUTPUT);

#ifdef IO_TEST
	/* should we do an IO test? */
	io_test();
#endif

    if (mem_ReadByte(0x01C0) == 0x60) {
        /* Patch datapointer from feedback object 1. ETS has located the data at 0x60 which is the systemstate
           Writes to this object overwrite the systemstate which results in a not working system.
           Here we patch the data location to 0x68 which isn't use anymore.
           Now we have fully working feedback objects.
         */
        uint8_t temp = 0x68;
        mem_WriteBlock(0x01C0, 1, &temp);
    }

    // check if at power loss we have to restore old values (see APP_RESTORE_AFTER_PL) and do it here
    DEBUG_PUTS("SET PINS ");
    app_dat.portValue = mem_ReadByte(APP_PIN_STATE_MEMORY);
	DEBUG_PUTHEX(app_dat.portValue);
    DEBUG_SPACE();
    initialPortValue=mem_Read2Bytes(APP_RESTORE_AFTER_PL);
    for(i=0; i<=7; i++) {
        temp = (initialPortValue>>(i*2)) & 0x03;
        // DEBUG_PUTHEX(temp);
        if(temp == 0x01) {
            // open contact
            app_dat.portValue &= (uint8_t)(~(1U<<i));
            // DEBUG_PUTHEX(i);
            // DEBUG_PUTS("P");
        } else if(temp == 0x02) {
            // close contact
            app_dat.portValue |= (1<<i);
            // DEBUG_PUTHEX(i);
            // DEBUG_PUTS("L");

        }
        // Send status of every object to bus on power on
        SetAndTransmitBit(i, (app_dat.portValue >> i) & 0x1 );
    }
    // DEBUG_PUTHEX(portValue);
    DEBUG_PUTS("Done.");
    DEBUG_NEWLINE();

    /* Reset State */
    RESET_STATE();

    /* check 0x01F2 for opener or closer and modify data to reflect that, then switch the port */
    portOperationMode = mem_ReadByte(APP_CLOSER_MODE);

	/* Flip each bit */
	app_dat.oldValue = ~app_dat.portValue;

    /* switch the output pins */
    switchPorts(app_dat.portValue^portOperationMode, app_dat.oldValue^portOperationMode);

    return 1;
} /* restartApplication() */


/**
 * Compare old / new port state and send a feedback telegram if enabled
 *
 */
static void sendFeedback(uint8_t port, uint8_t oldPort)
{
    uint8_t change = oldPort ^ port;

    if (change) {
        /* Output pins changed, send feedback */
        uint8_t invert = mem_ReadByte(APP_REPORT_BACK_INVERT);
        uint8_t mask = 0x01, i;
        for(i=0; i<=7; i++) {
            if (change & mask) {
                /* Changed */
                uint8_t val = port;
                if (invert & mask)
                    val = ~val;        /* invert value */
                /* Set and transmit feedback object */
                SetAndTransmitBit(OBJ_RESP1 + i, (val & mask) ? 1 : 0);
            }
            mask <<= 1;
        }
    }
}


/**
 * Switch the objects to state in portValue and save value to eeprom if necessary.
 *
 */
void switchObjects(void) {
    uint16_t initialPortValue;
    uint8_t portOperationMode;  /**< defines if IO is closer or opener, see address 0x01F2 in eeprom */
    uint8_t savedValue;
    uint8_t i;

    DEBUG_PUTS("Sw");
    DEBUG_SPACE();

    /* read saved status and check if it was changed */
    /// @todo maybe we can write to eeprom if we have a power failure only
    savedValue = mem_ReadByte(APP_PIN_STATE_MEMORY);
    if(savedValue != app_dat.portValue) {
        // now check if last status must be saved, we write to eeprom only if necessary
        initialPortValue = mem_Read2Bytes(APP_RESTORE_AFTER_PL);
        for(i=0; i<=7; i++) {
            if(((initialPortValue>>(i*2)) & 0x03) == 0x0) {
                mem_WriteBlock(APP_PIN_STATE_MEMORY, 1, &app_dat.portValue);
                DEBUG_PUTS("Sv");
                break;
            }
        }
    }

    /* Send feedback telegrams */
    sendFeedback(app_dat.portValue, app_dat.oldValue);

    /* check 0x01F2 for opener or closer and modify data to reflect that, then switch the port */
    portOperationMode = mem_ReadByte(APP_CLOSER_MODE);
    switchPorts(app_dat.portValue^portOperationMode, app_dat.oldValue^portOperationMode);

    return;
}

/**
 * switch all of the output pins
 * @todo check if it is ok to switch all 8 ports at once, or to we have to delay the switches to give the capacity enough time to recharge again.
 *
 * @param port contains values for 8 output pins. They may be on different ports of the avr.
 *
 */
void switchPorts(uint8_t port, uint8_t oldPort) {
    DEBUG_PUTS("SWITCH ");
	DEBUG_PUTHEX(oldPort);
	DEBUG_PUTS(" TO ");
	DEBUG_PUTHEX(port);
    DEBUG_SPACE();

    // Disable PWM only if we switch an IO to high, release a relay does not need power.
    if((oldPort ^ port) & port) {
        /* change PWM to supply relays with full power */
        DEBUG_PUTS("DISABLE PWM ");
        alloc_timer(&app_dat.pwmTimer, PWM_DELAY_TIME);
        SET_STATE(PWM_TIMER_ACTIVE);
        ENABLE_PWM(0xFF); // --> This is 100% negative duty cycle (active low)
    }

    IO_SET(1,(uint8_t)(port & 1<<0));
    IO_SET(2,(uint8_t)(port & 1<<1));
    IO_SET(3,(uint8_t)(port & 1<<2));
    IO_SET(4,(uint8_t)(port & 1<<3));
    IO_SET(5,(uint8_t)(port & 1<<4));
    IO_SET(6,(uint8_t)(port & 1<<5));
    IO_SET(7,(uint8_t)(port & 1<<6));
    IO_SET(8,(uint8_t)(port & 1<<7));

    app_dat.oldValue=app_dat.portValue;
    return;
}

#ifdef HAND
/**
* Function Check whether the hand actuation pushbutton is pressed
* Remember button status
* Toggle output status and write to app_dat.portValue
*
* \param  Communication object number 0-7
*
* @return false:nothing to do    true:change output
*/
uint8_t checkHandActuation(uint8_t commObjectNumber) {
	
	uint8_t inputState = INPUT_BUTTON;				//read input level
	uint8_t portState = getIO(commObjectNumber+1);	//read output level
	setIO(commObjectNumber+1,portState^0x01);		//toggle output
	_delay_us(1);									//wait for stable input level
	uint8_t inputState2 = INPUT_BUTTON;				//read input level again, if he has changed button is pressed
	setIO(commObjectNumber+1,portState);			//set output to originally level
	if (inputState == inputState2){
		//button is not pressed
		app_dat.buttonWasEvaluated &= ~(1<<commObjectNumber);
		return 0;
	}
	else{
		//button is pressed
		if (app_dat.buttonWasEvaluated & (1<<commObjectNumber)){
			return 0;
		}
		else{
			//first query
			app_dat.portValue ^= (1<<commObjectNumber);		//toggle port state
			app_dat.buttonWasEvaluated |= (1<<commObjectNumber);
			return 1;										//return true for needToSwitch
		}
	}
}
#endif

uint8_t getIO(uint8_t ioNr){
	switch(ioNr){
		case 1: return (uint8_t)IO_GET(1);
		case 2: return (uint8_t)IO_GET(2);
		case 3: return (uint8_t)IO_GET(3);
		case 4: return (uint8_t)IO_GET(4);
		case 5: return (uint8_t)IO_GET(5);
		case 6: return (uint8_t)IO_GET(6);
		case 7: return (uint8_t)IO_GET(7);
		case 8: return (uint8_t)IO_GET(8);
	}
	return 0; /* Shouldn't happen */
}


void setIO(uint8_t ioNr, uint8_t val){
	switch(ioNr){
		case 1:
		IO_SET(1,(uint8_t)val);
		break;
		case 2:
		IO_SET(2,(uint8_t)val);
		break;
		case 3:
		IO_SET(3,(uint8_t)val);
		break;
		case 4:
		IO_SET(4,(uint8_t)val);
		break;
		case 5:
		IO_SET(5,(uint8_t)val);
		break;
		case 6:
		IO_SET(6,(uint8_t)val);
		break;
		case 7:
		IO_SET(7,(uint8_t)val);
		break;
		case 8:
		IO_SET(8,(uint8_t)val);
		break;
	}
}


#ifdef IO_TEST
/**
 * Set all IO for IO pin for 1 second to high, with a break of 1 second.
 * Function is called on power on of the controller or after a reset.
 * Can be used to check if LEDs and relais are working correctly.
 *
 */
void io_test() {
    IO_SET(1,ON);
	_delay_ms(1000);
    IO_SET(1,OFF);

	_delay_ms(1000);
    IO_SET(2,ON);
	_delay_ms(1000);
    IO_SET(2,OFF);

	_delay_ms(1000);
    IO_SET(3,ON);
	_delay_ms(1000);
    IO_SET(3,OFF);

	_delay_ms(1000);
    IO_SET(4,ON);
	_delay_ms(1000);
    IO_SET(4,OFF);

	_delay_ms(1000);
    IO_SET(5,ON);
	_delay_ms(1000);
    IO_SET(5,OFF);

	_delay_ms(1000);
    IO_SET(6,ON);
	_delay_ms(1000);
    IO_SET(6,OFF);

	_delay_ms(1000);
    IO_SET(7,ON);
	_delay_ms(1000);
    IO_SET(7,OFF);

	_delay_ms(1000);
    IO_SET(8,ON);
	_delay_ms(1000);
    IO_SET(8,OFF);
}
#endif

#ifdef HARDWARETEST
/**
 * test function: processor and hardware
 *
 * @return
 *
 */
void hardwaretest(void) {
    static uint8_t pinstate = 0x01;

    switchPorts(pinstate);

    pinstate = pinstate<<1;
    if(pinstate == 0x00)
        pinstate = 1;
    return;
}
#endif

#endif /* _FB_RELAIS_APP_C */
/*********************************** EOF *********************************/
