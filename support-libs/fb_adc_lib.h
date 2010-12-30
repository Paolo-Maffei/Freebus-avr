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
* @file   fb_adc_lib.h
* @author Kent Filek
* @date   Sat Aug 28 21:40:00
* 
* @brief  library functions for ADC usage
*/
#ifndef _FB_ADC_LIB_H
#define _FB_ADC_LIB_H


/*************************************************************************
* INCLUDES
*************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include <avr/io.h>


/**************************************************************************
* DEFINITIONS
**************************************************************************/

/* Mode Values for Init */
#define ADC_REF_AREF (0)                        /**< Voltage Reference ARef Pin           */
#define ADC_REF_AVCC (1<<REFS0)                 /**< Voltage Reference AVcc               */
#define ADC_REF_INT  ((1<<REFS0)|(1<<REFS1))    /**< Voltage Reference Internal 1.1V      */

#define ADC_ADJUST_LEFT     (1<<ADLAR)          /**< Left adjust 10bits result in 16bits register */
#define ADC_ADJUST_RIGHT    (0)                 /**< Right adjust 10bits result in 16bits register */

#define ADC_MUX_0           (0)                 /**< ADC Channel 0 */
#define ADC_MUX_1           (1)                 /**< ADC Channel 1 */
#define ADC_MUX_2           (2)                 /**< ADC Channel 2 */
#define ADC_MUX_3           (3)                 /**< ADC Channel 3 */
#define ADC_MUX_4           (4)                 /**< ADC Channel 4 */
#define ADC_MUX_5           (5)                 /**< ADC Channel 5 */
#define ADC_MUX_6           (6)                 /**< ADC Channel 6 */
#define ADC_MUX_7           (7)                 /**< ADC Channel 7 */
#define ADC_MUX_REF         (0x0e)              /**< Internal 1.1V reference */
#define ADC_MUX_GND         (0x0f)              /**< 0V */

/**************************************************************************
* DECLARATIONS
**************************************************************************/



/*************************************************************************
* FUNCTION PROTOTYPES
**************************************************************************/

void        adc_Init(uint8_t nMode);
uint16_t    adc_ReadValue(void);
void        adc_SetChannel(uint8_t nChannel);


#endif /* _FB_ADC_LIB_H */
/*********************************** EOF *********************************/
