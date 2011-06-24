/* $Id: fb_relais_app.c 2132 2010-08-05 19:30:16Z do13 $ */
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
 *
 * To enable IO test compile with -DIO_TEST
 */
#ifndef _FB_RELAIS_APP_C
#define _FB_RELAIS_APP_C


/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include <util/delay.h>
#include <avr/boot.h>
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_eeprom.h"
#include "msg_queue.h"
//#include "1wire.h"
#include "fb_hal.h"
#include "fb_prot.h"
#include "fbrf_hal.h"
#include "fb_app.h"
#include "rf22.h"
#include "Spi.h"
#include "fb_bootloader.h"
#ifdef IO_TEST

#endif

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/
enum spm_state {
    SPM_NOP = 0x00,
    SPM_ERASE,
    SPM_WRITE,
    SPM_WREADY,
};

#define pApplication()     asm volatile ("call 0x00000"::)

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/

/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/

/**************************************************************************
 * IMPLEMENTATION
 **************************************************************************/
 
static  enum spm_state spmstatus;
static  uint16_t addr;

/**
 * Function is called when bootloading is finished.
 * That is when there aren't any write commands received for a couple of seconds.
 *
 */
void jumpToApplication(void)
{
    // Reenable RWW-section again. We need this if we want to jump back
    // to the application after bootloading.
    boot_rww_enable_safe();
    // reset the hardware by software, as we have no hardware reset
    cli();
    PROG_PORT &= ~(1<<PROG_PIN); //led on
    DDRB = 0;
    DDRD = 0;
    PORTB = 0;
    PORTD = 0;
    PROG_DDR |= (1<<PROG_PIN);  //prog led as output
    PROG_PORT &= ~(1<<PROG_PIN); //led on
    DISABLE_RX_INT()
    STOP_EIB_TIMER()
    TCCR1B = 0; // Timer 1 off
    TIFR1 = 1<<TOV1; // clear Tiner 1 overflow

    /* Enable change of Interrupt Vectors */
    MCUCR = (1<<IVCE);
    /* Move interrupts to normal section */
    MCUCR = 0;
    MCUSR = 0;

    pApplication();
}


/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return always 1
 */
uint8_t restartApplication(void)
{
    return 1;
} /* restartApplication() */

/**
 * A_Memory_Write telegrams are handled here. Program the application flash.
 *   
 */

uint8_t runApplication(struct msg *rxmsg)
{
    if (spmstatus != SPM_NOP) return FB_NACK;
    struct fbus_hdr * old = (struct fbus_hdr *) rxmsg->data;
    uint8_t len  = (old->apci & 0x0f) ;
    addr = (rxmsg->data[8] << 8) + rxmsg->data[9] - 0x1000;
    /* addr and len both must be even */
    if (( addr & 0x0001 ) || ( len & 0x01)) return FB_NACK;
    len /= 2;
    uint16_t myaddr = addr;
    uint16_t *pw;
    pw = (uint16_t*) &rxmsg->data[10];
    while (len--){
      boot_page_fill (myaddr, *pw);
      myaddr += 2;
      pw   += 1;
    }
    /* start programming when last address is at end of a page */
    if (( myaddr & 0x007f ) == 0 )  spmstatus = SPM_ERASE;
    return FB_ACK;
}
    
void  spm_handler(void)
{
    if ( spmstatus == SPM_NOP ) return;
    if ( SPMCSR & 1 ) return;    // programming still in progress, do something else.
    DISABLE_IRQS
    
    switch ( spmstatus )
    {
      case SPM_ERASE:
        PROG_PORT &= ~(1<<PROG_PIN); //led on
        boot_page_erase (addr);                 // Clear flash page
        spmstatus = SPM_WRITE;
        break;
      
      case SPM_WRITE:
        boot_page_write (addr);                 // Store buffer in flash page.
        spmstatus = SPM_WREADY;
        break;
      
      case SPM_WREADY:
         boot_rww_enable();
         spmstatus = SPM_NOP;
         PROG_PORT |= (1<<PROG_PIN); //led off

         break;
      default:
         break;
    }
    ENABLE_IRQS
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
    static volatile uint8_t addr0 = 0;

    /* check if we came here through a watchdog reset
       in tis case jump to application directly, skip bootloader*/
    if ( MCUSR & (1<<WDRF)) jumpToApplication();

    /* disable wd after restart_app via watchdog */
    DISABLE_WATCHDOG()

    DDRB=0;
    DDRC=0;
    DDRD=0;
    PORTB=0;
    PORTC=0;
    PORTD=0;

    /* Enable change of Interrupt Vectors */
    MCUCR = (1<<IVCE);
    /* Move interrupts to Boot Flash section */
    MCUCR = (1<<IVSEL);


    /* init internal Message System */
    msg_queue_init();

    DEBUG_INIT();
    DEBUG_NEWLINE_BLOCKING();
    DEBUG_PUTS_BLOCKING("V0.1");
    DEBUG_NEWLINE_BLOCKING();
       
    /* reset timers, they comes running from the application */
    TCCR1A = 0;
    TCCR1B = 1<<CS12 | 1<<CS00; // Timer 1 prescaler 1024, overflow after 6.7 sec.
    TCNT1  = 0;
    TIMSK1 = 0;                                 // disable timer 1 interrupts
    TIFR1  = 1<<TOV1 | 1<<OCF1A | 1<<OCF1B | 1<<ICF1;  // clear timer 1 flags

    TCCR2B = 0;
    TIMSK2 = 0;
    TIFR2  = 1<<TOV2 | 1<<OCF2A | 1<<OCF2B;

    /* init procerssor register */
    fbhal_Init();

#ifdef FB_RF
    fbrfhal_init();
#else
#ifdef EXT_CPU_CLOCK
    SpiInit();
    rf22_init();
#endif
#endif

    PROG_DDR |= (1<<PROG_PIN);  //prog led as output
    PROG_PORT |= (1<<PROG_PIN); //prog led off
    DDRC = 0x3F;
    /* enable interrupts */
    ENABLE_ALL_INTERRUPTS();

    /* init protocol layer */
    /* load default values */
    fbprot_Init(NULL);

    /* config application hardware */
    (void)restartApplication();

    /***************************/
    /* the main loop / polling */
    /***************************/
    while(1) {
        /* Auswerten des Programmiertasters */
//        if(fbhal_checkProgTaster()) {
//        }
        fbprot_msg_handler();
        spm_handler();
        // check for timeout, in case jump to application
        if (TIFR1 & (1<<TOV1)){
            /* check if there is a jump at address 0 of flash.
            if not, stay in the bootloader.
            DO NOT SAY pgm_read_byte(0), because that would optimized away */
            if ( pgm_read_byte(addr0) == 0x0c )
               jumpToApplication();
            TIFR1 = 1<<TOV1; // clear Timer 1 overflow
        }
#ifdef FB_RF
        fbrfhal_polling();
#endif
    }   /* while(1) */

}   /* main() */

#endif /* _FB_RELAIS_APP_C */
/*********************************** EOF *********************************/
