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
    OBJ_OUT12,
    OBJ_OUT13,
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
    timer_t timer[8];
    timer_t pwmTimer;           /// stores a reference to the generic timer
	uint8_t runningTimer;
    uint16_t objectStates;      /**< store logic state of objects, 1 bit each, 8 "real" + 4 sf*/
    uint8_t blockedStates;      /**< 1 bit per object to mark it "blocked" */
} app_dat;

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void switchObjects(void);
void switchPorts(uint8_t port);

#ifdef HARDWARETEST
/** test function: processor and hardware */
void hardwaretest(void);
#endif
#ifdef IO_TEST
void io_test(void);
#endif

/**************************************************************************
 * IMPLEMENTATION
 **************************************************************************/
void handleTimers( uint8_t commObjectNumber, uint8_t value ) {
                // Get delay base
                timer_t delayBase=mem_ReadByte(APP_DELAY_BASE+commObjectNumber);
    if((commObjectNumber & 0x01) == 0x01) {
        delayBase&=0x0F;
    } else {
        delayBase = (delayBase & 0xF0)>>4;
                }
                delayBase = pgm_read_byte(&delay_bases[delayBase]);

    // Set some variables to make next commands better readable
    uint8_t timerActive = mem_ReadByte(APP_DELAY_ACTIVE) & (1<<commObjectNumber);
    uint8_t timerOffActive = mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber);
    uint8_t timerOnActive = mem_ReadByte(APP_DELAY_FACTOR_ON+commObjectNumber);
    
    /// @bug a timer function with a delay on one will not work
                // Check for delay factor for off
    if(app_dat.portValue & (1<<commObjectNumber) && timerOffActive && !(timerActive) && value == 0) {
        DEBUG_PUTS("TIMER_OFF ");
        alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOffActive);
        app_dat.runningTimer |= 1<<commObjectNumber;
        SET_STATE(TIMER_ACTIVE);
                }
                // Check for delay factor for on
    if(((app_dat.portValue & (1<<commObjectNumber)) == 0x00) && timerOnActive && value == 1) {
        DEBUG_PUTS("TIMER_ON ");
        alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOnActive);
        app_dat.runningTimer |= 1<<commObjectNumber;
        SET_STATE(TIMER_ACTIVE);
                }
    // Check if we have a timer function
    if (timerActive && timerOffActive && (value == 1)) {
        DEBUG_PUTS("TIMER ");
        app_dat.portValue |= (1<<commObjectNumber);
        alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) timerOffActive);
        app_dat.runningTimer |= 1<<commObjectNumber;
        SET_STATE(TIMER_ACTIVE);
    }

    // check how to handle off telegram while in timer modus
    if (timerActive && timerOffActive && value == 0) {
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

    // Iterate over all objects and check if the status has changed
    for(commObjectNumber=OBJ_OUT0; commObjectNumber<=OBJ_OUT13; commObjectNumber++) {
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
            switchObjects();
        }
    }

    // check if we can enable PWM
    // if app_state==PWM_TIMER_ACTIVE and pwmTimer is reached enable PWM, else no change
    if(IN_STATE(PWM_TIMER_ACTIVE) && check_timeout(&app_dat.pwmTimer)) {
        DEBUG_PUTS("DISABLE PWM");
        DEBUG_NEWLINE();
        ENABLE_PWM(PWM_SETPOINT);
        UNSET_STATE(PWM_TIMER_ACTIVE);
    }

    if(IN_STATE(TIMER_ACTIVE)) {
        // action for timer
        for(commObjectNumber=0; commObjectNumber<8; commObjectNumber++) {
			if(app_dat.runningTimer && 1<<commObjectNumber) {
				//DEBUG_PUTS("CTIMEOUT ");
				if(app_dat.runningTimer & 1<<commObjectNumber && check_timeout(&app_dat.timer[commObjectNumber])) {
                    DEBUG_PUTS("TIMEOUT ");
					uint8_t j = 1<<commObjectNumber;
					app_dat.runningTimer &= ~(1<<commObjectNumber);
					app_dat.portValue ^= j;
                    
                    uint8_t value=(app_dat.portValue >> commObjectNumber) & 0x1;
                    SetAndTransmitBit(commObjectNumber, value);
					switchObjects();
                }
			}
		}
		if(app_dat.runningTimer == 0x0) {
            UNSET_STATE(TIMER_ACTIVE);
		}			
}
}

/** 
 * ISR is called if on TIMER1 the comparator B matches the defined condition.
 * 
 */
ISR(TIMER1_COMPB_vect) {
    return;
}

/** 
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 * 
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void) {
    uint8_t i,temp;
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

    // check if at power loss we have to restore old values (see APP_RESTORE_AFTER_PL) and do it here
    DEBUG_PUTS("SET PINS ");
    app_dat.portValue = mem_ReadByte(0x0100);
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

    /* switch the output pins */
    switchObjects();

    return 1;
} /* restartApplication() */

/** 
 * Switch the objects to state in portValue and save value to eeprom if necessary.
 * 
 */
void switchObjects(void) {
    uint16_t initialPortValue;
    uint8_t portOperationMode;  /**< defines if IO is closer or opener, see address 0x01F2 in eeprom */
    uint8_t savedValue;
    uint8_t i;
	uint8_t pattern;


    DEBUG_PUTS("Sw");
    DEBUG_SPACE();

    /* change PWM to supply relays with full power */
    DEBUG_PUTS("ENABLE PWM ");
    alloc_timer(&app_dat.pwmTimer, PWM_DELAY_TIME);
    SET_STATE(PWM_TIMER_ACTIVE);
    ENABLE_PWM(0xFF); // --> This is 100% negative duty cycle (active low)
    // check if timer is active on the commObjectNumber

    /* read saved status and check if it was changed */
    savedValue = mem_ReadByte(0x0100);
    if(savedValue != app_dat.portValue) {


		// Rückmeldungen Senden ( @todo pruefen ob ein rückmeldeobjekt besteht)
		for (i=0;i<8;i++) {
			pattern=1<<i;
			if((portValue&pattern)!=(savedValue&pattern)) {
				sendTelegram(i, (portValue & pattern) ? 1 : 0, 0x0C);
			}
			
		}




        // now check if last status must be saved, we write to eeprom only if necessary
        initialPortValue = mem_Read2Bytes(APP_RESTORE_AFTER_PL);
        for(i=0; i<=7; i++) {
            if(((initialPortValue>>(i*2)) & 0x03) == 0x0) {
                mem_WriteBlock(0x0100, 1, &app_dat.portValue);
                DEBUG_PUTS("Sv");
                break;
            }
        }
    }
     
    /* check 0x01F2 for opener or closer and modify data to reflect that, then switch the port */
    portOperationMode = mem_ReadByte(APP_CLOSER_MODE);
    switchPorts(app_dat.portValue^portOperationMode);

    return;
}

/**                                                                       
 * switch all of the output pins
 *
 * @param port contains values for 8 output pins. They may be on different ports of the avr.
 *   
 */
void switchPorts(uint8_t port) {
    DEBUG_PUTS("SWITCH ");
	DEBUG_PUTHEX(port);
	DEBUG_SPACE();
	
    IO_SET(1,(uint8_t)(port & 0x01));

    IO_SET(2,(uint8_t)(port & 0x02));

    IO_SET(3,(uint8_t)(port & 0x04));

    IO_SET(4,(uint8_t)(port & 0x08));

    IO_SET(5,(uint8_t)(port & 0x10));

    IO_SET(6,(uint8_t)(port & 0x20));

    IO_SET(7,(uint8_t)(port & 0x40));

    IO_SET(8,(uint8_t)(port & 0x80));

    return;
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

/**                                                                       
 * The start point of the program, init all libraries, start the bus interface,
 * the application and check the status of the program button.
 *
 * @return 
 *   
 */
int main(void) {
    fbprot_LibInit();

#ifdef HARDWARETEST
    sendTestTelegram();
#endif

    /* activate watchdog */
    //ENABLE_WATCHDOG ( WDTO_250MS );

    /***************************/
    /* the main loop / polling */
    /***************************/
    while(1) {
		fbprot_msg_handler();

        /* calm the watchdog */
        //wdt_reset();

        /* Auswerten des Programmiertasters */
        if(fbhal_checkProgTaster()) {
#ifdef SENDTESTTEL
			sendTestTelegram();
#endif
		}

        app_loop();

/*         /\* check if 130ms timer is ready */
/*            we use timer 1 for PWM, overflow each 100µsec, divide by 1300 -> 130msec. *\/ */
/*         if(TIMER1_OVERRUN) { */
/*             CLEAR_TIMER1_OVERRUN; */
/* 			/\** FBRFHAL_POLLING() is defined in fbrf_hal.h . */
/* 			   you may leave it out if you don't use rf *\/ */
/* 			FBRFHAL_POLLING(); */
/* 			/\** APP_TIMER_OVERRUN() is not defined if you use avr board rev. 3.01. *\/ */
/* #ifndef BOARD301 */
/* 			if ( ! APP_TIMER_OVERRUN() ) continue; */
/* #endif */
/* #ifndef HARDWARETEST */
/*             timerOverflowFunction(); */
/* #else */
/*     	    //sendTestTelegram(); */
/*             hardwaretest(); */
/* #endif */

        /* } */

        // go to sleep mode here
        // wakeup via interrupt check then the programming button and application timer for an overrun
        // for detailed list see datasheet page 40ff
        // MC need about 6 cyles to wake up at 8 MHZ that are 6*0.125µs
        //        PRR |= (1<<PRADC)|(1<<PRSPI)|(1<<PRTWI);
        //        set_sleep_mode(SLEEP_MODE_IDLE);
        //          sleep_enable();
        //          sleep_cpu();
        //          sleep_disable();
    }   /* while(1) */

}   /* main() */

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
