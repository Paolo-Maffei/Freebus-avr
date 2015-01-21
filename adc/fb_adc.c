/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2010 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/**
 * @file   fb_adc.c
 * @author Matthias Fechner
 * @date   Sun Aug 08 08:01:58 2010
 * 
 * @brief  Test program to use ADC to measure bus voltage
 * The ADC return a 10-bit value. Vin=(ADC * Vref) / 1024.
 * Example: If the ADC is 0x024D the value is 589. Vin=(589 * 5) / 1024=2,876V
 * We have a voltage seperator in place to measure 50V. It is 1M to 110k.
 * Vbus = Vadc * ( (R1+R2) / R2). Vbus = (ADC * Vref * (R1 + R2)) / (1024 * R1) = (589 * 5 * (1000k + 110k)) / (1024 * 110k) = 29,02V
 */
#ifndef _FB_ADC_C
#define _FB_ADC_C


/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb_adc.h"
#include <avr/sleep.h>

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
extern struct grp_addr_s grp_addr;

uint8_t nodeParam[PERSISTENT_MEMORY_SIZE];           /**< parameterstructure (RAM) */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM =
    {
        { SOFTWARE_VERSION_NUMBER, 0x01 },    /**< version number                               */
        { APPLICATION_RUN_STATUS,  0xFF },    /**< Run-Status (00=stop FF=run)                  */
        { COMMSTAB_ADDRESS,        0x9A },    /**< COMMSTAB Pointer                             */
        { APPLICATION_PROGRAMM,    0x00 },    /**< Port A Direction Bit Setting???              */

        { 0x0000,                  0x00 },    /**< default is off                               */
        { 0x01EA,                  0x00 },    /**< no timer active                              */
        { 0x01F6,                  0x55 },    /**< don't save status at power loss (number 1-4) */
        { 0x01F7,                  0x55 },    /**< don't save status at power loss (number 5-8) */
        { 0x01F2,                  0x00 },    /**< closer mode for all relais                   */

        { MANUFACTORER_ADR,        0x04 },    /**< Herstellercode 0x04 = Jung                   */
        { DEVICE_NUMBER_HIGH,      0x20 },    /**< device type (2038.10) 2060h                   */
        { DEVICE_NUMBER_LOW,       0x60 },    /**<                                              */

        { 0xFF,                    0xFF }     /**< END-sign; do not change                      */
    };


/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void configureAdc0(void);
inline void doAdcMeasurement(void);

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
    return FB_ACK;
}   /* runApplication() */

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
    DISABLE_WATCHDOG();

    /* ROM-Check */
    /** @todo Funktion fuer CRC-Check bei PowerOn fehlt noch */
    
    /* init internal Message System */
    msg_queue_init();
    
	DEBUG_INIT();
    DEBUG_NEWLINE_BLOCKING();
    DEBUG_PUTS_BLOCKING("V0.1");
    DEBUG_NEWLINE_BLOCKING();
       
    /* init procerssor register */
    fbhal_Init();

    /* enable interrupts */
    ENABLE_ALL_INTERRUPTS();

    /* init eeprom modul and RAM structure */ 
    freebus_memory_Init(&nodeParam[0], PERSISTENT_MEMORY_SIZE);

    /* init protocol layer */
    /* load default values */
    fbprot_Init(defaultParam);

    /* config application hardware */
    (void)restartApplication();

    configureAdc0();
    doAdcMeasurement();
    _delay_ms(2000);

    /***************************/
    /* the main loop / polling */
    /***************************/
    while(1) {
        _delay_ms(500);
        doAdcMeasurement();

        // go to sleep mode here
        //sleep_mode();
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

void configureAdc0(void)
{
    // set reference to Avcc, right adjusted result, channel 0
    ADMUX = (1<<REFS0);

    // disable DIO of pin
    DIDR0 |= (1<<ADC0D);

    // set ADC prescaler to 64, enable ADC interrupt, enable ADC
    ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADIE) | (1<<ADEN);
}

inline void doAdcMeasurement(void)
{
    ADCSRA |= (1<<ADSC);
}

ISR(ADC_vect)
{
    uint16_t value=ADCW;
    DEBUG_PUTS("ADC ");
    DEBUG_PUTHEX(value>>8);
    DEBUG_PUTHEX(value);
    DEBUG_NEWLINE();
}
#endif /* _FB_ADC_C */
/*********************************** EOF *********************************/
