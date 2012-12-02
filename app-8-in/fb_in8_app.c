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
#include <avr/wdt.h>
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_eeprom.h"
#include "msg_queue.h"
#include "fb_hal.h"
#include "rf22.h"
#include "fb_prot.h"
#include "fbrf_hal.h"
#include "fb_app.h"
#include "fb_in8_app.h"

/**************************************************************************
* DEFINITIONS
**************************************************************************/
/** Reset the internal variables used for the application timer and reload the timer itself
* @todo check if move of currentTime to this function really does not introduce a bug
*/
#define RESET_RELOAD_APPLICATION_TIMER() {                              \
          currentTimeOverflow=0;                                        \
          currentTimeOverflowBuffer=0;                                  \
          currentTime=0;                                                \
          RELOAD_APPLICATION_TIMER();                                   \
     }

/* application parameters */
#define PORTFUNCTION_12     0x01CE  ///< @todo add documentation
#define PORTFUNCTION_34     0x01CF  ///< @todo add documentation
#define PORTFUNCTION_56     0x01D0  ///< @todo add documentation
#define PORTFUNCTION_78     0x01D1  ///< @todo add documentation
#define DEBOUNCE_FACTOR     0x01D2  ///< @todo add documentation 
#define POWERONDELAY_FACTOR 0x01D4  ///< @todo add documentation
/* Funktion Schalten */
#define PORTFUNC_BASEADR    0x01D5  ///< @todo add documentation
#define PORTFUNC_EDGEFUNC   0x01D7  ///< @todo add documentation
/* Funktion Jalousie */
#define PORTFUNC_T1_FAKTOR  0x01D6  ///< @todo add documentation
#define PORTFUNC_T2_FAKTOR  0x01D7  ///< @todo add documentation
#define PORTFUNC_JALOMODE   0x01D8  ///< @todo add documentation
#define PORTFUNC_T1_BASIS   0x01F6  ///< @todo add documentation
#define PORTFUNC_T2_BASIS   0x01FA  ///< @todo add documentation

#define POWERONDELAY_BASE   0x01FE  ///< @todo add documentation

#define OBJ_SIZE            8       ///< @todo add documentation


/* EIB Device Parameter Values */

#define EIB_PAR_UP          0U     /**< Jalousie Actor value for UP */
#define EIB_PAR_DOWN        1U     /**< Jalousie Actor value for DOWN */


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
    struct
    {
        uint16_t timer   : 12;
        uint8_t StepVal    : 1;
        uint8_t MoveVal    : 1;
        uint8_t intState   : 2;
    } Jalousie; ///< @todo add documentation
} INTVAL_UNION;

/**************************************************************************
* DECLARATIONS
**************************************************************************/
extern struct grp_addr_s grp_addr;

static uint8_t portValue;                 /**< defines the port status. LSB IO0 and MSB IO8, ports with delay can be set to 1 here
                                               but will be switched delayed depending on the delay */
INTVAL_UNION intVal[OBJ_SIZE];              ///< @todo add documentation

//static uint8_t portSperre;                /**< ??? */
//static uint8_t portSperreOld;             /**< ???  */

static uint16_t currentTime;              /**< defines the current time in 10ms steps (2=20ms) */
static uint8_t currentTimeOverflow;       /**< the amount of overflows from currentTime */
//static uint8_t currentTimeOverflowBuffer; /**< is set to one if overflow happened, is 0 if overflow was processed */

static uint8_t powerOnDelay;              ///< @todo add documentation

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM =
{
    { MANUFACTORER_ADR,        0x04 },    /**< Herstellercode 0x04 = Jung                   */
    { DEVICE_NUMBER_HIGH,      0x70 },    /**< Geräte Typ (2118) 7054h                      */
    { DEVICE_NUMBER_LOW,       0x54 },
    { SOFTWARE_VERSION_NUMBER, 0x02 },    /**< Versionsnummer                               */
    { APPLICATION_RUN_STATUS,  0xFF },    /**< Run-Status (00=stop FF=run)                  */
    { COMMSTAB_ADDRESS,        0x84 },    /**< COMMSTAB Pointer                             */
    { APPLICATION_PROGRAMM,    0x00 },    /**< Port A Direction Bit Setting???              */

    { PA_ADDRESS_HIGH,         0x11 },    /**< default address is 1.1.52                    */
    { PA_ADDRESS_LOW,          0x34 },    /**<                                              */   

    { 0xFF,                    0xFF }     /**< END-sign; do not change                      */
};

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
            portValue = ReadPorts();

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
                        }
                        else if(powerOnFunction == 0xC0)
                        {
                            /* send message with port value */
                            sendTelegram(i, ((portValue>>i) & 0x01), 0x00);
                            sendTelegram((i + 8), ((portValue>>i) & 0x01), 0x00);
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
        portChanged = portValue ^ ReadPorts();

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
            portNewValue = portValue;
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

        if(portChanged != 0)
        {
            portValue =  portNewValue;
        }
    }   /* END process application */
    return;
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
* @return 
*/
uint8_t restartApplication(void) {
    uint8_t i;

    /* Reset Object and Port State */
    portValue = 0U;
    for (i = 0; i < OBJ_SIZE; i++) {
        intVal[i].Schalten.objectVal_1 = 0U;
        intVal[i].Schalten.objectVal_2 = 0U;
    }

    /* Verzoegerungszeit bei Bussspannungswiederkehr */
    powerOnDelay = mem_ReadByte(POWERONDELAY_FACTOR)<<(mem_ReadByte(POWERONDELAY_BASE)>>4);

    /* reset global timer values */
    currentTime=0;
    currentTimeOverflow=0;

#ifndef HARDWARETEST
    /* IO configuration */
    SET_IO_IO1(IO_INPUT);
    SET_IO_IO2(IO_INPUT);
    SET_IO_IO3(IO_INPUT);
    SET_IO_IO4(IO_INPUT);
    SET_IO_IO5(IO_INPUT);
    SET_IO_IO6(IO_INPUT);
    SET_IO_IO7(IO_INPUT);
    SET_IO_IO8(IO_INPUT);
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
* Read status from port and return it.
* 
* @param rxmsg 
* 
* @return 
*/
uint8_t readApplication(struct msg *rxmsg) {
    return FB_ACK;
}   /* readApplication() */

/** 
* Function is called if A_GroupValue_Write is received. The type it is the function "EIS1" or "Data Type Boolean" for the relais module.
* Read all parameters in that function and set global variables.
*
* @param rxmsg 
* 
* @return The return value defies if a ACK or a NACK should be sent (FB_ACK, FB_NACK)
*/
uint8_t runApplication(struct msg *rxmsg) {

	return FB_ACK; // must not return NACK, causes confusion with other devices responding
}   /* runApplication() */

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
            portfunction = ((mem_ReadByte(PORTFUNCTION_12)>>4) & 0x0F);
            break;
        case 2:
            portfunction = (mem_ReadByte(PORTFUNCTION_34) & 0x0F);
            break;

        case 3:
            portfunction = ((mem_ReadByte(PORTFUNCTION_34)>>4) & 0x0F);
            break;

        case 4:
            portfunction = (mem_ReadByte(PORTFUNCTION_56) & 0x0F);
            break;

        case 5:
            portfunction = ((mem_ReadByte(PORTFUNCTION_56)>>4) & 0x0F);
            break;

        case 6:
            portfunction = (mem_ReadByte(PORTFUNCTION_78) & 0x0F);
            break;

        case 7:
            portfunction = ((mem_ReadByte(PORTFUNCTION_78)>>4) & 0x0F);
            break;

        default:
            portfunction = 0;
            break;
    }

    return (EFUNC_PORT)portfunction;
}


/**                                                                       
* Read all inputpins and store values into a byte
*
* @return port
*   
*/
uint8_t ReadPorts(void)
{
    uint8_t port = 0;

    port |= (uint8_t)GETPIN_IO1();
    port |= (((uint8_t)GETPIN_IO2())<<1);
    port |= (((uint8_t)GETPIN_IO3())<<2);
    port |= (((uint8_t)GETPIN_IO4())<<3);
    port |= (((uint8_t)GETPIN_IO5())<<4);
    port |= (((uint8_t)GETPIN_IO6())<<5);
    port |= (((uint8_t)GETPIN_IO7())<<6);
    port |= (((uint8_t)GETPIN_IO8())<<7);

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

    }
    else
    {
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
void PortFunc_Jalousie(uint8_t port, uint8_t newPortValue, uint8_t portChanged)
{
uint8_t jalousieMode;
uint16_t jalousieTimer;

    jalousieMode = mem_ReadByte(PORTFUNC_JALOMODE + (port * 4));

    switch(intVal[port].Jalousie.intState)
    {
        case 0:     /* waiting */
            if( (newPortValue & portChanged & (1U<<port)) != 0U)
            {
                /* detect rising edge */
                if( (jalousieMode & 0x08) == 0U)
                {
                    /* Bedienkonzept kurz-lang-kurz */
                    /* send STEP (STOP) telegramm */
                    sendTelegram(port, 0, 0x00);
                }
                else
                {
                    /* Bedienkonzept lang-kurz */
                    if( (jalousieMode & 0x10) != 0U)
                    {
                        /* send MOVE telegramm with UP command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                        sendTelegram((port+8), EIB_PAR_UP, 0x00); 
                    }
                    else if( (jalousieMode & 0x20) != 0U)
                    {
                        /* send MOVE telegramm with DOWN command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                        sendTelegram((port+8), EIB_PAR_DOWN, 0x00); 
                    }
                    else if( (jalousieMode & 0x40) != 0U)
                    {
                        /* send MOVE telegramm with CHANGE command */
                        if(intVal[port].Jalousie.MoveVal == EIB_PAR_DOWN)
                        {
                            /* send MOVE telegramm with UP command */
                            intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                            sendTelegram((port+8), EIB_PAR_UP, 0x00); 
                        }
                        else
                        {
                            /* send MOVE telegramm with DOWN command */
                            intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                            sendTelegram((port+8), EIB_PAR_DOWN, 0x00); 
                        }
                    }
                }

                /* Delaytime T1 */
                jalousieTimer = mem_ReadByte(PORTFUNC_T1_BASIS + ((port+1)>>1));
                if( (port & 0x01) != 0)
                {
                    jalousieTimer =  (jalousieTimer >> 4) + 1;
                }

                jalousieTimer =  jalousieTimer & 0x0F;
                jalousieTimer *= mem_ReadByte(PORTFUNC_T1_FAKTOR + (port * 4));

                intVal[port].Jalousie.timer =  jalousieTimer - 1U;
                intVal[port].Jalousie.intState = 1U;
            }

            break;

        case 1:     // State Kurz
            if( (jalousieMode & 0x08) == 0U)
            {
                /* Bedienkonzept kurz-lang-kurz */
                if( intVal[port].Jalousie.timer != 0)
                {
                    intVal[port].Jalousie.timer--;

                    /* detect falling edge */
                    if( (~newPortValue & portChanged & (1U<<port)) != 0U)
                    {
                        /* send STEP (STOP) telegramm */
                        intVal[port].Jalousie.intState = 0U;
                    }
                }
                else
                {
                    if( (jalousieMode & 0x10) != 0U)
                    {
                        /* send MOVE telegramm with UP command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                        sendTelegram((port+8), EIB_PAR_UP, 0x00); 
                    }
                    else if( (jalousieMode & 0x20) != 0U)
                    {
                        /* send MOVE telegramm with DOWN command */
                        intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                        sendTelegram((port+8), EIB_PAR_DOWN, 0x00); 
                    }
                    else if( (jalousieMode & 0x40) != 0U)
                    {
                        /* send MOVE telegramm with CHANGE command */
                        if(intVal[port].Jalousie.MoveVal == EIB_PAR_DOWN)
                        {
                            /* send MOVE telegramm with UP command */
                            intVal[port].Jalousie.MoveVal = EIB_PAR_UP;
                            sendTelegram((port+8), EIB_PAR_UP, 0x00); 
                        }
                        else
                        {
                            /* send MOVE telegramm with DOWN command */
                            intVal[port].Jalousie.MoveVal = EIB_PAR_DOWN;
                            sendTelegram((port+8), EIB_PAR_DOWN, 0x00); 
                        }
                    }

                    /* Delaytime T2 */
                jalousieTimer = mem_ReadByte(
                        PORTFUNC_T2_BASIS + ((port + 1) >> 1));
                if ((port & 0x01) != 0) {
                        jalousieTimer =  (jalousieTimer >> 4) + 1;
                    }

                    jalousieTimer =  jalousieTimer & 0x0F;
                    jalousieTimer *= mem_ReadByte(PORTFUNC_T2_FAKTOR + (port * 4));

                    intVal[port].Jalousie.timer =  jalousieTimer - 1U;
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

#ifdef HARDWARETEST
/**                                                                       
* switch all of the output pins
*
* @param 
*   
*/
void switchPorts(uint8_t port)
{
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
#endif
/**                                                                       
* The start point of the program, init all libraries, start the bus interface,
* the application and check the status of the program button.
*
* @return 
*   
*/
int main(void) {
    uint16_t t1cnt;
#ifdef FB_RF
    uint8_t pollcnt;
#endif
    /* disable wd after restart_app via watchdog */DISABLE_WATCHDOG()

    /* ROM-Check */
    /** @todo Funktion fuer CRC-Check bei PowerOn fehlt noch */

    /* init internal Message System */
    msg_queue_init();

    /* init eeprom modul and RAM structure already here,
       because we need eeprom values for fbrfhal_init() */
    eeprom_Init(&nodeParam[0], EEPROM_SIZE);

    /* init procerssor register */
    fbhal_Init();
	/** FBRFHAL_INIT() is defined in fbrf_hal.h .
	   you may leave it out if you don't use rf */
    FBRFHAL_INIT();
    /* enable interrupts */ENABLE_ALL_INTERRUPTS();

    /* init protocol layer */
    /* load default values */
    fbprot_Init(defaultParam);

    /* config application hardware */
    (void)restartApplication();


#ifdef HARDWARETEST
    sendTestTelegram();
#endif
    /* activate watchdog */
    ENABLE_WATCHDOG ( WDTO_250MS );

    /***************************/
    /* the main loop / polling */
    /***************************/
    while (1) {
        /* calm the watchdog */
        wdt_reset();
        /* Auswerten des Programmiertasters */
        fbhal_checkProgTaster();

        fbprot_msg_handler();

        /* check if 130ms timer is ready */
        if (TIMER1_OVERRUN) {
            CLEAR_TIMER1_OVERRUN;
			/** FBRFHAL_POLLING() is defined in fbrf_hal.h .
			   you may leave it out if you don't use rf */
			FBRFHAL_POLLING();
			/** APP_TIMER_OVERRUN() is not defined if you use avr board rev. 3.01. */
#ifndef BOARD301
            if (!APP_TIMER_OVERRUN())
                continue;
#endif
#ifndef HARDWARETEST
            timerOverflowFunction();
#else
//    		sendTestTelegram();
            hardwaretest();
#endif
        }

        // go to sleep mode here
        // wakeup via interrupt check then the programming button and application timer for an overrun
        // for detailed list see datasheet page 40ff
        // MC need about 6 cyles to wake up at 8 MHZ that are 6*0.125µs
//        PRR |= (1<<PRADC)|(1<<PRSPI)|(1<<PRTWI);
//        set_sleep_mode(SLEEP_MODE_IDLE);

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
    {
        pinstate = 1;
    }
    return;
}
#endif

#endif /* _FB_IN8_APP_C */
/*********************************** EOF *********************************/
