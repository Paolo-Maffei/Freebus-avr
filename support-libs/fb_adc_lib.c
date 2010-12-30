/*  */
/*
*      __________  ________________  __  _______
*     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
*    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
*   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
*  /_/   /_/ |_/_____/_____/_____/\____//____/  
*                                      
*  Copyright (c) 2010 Kent Filek <kent@filek.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/
/**
* @file   fb_adc_lib.c
* @author Kent Filek
* @date   Sat Aug 28 21:40:00
* 
* @brief  library functions for ADC usage
*/
#ifndef _FB_ADC_LIB_C
#define _FB_ADC_LIB_C


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_hal.h"
#include "fb_adc_lib.h"


/**************************************************************************
* DEFINITIONS
**************************************************************************/

#define ADC_PORT     PORTC      /**< output port (byte) of ADC   */
#define ADC_DDR      DDRC       /**< data direction byte of ADC */


/**************************************************************************
* DECLARATIONS
**************************************************************************/


/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

/*************************************************************************
* GLOBALS
**************************************************************************/

uint16_t adc_value = 0;

/**************************************************************************
* IMPLEMENTATION
**************************************************************************/

void adc_Init(uint8_t nMode)
{

    PRR = PRR & ~PRADC; // disable power reduction

    ADMUX = nMode;

    adc_SetChannel(nMode);

    // control register B
    ADCSRB =   (0x00)        // free run ADC
             | (0)           // disable analog comparator mode
             ;

    // control register A
    ADCSRA =   (1<<ADEN)     // enable ADC
             | (1<<ADSC)     // start ADC
             | (1<<ADATE)    // auto trigger enable
             | (1<<ADIE)     // ADC Interrupt enable
             | (0x07)        // clk/128 -> appr. 60kHz -> enough
             ;
}

void adc_SetChannel(uint8_t nChannel)
{
    nChannel &= 0x07;

    // set multiplexer
    ADMUX |= (ADMUX & ~0x07) | nChannel;
    
    // disable digital input buffer to save power
    DIDR0 = (1<<nChannel);

    // set port to input and disable pull-up
    ADC_DDR  = ADC_DDR & ~(1<<nChannel);
    ADC_PORT = ADC_PORT & ~(1<<nChannel);
}

uint16_t adc_ReadValue()
{
    return adc_value;
}

ISR(ADC_vect)
{
    adc_value = ADCW;
    
/*
            DEBUG_PUTS_BLOCKING("ADC ");
            DEBUG_PUTHEX_BLOCKING(adc_value>>8);
            DEBUG_PUTHEX_BLOCKING(adc_value);
            DEBUG_PUTC_BLOCKING(' ');
*/
}

#endif /* _FB_ADC_LIB_C */
/*********************************** EOF *********************************/
