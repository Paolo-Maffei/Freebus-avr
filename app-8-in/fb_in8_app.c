/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
 *  /_/   /_/ |_/_____/_____/_____/\____//____/
 *
*  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
*  Copyright (c) 2009 Christian Bode <Bode_Christian@t-online.de>
*  Copyright (c) 2010 Dirk Armbrust (tuxbow) <dirk.armbrust@freenet.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb_in8_app.c
* @author Matthias Fechner, Christian Bode, Dirk Armbrust
* @date   Sat Jan 05 17:44:47 2008
*
* @brief  The application for 8 binary inputs
* Manufactorer code is 0x04 = Jung\n
* Device type (2118) 0x7054 Ordernumber: 2118 REG\n
 *
 * To enable IO test compile with -DIO_TEST
 *
 * This version is designed to be used with the new API.
 */
#ifndef _FB_APP_C
#define _FB_APP_C

/*************************************************************************
 * INCLUDES
 *************************************************************************/
//#include "1wire.h"
#include "fb_in8_app.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/
/* Objects for the app-8-in */
enum EIGHT_IN_Objects_e {
    OBJ_OUT0 = 0,
    OBJ_OUT1,
    OBJ_OUT2,
    OBJ_OUT3,
    OBJ_OUT4,
    OBJ_OUT5,
    OBJ_OUT6,
    OBJ_OUT7,
    OBJ_OUT8, 
    OBJ_OUT9,
    OBJ_OUT10,
    OBJ_OUT11,
    OBJ_OUT12,
    OBJ_OUT13,
    OBJ_OUT14,
    OBJ_OUT15,
    OBJ_OUT16, // sperren Eingang 1
    OBJ_OUT17, // sperren Eingang 2
    OBJ_OUT18, // sperren Eingang 3
    OBJ_OUT19, // sperren Eingang 4
    OBJ_OUT20, // sperren Eingang 5
    OBJ_OUT21, // sperren Eingang 6
    OBJ_OUT22, // sperren Eingang 7
    OBJ_OUT23  // sperren Eingang 8
};

/* Objekte:
Nr. Objectname        Function          Typ             Flags
Funktion: Schalten
0   Eingang 1         Schalten 1.1      EIS 1   1 Bit   K  S  � (L)
1   Eingang 2         Schalten 2.1      EIS 1   1 Bit   K  S  � (L)
2   Eingang 3         Schalten 3.1      EIS 1   1 Bit   K  S  � (L)
3   Eingang 4         Schalten 4.1      EIS 1   1 Bit   K  S  � (L)
4   Eingang 5         Schalten 5.1      EIS 1   1 Bit   K  S  � (L)
5   Eingang 6         Schalten 6.1      EIS 1   1 Bit   K  S  � (L)
6   Eingang 7         Schalten 7.1      EIS 1   1 Bit   K  S  � (L)
7   Eingang 8         Schalten 8.1      EIS 1   1 Bit   K  S  � (L)
8   Eingang 1         Schalten 1.2      EIS 1   1 Bit   K  S  � (L)
9   Eingang 2         Schalten 2.2      EIS 1   1 Bit   K  S  � (L)
10  Eingang 3         Schalten 3.2      EIS 1   1 Bit   K  S  � (L)
11  Eingang 4         Schalten 4.2      EIS 1   1 Bit   K  S  � (L)
12  Eingang 5         Schalten 5.2      EIS 1   1 Bit   K  S  � (L)
13  Eingang 6         Schalten 6.2      EIS 1   1 Bit   K  S  � (L)
14  Eingang 7         Schalten 7.2      EIS 1   1 Bit   K  S  � (L)
15  Eingang 8         Schalten 8.2      EIS 1   1 Bit   K  S  � (L)

Funktion: Dimmen

Funktion: Jalousie
0   Eingang 1         Kurzzeitbetr. 1   EIS 1   1 Bit   K  S  � (L)
1   Eingang 2         Kurzzeitbetr. 2   EIS 1   1 Bit   K  S  � (L)
2   Eingang 3         Kurzzeitbetr. 3   EIS 1   1 Bit   K  S  � (L)
3   Eingang 4         Kurzzeitbetr. 4   EIS 1   1 Bit   K  S  � (L)
4   Eingang 5         Kurzzeitbetr. 5   EIS 1   1 Bit   K  S  � (L)
5   Eingang 6         Kurzzeitbetr. 6   EIS 1   1 Bit   K  S  � (L)
6   Eingang 7         Kurzzeitbetr. 7   EIS 1   1 Bit   K  S  � (L)
7   Eingang 8         Kurzzeitbetr. 8   EIS 1   1 Bit   K  S  � (L)
8   Eingang 1         Langzeitbetr. 1   EIS 1   1 Bit   K  S  � (L)
9   Eingang 2         Langzeitbetr. 2   EIS 7   1 Bit   K  S  � (L)
10  Eingang 3         Langzeitbetr. 3   EIS 7   1 Bit   K  S  � (L)
11  Eingang 4         Langzeitbetr. 4   EIS 7   1 Bit   K  S  � (L)
12  Eingang 5         Langzeitbetr. 5   EIS 7   1 Bit   K  S  � (L)
13  Eingang 6         Langzeitbetr. 6   EIS 7   1 Bit   K  S  � (L)
14  Eingang 7         Langzeitbetr. 7   EIS 7   1 Bit   K  S  � (L)
15  Eingang 8         Langzeitbetr. 8   EIS 7   1 Bit   K  S  � (L)

Funktion: Wertgeber (Dimmwertgeber)

Funktion: Wertgeber (Lichtszenennebenstelle)

Funktion: Wertgeber (Temperaturwertgeber)

Funktion: Wertgeber (Helligkeitswertgeber)

Funktion: Impulszaehler (nur Eingang 1 und 2)

Funktion: Schaltz�hler (nur Eingang 1 und 2)

16  Sperren 1         Sicherheit        EIS 1   1 Bit   K  S  (L)
17  Sperren 2         Sicherheit        EIS 1   1 Bit   K  S  (L)
18  Sperren 3         Sicherheit        EIS 1   1 Bit   K  S  (L)
19  Sperren 4         Sicherheit        EIS 1   1 Bit   K  S  (L)
20  Sperren 5         Sicherheit        EIS 1   1 Bit   K  S  (L)
21  Sperren 6         Sicherheit        EIS 1   1 Bit   K  S  (L)
22  Sperren 7         Sicherheit        EIS 1   1 Bit   K  S  (L)
23  Sperren 8         Sicherheit        EIS 1   1 Bit   K  S  (L)

EIS 1   0  OFF
        1  ON
EIS 7   0  UP   / START
        1  DOWN / STOP

Flag  Name            Bedeutung
K     Kummunikation   Objekt ist kommunikationsf�hig
L     Lesen           Objektstatus kann abgefragt werden (ETS / Display usw.)
S     Schreiben       Objekt kann empfangen
�     �bertragen      Objekt kann senden

*/

typedef enum
{
    eFunc_none,                 ///< @todo add documentation
    eFunc_schalten,             ///< @todo add documentation
    eFunc_dimmen,               ///< @todo add documentation
    eFunc_jalousie,             ///< @todo add documentation
    eFunc_dimmwertgeber,        ///< @todo add documentation
    eFunc_Lichtszene,           ///< @todo add documentation
    eFunc_Wertgeber,            ///< @todo add documentation
    eFunc_Temperaturwertgeber,  ///< @todo add documentation
    eFunc_Helligkeitswertgeber, ///< @todo add documentation
    eFunc_Impulszaehler,        ///< @todo add documentation
    eFunc_Schaltzaehler,        ///< @todo add documentation
} EFUNC_PORT;

#define TIME_MSG_RATE_LIM       (timer_t)128*M2TICS(130)        // 17 sec fuer Telegrammratenberenzung


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
    uint8_t portVal;               /// stores port states
    uint8_t portVal_old;           /// stores port states

    // Variablen fuer Funktion schalten
    uint8_t objVal1;               /// stores the output objects x.1
    uint8_t objVal2;               /// stores the output objects x.2
    timer_t cycltimer1[8];         /// stores timer value for cyclic sending
    uint8_t runningCyclTimer1;     /// bitfield for timer active flags
    timer_t cycltimer2[8];         /// stores timer value for cyclic sending
    uint8_t runningCyclTimer2;     /// bitfield for timer active flags

    // Variablen fuer Funktion Jalousie
    uint8_t stateJalousie[8];      /// store states for internal jalousie states

    uint8_t safty;                 /// bitfield for the safty commands
    uint8_t safty_old;             /// bitfield for the last safty commands
    uint8_t msgCntr;               /// counter for the message limiter
    timer_t deftimer;              /// stores actual timer values
    uint8_t runningdefTimer;       /// bitfield for timer active flags
} app_dat;


/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
uint8_t ReadPorts(void);
void handlePowerON(void);
void handleAppSchalten(uint8_t port);
void handleAppDimmen(uint8_t port);
void handleAppJalousie(uint8_t port);


/**
 * Function os called periodically of the application is enabled in the system_state
 *
 */
void app_loop() {
    uint8_t commObjectNumber;
    uint8_t msgVal;
    uint8_t portFunc;
    uint8_t port;
    uint8_t i;
    uint8_t portmask = 0;

    // Iterate over all objects and check if the status has changed
    for(commObjectNumber=0; commObjectNumber<=OBJ_OUT23; commObjectNumber++) {
        portmask <<= 1;
        if (portmask == 0)
            portmask = 1;
        /* check if an object has changed its status */
        if (TestAndCopyObject(commObjectNumber, &msgVal, 0)) {

            DEBUG_NEWLINE();
            DEBUG_PUTS("OBJ_");
            DEBUG_PUTHEX(commObjectNumber);
            DEBUG_SPACE();
            DEBUG_PUTHEX(msgVal);
            DEBUG_SPACE();

            if (commObjectNumber <= OBJ_OUT7) {
                if (msgVal)
                    app_dat.objVal1 |= portmask;	/* Set value */
                else
                    app_dat.objVal1 &= ~(portmask);	/* clear value */
            } else if (commObjectNumber <= OBJ_OUT15) {
                if (msgVal)
                    app_dat.objVal2 |= portmask;	/* Set value */
                else
                    app_dat.objVal2 &= ~(portmask);	/* clear value */
            } else {
                // Tranfer MsgData to bit structure (xxValue)
                if(msgVal)
                    app_dat.safty |= (portmask);	/* Set value */
                else
                    app_dat.safty &= ~(portmask);	/* clear value */
            }
        }
    }   // for()

    // check for power on delay activ
    if(app_dat.runningdefTimer & 0x01) {
        if(check_timeout(&app_dat.deftimer)) {
            app_dat.runningdefTimer &= ~(0x01);
            handlePowerON();

            // check Telegrammratenbegrenzung
            app_dat.msgCntr = mem_ReadByte(APP_MSG_RATE_LIM);
            if(app_dat.msgCntr >= 30) {
                alloc_timer(&app_dat.deftimer, (timer_t)TIME_MSG_RATE_LIM);
                // keine Teleggramme in den ersten 17 sec
                app_dat.msgCntr = 0;
                app_dat.runningdefTimer |= 0x02;
            }
        }
    }
    else {
        // Telegrammratenbegrenzung neu zuruecksetzen
        if(app_dat.runningdefTimer & 0x02) {
            // Begrenzung aktiv
            if(check_timeout(&app_dat.deftimer)) {
                // after 17 sec reset the counter
                alloc_timer(&app_dat.deftimer, (timer_t)TIME_MSG_RATE_LIM);
                app_dat.msgCntr = mem_ReadByte(APP_MSG_RATE_LIM);
            }
        }
        else {
            app_dat.msgCntr = 255;
        }

        // debounce port states
        if(app_dat.portVal_old ^ ReadPorts()) {
            // new input level
            uint8_t debounceFactor = mem_ReadByte(APP_DEBOUNCE_FACTOR);

            for(i = 0; i < debounceFactor; i++) {
                /* delay time 0.5 ms */
                _delay_us(500);
            }
            app_dat.portVal = ReadPorts();
        }

        for(port = 0; port<8; port++) {
            // read port function
            portFunc = mem_ReadByte(APP_PORTFUNCTION + (port>>1));
            if(port & 0x01) {
                // Eingaenge 2, 4, 6, 8
                portFunc = portFunc>>4;
            }
            portFunc &= 0x0F;

            // call special app function
            switch(portFunc) {
                case eFunc_none:
                    break;

                case eFunc_schalten:
                    handleAppSchalten(port);
                    break;

                case eFunc_dimmen:
                    handleAppDimmen(port);
                    break;

                case eFunc_jalousie:
                    handleAppJalousie(port);
                    break;

                case eFunc_dimmwertgeber:
                    break;

                case eFunc_Lichtszene:
                    break;

                case eFunc_Wertgeber:
                    break;

                case eFunc_Temperaturwertgeber:
                    break;

                case eFunc_Helligkeitswertgeber:
                    break;

                case eFunc_Impulszaehler:
                    break;

                case eFunc_Schaltzaehler:
                    break;

                default:
                    break;
            }
        }   // for()

        app_dat.portVal_old = app_dat.portVal;
        app_dat.safty_old   = app_dat.safty;
    }

}


/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void) {

    /* IO configuration */
    IO_SET_DIR(1, IO_INPUT);
    IO_SET_DIR(2, IO_INPUT);
    IO_SET_DIR(3, IO_INPUT);
    IO_SET_DIR(4, IO_INPUT);
    IO_SET_DIR(5, IO_INPUT);
    IO_SET_DIR(6, IO_INPUT);
    IO_SET_DIR(7, IO_INPUT);
    IO_SET_DIR(8, IO_INPUT);
#if REVISION==1
    SET_IO_RES1(IO_OUTPUT);
    SET_IO_RES2(IO_OUTPUT);
    SET_IO_RES3(IO_OUTPUT);
    SET_IO_RES4(IO_OUTPUT);
#endif

    /* CTRL-Port */
    SET_IO_CTRL(IO_OUTPUT);

    // States after Restart
    memset(&app_dat, 0, sizeof(app_dat));

    // init power ON delay
    uint8_t delayBaseIndex = (mem_ReadByte(APP_POWERONDELAY_BASE)>>4) & 0x0F;
    timer_t delayBase = pgm_read_dword(&delay_bases[delayBaseIndex]);
    uint8_t delayfactor = mem_ReadByte(APP_POWERONDELAY_FACTOR) & 0x7F;     // Bereich 3...127

    timer_t time = delayBase * delayfactor;
    alloc_timer(&app_dat.deftimer, time);
    app_dat.runningdefTimer |= 0x01;

    return 1;
} /* restartApplication */





/**
* Read all inputpins and store values into a byte
*
* @return port  bitfield with port level
*
*/
uint8_t ReadPorts(void) {
    uint8_t portVal = 0;

    portVal |= (uint8_t)IO_GET(1);
    portVal |= (((uint8_t)IO_GET(2))<<1);
    portVal |= (((uint8_t)IO_GET(3))<<2);
    portVal |= (((uint8_t)IO_GET(4))<<3);
    portVal |= (((uint8_t)IO_GET(5))<<4);
    portVal |= (((uint8_t)IO_GET(6))<<5);
    portVal |= (((uint8_t)IO_GET(7))<<6);
    portVal |= (((uint8_t)IO_GET(8))<<7);

    return portVal;
}

/**
* handle the application specific power on function
*
* @return
*
*/
void handlePowerON(void) {
    uint8_t portFunc;
    uint8_t port;
    uint8_t func;

    for(port = 0; port<8; port++) {
        // read port function
        portFunc = mem_ReadByte(APP_PORTFUNCTION + (port>>1));
        if(port & 0x01) {
            // Eingaenge 2, 4, 6, 8
            portFunc = portFunc>>4;
        }
        portFunc &= 0x0F;

        // call special app function
        switch(portFunc) {
            case eFunc_none:
                break;

            case eFunc_schalten:
                func = mem_ReadByte(APP_PORTFUNC_CONFIG + (port * 4)) & 0xC0;
                if(func == 0x40) {
                    // switch object ON
                    app_dat.objVal1 |= (1<<port);
                    app_dat.objVal2 |= (1<<port);
                    SetAndTransmitBit(port,  0x01 );
                    SetAndTransmitBit(port + 8, 0x01 );
                }
                else if(func == 0x80) {
                    // switch object OFF
                    SetAndTransmitBit(port,  0x00 );
                    SetAndTransmitBit(port + 8, 0x00 );
                }
                else if(func == 0xC0) {
                    // send port state
                    if( (ReadPorts()>>port) & 0x01) {
                        // switch object ON
                        app_dat.objVal1 |= (1<<port);
                        app_dat.objVal2 |= (1<<port);
                        SetAndTransmitBit(port,  0x01 );
                        SetAndTransmitBit(port + 8, 0x01 );
                    }
                    else {
                        // switch object OFF
                        SetAndTransmitBit(port,  0x00 );
                        SetAndTransmitBit(port + 8, 0x00 );
                    }

                }
                break;

            case eFunc_dimmen:
                break;

            case eFunc_jalousie:
                // Bedienkonzept unterscheiden
                func = mem_ReadByte(APP_PORTFUNC_JALOCONFIG + (port * 4)) & 0xC0;
                if(func == 0x40) {
                    // send MOVE DOWN
                    SetAndTransmitBit(port + 8, 0x01 );
                }
                else if(func == 0x80) {
                    // send MOVE UP
                    SetAndTransmitBit(port + 8, 0x00 );
                }
                break;

            case eFunc_dimmwertgeber:
                break;

            case eFunc_Lichtszene:
                break;

            case eFunc_Wertgeber:
                break;

            case eFunc_Temperaturwertgeber:
                break;

            case eFunc_Helligkeitswertgeber:
                break;

            case eFunc_Impulszaehler:
                break;

            case eFunc_Schaltzaehler:
                break;

            default:
                break;
        }
    }   // for()
    return;
}


/**
* handle the aplication function "schalten"
*
* @return
*
*/
void handleAppSchalten(uint8_t port) {
    uint8_t pinVal;
    uint8_t pinVal_old;
    uint8_t edgeFunc;
    uint8_t sendObj = 0;
    uint8_t saftyFunc;
    uint8_t safty_active = 0;

    pinVal = (app_dat.portVal>>port) & 0x01;
    pinVal_old = (app_dat.portVal_old>>port) & 0x01;

    // **************
    // Sperrfunktion
    // **************
    saftyFunc =  mem_ReadByte(APP_PORTFUNC_CONFIG + (port * 4)) & 0x03;
    if( saftyFunc) {
        // freigegeben
        uint8_t safty = (app_dat.safty>>port) & 0x01;
        uint8_t safty_old = (app_dat.safty_old>>port) & 0x01;

        // invertieren
        if( saftyFunc == 0x02) {
            safty = ~safty & 0x01;
            safty_old = ~safty_old & 0x01;
        }

        if(safty == 0x01) {
            if(safty_old == 0x00) {
                // Beginn einer Sperrung
                saftyFunc = mem_ReadByte(APP_PORTFUNC_CONFIG + (port * 4)) & 0x30;
                if(saftyFunc == 0x10) {
                    // EIN senden
                    app_dat.objVal1 |= (1<<port);
                    app_dat.objVal2 |= (1<<port);
                    sendObj = 0x03;
                }
                else if(saftyFunc == 0x20) {
                    // AUS senden
                    app_dat.objVal1 &= ~(1<<port);
                    app_dat.objVal2 &= ~(1<<port);
                    sendObj = 0x03;
                }
                else if(saftyFunc == 0x30) {
                    // UM senden
                    if(app_dat.objVal1 & (1<<port)) {
                        app_dat.objVal1 &= ~(1<<port);
                    }
                    else {
                        app_dat.objVal1 |= (1<<port);
                    }
                    if(app_dat.objVal2 & (1<<port)) {
                        app_dat.objVal2 &= ~(1<<port);
                    }
                    else {
                        app_dat.objVal2 |= (1<<port);
                    }
                    sendObj = 0x03;
                }
            }
            safty_active = 1;
        }
        else {
            if(safty_old == 0x01) {
                // Ende einer Sperrung
                saftyFunc = mem_ReadByte(APP_PORTFUNC_CONFIG2 + (port * 4)) & 0x03;
                if(saftyFunc == 0x01) {
                    // EIN senden
                    app_dat.objVal1 |= (1<<port);
                    app_dat.objVal2 |= (1<<port);
                    sendObj = 0x03;
                }
                else if(saftyFunc == 0x02) {
                    // AUS senden
                    app_dat.objVal1 &= ~(1<<port);
                    app_dat.objVal2 &= ~(1<<port);
                    sendObj = 0x03;
                }
                else if(saftyFunc == 0x03) {
                    // aktueller Wert
                    if(pinVal) {
                        app_dat.objVal1 |= (1<<port);
                        app_dat.objVal2 |= (1<<port);
                    }
                    else {
                        app_dat.objVal1 &= ~(1<<port);
                        app_dat.objVal2 &= ~(1<<port);
                    }
                    sendObj = 0x03;
                }
            }
        }
    }

    // ********************
    // Eingaenge auswerten
    // ********************
    if((pinVal == pinVal_old) || safty_active) {
        // do nothing
    }
    else {
        edgeFunc = mem_ReadByte(APP_PORTFUNC_EDGEFUNC + (port * 4));

        if( (pinVal == 0) && (pinVal_old  == 1) ) {
            // falling edge
            if((edgeFunc & 0x03) == 0x01) {
                // switch object x.1 ON
                app_dat.objVal1 |= (1<<port);
                sendObj = 0x01;
            }
            else if((edgeFunc & 0x03) == 0x02) {
                // switch object x.1 OFF
                app_dat.objVal1 &= ~(1<<port);
                sendObj = 0x01;
            }
            else if((edgeFunc & 0x03) == 0x03) {
                // switch object x.1
                if(app_dat.objVal1 & (1<<port)) {
                    // switch object x.1 OFF
                    app_dat.objVal1 &= ~(1<<port);
                }
                else {
                    // switch object x.1 ON
                    app_dat.objVal1 |= (1<<port);
                }
                sendObj = 0x01;
            }

            if((edgeFunc & 0x30) == 0x10) {
                // switch object x.2 ON
                app_dat.objVal2 |= (1<<port);
                sendObj |= 0x02;
            }
            else if((edgeFunc & 0x30) == 0x20) {
                // switch object x.2 OFF
                app_dat.objVal2 &= ~(1<<port);
                sendObj |= 0x02;
            }
            else if((edgeFunc & 0x30) == 0x30) {
                // switch object x.2
                if(app_dat.objVal2 & (1<<port)) {
                    // switch object x.2 OFF
                    app_dat.objVal2 &= ~(1<<port);
                }
                else {
                    // switch object x.2 ON
                    app_dat.objVal2 |= (1<<port);
                }
                sendObj |= 0x02;
            }
        }
        else {
            // rising edge
            if((edgeFunc & 0x0C) == 0x04) {
                // switch object x.1 ON
                app_dat.objVal1 |= (1<<port);
                sendObj = 0x01;
            }
            else if((edgeFunc & 0x0C) == 0x08) {
                // switch object x.1 OFF
                app_dat.objVal1 &= ~(1<<port);
                sendObj = 0x01;
            }
            else if((edgeFunc & 0x0C) == 0x0C) {
                // switch object x.1
                if(app_dat.objVal1 & (1<<port)) {
                    // switch object x.1 OFF
                    app_dat.objVal1 &= ~(1<<port);
                }
                else {
                    // switch object x.1 ON
                    app_dat.objVal1 |= (1<<port);
                }
                sendObj = 0x01;
            }

            if((edgeFunc & 0xC0) == 0x40) {
                // switch object x.2 ON
                app_dat.objVal2 |= (1<<port);
                sendObj |= 0x02;
            }
            else if((edgeFunc & 0xC0) == 0x80) {
                // switch object x.2 OFF
                app_dat.objVal2 &= ~(1<<port);
                sendObj |= 0x02;
            }
            else if((edgeFunc & 0xC0) == 0xC0) {
                // switch object x.2
                if(app_dat.objVal2 & (1<<port)) {
                    // switch object x.2 OFF
                    app_dat.objVal2 &= ~(1<<port);
                }
                else {
                    // switch object x.2 ON
                    app_dat.objVal2 |= (1<<port);
                }
                sendObj |= 0x02;
            }
        }
    }

    // ******************
    // zyklisches senden
    // ******************
    uint8_t sendConfig = mem_ReadByte(APP_PORTFUNC_CONFIG + (port * 4)) & 0x0C;
    uint8_t cyclicsendactive = 0;

    if(sendConfig == 0x04) {
        // zyklisch senden bei EIN
        if((app_dat.objVal1>>port) & 0x1) {
            cyclicsendactive = 1;
        }
    }
    else if(sendConfig == 0x08) {
        // zyklisch senden bei AUS
        if(((app_dat.objVal1>>port) & 0x1) == 0) {
            cyclicsendactive = 1;
        }
    }
    else if(sendConfig == 0x0C) {
        cyclicsendactive = 1;
    }

    if(cyclicsendactive && (safty_active == 0)) {
        // zyklisch senden x.1
        if(app_dat.runningCyclTimer1 & (1<<port)) {
            // Check for timeout
            if(check_timeout(&app_dat.cycltimer1[port])) {
                app_dat.runningCyclTimer1 &= ~(1<<port);
            }
        }
        else {
            // Timer starten
            uint8_t delayBaseIndex=mem_ReadByte(APP_PORTFUNC_CYCLIC_BASE1 + ((port+1)>>1));

            if((port & 0x01) == 0x01) {
                delayBaseIndex &= 0x0F;
            } else {
                delayBaseIndex = (delayBaseIndex>>4) & 0x0F;
            }
            timer_t delayBase = pgm_read_dword(&delay_bases[delayBaseIndex]);
            uint8_t factor    = mem_ReadByte(APP_PORTFUNC_CYCLIC_FACTOR + (port * 4)) & 0x7F;
            timer_t time      = delayBase * factor;
            alloc_timer(&app_dat.cycltimer1[port], time);
            app_dat.runningCyclTimer1 |= (1<<port);
            sendObj |= 0x01;
        }

        // zyklisch senden x.2
        if(app_dat.runningCyclTimer2 & (1<<port)) {
            // Check for timeout
            if(check_timeout(&app_dat.cycltimer2[port])) {
                app_dat.runningCyclTimer2 &= ~(1<<port);
            }
        }
        else {
            // Timer starten
            uint8_t delayBaseIndex=mem_ReadByte(APP_PORTFUNC_CYCLIC_BASE2 + ((port+1)>>1));

            if((port & 0x01) == 0x01) {
                delayBaseIndex &= 0x0F;
            } else {
                delayBaseIndex = (delayBaseIndex>>4) & 0x0F;
            }
            timer_t delayBase = pgm_read_dword(&delay_bases[delayBaseIndex]);
            uint8_t factor    = mem_ReadByte(APP_PORTFUNC_CYCLIC_FACTOR + (port * 4)) & 0x7F;
            timer_t time      = delayBase * factor;
            alloc_timer(&app_dat.cycltimer2[port], time);
            app_dat.runningCyclTimer2 |= (1<<port);
            sendObj |= 0x02;
        }
    }
    else {
        // Timer deaktivieren
        app_dat.runningCyclTimer1 &= ~(1<<port);
        app_dat.runningCyclTimer2 &= ~(1<<port);
    }


    // ***********************************
    // check if we have to send a message
    // ***********************************
    if( sendObj & 0x01) {
        // check Telegrammratenbegrenzer
        if(app_dat.msgCntr) {
            SetAndTransmitBit(port, (app_dat.objVal1>>port) & 0x1 );
            app_dat.msgCntr--;
        }
    }
    if( sendObj & 0x02) {
        if(app_dat.msgCntr) {
            SetAndTransmitBit(port + 8, (app_dat.objVal2>>port) & 0x1 );
            app_dat.msgCntr--;
        }
    }

    return;
}

/**
* handle the aplication function "dimmen"
*
* @return
*
*/
void handleAppDimmen(uint8_t port) {
    return;
}

/**
* handle the aplication function "Jalousie"
*
* @return
*
*/
void handleAppJalousie(uint8_t port) {
    uint8_t pinVal;
    uint8_t pinVal_old;
    uint8_t opMode;
    uint8_t saftyFunc;
    uint8_t sendObj = 0;    // 1 UP, 2 DOWN, 4 Step

    pinVal = (app_dat.portVal>>port) & 0x01;
    pinVal_old = (app_dat.portVal_old>>port) & 0x01;

    switch(app_dat.stateJalousie[port]) {
        case 0:
            // check rising edge
            if( (pinVal == 1) && (pinVal_old  == 0) ) {
                // Timer T1 starten
                uint8_t delayBaseIndex=mem_ReadByte(APP_PORTFUNC_T1_BASIS + ((port+1)>>1));

                if((port & 0x01) == 0x01) {
                    delayBaseIndex &= 0x0F;
                } else {
                    delayBaseIndex = (delayBaseIndex>>4) & 0x0F;
                }
                timer_t delayBase = pgm_read_dword(&delay_bases[delayBaseIndex]);
                uint8_t factor    = mem_ReadByte(APP_PORTFUNC_T1_FAKTOR + (port * 4)) & 0x7F;
                timer_t time      = delayBase * factor;
                alloc_timer(&app_dat.cycltimer1[port], time);

                // Bedienkonzept unterscheiden
                opMode = mem_ReadByte(APP_PORTFUNC_JALOCONFIG2 + (port * 4));
                if(opMode & 0x08) {
                    // lang-kurz
                    if((opMode & 0x30) == 0x10) {
                        // send MOVE UP
                        sendObj = MOVE_UP;
                    }
                    else if((opMode & 0x30) == 0x20) {
                        // send MOVE DOWN
                        sendObj = MOVE_DOWN;
                    }
                    else if((opMode & 0x30) == 0x30) {
                        // send MOVE UM
                        if(app_dat.objVal2 & (1<<port)) {
                            app_dat.objVal2 &= ~(1<<port);
                            sendObj = MOVE_UP;
                        }
                        else {
                            app_dat.objVal2 |= (1<<port);
                            sendObj = MOVE_DOWN;
                        }
                    }
                    app_dat.stateJalousie[port] = 3;
                }
                else {
                    // kurz-lang-kurz
                    // send STEP
                    sendObj = STEP;
                    app_dat.stateJalousie[port] = 1;
                }
            }
            break;

        case 1:         // State 1 (kurz-lang-kurz)
            if(check_timeout(&app_dat.cycltimer1[port])) {
                opMode = mem_ReadByte(APP_PORTFUNC_JALOCONFIG2 + (port * 4));
                if((opMode & 0x30) == 0x10) {
                    // send MOVE UP
                    sendObj = MOVE_UP;
                }
                else if((opMode & 0x30) == 0x20) {
                    // send MOVE DOWN
                    sendObj = MOVE_DOWN;
                }
                else if((opMode & 0x30) == 0x30) {
                    // send MOVE UM
                    if(app_dat.objVal2 & (1<<port)) {
                        app_dat.objVal2 &= ~(1<<port);
                        sendObj = MOVE_UP;
                    }
                    else {
                        app_dat.objVal2 |= (1<<port);
                        sendObj = MOVE_DOWN;
                    }
                }

                // Timer T2 starten
                uint8_t delayBaseIndex=mem_ReadByte(APP_PORTFUNC_T2_BASIS + ((port+1)>>1));

                if((port & 0x01) == 0x01) {
                    delayBaseIndex &= 0x0F;
                } else {
                    delayBaseIndex = (delayBaseIndex>>4) & 0x0F;
                }
                timer_t delayBase = pgm_read_dword(&delay_bases[delayBaseIndex]);
                uint8_t factor    = mem_ReadByte(APP_PORTFUNC_T2_FAKTOR + (port * 4)) & 0x7F;
                timer_t time      = delayBase * factor;
                alloc_timer(&app_dat.cycltimer1[port], time);

                app_dat.stateJalousie[port] = 2;
            }
            else {
                // T1 is active
                if( (pinVal == 0) && (pinVal_old  == 1) ) {
                    // detect falling edge => ready
                    app_dat.stateJalousie[port] = 0;
                }
            }
            break;

        case 2:         // State 2 (kurz-lang-kurz)
            if(check_timeout(&app_dat.cycltimer1[port])) {
                app_dat.stateJalousie[port] = 0;
            }
            else {
                // T2 is active
                if( (pinVal == 0) && (pinVal_old  == 1) ) {
                    // detect falling edge => send Step
                    sendObj = STEP;
                    app_dat.stateJalousie[port] = 0;
                }
            }
            break;

        case 3:     // State 1 (lang-kurz)
            if(check_timeout(&app_dat.cycltimer1[port])) {
                app_dat.stateJalousie[port] = 0;
            }
            else {
                // T1 is active
                if( (pinVal == 0) && (pinVal_old  == 1) ) {
                    // detect falling edge => send Step
                    sendObj = STEP;
                    app_dat.stateJalousie[port] = 0;
                }
            }
            break;

        case 4:     // sperre aktiv
            break;

        default:
            app_dat.stateJalousie[port] = 0;
            break;
    }

    // **************
    // Sperrfunktion
    // **************
    saftyFunc =  mem_ReadByte(APP_PORTFUNC_JALOCONFIG + (port * 4)) & 0x03;
    if( saftyFunc) {
        // freigegeben
        uint8_t safty = (app_dat.safty>>port) & 0x01;
        uint8_t safty_old = (app_dat.safty_old>>port) & 0x01;

        // invertieren
        if( saftyFunc == 0x02) {
            safty = ~safty & 0x01;
            safty_old = ~safty_old & 0x01;
        }
        if(safty == 0x01) {
            if(safty_old == 0x00) {
                // Beginn einer Sperrung
                saftyFunc = mem_ReadByte(APP_PORTFUNC_JALOCONFIG + (port * 4)) & 0x30;
                if(saftyFunc == 0x10) {
                    // MOVE DOWN senden
                    sendObj = MOVE_DOWN;
                }
                else if(saftyFunc == 0x20) {
                    // MOVE UP senden
                    sendObj = MOVE_UP;
                }
                else if(saftyFunc == 0x30) {
                    // MOVE UM senden
                    if(app_dat.objVal2 & (1<<port)) {
                        app_dat.objVal2 &= ~(1<<port);
                        sendObj = MOVE_UP;
                    }
                    else {
                        app_dat.objVal2 |= (1<<port);
                        sendObj = MOVE_DOWN;
                    }
                }
            }
            app_dat.stateJalousie[port] = 4;
        }
        else {
            if(safty_old == 0x01) {
                // Ende einer Sperrung
                saftyFunc = mem_ReadByte(APP_PORTFUNC_JALOCONFIG + (port * 4)) & 0x0C;
                if(saftyFunc == 0x04) {
                    // MOVE DOWN senden
                    sendObj = MOVE_DOWN;
                }
                else if(saftyFunc == 0x08) {
                    // MOVE UP senden
                    sendObj = MOVE_UP;
                }
                else if(saftyFunc == 0x0C) {
                    // MOVE UM senden
                    if(app_dat.objVal2 & (1<<port)) {
                        app_dat.objVal2 &= ~(1<<port);
                        sendObj = MOVE_UP;
                    }
                    else {
                        app_dat.objVal2 |= (1<<port);
                        sendObj = MOVE_DOWN;
                    }
                }
            }
            app_dat.stateJalousie[port] = 0;
        }
    }

    // ***********************************
    // check if we have to send a message
    // ***********************************
    if( sendObj & 0x03) {
        // check Telegrammratenbegrenzer
        if(app_dat.msgCntr) {
            // send MOVE
            SetAndTransmitBit(port + 8, (sendObj>>1) & 0x1 );
            app_dat.msgCntr--;
        }
    }
    if( sendObj & 0x04) {
        // check Telegrammratenbegrenzer
        if(app_dat.msgCntr) {
            // send STEP
            SetAndTransmitBit(port, 1 );
            app_dat.msgCntr--;
        }
    }

    return;
}

#endif /* _FB_IN8_APP_C */
/*********************************** EOF *********************************/
