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
*  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/**
 * @file   fb_relais_app.c
 * @author Matthias Fechner, Dirk Armbrust, Christian Bode
 * @date   Sat Jan 05 17:44:47 2008
 *                    4
 * @brief  The rollo application to switch 4 rollo
 * Manufactorer code is 0x04 = Jung\n
 * Device type (2038.10) 0x2070 Ordernumber: 2204REG HR\n
 *
 * To enable IO test compile with -DIO_TEST
 *
 * This version is designed to be used with the new API.
 */
#ifndef _FB_ROLLO_APP_C
#define _FB_ROLLO_APP_C

/*************************************************************************
 * INCLUDES
 *************************************************************************/
//#include "1wire.h"
#include "fb_rollo_app.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/
//#define IO_TEST

/* Objects for the app-4-rollo */
enum EIGHT_OUT_Objects_e {
    OBJ_OUT0 = 0,
    OBJ_OUT1,
    OBJ_OUT2,
    OBJ_OUT3,
    OBJ_OUT4,
    OBJ_OUT5,
    OBJ_OUT6,
    OBJ_OUT7,
    OBJ_OUT8, // Verwendung von 8 .. 15 unklar
    OBJ_OUT9,
    OBJ_OUT10,
    OBJ_OUT11,
    OBJ_OUT12,
    OBJ_OUT13,
    OBJ_OUT14,
    OBJ_OUT15,
    OBJ_OUT16, // start of special objects used to lock or combine objects
    OBJ_OUT17,
};

/* Objekte:
Nr. Objectname        Funktion          Typ             Flags
0   Ausgang 1         Kurzzeitbetrieb   EIS 1   1 Bit   K  S  L
1   Ausgang 2         Kurzzeitbetrieb   EIS 1   1 Bit   K  S  L
2   Ausgang 3         Kurzzeitbetrieb   EIS 1   1 Bit   K  S  L
3   Ausgang 4         Kurzzeitbetrieb   EIS 1   1 Bit   K  S  L
4   Ausgang 1         Langzeitbetrieb   EIS 7   1 Bit   K  S  L
5   Ausgang 2         Langzeitbetrieb   EIS 7   1 Bit   K  S  L
6   Ausgang 3         Langzeitbetrieb   EIS 7   1 Bit   K  S  L
7   Ausgang 4         Langzeitbetrieb   EIS 7   1 Bit   K  S  L

16  Sicherheit 1      Sicherheit        EIS 1   1 Bit   K  S  L
17  Sicherheit 2      Sicherheit        EIS 1   1 Bit   K  S  L


EIS 7   0  UP   / START
        1  DOWN / STOP

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

enum states_port {
    PORT_INACTIVE = 0,
    PORT_UP,
    UP_WAIT,
    PORT_DOWN,
    DOWN_WAIT
};    


enum states_ctrl {
    NONE,
    MOVE_UP_START,
    MOVE_UP_ACTIVE,
    STEP_UP_START,
    STEP_UP_ACTIVE,
    MOVE_DOWN_START,
    MOVE_DOWN_ACTIVE,
    STEP_DOWN_START,
    STEP_DOWN_ACTIVE,
    SAFTY_ACTIVE
};    

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
static const timer_t time_bases[] PROGMEM    ={ 0,                      // not used
                                                0,                      // not used
                                                1,                      // 8 msec
                                                1*M2TICS(130),          // 130 msec
                                                16*M2TICS(130),         // 2,1 sec
                                                253*M2TICS(130)};       // 33 sec

static const timer_t delay_bases[] PROGMEM   ={ 4*M2TICS(130),          // 0,5 sec
                                                8*M2TICS(130),          // 1 sec
                                                16*M2TICS(130),         // 2 sec
                                                38*M2TICS(130)};        // 5 sec

static const timer_t safty_timeout[] PROGMEM ={ 0L*M2TICS(130),          // none
                                                462L*M2TICS(130),        // 1 min
                                                923L*M2TICS(130),        // 2 min
                                                1385L*M2TICS(130),       // 3 min
                                                1846L*M2TICS(130),       // 4 min
                                                2308L*M2TICS(130),       // 5 min
                                                2769L*M2TICS(130),       // 6 min
                                                3231L*M2TICS(130),       // 7 min
                                                3692L*M2TICS(130),       // 8 min

                                                4615L*M2TICS(130),       // 10 min
                                                5077L*M2TICS(130),       // 11 min
                                                5538L*M2TICS(130),       // 12 min
                                                9231L*M2TICS(130),       // 20 min
                                                18462L*M2TICS(130),      // 40 min
                                                27692L*M2TICS(130),      // 1 h
                                                55385L*M2TICS(130)};     // 2 h

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

static enum states_e app_state;     // States for PWM handling


/**< Bitbelegung der internen Variablen (xxCmd, xxxValue )
    xxxx xxxX   Rollo 1 UP      (Ausgang 1)
    xxxx xxXx   Rollo 1 DOWN    (Ausgang 2)
    xxxx xXxx   Rollo 2 UP      (Ausgang 3)
    xxxx Xxxx   Rollo 2 DOWN    (Ausgang 4)

    xxxX xxxx   Rollo 3 UP      (Ausgang 5)
    xxXx xxxx   Rollo 3 DOWN    (Ausgang 6)
    xXxx xxxx   Rollo 4 UP      (Ausgang 7)
    Xxxx xxxx   Rollo 4 DOWN    (Ausgang 8)
*/

struct {
    uint8_t MoveCmd;               /// bitfield for the long time commands
    uint8_t StepCmd;               /// bitfield for the short time commands

    enum states_ctrl state[4];     /// state maschine for internal app logic
    uint8_t portCmd;               /// setpoint from the application
    timer_t timer[4];              /// stores actual timer values
    uint8_t runningTimer;          /// bitfield for timer active flags

    uint8_t specFunc;              /// bitfield for special function (safty 1 and 2, ..)
    timer_t specTimer[2];          /// stores actual timer values
    uint8_t runningSpecTimer;      /// bitfield for timer active flags


    enum states_port portState[4]; /// switching between up and down
    uint8_t portValue;             /// setpoint for th eactual port states
    uint8_t portValue_old;         /// hold the old value to check if we must enable a PWM or not (enable PWM only if switching from low -> high
    timer_t portTimer[4];          /// stores actual timer values
    uint8_t runningportTimer;      /// bitfield for timer active flags

    timer_t pwmTimer;              /// stores a reference to the generic timer
} app_dat;

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void handleAppLogic(void);
void handleAppSpecFunc(void);
void switchObjects(void);
void switchPorts(uint8_t port, uint8_t oldPort);

#ifdef HARDWARETEST
/** test function: processor and hardware */
void hardwaretest(void);
#endif
#ifdef IO_TEST
void io_test(void);
#endif

/**
 * Function os called periodically of the application is enabled in the system_state
 *
 */
void app_loop() {
    uint8_t commObjectNumber;
    uint8_t MsgVal;
    uint8_t index ;
    uint8_t saftyTyp;
    
    // Iterate over all objects and check if the status has changed
    for(commObjectNumber=OBJ_OUT0; commObjectNumber<=OBJ_OUT17; commObjectNumber++) {
        // check if an object has changed its status
        if(TestObject(commObjectNumber)) {
            DEBUG_NEWLINE();
            DEBUG_PUTS("OBJ_");
            DEBUG_PUTHEX(commObjectNumber);
            DEBUG_SPACE();

            // get value of object (0=off, 1=on)
            MsgVal = userram[commObjectNumber + 2];

            // reset object status flag
            SetRAMFlags(commObjectNumber, 0);

            DEBUG_PUTHEX(MsgVal);
            DEBUG_SPACE();

            // Tranfer MsgData to bit structure (xxValue)
            if(commObjectNumber < OBJ_OUT4) {
                // Kurzzeitbetrieb
                app_dat.StepCmd &= ~(PORTMASK<<(commObjectNumber * 2));
                if(MsgVal == EIS7_UP) {
                    app_dat.StepCmd |= (PORTMASK_UP<<(commObjectNumber * 2));
                }
                else if(MsgVal == EIS7_DOWN) {
                    app_dat.StepCmd |= (PORTMASK_DOWN << (commObjectNumber * 2));
                }
            }
            else if(commObjectNumber < OBJ_OUT8) {
                // Langzeitbetrieb (obj 4 ... 7)
                app_dat.MoveCmd &= ~(PORTMASK<<((commObjectNumber - 4) * 2));
                if(MsgVal == EIS7_UP) {
                    app_dat.MoveCmd |= (PORTMASK_UP<<((commObjectNumber - 4) * 2));
                }
                else if(MsgVal == EIS7_DOWN) {
                    app_dat.MoveCmd |= (PORTMASK_DOWN<<((commObjectNumber - 4) * 2));
                }
            }
            else if(commObjectNumber == 16 ) {
                // Sicherheit 1, bitfiled xXxx xxxx
                saftyTyp = mem_ReadByte(APP_ROLLO_SAFTY_POLARITY);
                app_dat.specFunc &= 0xBF;
                app_dat.specFunc |= ((~((MsgVal<<6) ^ saftyTyp)) & 0x40);

                //Reset timeout function
                index = mem_ReadByte(APP_ROLLO_SAFTY_TIMEOUT) & 0x0F; 
                if(index) {
                    timer_t time = pgm_read_dword(&safty_timeout[index]);
                    alloc_timer(&app_dat.specTimer[0], time);
                    app_dat.runningSpecTimer |= 0x01;
                }                
            }
            else if(commObjectNumber == 17 ) {
                // Sicherheit 2, bitfiled Xxxx xxxx
                saftyTyp = mem_ReadByte(APP_ROLLO_SAFTY_POLARITY);
                app_dat.specFunc &= 0x7F;
                app_dat.specFunc |= ((~((MsgVal<<7) ^ saftyTyp)) & 0x80);

                //Reset timeout function
                index = mem_ReadByte(APP_ROLLO_SAFTY_TIMEOUT) & 0x0F; 
                if(index) {
                    timer_t time = pgm_read_dword(&safty_timeout[index]);
                    alloc_timer(&app_dat.specTimer[1], time);
                    app_dat.runningSpecTimer |= 0x02;
                }                
            }
        }
    }

    // handle special function (safty function, ...)
    handleAppSpecFunc();        
    
    // handle application logic
    handleAppLogic();

    // check if we can enable PWM
    // if app_state==PWM_TIMER_ACTIVE and pwmTimer is reached enable PWM, else no change
    if(IN_STATE(PWM_TIMER_ACTIVE) && check_timeout(&app_dat.pwmTimer)) {
        DEBUG_PUTS("ENABLE PWM");
        DEBUG_NEWLINE();
        ENABLE_PWM(PWM_SETPOINT);
        UNSET_STATE(PWM_TIMER_ACTIVE);
    }

    // Handle port pins
    switchObjects();
}

/**
 * Function is called to handle the application logic
 * It restores data in the app_dat structure
 *
 * @return  */
void handleAppLogic(void) {
    uint8_t delayBaseIndex;
    uint8_t port;

    // handle MOVE and STEP commands
    for(port=0; port<4; port++)  {
        DEBUG_PUTS("Port: ");
	    DEBUG_PUTHEX(port);
        DEBUG_NEWLINE();
        DEBUG_PUTS("State: ");

        switch(app_dat.state[port]) {
            case NONE:
                DEBUG_PUTS("NONE");
                // Change command flags
                app_dat.portCmd &= ~(PORTMASK<<(port * 2));

                // change state
                if(app_dat.StepCmd & (PORTMASK_UP<<(port * 2)) ) {
                    app_dat.state[port] = STEP_UP_START;
                }
                else if(app_dat.StepCmd & (PORTMASK_DOWN<<(port * 2)) ) {
                    app_dat.state[port] = STEP_DOWN_START;
                }
                else if(app_dat.MoveCmd & (PORTMASK_UP<<(port * 2)) ) {
                    app_dat.state[port] = MOVE_UP_START;
                }
                else if(app_dat.MoveCmd & (PORTMASK_DOWN<<(port * 2)) ) {
                    app_dat.state[port] = MOVE_DOWN_START;
                }
                break;

            case MOVE_UP_START:
                DEBUG_PUTS("MOVE_UP_START");
                // Change command flags
                app_dat.MoveCmd &= ~(PORTMASK_UP<<(port * 2));                        // Reset long up flag
                app_dat.portCmd |= (PORTMASK_UP<<(port * 2));

                // change state
                if(app_dat.StepCmd & (PORTMASK<<(port * 2)) ) {
                    app_dat.portCmd &= ~(PORTMASK<<(port * 2));
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.portValue & (PORTMASK_UP<<(port * 2))) {
                    // start timer
                    if((mem_ReadByte(APP_ROLLO_LONGTIME_MODE) & (0x10<<port)) == 0) {
                        // Zeit + 20%
                        delayBaseIndex = mem_ReadByte(APP_ROLLO_LONGTIME_BASE + (port>>1));
                        if((port & 0x01) == 0x01) {     // Port 2 and 4
                            delayBaseIndex = (delayBaseIndex>>3) & 0x07;
                        }
                        else {                          // Port 1 and 3
                            delayBaseIndex &= 0x07;
                        }

                        timer_t timerBase   = pgm_read_dword(&time_bases[delayBaseIndex]);
                        uint8_t timerfactor = mem_ReadByte(APP_ROLLO_LONGTIME_FACTOR + port);
                        timer_t time = ((timer_t)(timerBase * timerfactor) * 157)>>7;       // 157 = 1.23 * 128            
                        alloc_timer(&app_dat.timer[port], time);
                        app_dat.runningTimer |= (1<<port);
                    }
                    app_dat.state[port] = MOVE_UP_ACTIVE;
                }
                break;

            case MOVE_UP_ACTIVE:
                DEBUG_PUTS("MOVE_UP_ACTIVE");
                // Change command flags
                app_dat.portCmd |= (PORTMASK_UP<<(port * 2));

                // change state
                if(app_dat.StepCmd & (PORTMASK<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.MoveCmd & (PORTMASK_DOWN<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.MoveCmd & (PORTMASK_UP<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = MOVE_UP_START;
                }
                else if((app_dat.runningTimer & (1<<port)) && check_timeout(&app_dat.timer[port])) {
                    app_dat.runningTimer &= ~(1<<port); 
                    if(app_dat.specFunc & (1<<port))  {
                        app_dat.state[port] = SAFTY_ACTIVE;
                    }
                    else {
                        app_dat.state[port] = NONE;
                    }
                }
                break;

            case STEP_UP_START:
                DEBUG_PUTS("STEP_UP_START");
                // Change command flags
                app_dat.StepCmd &= ~(PORTMASK_UP<<(port * 2));                        // Reset short up flag
                app_dat.portCmd |= (PORTMASK_UP<<(port * 2));

                // change state
                if(mem_ReadByte(APP_ROLLO_SHORTTIME_FACTOR + port) == 0)  {
                    // only stop command
                    app_dat.portCmd &= ~(PORTMASK<<(port * 2));
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.portValue & (PORTMASK_UP<<(port * 2))) {
                    // start timer
                    delayBaseIndex = mem_ReadByte(APP_ROLLO_SHORTTIME_BASE + (port>>1));
                    if((port & 0x01) == 0x01) {     // Port 2 and 4
                        delayBaseIndex = (delayBaseIndex>>3) & 0x07;
                    }
                    else {                          // Port 1 and 3
                        delayBaseIndex &= 0x07;
                    }

                    timer_t timerBase   = pgm_read_dword(&time_bases[delayBaseIndex]);
                    uint8_t timerfactor = mem_ReadByte(APP_ROLLO_SHORTTIME_FACTOR + port);
                    timer_t time = (timer_t)(timerBase * timerfactor);
                    alloc_timer(&app_dat.timer[port], time);
                    app_dat.runningTimer |= (1<<port);

                    app_dat.state[port] = STEP_UP_ACTIVE;
                }
                break;

            case STEP_UP_ACTIVE:
                DEBUG_PUTS("STEP_UP_ACTIVE");
                // Change command flags
                app_dat.portCmd |= (PORTMASK_UP<<(port * 2));

                // change state
                if(app_dat.MoveCmd & (PORTMASK<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.StepCmd & (PORTMASK_DOWN<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.StepCmd & (PORTMASK_UP<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = STEP_UP_START;
                }
                else if((app_dat.runningTimer & (1<<port)) && check_timeout(&app_dat.timer[port])) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                break;

            case MOVE_DOWN_START:
                DEBUG_PUTS("MOVE_DOWN_START");
                // Change command flags
                app_dat.MoveCmd &= ~(PORTMASK_DOWN<<(port * 2));                        // Reset long up flag
                app_dat.portCmd |= (PORTMASK_DOWN<<(port * 2));

                // change state
                if(app_dat.StepCmd & (PORTMASK<<(port * 2)) ) {
                    app_dat.portCmd &= ~(PORTMASK<<(port * 2));
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.portValue & (PORTMASK_DOWN<<(port * 2))) {
                    // start timer
                    if((mem_ReadByte(APP_ROLLO_LONGTIME_MODE) & (0x10<<port)) == 0) {
                        // Zeit + 20%
                        delayBaseIndex = mem_ReadByte(APP_ROLLO_LONGTIME_BASE + (port>>1));
                        if((port & 0x01) == 0x01) {     // Port 2 and 4
                            delayBaseIndex = (delayBaseIndex>>3) & 0x07;
                        }
                        else {                          // Port 1 and 3
                            delayBaseIndex &= 0x07;
                        }

                        timer_t timerBase   = pgm_read_dword(&time_bases[delayBaseIndex]);
                        uint8_t timerfactor = mem_ReadByte(APP_ROLLO_LONGTIME_FACTOR + port);
                        timer_t time = ((timer_t)(timerBase * timerfactor) * 153)>>7;       // 153 = 1.2 * 128            
                        alloc_timer(&app_dat.timer[port], time);
                        app_dat.runningTimer |= (1<<port);
                    }
                    app_dat.state[port] = MOVE_DOWN_ACTIVE;
                }
                break;

            case MOVE_DOWN_ACTIVE:
                DEBUG_PUTS("MOVE_DOWN_ACTIVE");
                // Change command flags
                app_dat.portCmd |= (PORTMASK_DOWN<<(port * 2));

                // change state
                if(app_dat.StepCmd & (PORTMASK<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.MoveCmd & (PORTMASK_UP<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.MoveCmd & (PORTMASK_DOWN<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = MOVE_DOWN_START;
                }
                else if((app_dat.runningTimer & (1<<port)) && check_timeout(&app_dat.timer[port])) {
                    app_dat.runningTimer &= ~(1<<port); 
                    if(app_dat.specFunc & (1<<port))  {
                        app_dat.state[port] = SAFTY_ACTIVE;
                    }
                    else {
                        app_dat.state[port] = NONE;
                    }
                }
                break;

            case STEP_DOWN_START:
                DEBUG_PUTS("STEP_DOWN_START");
                // Change command flags
                app_dat.StepCmd &= ~(PORTMASK_DOWN<<(port * 2));                        // Reset short up flag
                app_dat.portCmd |= (PORTMASK_DOWN<<(port * 2));

                // change state
                if(mem_ReadByte(APP_ROLLO_SHORTTIME_FACTOR + port) == 0)  {
                    // only stop command
                    app_dat.portCmd &= ~(PORTMASK<<(port * 2));
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.portValue & (PORTMASK_DOWN<<(port * 2))) {
                    // start timer
                    delayBaseIndex = mem_ReadByte(APP_ROLLO_SHORTTIME_BASE + (port>>1));
                    if((port & 0x01) == 0x01) {     // Port 2 and 4
                        delayBaseIndex = (delayBaseIndex>>3) & 0x07;
                    }
                    else {                          // Port 1 and 3
                        delayBaseIndex &= 0x07;
                    }

                    timer_t timerBase   = pgm_read_dword(&time_bases[delayBaseIndex]);
                    uint8_t timerfactor = mem_ReadByte(APP_ROLLO_SHORTTIME_FACTOR + port);
                    timer_t time = (timer_t)(timerBase * timerfactor);
                    alloc_timer(&app_dat.timer[port], time);
                    app_dat.runningTimer |= (1<<port);

                    app_dat.state[port] = STEP_DOWN_ACTIVE;
                }
                break;

            case STEP_DOWN_ACTIVE:
                DEBUG_PUTS("STEP_DOWN_ACTIVE");
                // Change command flags
                app_dat.portCmd |= (PORTMASK_DOWN<<(port * 2));

                // change state
                if(app_dat.MoveCmd & (PORTMASK<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.StepCmd & (PORTMASK_UP<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                else if(app_dat.StepCmd & (PORTMASK_DOWN<<(port * 2)) ) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = STEP_DOWN_START;
                }
                else if((app_dat.runningTimer & (1<<port)) && check_timeout(&app_dat.timer[port])) {
                    app_dat.runningTimer &= ~(1<<port); 
                    app_dat.state[port] = NONE;
                }
                break;

            case SAFTY_ACTIVE:
                if(app_dat.specFunc & (1<<port))  {
                    // delete all new commands
                    app_dat.portCmd &= ~(PORTMASK<<(port * 2));
                    app_dat.MoveCmd &= ~(PORTMASK<<(port * 2));                        // Reset long flag
                    app_dat.StepCmd &= ~(PORTMASK<<(port * 2));                        // Reset short flag
                }
                else {
                    app_dat.state[port] = NONE;
                }
                
                break;
            default:
                app_dat.state[port] = NONE;
                break;
        }
    }

    return;
}

/**
 * Function is called to handle the special application logic
 * It restores data in the app_dat structure
 *
 * @return  */
void handleAppSpecFunc(void) {
    uint8_t saftyCmd = 0;
    uint8_t port;

    if((app_dat.runningSpecTimer & 0x01) && check_timeout(&app_dat.specTimer[0])) {
        // timeout safty 1
        app_dat.specFunc |= 0x40;
    }
    if((app_dat.runningSpecTimer & 0x02) && check_timeout(&app_dat.specTimer[1])) {
        // timeout safty 2
        app_dat.specFunc |= 0x80;
    }

    for(port=0; port<4; port++)  {
        switch((mem_ReadByte(APP_ROLLO_SAFTY_PORT)>>port) & 0x11) {
            case 0x01:      // nur Sicherheit 1
                    if(app_dat.specFunc & 0x40) {
                        saftyCmd |= 1<<port;
                    }
                break;
            case 0x10:      // nur Sicherheit 2
                    if(app_dat.specFunc & 0x80) {
                        saftyCmd |= 1<<port;
                    }
                break;
            case 0x11:      // Sicherheit 1 ODER 2
                    if(app_dat.specFunc & 0xC0) {
                        saftyCmd |= 1<<port;
                    }
                break;
            default:
                break;
        }

        if(saftyCmd & (1<<port)) {
            if((app_dat.specFunc & (1<<port)) == 0) {
                // Anfang Sicherheit
                uint8_t cmd = mem_ReadByte(APP_ROLLO_SAFTY_BEGINN) & (PORTMASK<<(port * 2));
                if(cmd) {
                    app_dat.MoveCmd &= ~(PORTMASK<<(port * 2)); 
                    app_dat.MoveCmd |= cmd; 
                }
            }
        }
        else {
            if(app_dat.specFunc & (1<<port)) {
                // Ende Sicherheit
                uint8_t cmd = mem_ReadByte(APP_ROLLO_SAFTY_END) & (PORTMASK<<(port * 2));
                if(cmd) {
                    app_dat.MoveCmd &= ~(PORTMASK<<(port * 2)); 
                    app_dat.MoveCmd |= cmd; 
                }
            }
        }
    }   /* for(port=0; port<4; port++) */
    
    /* save states for next call */
    app_dat.specFunc &= 0xF0;
    app_dat.specFunc |= saftyCmd;

    return;
}


/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void) {

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
    SET_IO_RES1(IO_OUTPUT);
    SET_IO_RES2(IO_OUTPUT);
    SET_IO_RES3(IO_OUTPUT);
    SET_IO_RES4(IO_OUTPUT);
#endif

    /* CTRL-Port */
    SET_IO_CTRL(IO_OUTPUT);

#ifdef IO_TEST
	/* should we do an IO test? */
	io_test();
#endif

    // States after Restart
    switch(mem_ReadByte(APP_ROLLO_AFTER_PL) & 0xC0) {
        case 0x00:  // Stopp
        default:
            app_dat.state[0] = NONE;
            app_dat.state[1] = NONE;
            app_dat.state[2] = NONE;
            app_dat.state[3] = NONE;
            break;
        case 0x40:  // UP
            app_dat.state[0] = MOVE_UP_START;
            app_dat.state[1] = MOVE_UP_START;
            app_dat.state[2] = MOVE_UP_START;
            app_dat.state[3] = MOVE_UP_START;
            break;
        case 0x80:  // DOWN
            app_dat.state[0] = MOVE_DOWN_START;
            app_dat.state[1] = MOVE_DOWN_START;
            app_dat.state[2] = MOVE_DOWN_START;
            app_dat.state[3] = MOVE_DOWN_START;
            break;
    }        

    // Init app_dat structure
    app_dat.MoveCmd = 0;
    app_dat.StepCmd = 0;
    app_dat.portCmd = 0;
    app_dat.runningTimer = 0;
    app_dat.portState[0] = PORT_INACTIVE;
    app_dat.portState[1] = PORT_INACTIVE;
    app_dat.portState[2] = PORT_INACTIVE;
    app_dat.portState[3] = PORT_INACTIVE;
    app_dat.portValue = 0;
    app_dat.portValue_old = 0;
    app_dat.runningportTimer = 0;
    app_dat.specFunc = 0;

	DEBUG_PUTHEX(app_dat.state[0]);
    DEBUG_SPACE();

    DEBUG_PUTS("Done.");
    DEBUG_NEWLINE();

    /* Reset State */
    RESET_STATE();

    return 1;
} /* restartApplication() */

/**
 * Switch the objects to state in portValue and save value to eeprom if necessary.
 *
 * gloabal variables    app_dat.portValue
 *                      app_dat.oldValue
 */
void switchObjects(void) {
    uint8_t port;
    uint8_t delayBaseIndex;

    DEBUG_PUTS("Sw");
    DEBUG_SPACE();

    // handle switching between up and down
    for(port=0; port<4; port++)  {
        switch(app_dat.portState[port]) {
            case PORT_INACTIVE:
                app_dat.portValue &= ~(PORTMASK<<(port * 2));

                if(app_dat.portCmd & (PORTMASK_UP<<(port * 2))) {
                    app_dat.portState[port] = PORT_UP;
                }
                else if(app_dat.portCmd & (PORTMASK_DOWN<<(port * 2))) {
                    app_dat.portState[port] = PORT_DOWN;
                }
                break;
            case PORT_UP:
                app_dat.portValue |= (PORTMASK_UP<<(port * 2));

                if((app_dat.portCmd & (PORTMASK_UP<<(port * 2))) == 0) {
                    delayBaseIndex = mem_ReadByte(APP_ROLLO_DIRECTION_DELAY);
                    delayBaseIndex = (delayBaseIndex>>(port * 2)) & 0x03;
                    timer_t time = pgm_read_dword(&delay_bases[delayBaseIndex]);
                    alloc_timer(&app_dat.portTimer[port], time);
                    app_dat.runningportTimer |= (1<<port);

                    app_dat.portState[port] = UP_WAIT;
                }
                break;
            case UP_WAIT:
                app_dat.portValue &= ~(PORTMASK<<(port * 2));

                if(app_dat.portCmd & (PORTMASK_UP<<(port * 2))) {
                    app_dat.portState[port] = PORT_UP;
                    app_dat.runningportTimer &= ~(1<<port); 
                }
                else if((app_dat.runningportTimer & (1<<port)) && check_timeout(&app_dat.portTimer[port])) {
                    app_dat.portState[port] = PORT_INACTIVE;
                    app_dat.runningportTimer &= ~(1<<port); 
                }
                break;
            case PORT_DOWN:
                app_dat.portValue |= (PORTMASK_DOWN<<(port * 2));

                if((app_dat.portCmd & (PORTMASK_DOWN<<(port * 2))) == 0) {
                    delayBaseIndex = mem_ReadByte(APP_ROLLO_DIRECTION_DELAY);
                    delayBaseIndex = (delayBaseIndex>>(port * 2)) & 0x03;
                    timer_t time = pgm_read_dword(&delay_bases[delayBaseIndex]);
                    alloc_timer(&app_dat.portTimer[port], time);
                    app_dat.runningportTimer |= (1<<port);

                    app_dat.portState[port] = DOWN_WAIT;
                }
                break;
            case DOWN_WAIT:
                app_dat.portValue &= ~(PORTMASK<<(port * 2));

                if(app_dat.portCmd & (PORTMASK_DOWN<<(port * 2))) {
                    app_dat.portState[port] = PORT_DOWN;
                    app_dat.runningportTimer &= ~(1<<port); 
                }
                else if((app_dat.runningportTimer & (1<<port)) && check_timeout(&app_dat.portTimer[port])) {
                    app_dat.portState[port] = PORT_INACTIVE;
                    app_dat.runningportTimer &= ~(1<<port); 
                }
                break;
            default:
                app_dat.portState[port] = PORT_INACTIVE;
                break;
        }

    }

    // Handling for 'Betriebsart' (4 Kanal / 2x2 Kanal)
    // check bit 7 for opmode
    if( mem_ReadByte(APP_ROLLO_OPMODE) & 0x80 ) {
        // 2x2 Kanal (Ausgang 1 und 3 / Ausgang 2 und 4)
        uint8_t tmpValue = app_dat.portValue;
        app_dat.portValue = ((tmpValue & 0x0F)<<4) | (tmpValue & 0x0F);
    }   

    for(port=0; port<8; port+=2)  {
        // UP/DOWN Verriegelung pruefen
        if((app_dat.portValue & (0x03<<port)) == (0x03<<port)) {
            app_dat.portValue = 0;  // Kann nie auftreten
        }
    }

    // switch portpins
    switchPorts(app_dat.portValue, app_dat.portValue_old);
    app_dat.portValue_old = app_dat.portValue;

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
