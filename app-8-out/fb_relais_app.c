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
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb_relais_app.c
* @author Matthias Fechner, Christian Bode
* @date   Sat Jan 05 17:44:47 2008
* 
* @brief  The relais application to switch 8 relais
* Manufactorer code is 0x04 = Jung\n
* Device type (2038.10) 0x2060 Ordernumber: 2138.10REG\n
*/
#ifndef _FB_RELAIS_APP_C
#define _FB_RELAIS_APP_C


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_eeprom.h"
#include "msg_queue.h"
//#include "1wire.h"
#include "fb_hal.h"
#include "fb_prot.h"
#include "fb_app.h"
#include "fb_relais_app.h"

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
/* configure pwm timer, we use timer2 for this */
/* 0xF2=4,8% duty cycle, 0xE6=10%, 0x01=100%   */
/* The 74HCT573 is active low, changed to 0x33=51 -> This is about 20% negative duty cycle */
#define PWM_SETPOINT    0x64 //0x64=100 //0x4B=75 besser aber bei erschütterung nicht okay //=51=20% neg 
#define PWM_DELAY_TIME  10


/**************************************************************************
* DECLARATIONS
**************************************************************************/
extern struct grp_addr_s grp_addr;
static uint8_t portValue;                 /**< defines the port status. LSB IO0 and MSB IO8, ports with delay can be set to 1 here
                                               but will be switched delayed depending on the delay */
static uint16_t currentTime;              /**< defines the current time in 10ms steps (2=20ms) */
static uint8_t currentTimeOverflow;       /**< the amount of overflows from currentTime */
static uint8_t currentTimeOverflowBuffer; /**< is set to one if overflow happened, is 0 if overflow was processed */
static uint16_t delayValues[8];           /**< save value for delays */
static uint8_t timerRunning;              /**< bit is set if timer is running */
static uint8_t waitToPWM;                 /**< defines wait time until PWM get active again (counts down in 130ms steps), 1==enable PWM, 0==no change */

uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM =
{
    { MANUFACTORER_ADR,        0x04 },    /**< Herstellercode 0x04 = Jung                   */
    { DEVICE_NUMBER_HIGH,      0x20 },    /**< device type (2038.10) 2060h                   */
    { DEVICE_NUMBER_LOW,       0x60 },    /**<                                              */
    { SOFTWARE_VERSION_NUMBER, 0x01 },    /**< version number                               */
    { APPLICATION_RUN_STATUS,  0xFF },    /**< Run-Status (00=stop FF=run)                  */
    { COMMSTAB_ADDRESS,        0x9A },    /**< COMMSTAB Pointer                             */
    { APPLICATION_PROGRAMM,    0x00 },    /**< Port A Direction Bit Setting???              */

    { 0x0000,                  0x00 },    /**< default is off                               */
    { 0x01EA,                  0x00 },    /**< no timer active                              */
    { 0x01F6,                  0x55 },    /**< don't save status at power loss (number 1-4) */
    { 0x01F7,                  0x55 },    /**< don't save status at power loss (number 5-8) */
    { 0x01F2,                  0x00 },    /**< closer mode for all relais                   */

    { PA_ADDRESS_HIGH,         0x11 },    /**< default address is 1.1.51                    */
    { PA_ADDRESS_LOW,          0x33 },    /**<                                              */   

    { 0xFF,                    0xFF }     /**< END-sign; do not change                      */
};


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/
void timerOverflowFunction(void);
void switchObjects(void);
void switchPorts(uint8_t port);

#ifdef HARDWARETEST
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
* @todo test interrupt lock in this function that it is not disturbing TX and RX of telegrams
*/
void timerOverflowFunction(void)
{
    uint8_t needToSwitch = 0; 
    uint8_t i;

    /* check if programm is running */
    if(eeprom_ParamRead(APPLICATION_RUN_STATUS) != 0xFF)
    {
        return;
    }
     
    if(currentTime == 0xFFFF)
    {
        currentTime = 0;
        currentTimeOverflow++;
    }else
    {
        currentTime++;
    }
     
    /* check if we can enable PWM */
    /* if waitToPWM==1 enable PWM, 0==no change */
    if(waitToPWM == 1)
    {
        DEBUG_PUTS("PWM");
        DEBUG_NEWLINE();
        ENABLE_PWM(PWM_SETPOINT);
    }

    /* check if we need to lower PWM delay mode */
    if(waitToPWM > 0)
    {
        waitToPWM--;
    }
     
    /* now check if we have to switch a port */
    for(i=0; i<=7; i++)
    {
        if(timerRunning & (1<<i))
        {
            // DEBUG_PUTHEX(timerRunning);
            // we need to check timer for port i
            /** @todo Problem bei einem Timerueberlauf !!! */
            if((delayValues[i] != 0) && (currentTime >= delayValues[i]))
            {
                // we need to switch the port delete delay pin
                // DEBUG_PUTS("SDP");
                DEBUG_PUTHEX(i);

                DISABLE_IRQS;  //disable IRQ here to make sure values are not overwritten, if that delays receiption of telegram the checksum should be wrong a NACK will be send and the sender must resend the telegram

                timerRunning  &= ~((uint8_t)(1U<<i));    // clear timer is running bit
                delayValues[i] = 0;                     // clear delay value for this port
                needToSwitch   = 1;                     // force switch of IOs

                if(portValue & (1<<i))                  // set IO to the new value
                {
                    portValue &= ~((uint8_t)(1U<<i));
                }
                else
                {
                    portValue |= (1<<i);
                }

                ENABLE_IRQS;
                // DEBUG_PUTHEX(portValue);

                /* send response telegram to inform other devices that port was switched */
                sendTelegram(i,(portValue & (1<<i))?1:0, 0x0C);
            }
        }
    }

    if(needToSwitch)
    {
        // DEBUG_PUTS("IRQ");
        switchObjects();
    }
     
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
* @return FB_ACK or FB_NACK
*/
uint8_t restartApplication(void)
{
    uint8_t i,temp;
    uint16_t initialPortValue;

    /* reset global timer values */
    currentTime=0;
    currentTimeOverflow=0;
    timerRunning=0;

    /* IO configuration */
    SET_IO_IO1(IO_OUTPUT);
    SET_IO_IO2(IO_OUTPUT);
    SET_IO_IO3(IO_OUTPUT);
    SET_IO_IO4(IO_OUTPUT);
    SET_IO_IO5(IO_OUTPUT);
    SET_IO_IO6(IO_OUTPUT);
    SET_IO_IO7(IO_OUTPUT);
    SET_IO_IO8(IO_OUTPUT);
#if (HARDWARETEST != 1)
    SET_IO_RES1(IO_INPUT);
    SET_IO_RES2(IO_INPUT);
    SET_IO_RES3(IO_INPUT);
    SET_IO_RES4(IO_INPUT);
#else
    /* Port configuration for hardwaretest */
    SET_IO_RES1(IO_OUTPUT);
    SET_IO_RES2(IO_OUTPUT);
    SET_IO_RES3(IO_OUTPUT);
    SET_IO_RES4(IO_OUTPUT);
#endif

    /* CTRL-Port */
    SET_IO_CTRL(IO_OUTPUT);

    /* configure pwm timer, we use timer2 for this */
    /* 0xF2=4,8% duty cycle, 0xE6=10%, 0x01=100%   */
    waitToPWM = 0;
    ENABLE_PWM(PWM_SETPOINT);

    // check if at power loss we have to restore old values (see 0x01F6) and do it here
    portValue = eeprom_ParamRead(0x0000);
    initialPortValue = ((uint16_t)eeprom_ParamRead(0x01F7) << 8) | ((uint16_t)eeprom_ParamRead(0x01F6));
    for(i=0; i<=7; i++)
    {
        temp = (initialPortValue>>(i*2)) & 0x03;
        // DEBUG_PUTHEX(temp);
        if(temp == 0x01)
        {
            // open contact
            portValue &= (uint8_t)(~(1U<<i));
            // DEBUG_PUTHEX(i);
            // DEBUG_PUTS("P");
        }
        else if(temp == 0x02)
        {
            // close contact
            portValue |= (1<<i);
            // DEBUG_PUTHEX(i);
            // DEBUG_PUTS("L");

        }
    }
    // DEBUG_PUTHEX(portValue);

    /* switch the output pins */
    switchObjects();

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

    assocTabPtr = eeprom_ParamRead(ASSOCTABPTR);
    countAssociations = eeprom_ParamRead(BASE_ADDRESS_OFFSET + assocTabPtr);
 
    for(i=0; i<countAssociations; i++)
    {
        numberInGroupAddress = eeprom_ParamRead(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (i*2));

        // check if valid group address reference
        if(numberInGroupAddress == 0xFE)
        {
            continue;
        }

        commObjectNumber = eeprom_ParamRead(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (i*2) + 1);

        // now check if received address is equal with the safed group addresses, substract one
        // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
        // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
        if((destAddr == grp_addr.ga[numberInGroupAddress-1]) && (commObjectNumber <= 7))
        {
            // found group address

            /** @todo check if read value is allowed */
            struct msg * resp = AllocMsgI();
            if(!resp)
            {
                return FB_NACK;
            }
            /** @todo declaration hides hdr line 268 */
            // struct fbus_hdr * hdr = (struct fbus_hdr *) resp->data;
            hdr = (struct fbus_hdr *) resp->data;

            resp->repeat = 3;
            resp->len    = 9;

            hdr->ctrl    = 0xBC;
            hdr->src[0]  = eeprom_ParamRead(PA_ADDRESS_HIGH);
            hdr->src[1]  = eeprom_ParamRead(PA_ADDRESS_LOW);
            hdr->dest[0] = grp_addr.ga[numberInGroupAddress-1]>>8;
            hdr->dest[1] = grp_addr.ga[numberInGroupAddress-1];
            hdr->npci    = 0xE1;
            hdr->tpci    = 0x00;
            // put data into the apci octet
            hdr->apci    = 0x40 + ((portValue & (1<<commObjectNumber)) ? 1 : 0);

            fb_hal_txqueue_msg(resp);
        }
        else if((commObjectNumber > 7) && (commObjectNumber < 12))
        {
            // additinal function
            /** @todo write part additional functions */
            // DEBUG_PUTS("ZF");
            // DEBUG_NEWLINE();
        }
    }
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
uint8_t runApplication(struct msg *rxmsg)
{
    struct fbus_hdr * hdr= (struct fbus_hdr *) rxmsg->data;
    uint8_t i;
    uint16_t destAddr=((hdr->dest[0])<<8 | hdr->dest[1]);
    uint8_t assocTabPtr = eeprom_ParamRead(ASSOCTABPTR);                             // points to start of association table (0x0100+assocTabPtr)
    uint8_t countAssociations = eeprom_ParamRead(BASE_ADDRESS_OFFSET+assocTabPtr);   // number of associations saved in associations table
    uint8_t numberInGroupAddress;                              // reference from association table to group address table
    uint8_t commObjectNumber;                                  // reference from association table to communication object table
    uint8_t commStabPtr = eeprom_ParamRead(COMMSTAB_ADDRESS);                   // points to communication object table (0x0100+commStabPtr)
    //uint8_t countCommObjects = eeprom_ParamRead(0x0100+commStabPtr);  // number of communication objects in table
    //uint8_t userRamPointer = eeprom_ParamRead(0x0100+commStabPtr+1);  // points to user ram
    uint8_t commValuePointer;                                  // pointer to value
    uint8_t commConfigByte;                                    // configuration byte
    uint8_t commValueType;                                     // defines type of byte
    uint8_t data;
    uint8_t delayFactorOn=0;                                     // the factor for the delay timer (on delay)
    uint8_t delayFactorOff=0;                                     // the factor for the delay timer (off delay)
    uint8_t delayActive;                                       // is timer active 1=yes
    uint8_t delayBase;
    uint8_t timerActive = eeprom_ParamRead(0x01EA);               // set bit value if delay on a channel is active
     
    // handle here only data with 6-bit length, maybe we have to add here more code to handle longer data
    /** @todo handle data with more then 6 bit */
    data = (hdr->apci) & 0x3F;
     
    for(i=0; i<countAssociations; i++)
    {
        numberInGroupAddress = eeprom_ParamRead(BASE_ADDRESS_OFFSET+assocTabPtr+1+(i*2));

        // check if valid group address reference
        if(numberInGroupAddress == 0xFE)
        {
            continue;
        }
        commObjectNumber = eeprom_ParamRead(BASE_ADDRESS_OFFSET+assocTabPtr+1+(i*2)+1);

        // now check if received address is equal with the safed group addresses, substract one
        // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
        // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
        if((destAddr == grp_addr.ga[numberInGroupAddress-1]) && (commObjectNumber <= 7))
        {
            // found group address

            // read communication object (3 Byte)
            commValuePointer = eeprom_ParamRead(BASE_ADDRESS_OFFSET+commStabPtr+2+(commObjectNumber*3));
            commConfigByte   = eeprom_ParamRead(BASE_ADDRESS_OFFSET+commStabPtr+2+(commObjectNumber*3+1));
            commValueType    = eeprom_ParamRead(BASE_ADDRESS_OFFSET+commStabPtr+2+(commObjectNumber*3+2));
            delayActive      = eeprom_ParamRead(0x01EA);
            // read delay factor for on and off
            delayFactorOn    = eeprom_ParamRead(0x01DA+commObjectNumber);
            delayFactorOff   = eeprom_ParamRead(0x01E2+commObjectNumber);

            // read delay base, 0=130ms, 1=260 and so on
            delayBase        = eeprom_ParamRead(0x01F9+((commObjectNumber+1)>>1));
            if((commObjectNumber & 0x01) == 0x01)
            {
                delayBase&=0x0F;
            }
            else
            {
                delayBase = (delayBase & 0xF0)>>4;
            }               

            /** @todo check if object is blocked and/or write is enabled */

            // reset saved timer settings
            // delayValues[commObjectNumber]=0;

            // we received a new state for object commObjectNumber
            // check if we have a delay on that port

            // check if we must switch off a port where timers are running
            if((!delayFactorOff) && (data == 0))
            {
                DEBUG_PUTC('K');
                delayValues[commObjectNumber] = 0;
                timerRunning &= ~(1<<commObjectNumber);
            }
               
            // check for delayed switch off
            if(portValue & (1<<commObjectNumber) && delayFactorOff && !(timerActive & (1<<commObjectNumber)) && (data==0))
            {
                // switch of but delayed
                delayValues[commObjectNumber] = (uint16_t)(1<<delayBase)*(uint16_t)delayFactorOff;
            }

            // check if we have a delayed switch on
            if(((portValue & (1<<commObjectNumber)) == 0x00) && delayFactorOn && (data == 1))
            {
                // switch on but delayed
                delayValues[commObjectNumber] = (uint16_t)(1<<delayBase) * (uint16_t)delayFactorOn;
            }
               
            // check if we have a timer function
            if(timerActive & (1<<commObjectNumber) && delayFactorOff && (data == 1))
            {
                // special case (switch on immediatly and off after a defined time
                DEBUG_PUTS("Fl");
                portValue |= (1<<commObjectNumber);
                switchObjects();
                delayValues[commObjectNumber] = (uint16_t)(1<<delayBase) * (uint16_t)delayFactorOff;
            }

            /** @todo check how to handle switch off if timer is currently active (0x01EB) */
            // check who to handle off telegramm while in timer modus
            if(timerActive & (1<<commObjectNumber) && delayFactorOff && (data == 0))
            {
                DEBUG_PUTS("TK");
                // only switch off if on 0x01EB the value is equal zero
                if(!(eeprom_ParamRead(0x01EB) & (1<<commObjectNumber)))
                {
                    delayValues[commObjectNumber] = 0;
                    timerRunning &= ~(1<<commObjectNumber);
                    portValue    &= ~(1<<commObjectNumber);
                }
            }
            DEBUG_PUTHEX(commObjectNumber);
               
            /** check for delays */
            if(delayValues[commObjectNumber])
            {
                // DEBUG_PUTC('T');
                // check if queue is empty and reset timer if it's the first timer which is running
                if(timerRunning == 0)
                {
                    // DEBUG_PUTC('R');
                    RESET_RELOAD_APPLICATION_TIMER();
                }
                else
                {
                    // another timer is running add current time to timer value
                    delayValues[commObjectNumber]+=currentTime;
                }
                timerRunning |= (1<<commObjectNumber);
            }
            else
            {
                // no delay is defined so we switch immediatly
                if(data == 0)
                {
                    // switch port off
                    portValue &= ~(1<<commObjectNumber);
                }
                else if(data == 1)
                {
                    portValue |= (1<<commObjectNumber);
                }
    
                //** @todo need to check here for respond
                // send response telegram to inform other devices that port was switched
                //sendRespondTelegram(i,(portValue & (1<<i))?1:0, 0x0C);
            }
        }
        else if((commObjectNumber > 7) && (commObjectNumber < 12))
        {
            // additinal function
            /** @todo write part additional functions */
//               DEBUG_PUTS("ZF");
//               DEBUG_NEWLINE();
        }
    }

    switchObjects();

    return FB_ACK;
}   /* runApplication() */

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

    DEBUG_PUTS("Sw");
    DEBUG_NEWLINE();

    /* change PWM to supply relais with full power */
    waitToPWM = PWM_DELAY_TIME;
    ENABLE_PWM(0xFF); //0xFF = 255 --> This is 100% negative duty cycle (active low)
	
    // check if timer is active on the commObjectNumber

    /* read saved status and check if it was changed */
    savedValue = eeprom_ParamRead(0x0000);
    if(savedValue != portValue)
    {
        // now check if last status must be saved, we write to eeprom only if necessary
        initialPortValue = ((uint16_t)eeprom_ParamRead(0x01F7) << 8) | ((uint16_t)eeprom_ParamRead(0x01F6));
        for(i=0; i<=7; i++)
        {
            if(((initialPortValue>>(i*2)) & 0x03) == 0x0)
            {
                eeprom_ParamWrite(0x0000, 1, &portValue);
                DEBUG_PUTS("Sv");
                break;
            }
        }
     }
     
    /* check 0x01F2 for opener or closer and modify data to relect that, then switch the port */
    portOperationMode = eeprom_ParamRead(0x01F2);
    switchPorts(portValue^portOperationMode);

    return;
}

/**                                                                       
* switch all of the output pins
*
* @param port
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

/**                                                                       
* The start point of the program, init all libraries, start the bus interface,
* the application and check the status of the program button.
*
* @return 
*   
*/
int main(void)
{
    /* disable wd after restart_app via watchdog */
    DISABLE_WATCHDOG()

    /* ROM-Check */
    /** @todo Funktion fuer CRC-Check bei PowerOn fehlt noch */

    /* init internal Message System */
    msg_queue_init();
    
	DEBUG_INIT();
     DEBUG_NEWLINE();
     DEBUG_PUTS("V0.1");
     DEBUG_NEWLINE();
       
    /* init procerssor register */
    fbhal_Init();

    /* enable interrupts */
    ENABLE_ALL_INTERRUPTS();

    /* init eeprom modul and RAM structure */ 
    eeprom_Init(&nodeParam[0], EEPROM_SIZE);

    /* init protocol layer */
    /* load default values */
    fbprot_Init(defaultParam);

    /* config application hardware */
    (void)restartApplication();


#ifdef HARDWARETEST
    sendTestTelegram();
#endif

    /***************************/
    /* the main loop / polling */
    /***************************/
    while(1)
    {
        /* Auswerten des Programmiertasters */
        if(fbhal_checkProgTaster())
		{
#ifdef SENDTESTTEL
			sendTestTelegram();
#endif
		}

        /* check if 130ms timer is ready */
        if(TIMER1_OVERRUN)
        {
            CLEAR_TIMER1_OVERRUN;
#ifndef HARDWARETEST
            timerOverflowFunction();
#else
    		//sendTestTelegram();
            hardwaretest();
#endif
        }

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
    {
        pinstate = 1;
    }
    return;
}
#endif

#endif /* _FB_RELAIS_APP_C */
/*********************************** EOF *********************************/
