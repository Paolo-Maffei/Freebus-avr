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
#include <avr/wdt.h>
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_eeprom.h"
#include "msg_queue.h"
//#include "1wire.h"
#include "fb_hal.h"
#include "Spi.h"
#include "rf22.h"
#include "fb_prot.h"
#include "timer.h"
#include "fbrf_hal.h"
#include "fb_app.h"
#include "fb_relais_app.h"
#ifdef IO_TEST
#include <util/delay.h>
#endif

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/

/* State used for new lib api */
#define NEXT_STATE(x) app_state = x
#define GET_STATE() app_state

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
 
enum states_e {
    IDLE = 0,
	INIT_TIMER,
    TIMER_ACTIVE,
};

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
static const timer_t delay_bases[] PROGMEM = { 1*M2TICS(130), 2*M2TICS(130), 4*M2TICS(130), 8*M2TICS(130), 16*M2TICS(130), 32*M2TICS(130), 64*M2TICS(130),
                                                128*M2TICS(130), 256*M2TICS(130), 512*M2TICS(130), 1024*M2TICS(130), 2048*M2TICS(130), 4096*M2TICS(130), 8192*M2TICS(130),
                                                16384*M2TICS(130), 32768*M2TICS(130)};

extern struct grp_addr_s grp_addr;
static uint8_t portValue;                 /**< defines the port status. LSB IO0 and MSB IO8, ports with delay can be set to 1 here
                                             but will be switched delayed depending on the delay */
static uint16_t delayValues[8];           /**< save value for delays */
static uint8_t waitToPWM;                 /**< defines wait time until PWM get active again (counts down in 130ms steps), 1==enable PWM, 0==no change */

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

static uint16_t objectStates;             /**< store logic state of objects, 1 bit each, 8 "real" + 4 sf*/
static uint8_t blockedStates;             /**< 1 bit per object to mark it "blocked" */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM = {
    { SOFTWARE_VERSION_NUMBER,      0x01 },    /**< version number                               */
    { APPLICATION_RUN_STATUS,       0xFF },    /**< Run-Status (00=stop FF=run)                  */
    { COMMSTAB_ADDRESS,             0x9A },    /**< COMMSTAB Pointer                             */
    { APPLICATION_PROGRAMM,         0x00 },    /**< Port A Direction Bit Setting???              */

    { 0x0000,                       0x00 },    /**< default is off                               */
    { APP_DELAY_ACTIVE,             0x00 },    /**< no timer active                              */
    { APP_CLOSER_MODE,              0x00 },    /**< closer mode for all relais                   */
    { APP_RESTORE_AFTER_PL_LOW,     0x55 },    /**< don't save status at power loss (number 1-4) */
    { APP_RESTORE_AFTER_PL_HIGH,    0x55 },    /**< don't save status at power loss (number 5-8) */

    { MANUFACTORER_ADR_HIGH,   0x00 },    /**< Herstellercode 0x0004 = Jung                 */
    { MANUFACTORER_ADR_LOW,    0x04 },    /**< Herstellercode 0x0004 = Jung                 */
    { DEVICE_NUMBER_HIGH,           0x20 },    /**< device type (2038.10) 2060h                  */
    { DEVICE_NUMBER_LOW,            0x60 },    /**<                                              */

    { 0xFF,                         0xFF }     /**< END-sign; do not change                      */
    };

const struct FBAppInfo AppInfo PROGMEM = {
    .FBApiVersion = 0x01,
    .pParam = defaultParam,
};

static enum states_e app_state;

struct {
    uint8_t portValue;
    timer_t timer[8];
	uint8_t runningTimer;
} app_dat;

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void timerOverflowFunction(void);
void switchObjects(void);
void switchPorts(uint8_t port);
void processOutputs ( uint8_t commObjectNumber, uint8_t data );

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
			value = userram[commObjectNumber + 4];

            // check if we have a delayed action for this object, only Outputs
            if(commObjectNumber<= OBJ_OUT7) {
                // Get delay base
                timer_t delayBase=mem_ReadByte(APP_DELAY_BASE+commObjectNumber);
				if((commObjectNumber & 0x01) == 0x01) {
				    delayBase&=0x0F;
			    } else {
				    delayBase = (delayBase & 0xF0)>>4;
                }
                DEBUG_PUTHEX(delayBase);
                DEBUG_SPACE();
                delayBase = pgm_read_byte(&delay_bases[delayBase]);
                DEBUG_PUTHEX(delayBase);
                DEBUG_SPACE();

                // Check for delay factor for off
                if(app_dat.portValue & (1<<commObjectNumber) && mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber) && !(mem_ReadByte(APP_DELAY_ACTIVE) & (1<<commObjectNumber)) && value == 0) {
					DEBUG_PUTS("TIMER_OFF ");
					if(app_dat.runningTimer & 1<<commObjectNumber)
                        dealloc_timer(&app_dat.timer[commObjectNumber]);
                    alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber));
					app_dat.runningTimer |= 1<<commObjectNumber;
					//NEXT_STATE(TIMER_ACTIVE);
                }
                // Check for delay factor for on
                if(((app_dat.portValue & (1<<commObjectNumber)) == 0x00) && mem_ReadByte(APP_DELAY_FACTOR_ON+commObjectNumber) && value == 1) {
					DEBUG_PUTS("TIMER_ON ");
					if(app_dat.runningTimer & 1<<commObjectNumber)
    					dealloc_timer(&app_dat.timer[commObjectNumber]);
					alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) mem_ReadByte(APP_DELAY_FACTOR_ON+commObjectNumber));
					app_dat.runningTimer |= 1<<commObjectNumber;
					//NEXT_STATE(TIMER_ACTIVE);
                }
				// Check if we have a timer function
				DEBUG_PUTS("CHECK ");
				DEBUG_PUTHEX(app_dat.portValue);
				DEBUG_SPACE();
                DEBUG_PUTHEX(delayBase);
				DEBUG_SPACE();
				DEBUG_PUTHEX(mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber));
				DEBUG_SPACE();
				DEBUG_PUTHEX(value);
				DEBUG_SPACE();
				if (mem_ReadByte(APP_DELAY_ACTIVE) & (1<<commObjectNumber) && mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber) && (value == 1)) {
					DEBUG_PUTS("TIMER ");
					app_dat.portValue |= (1<<commObjectNumber);
					if(app_dat.runningTimer & 1<<commObjectNumber)
    					dealloc_timer(&app_dat.timer[commObjectNumber]);
					alloc_timer(&app_dat.timer[commObjectNumber], delayBase * (uint16_t) mem_ReadByte(APP_DELAY_FACTOR_OFF+commObjectNumber));
					app_dat.runningTimer |= 1<<commObjectNumber;
					NEXT_STATE(TIMER_ACTIVE);
				}

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

    if(GET_STATE() == TIMER_ACTIVE) {
        // action for timer
        for(commObjectNumber=0; commObjectNumber<8; commObjectNumber++) {
			if(app_dat.runningTimer && 1<<commObjectNumber) {
				//DEBUG_PUTS("CTIMEOUT ");
				if(app_dat.runningTimer & 1<<commObjectNumber && check_timeout(&app_dat.timer[commObjectNumber])) {
					uint8_t j = 1<<commObjectNumber;
					app_dat.runningTimer &= ~(1<<commObjectNumber);
                    dealloc_timer(&app_dat.timer[commObjectNumber]);
					app_dat.portValue ^= j;
					switchObjects();
    }
			}
		}
		if(app_dat.runningTimer == 0x0) {
		NEXT_STATE(IDLE);
		}			
}
}

/** 
 * called at overflow of application timer, should occur every 130ms.
 * handle finishing of 100% PWM after switchObjects().
 * handle delayed switching.
 * 
 * @return 
 * @todo test interrupt lock in this function that it is not disturbing TX and RX of telegrams
 */
void timerOverflowFunction(void)
{
    uint8_t i;

    /* check if programm is running */
    if(mem_ReadByte(APPLICATION_RUN_STATUS) != 0xFF)
        return;
 
    /* check if we can enable PWM */
    /* if waitToPWM==1 enable PWM, 0==no change */
    if(waitToPWM == 1) {
        DEBUG_PUTS("PWM");
        DEBUG_NEWLINE();
        ENABLE_PWM(PWM_SETPOINT);
    }

    /* check if we need to lower PWM delay mode */
    if(waitToPWM > 0)
        waitToPWM--;
     
    /* now check if we have to switch a port */
    for(i=0; i<8; i++) {
        uint8_t j = 1<<i;
        // DEBUG_PUTHEX(timerRunning);
        /* check if we have to switch a port */
        // we need to check timer for port i
        if ( delayValues[i] ) {
            if (--delayValues[i]) continue;
            // delayValue changed from 1 to 0
            // DEBUG_PUTS("SDP");
            
            DEBUG_PUTHEX(i);
            portValue ^= j;
            // DEBUG_PUTHEX(portValue);

            /* send response telegram to inform other devices that port was switched */
            //sendTelegram(i,(portValue & j)?1:0, 0x0C);

            switchObjects();
        }
    }
}

/** 
 * ISR is called if on TIMER1 the comparator B matches the defined condition.
 * 
 */
ISR(TIMER1_COMPB_vect)
{
    return;
}

/** 
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 * 
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void)
{
    uint8_t i,temp;
    uint16_t initialPortValue;

    /* reset global timer values */

    /* IO configuration */
    SET_IO_IO1(IO_OUTPUT);
    SET_IO_IO2(IO_OUTPUT);
    SET_IO_IO3(IO_OUTPUT);
    SET_IO_IO4(IO_OUTPUT);
    SET_IO_IO5(IO_OUTPUT);
    SET_IO_IO6(IO_OUTPUT);
    SET_IO_IO7(IO_OUTPUT);
    SET_IO_IO8(IO_OUTPUT);
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

    // check if at power loss we have to restore old values (see 0x01F6) and do it here
    portValue = mem_ReadByte(0x0100);
    initialPortValue = ((uint16_t)mem_ReadByte(0x01F7) << 8) | ((uint16_t)mem_ReadByte(0x01F6));
    for(i=0; i<=7; i++) {
        temp = (initialPortValue>>(i*2)) & 0x03;
        // DEBUG_PUTHEX(temp);
        if(temp == 0x01) {
            // open contact
            portValue &= (uint8_t)(~(1U<<i));
            // DEBUG_PUTHEX(i);
            // DEBUG_PUTS("P");
        } else if(temp == 0x02) {
            // close contact
            portValue |= (1<<i);
            // DEBUG_PUTHEX(i);
            // DEBUG_PUTS("L");

        }
    }
    // DEBUG_PUTHEX(portValue);

    /* switch the output pins */
    switchObjects();

    /* Reset State */
    NEXT_STATE(IDLE);

    return 1;
} /* restartApplication() */

/** 
 * Function is called if A_GroupValue_Read is received.
 * Check if group adress is in address table. If yes,
 * read status from port and put a A_GroupValue_Response into the tx queue.
 * 
 * @param rxmsg pointer to received telegram.
 * 
 * @return  The return value defies if a ACK or a NACK should be sent (FB_ACK, FB_NACK)
 */
/*
uint8_t readApplication(struct msg *rxmsg)
{
    struct fbus_hdr *hdr =( struct fbus_hdr *) rxmsg->data;
    DEBUG_PUTS("Read");

    uint8_t i;
    uint16_t destAddr = ((uint16_t)(hdr->dest[0])<<8) | (hdr->dest[1]);

    uint8_t assocTabPtr;            // points to start of association table (0x0100+assocTabPtr)
    uint8_t countAssociations;      // number of associations saved in associations table
    uint8_t numberInGroupAddress;   // reference from association table to group address table
    uint8_t commObjectNumber;       // reference from association table to communication object table

    assocTabPtr = mem_ReadByte(ASSOCTABPTR);
    countAssociations = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr);
 
    for(i=0; i<countAssociations; i++) {
        numberInGroupAddress = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (i*2));

        // check if valid group address reference
        if(numberInGroupAddress == 0xFE)
            continue;

        commObjectNumber = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (i*2) + 1);

        // now check if received address is equal with the safed group addresses, substract one
        // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
        // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
        if((destAddr == grp_addr.ga[numberInGroupAddress-1]) && (commObjectNumber <= 7)) {
            // found group address

            /// @todo check if read value is allowed
            struct msg * resp = AllocMsgI();
            if(!resp)
                return FB_NACK;

            hdr = (struct fbus_hdr *) resp->data;

            resp->repeat = 3;
            resp->len    = 9;

            hdr->ctrl    = 0xBC;
            hdr->src[0]  = mem_ReadByte(PA_ADDRESS_HIGH);
            hdr->src[1]  = mem_ReadByte(PA_ADDRESS_LOW);
            hdr->dest[0] = grp_addr.ga[numberInGroupAddress-1]>>8;
            hdr->dest[1] = grp_addr.ga[numberInGroupAddress-1];
            hdr->npci    = 0xE1;
            hdr->tpci    = 0x00;
            // put data into the apci octet
            hdr->apci    = 0x40 + ((portValue & (1<<commObjectNumber)) ? 1 : 0);

            fb_hal_txqueue_msg(resp);
        } else if((commObjectNumber > 7) && (commObjectNumber < 12)) {
            // additinal function
            /// @todo write part additional functions
            // DEBUG_PUTS("ZF");
            // DEBUG_NEWLINE();
        }
    }
    return FB_ACK;
}   // readApplication()
*/

/** 
 * Function is called if A_GroupValue_Write is received.
 * The type it is the function "EIS1" or "Data Type Boolean" for the relais module.
 * Determine the addressed object and call processOutputs()
 *
 * @param rxmsg pointer to received telegram.
 * 
 * @return The return value defies if a ACK or a NACK should be sent (FB_ACK, FB_NACK)
 */
 /*
uint8_t runApplication(struct msg *rxmsg)
{
    struct fbus_hdr * hdr= (struct fbus_hdr *) rxmsg->data;
    uint8_t i;
    uint16_t destAddr=((hdr->dest[0])<<8 | hdr->dest[1]);
    uint8_t assocTabPtr = mem_ReadByte(ASSOCTABPTR);                             // points to start of association table (0x0100+assocTabPtr)
    uint8_t countAssociations = mem_ReadByte(BASE_ADDRESS_OFFSET+assocTabPtr);   // number of associations saved in associations table
    uint8_t numberInGroupAddress;                              // reference from association table to group address table
    uint8_t commObjectNumber;                                  // reference from association table to communication object table
    //uint8_t countCommObjects = mem_ReadByte(0x0100+commStabPtr);  // number of communication objects in table
    //uint8_t userRamPointer = mem_ReadByte(0x0100+commStabPtr+1);  // points to user ram

    // handle here only data with 1-bit length, maybe we have to add here more code to handle longer data
    /// @todo handle data with more then 1 bit
    uint8_t data = (hdr->apci) & 1;
     
    for(i=0; i<countAssociations; i++) {
        numberInGroupAddress = mem_ReadByte(BASE_ADDRESS_OFFSET+assocTabPtr+1+(i*2));

        // check if valid group address reference
        if(numberInGroupAddress == 0xFE)
            continue;

        commObjectNumber = mem_ReadByte(BASE_ADDRESS_OFFSET+assocTabPtr+1+(i*2)+1);

        // now check if received address is equal with the safed group addresses, substract one
        // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
        // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
        if(destAddr == grp_addr.ga[numberInGroupAddress-1]){
            // found group address
            processOutputs ( commObjectNumber, data );
        }
    }
                 // SETPIN_IO3(1)

    return FB_ACK;
}   // runApplication()
 */
/** 
 *
 * @param commObjectNumber the adressed object
 * @param data data to be assigned to that object
 *
 * store data (1 bit) in objectStates.
 * If a special function is addressed, then determine the "real" object belongin to it,
 * and continue as follows.
 * If a "real" object is addressed, evaluate the logic and blocking (if avail. for the object).
 * if no delay is programmed for the object, switch the associated output.
 * If a delay is programmed, then set the corresponding delayValue.
 * 
 * @return
 */
void processOutputs ( uint8_t commObjectNumber, uint8_t data )
{
    uint8_t delayFactorOn=0;            // the factor for the delay timer (on delay)
    uint8_t delayFactorOff=0;           // the factor for the delay timer (off delay)
    uint8_t delayActive;                // is timer active 1=yes
    uint8_t delayBase;
    uint8_t timerActive = mem_ReadByte(0x01EA);      // set bit value if delay on a channel is active
    uint8_t commStabPtr = mem_ReadByte(COMMSTAB_ADDRESS);     // points to communication object table (0x0100+commStabPtr)
    uint8_t specialFunc ;                               // special function number (0: no sf)
    uint8_t specialFuncTyp ;                            // special function type
    uint8_t logicFuncTyp ;                              // type of logic function ( 1: or, 2: and)
    uint8_t logicState   ;                              // state of logic function
    uint8_t sfOut;                                      // output belonging to sf
    uint8_t sfMask;                                     // special function bitmask (1 of 4)

    if(commObjectNumber >= 12) return;
    if (data) objectStates |=  (1<<commObjectNumber);
        else  objectStates &= ~(1<<commObjectNumber);
    if (commObjectNumber >= 8){
		/** if a special function is adressed (and changed in most cases),
		* then the "real" object belonging to that sf. has to be evaluated again
		* taking into account the changed logic and blocking states. */
        /* detemine the output belonging to that sf */
        sfOut = mem_ReadByte(0x01D8+((commObjectNumber-8)>>1))>>
                    (((commObjectNumber-8)&1)*4) & 0x0F;
        /* get associated object no. and state of that object*/
        if (sfOut){
             if (sfOut > 8) return;
             commObjectNumber =  sfOut-1;
             data = (objectStates>>(sfOut-1))&1;
        }
        else return;
		/* do new evaluation of that object */
    }
    
    // read communication object (3 Byte)
    uint8_t commValuePointer = mem_ReadByte(BASE_ADDRESS_OFFSET+commStabPtr+2+(commObjectNumber*3));
    uint8_t commConfigByte   = mem_ReadByte(BASE_ADDRESS_OFFSET+commStabPtr+2+(commObjectNumber*3+1));
    uint8_t commValueType    = mem_ReadByte(BASE_ADDRESS_OFFSET+commStabPtr+2+(commObjectNumber*3+2));
    
    delayActive      = mem_ReadByte(0x01EA);
    // read delay factor for on and off
    delayFactorOn    = mem_ReadByte(0x01DA+commObjectNumber);
    delayFactorOff   = mem_ReadByte(0x01E2+commObjectNumber);

    // read delay base, 0=130ms, 1=260 and so on
    delayBase        = mem_ReadByte(0x01F9+((commObjectNumber+1)>>1));
    if((commObjectNumber & 0x01) == 0x01)
        delayBase&=0x0F;
    else
        delayBase = (delayBase & 0xF0)>>4;

    /** logic function */
    /* check if we have a special function for this object */
    specialFunc  = 0;
    logicFuncTyp = 0;
    for ( specialFunc=0; specialFunc < 4; specialFunc++ ){
        sfMask = 1<<specialFunc;
        sfOut = mem_ReadByte(0x01D8 + (specialFunc>>1))>> ((specialFunc&1)*4) & 0x0F;
        if (sfOut == (commObjectNumber+1)){
            /* we have a special function, see which type it is */
            specialFuncTyp = (mem_ReadByte(0x01ED))>>(specialFunc*2)&0x03;
            /* get the logic state from the special function object */
            logicState = ((objectStates>>specialFunc)>>8)&0x01;
            if ( specialFuncTyp == 0 ){
                /* logic function */
                logicFuncTyp = (mem_ReadByte(0x01EE))>>(specialFunc*2)&0x03;
                if ( logicFuncTyp == 1 ){  // or
                data |= logicState;
                }
                if ( logicFuncTyp == 2 ){  // and
                    data &= logicState;
                }
            }

            if ( specialFuncTyp == 1 ){
                /* blocking function */
                if ( ((objectStates>>8) ^ mem_ReadByte(0x01F1)) & sfMask ){
                    /* start blocking */
                    if ( blockedStates & sfMask ) return; // we are blocked, do nothing
                    blockedStates |= sfMask;
                    data = (mem_ReadByte(0x01EF + (specialFunc>>1)))>>((specialFunc&1)*4)&0x03;
                    if (data == 0) return;
                    if (data == 1)
                        portValue &= ~(1<<commObjectNumber);
                    if (data == 2)
                        portValue |= (1<<commObjectNumber);
                    switchObjects();
                    return;

                }
                else {
                    /* end blocking */
                    if ( blockedStates & sfMask ){  // we have to unblock
                        blockedStates &= ~sfMask;
                        /* action at end of blocking, 0: nothing, 1: off, 2: on */
                        data = (mem_ReadByte(0x01EF + (specialFunc>>1)))
                            >>((specialFunc&1)*4+2)&0x03;
                        if (data == 0) return;
                        data--;
                    /* we are unblocked, continue as normal */
                    }
                }
            }

        }
    }
    /** @todo check if write is enabled */

    // reset saved timer settings
    // delayValues[commObjectNumber]=0;

    // we received a new state for object commObjectNumber
    // check if we have a delay on that port

    // check if we must switch off a port where timers are running
    if((!delayFactorOff) && (data == 0))
    {
        DEBUG_PUTC('K');
        delayValues[commObjectNumber] = 0;
    }

    // check for delayed switch off
    if(portValue & (1<<commObjectNumber) && delayFactorOff && !(timerActive & (1<<commObjectNumber)) && (data==0)) {
        // switch of but delayed
        delayValues[commObjectNumber] = (uint16_t)(1<<delayBase)*(uint16_t)delayFactorOff;
    }

    // check if we have a delayed switch on
    if(((portValue & (1<<commObjectNumber)) == 0x00) && delayFactorOn && (data == 1)) {
        // switch on but delayed
        delayValues[commObjectNumber] = (uint16_t)(1<<delayBase) * (uint16_t)delayFactorOn;
    }
        
    // check if we have a timer function
    if(timerActive & (1<<commObjectNumber) && delayFactorOff && (data == 1)) {
        // special case (switch on immediately and off after a defined time
        DEBUG_PUTS("Fl");
        portValue |= (1<<commObjectNumber);
        delayValues[commObjectNumber] = (uint16_t)(1<<delayBase) * (uint16_t)delayFactorOff;
    }

    // check who to handle off telegram while in timer modus
    if(timerActive & (1<<commObjectNumber) && delayFactorOff && (data == 0)) {
        DEBUG_PUTS("TK");
        // only switch off if on 0x01EB the value is equal zero
        if(!(mem_ReadByte(0x01EB) & (1<<commObjectNumber))) {
            delayValues[commObjectNumber] = 0;
            portValue    &= ~(1<<commObjectNumber);
        }
    }
    DEBUG_PUTHEX(commObjectNumber);
        
    /** check for delays */
    if( !delayValues[commObjectNumber]) {
        // no delay is defined so we switch immediatly
        if(data == 0) {
            // switch port off
            portValue &= ~(1<<commObjectNumber);
        } else if(data == 1) {
            portValue |= (1<<commObjectNumber);
        }

        //** @todo need to check here for respond
        // send response telegram to inform other devices that port was switched
        //sendTelegram(commObjectNumber, data, 0x0C);
    }
    switchObjects();
}


/** 
 * Switch the objects to state in portValue and save value to eeprom if necessary.
 * 
 */
void switchObjects(void)
{
    uint16_t initialPortValue;
    uint8_t portOperationMode;  /**< defines if IO is closer or opener, see address 0x01F2 in eeprom */
    uint8_t savedValue;
    uint8_t i;
	uint8_t pattern;


    DEBUG_PUTS("Sw");
    DEBUG_SPACE();

    /* change PWM to supply relays with full power */
    waitToPWM = PWM_DELAY_TIME;
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
        initialPortValue = ((uint16_t)mem_ReadByte(APP_RESTORE_AFTER_PL_HIGH) << 8) | ((uint16_t)mem_ReadByte(APP_RESTORE_AFTER_PL_LOW));
        for(i=0; i<=7; i++) {
            if(((initialPortValue>>(i*2)) & 0x03) == 0x0) {
                mem_WriteBlock(0x0100, 1, &app_dat.portValue);
                DEBUG_PUTS("Sv");
                break;
            }
        }
    }
     
    /* check 0x01F2 for opener or closer and modify data to relect that, then switch the port */
    portOperationMode = mem_ReadByte(0x01F2);
    switchPorts(app_dat.portValue^portOperationMode);

    return;
}

/**                                                                       
 * switch all of the output pins
 *
 * @param port contains values for 8 output pins. They may be on different ports of the avr.
 *   
 */
void switchPorts(uint8_t port)
{
    DEBUG_PUTS("SWITCH ");
	DEBUG_PUTHEX(port);
	DEBUG_SPACE();
	
    SETPIN_IO1((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO2((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO3((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO4((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO5((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO6((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO7((uint8_t)(port & 0x01));
    port = port>>1;

    SETPIN_IO8((uint8_t)(port & 0x01));

    return;
}

#ifdef IO_TEST
/** 
 * Set all IO for IO pin for 1 second to high, with a break of 1 second.
 * Function is called on power on of the controller or after a reset.
 * Can be used to check if LEDs and relais are working correctly.
 * 
 */
void io_test()
{
	SETPIN_IO1(ON);
	_delay_ms(1000);
	SETPIN_IO1(OFF);

	_delay_ms(1000);
	SETPIN_IO2(ON);
	_delay_ms(1000);
	SETPIN_IO2(OFF);

	_delay_ms(1000);
	SETPIN_IO3(ON);
	_delay_ms(1000);
	SETPIN_IO3(OFF);

	_delay_ms(1000);
	SETPIN_IO4(ON);
	_delay_ms(1000);
	SETPIN_IO4(OFF);

	_delay_ms(1000);
	SETPIN_IO5(ON);
	_delay_ms(1000);
	SETPIN_IO5(OFF);

	_delay_ms(1000);
	SETPIN_IO6(ON);
	_delay_ms(1000);
	SETPIN_IO6(OFF);

	_delay_ms(1000);
	SETPIN_IO7(ON);
	_delay_ms(1000);
	SETPIN_IO7(OFF);

	_delay_ms(1000);
	SETPIN_IO8(ON);
	_delay_ms(1000);
	SETPIN_IO8(OFF);
}
#endif

/**                                                                       
 * The start point of the program, init all libraries, start the bus interface,
 * the application and check the status of the program button.
 *
 * @return 
 *   
 */
int main(void)
{
//    uint16_t t1cnt;
    /* disable wd after restart_app via watchdog */
    DISABLE_WATCHDOG();

        /* ROM-Check */
        /** @todo Funktion fuer CRC-Check bei PowerOn fehlt noch */

        /* init internal Message System */
        msg_queue_init();
    
	DEBUG_INIT();
    DEBUG_NEWLINE();
    DEBUG_PUTS("V0.1");
    DEBUG_NEWLINE();

    /* init eeprom modul and RAM structure already here,
       because we need eeprom values for fbrfhal_init() */
    eeprom_Init(&nodeParam[0], EEPROM_SIZE);

    /* init procerssor register */
    fbhal_Init();
	/** FBRFHAL_INIT() is defined in fbrf_hal.h .
	   you may leave it out if you don't use rf */
    //FBRFHAL_INIT();

    /* init generic timer */
    timer_init();

	/* enable interrupts */
    ENABLE_ALL_INTERRUPTS();

    /* init protocol layer */
    /* load default values */
    fbprot_Init(&AppInfo);

    /* config application hardware */
    (void)restartApplication();

    /* Reset state */
    NEXT_STATE(IDLE);

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
void hardwaretest(void)
{
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
