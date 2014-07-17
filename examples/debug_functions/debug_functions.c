#ifndef _FB_EXAMPLE_DEBUG
#define _FB_EXAMPLE_DEBUG

/*
 * debug_functions.c
 *
 * Created: 16.07.2014 18:02:17
 *  Author: idefix
 *
 * Make sure to define compile switch DEBUG_UART.
 */

#include "debug_functions.h"

uint8_t nodeParam[EEPROM_SIZE];
extern uint8_t userram[USERRAM_SIZE];

void app_loop() {
	// this function is called regularly from the lib
	// Is button pressed?
	if (ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) {
		// Yes, so turn LED on.
		ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);
	} else {
		// No, so turn LED off.
		ioport_set_pin_level(LED_0_PIN, !LED_0_ACTIVE);
	}

}

uint8_t restartApplication(void) {
	DEBUG_PUTS("Test Application started");
	DEBUG_NEWLINE();
	return 1;
}

#endif /* _FB_EXAMPLE_DEBUG */