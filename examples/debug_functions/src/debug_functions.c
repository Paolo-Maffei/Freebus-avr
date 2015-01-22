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

void app_loop() {
	// this function is called regularly from the lib
	// Is button pressed?
	/*
	if (ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) {
		// Yes, so turn LED on.
		ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);
	} else {
		// No, so turn LED off.
		ioport_set_pin_level(LED_0_PIN, !LED_0_ACTIVE);
	}
	*/
	delay_ms(10);
}

uint8_t restartApplication(void) {
	delay_init(sysclk_get_cpu_hz());
	DEBUG_PUTS("Test Application started");
	DEBUG_NEWLINE();
	uint32_t flashSize=flashcalw_get_flash_size();
	DEBUG_PUTS("F_SIZE: ");
	DEBUG_PUTHEX32(flashSize);
	DEBUG_NEWLINE();

	uint32_t pageCount=flashcalw_get_page_count();
	DEBUG_PUTS("Page_COUNT: ");
	DEBUG_PUTHEX32(pageCount);
	DEBUG_NEWLINE();

	delay_ms(1000);
	return 1;
}

#endif /* _FB_EXAMPLE_DEBUG */