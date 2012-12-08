/* $Id: fb_in8_app.c 624 2008-10-17 08:03:57Z idefix $ */
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
 * Device type (2118) 0x???? Ordernumber: 2118.??REG\n
 */
#ifndef _FB_IN8_APP_C
#define _FB_IN8_APP_C

/* Test new Hardware */
/* - Lauflicht mit 130 ms Pulslaenge (Applikationtimer) */
/* - Testtelegramme (Ausmessen des Bustimings */

/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb_in8_app.h"

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/
/* Objects for the 8-out */
enum EIGHT_IN_Objects_e {
    OBJ_IN0 = 0,
    OBJ_IN1,
    OBJ_IN2,
    OBJ_IN3,
    OBJ_IN4,
    OBJ_IN5,
    OBJ_IN6,
    OBJ_IN7,
    OBJ_IN8, // start of special objects used to lock or combine objects
    OBJ_IN9,
    OBJ_IN10,
    OBJ_IN11,
    OBJ_IN12,
    OBJ_IN13,
};

/// Bit list of states the program can be in
enum states_e {
    IDLE = 0,
    INIT_TIMER = (1),
    TIMER_ACTIVE = (1 << 1),
    PWM_TIMER_ACTIVE = (1 << 2),
};

/** @todo add documentation */
typedef enum {
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

/** 
 * @todo add documentation
 */
typedef union {
    struct {
        uint8_t objectVal_1;
        uint8_t objectVal_2;
    } Schalten; ///< @todo add documentation
    struct {
        uint16_t timer :12;
        uint8_t StepVal :1;
        uint8_t MoveVal :1;
        uint8_t intState :2;
    } Jalousie; ///< @todo add documentation
} INTVAL_UNION;

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/

struct {
    uint8_t portValue;          /**< defines the port status. LSB IO0 and MSB IO8, ports with delay can be set to 1 here
                                             but will be switched delayed depending on the delay */
    uint8_t oldValue;           /// hold the old value to check if we must enable a PWM or not (enable PWM only if switching from low -> high
    timer_t timer[8];
    timer_t pwmTimer;           /// stores a reference to the generic timer
    uint8_t runningTimer;
    uint16_t objectStates;      /**< store logic state of objects, 1 bit each, 8 "real" + 4 sf*/
    uint8_t blockedStates;      /**< 1 bit per object to mark it "blocked" */
} app_dat;

INTVAL_UNION intVal[OBJ_SIZE];              ///< @todo add documentation

static uint16_t currentTime; /**< defines the current time in 10ms steps (2=20ms) */
static uint8_t currentTimeOverflow; /**< the amount of overflows from currentTime */
//static uint8_t currentTimeOverflowBuffer; /**< is set to one if overflow happened, is 0 if overflow was processed */

static uint8_t powerOnDelay;              ///< @todo add documentation

uint8_t nodeParam[EEPROM_SIZE]; /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void timerOverflowFunction(void);
EFUNC_PORT getPortFunction(uint8_t port);
uint8_t ReadPorts(void);
void PortFunc_Switch(uint8_t port, uint8_t newPortValue, uint8_t portChanged);
void PortFunc_Jalousie(uint8_t port, uint8_t newPortValue, uint8_t portChanged);

#ifdef HARDWARETEST
void switchPorts(uint8_t port);
/** test function: processor and hardware */
void hardwaretest(void);
#endif

/**************************************************************************
 * IMPLEMENTATION
 **************************************************************************/

/** 
 * Timer1 is used as application timer. It increase the variable currentTime every 130ms and currentTimeOverflow if
 * currentTime runs over 16-bit.
 *
 * @return
 */
void timerOverflowFunction(void) {
    uint8_t i;
    uint8_t powerOnFunction;
    uint8_t portChanged;
    uint8_t portNewValue;
    uint8_t debounceFactor;

    /* check if programm is running */
    if (mem_ReadByte(APPLICATION_RUN_STATUS) != 0xFF) {
        /* Do nothing */
        ;
    }

    /* Verzoegerungszeit bei Bussspannungswiederkehr */
    else if (powerOnDelay) {
        powerOnDelay--;
        if (powerOnDelay == 0) {
            /* Read Input Ports */
            app_dat.portValue = ReadPorts();

            for (i = 0; i < 8; i++) {
                powerOnFunction = (mem_ReadByte(PORTFUNC_BASEADR + (i * 4))
                        & 0xC0);

                switch (getPortFunction(i)) {
                case eFunc_schalten:
                    if (powerOnFunction == 0x40) {
                        /* send ON message */
                        sendTelegram(i, 1, 0x00);
                        sendTelegram((i + 8), 1, 0x00);
                    } else if (powerOnFunction == 0x80) {
                        /* send OFF message */
                        sendTelegram(i, 0, 0x00);
                        sendTelegram((i + 8), 0, 0x00);
                    } else if (powerOnFunction == 0xC0) {
                        /* send message with port value */
                        sendTelegram(i, ((app_dat.portValue >> i) & 0x01), 0x00);
                        sendTelegram((i + 8), ((app_dat.portValue >> i) & 0x01), 0x00);
                    }
                    break;
                case eFunc_dimmen:
                    break;
                case eFunc_jalousie:
                    if (powerOnFunction == 0x40) {
                        /* send DOWN message */
                        sendTelegram((i + 8), EIB_PAR_DOWN, 0x00);
                    } else if (powerOnFunction == 0x80) {
                        /* send UP message */
                        sendTelegram((i + 8), EIB_PAR_UP, 0x00);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    } else {
        /* process application */

        if (currentTime == 0xFFFF) {
            currentTime = 0;
            currentTimeOverflow++;
        } else {
            currentTime++;
        }

        /* set flags for new input level */
        portChanged = app_dat.portValue ^ ReadPorts();

        /* new input level => start debaunce time */
        if (portChanged != 0) {
            /* Debounce */
            debounceFactor = mem_ReadByte(DEBOUNCE_FACTOR);

            for (i = 0; i < debounceFactor; i++) {
                /* delay time 0.5 ms */
                _delay_us(500);
            }

            portNewValue = ReadPorts();
        } else {
            portNewValue = app_dat.portValue;
        }

        /* process the port function */
        for (i = 0; i < 8; i++) {
            switch (getPortFunction(i)) {
            case eFunc_schalten:
                /* toDo: Hier Sperrobjekt checken */
                PortFunc_Switch(i, portNewValue, portChanged);
                break;
            case eFunc_dimmen:
                break;
            case eFunc_jalousie:
                PortFunc_Jalousie(i, portNewValue, portChanged);
                break;
            default:
                break;
            }
        }

        if (portChanged != 0) {
            app_dat.portValue = portNewValue;
        }
    } /* END process application */
    return;
}

/** 
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return
 */
uint8_t restartApplication(void) {
    uint8_t i;

    /* Reset Object and Port State */
    app_dat.portValue = 0U;
    for (i = 0; i < OBJ_SIZE; i++) {
        intVal[i].Schalten.objectVal_1 = 0U;
        intVal[i].Schalten.objectVal_2 = 0U;
    }

    /* Verzoegerungszeit bei Bussspannungswiederkehr */
    powerOnDelay = mem_ReadByte(POWERONDELAY_FACTOR)
            << (mem_ReadByte(POWERONDELAY_BASE) >> 4);

    /* reset global timer values */
    currentTime = 0;
    currentTimeOverflow = 0;

#ifndef HARDWARETEST
    /* IO configuration */
    IO_SET_DIR(1, IO_INPUT);
    IO_SET_DIR(2, IO_INPUT);
    IO_SET_DIR(3, IO_INPUT);
    IO_SET_DIR(4, IO_INPUT);
    IO_SET_DIR(5, IO_INPUT);
    IO_SET_DIR(6, IO_INPUT);
    IO_SET_DIR(7, IO_INPUT);
    IO_SET_DIR(8, IO_INPUT);
#ifdef BOARD301
    SET_IO_RES1(IO_INPUT);
    SET_IO_RES2(IO_INPUT);
    SET_IO_RES3(IO_INPUT);
    SET_IO_RES4(IO_INPUT);
#endif
#else
    /* Port configuration for hardwaretest */
    SET_IO_IO1(IO_OUTPUT);
    SET_IO_IO2(IO_OUTPUT);
    SET_IO_IO3(IO_OUTPUT);
    SET_IO_IO4(IO_OUTPUT);
    SET_IO_IO5(IO_OUTPUT);
    SET_IO_IO6(IO_OUTPUT);
    SET_IO_IO7(IO_OUTPUT);
    SET_IO_IO8(IO_OUTPUT);
#ifdef BOARD301
    SET_IO_RES1(IO_OUTPUT);
    SET_IO_RES2(IO_OUTPUT);
    SET_IO_RES3(IO_OUTPUT);
    SET_IO_RES4(IO_OUTPUT);
#endif
#endif

    /* CTRL-Port */
    SET_IO_CTRL(IO_INPUT);

    /* enable timer to increase user timer used for timer functions etc. */
    RELOAD_APPLICATION_TIMER();

    return 1;
} /* restartApplication() */

/**                                                                       
 * Get the function code for the select port
 *
 * @return port function
 *
 */
EFUNC_PORT getPortFunction(uint8_t port) {
    uint8_t portfunction;

    switch (port) {
    case 0:
        portfunction = (mem_ReadByte(PORTFUNCTION_12) & 0x0F);
        break;

    case 1:
        portfunction = ((mem_ReadByte(PORTFUNCTION_12) >> 4) & 0x0F);
        break;
    case 2:
        portfunction = (mem_ReadByte(PORTFUNCTION_34) & 0x0F);
        break;

    case 3:
        portfunction = ((mem_ReadByte(PORTFUNCTION_34) >> 4) & 0x0F);
        break;

    case 4:
        portfunction = (mem_ReadByte(PORTFUNCTION_56) & 0x0F);
        break;

    case 5:
        portfunction = ((mem_ReadByte(PORTFUNCTION_56) >> 4) & 0x0F);
        break;

    case 6:
        portfunction = (mem_ReadByte(PORTFUNCTION_78) & 0x0F);
        break;

    case 7:
        portfunction = ((mem_ReadByte(PORTFUNCTION_78) >> 4) & 0x0F);
        break;

    default:
        portfunction = 0;
        break;
    }

    return (EFUNC_PORT) portfunction;
}

/**                                                                       
 * Read all inputpins and store values into a byte
 *
 * @return port
 * @todo add parameter, how many ports to read
 */
uint8_t ReadPorts(void) {
    uint8_t port = 0;

    port |= (uint8_t) IO_GET(1);
    port |= (((uint8_t) IO_GET(2)) << 1);
    port |= (((uint8_t) IO_GET(3)) << 2);
    port |= (((uint8_t) IO_GET(4)) << 3);
    port |= (((uint8_t) IO_GET(5)) << 4);
    port |= (((uint8_t) IO_GET(6)) << 5);
    port |= (((uint8_t) IO_GET(7)) << 6);
    port |= (((uint8_t) IO_GET(8)) << 7);

    return port;
}

/**                                                                       
 * Port Function: switch
 *
 * @param port
 * @param newPortValue
 * @param portChanged
 */
void PortFunc_Switch(uint8_t port, uint8_t newPortValue, uint8_t portChanged) {
    uint8_t edgeFunc;

    edgeFunc = mem_ReadByte(PORTFUNC_EDGEFUNC + (port * 4));

    if ((portChanged & (1U << port)) != 0) {
        if (((newPortValue >> port) & 0x01) != 0) {
            /* rising edge */
            if ((edgeFunc & 0x0C) == 0x04) {
                /* object x.1 = EIN */
                sendTelegram(port, 1, 0x00);
                intVal[port].Schalten.objectVal_1 = 1U;

            } else if ((edgeFunc & 0x0C) == 0x08) {
                /* object x.1 = AUS */
                sendTelegram(port, 0, 0x00);
                intVal[port].Schalten.objectVal_1 = 0U;
            } else if ((edgeFunc & 0x0C) == 0x0C) {
                /* object x.1 = UM */
                if (intVal[port].Schalten.objectVal_1) {
                    /* switch 1 => 0 */
                    sendTelegram(port, 0, 0x00);
                    intVal[port].Schalten.objectVal_1 = 0U;
                } else {
                    /* switch 0 => 1 */
                    sendTelegram(port, 1, 0x00);
                    intVal[port].Schalten.objectVal_1 = 1U;
                }
            }

            if ((edgeFunc & 0xC0) == 0x40) {
                /* object x.2 = EIN */
                sendTelegram((port + 8), 1, 0x00);
                intVal[port].Schalten.objectVal_2 = 1U;
            } else if ((edgeFunc & 0xC0) == 0x80) {
                /* object x.2 = AUS */
                sendTelegram((port + 8), 0, 0x00);
                intVal[port].Schalten.objectVal_2 = 0U;
            } else if ((edgeFunc & 0xC0) == 0xC0) {
                /* object x.2 = UM */
                if (intVal[port].Schalten.objectVal_2) {
                    /* switch 1 => 0 */
                    sendTelegram((port + 8), 0, 0x00);
                    intVal[port].Schalten.objectVal_2 = 0U;
                } else {
                    /* switch 0 => 1 */
                    sendTelegram((port + 8), 1, 0x00);
                    intVal[port].Schalten.objectVal_2 = 1U;
                }
            }
        } else {
            /* falling edge */
            if ((edgeFunc & 0x03) == 0x01) {
                /* object x.1 = EIN */
                sendTelegram(port, 1, 0x00);
                intVal[port].Schalten.objectVal_1 = 1U;
            } else if ((edgeFunc & 0x03) == 0x02) {
                /* object x.1 = AUS */
                sendTelegram(port, 0, 0x00);
                intVal[port].Schalten.objectVal_1 = 0U;
            } else if ((edgeFunc & 0x03) == 0x03) {
                /* object x.1 = UM */
                if (intVal[port].Schalten.objectVal_1) {
                    /* switch 1 => 0 */
                    sendTelegram(port, 0, 0x00);
                    intVal[port].Schalten.objectVal_1 = 0U;
                } else {
                    /* switch 0 => 1 */
                    sendTelegram(port, 1, 0x00);
                    intVal[port].Schalten.objectVal_1 = 1U;
                }
            }

            if ((edgeFunc & 0x30) == 0x10) {
                /* object x.2 = EIN */
                sendTelegram((port + 8), 1, 0x00);
                intVal[port].Schalten.objectVal_2 = 1U;
            } else if ((edgeFunc & 0x30) == 0x20) {
                /* object x.2 = AUS */
                sendTelegram((port + 8), 0, 0x00);
                intVal[port].Schalten.objectVal_2 = 0U;
            } else if ((edgeFunc & 0x30) == 0x30) {
                /* object x.2 = UM */
                if (intVal[port].Schalten.objectVal_2) {
                    /* switch 1 => 0 */
                    sendTelegram((port + 8), 0, 0x00);
                    intVal[port].Schalten.objectVal_2 = 0U;
                } else {
                    /* switch 0 => 1 */
                    sendTelegram((port + 8), 1, 0x00);
                    intVal[port].Schalten.objectVal_2 = 1U;
                }
            }
        }

    } else {
        /* check for cyclic transmit is active */
    }

    return;
}

/**                                                                       
 * Port Function: Jalousie
 *
 * @param port
 * @param newPortValue
 * @param portChanged
 */
void PortFunc_Jalousie(uint8_t port, uint8_t newPortValue, uint8_t portChanged) {
    uint8_t jalousieMode;
    uint16_t jalousieTimer;

    jalousieMode = mem_ReadByte(PORTFUNC_JALOMODE + (port * 4));

    switch (intVal[port].Jalousie.intState) {
    case 0: /* waiting */
        if ((newPortValue & portChanged & (1U << port)) != 0U) {
            /* detect rising edge */
            if ((jalousieMode & 0x08) == 0U) {
                /* Bedienkonzept kurz-lang-kurz */
                /* send STEP (STOP) telegramm */
                sendTelegram(port, 0, 0x00);
            } else {
                /* Bedienkonzept lang-kurz */
                if ((jalousieMode & 0x10) != 0U) {
                    /* send MOVE telegramm with UP command */
                    intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                    sendTelegram((port + 8), EIB_PAR_UP, 0x00);
                } else if ((jalousieMode & 0x20) != 0U) {
                    /* send MOVE telegramm with DOWN command */
                    intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                    sendTelegram((port + 8), EIB_PAR_DOWN, 0x00);
                } else if ((jalousieMode & 0x40) != 0U) {
                    /* send MOVE telegramm with CHANGE command */
                    if (intVal[port].Jalousie.MoveVal == EIB_PAR_DOWN) {
                        /* send MOVE telegramm with UP command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                        sendTelegram((port + 8), EIB_PAR_UP, 0x00);
                    } else {
                        /* send MOVE telegramm with DOWN command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                        sendTelegram((port + 8), EIB_PAR_DOWN, 0x00);
                    }
                }
            }

            /* Delaytime T1 */
            jalousieTimer = mem_ReadByte(PORTFUNC_T1_BASIS + ((port + 1) >> 1));
            if ((port & 0x01) != 0) {
                jalousieTimer = (jalousieTimer >> 4) + 1;
            }

            jalousieTimer = jalousieTimer & 0x0F;
            jalousieTimer *= mem_ReadByte(PORTFUNC_T1_FAKTOR + (port * 4));

            intVal[port].Jalousie.timer = jalousieTimer - 1U;
            intVal[port].Jalousie.intState = 1U;
        }

        break;

    case 1:     // State Kurz
        if ((jalousieMode & 0x08) == 0U) {
            /* Bedienkonzept kurz-lang-kurz */
            if (intVal[port].Jalousie.timer != 0) {
                intVal[port].Jalousie.timer--;

                /* detect falling edge */
                if ((~newPortValue & portChanged & (1U << port)) != 0U) {
                    /* send STEP (STOP) telegramm */
                    intVal[port].Jalousie.intState = 0U;
                }
            } else {
                if ((jalousieMode & 0x10) != 0U) {
                    /* send MOVE telegramm with UP command */
                    intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                    sendTelegram((port + 8), EIB_PAR_UP, 0x00);
                } else if ((jalousieMode & 0x20) != 0U) {
                    /* send MOVE telegramm with DOWN command */
                    intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                    sendTelegram((port + 8), EIB_PAR_DOWN, 0x00);
                } else if ((jalousieMode & 0x40) != 0U) {
                    /* send MOVE telegramm with CHANGE command */
                    if (intVal[port].Jalousie.MoveVal == EIB_PAR_DOWN) {
                        /* send MOVE telegramm with UP command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                        sendTelegram((port + 8), EIB_PAR_UP, 0x00);
                    } else {
                        /* send MOVE telegramm with DOWN command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                        sendTelegram((port + 8), EIB_PAR_DOWN, 0x00);
                    }
                }

                /* Delaytime T2 */
                jalousieTimer = mem_ReadByte(
                        PORTFUNC_T2_BASIS + ((port + 1) >> 1));
                if ((port & 0x01) != 0) {
                    jalousieTimer = (jalousieTimer >> 4) + 1;
                }

                jalousieTimer = jalousieTimer & 0x0F;
                jalousieTimer *= mem_ReadByte(PORTFUNC_T2_FAKTOR + (port * 4));

                intVal[port].Jalousie.timer = jalousieTimer - 1U;
                intVal[port].Jalousie.intState = 2U;
            }
        } else {
            /* Bedienkonzept lang-kurz */
            if (intVal[port].Jalousie.timer != 0) {
                intVal[port].Jalousie.timer--;

                /* detect falling edge */
                if ((~newPortValue & portChanged & (1U << port)) != 0U) {
                    /* send STEP (STOP) telegramm */
                    sendTelegram(port, 0, 0x00);
                    intVal[port].Jalousie.intState = 0U;
                }
            } else {
                intVal[port].Jalousie.intState = 0U;
            }
        }
        break;

    case 2:     // State Lang
        if ((jalousieMode & 0x08) == 0U) {
            /* Bedienkonzept kurz-lang-kurz */
            if (intVal[port].Jalousie.timer != 0) {
                intVal[port].Jalousie.timer--;

                /* detect falling edge */
                if ((~newPortValue & portChanged & (1U << port)) != 0U) {
                    sendTelegram(port, 0, 0x00);
                    intVal[port].Jalousie.intState = 0U;
                }
            } else {
                intVal[port].Jalousie.intState = 0U;
            }

        } else {
            intVal[port].Jalousie.intState = 0U;
        }
        break;
    default:
        break;
    }

    return;
}

/**
 * Function os called periodically of the application is enabled in the system_state
 *
 */
void app_loop() {
    timerOverflowFunction();
}

#endif /* _FB_IN8_APP_C */
/*********************************** EOF *********************************/
