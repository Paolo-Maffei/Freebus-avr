#ifndef _FB_APP_C
#define _FB_APP_C

#include "test_uart.h"

uint8_t nodeParam[MEMORY_SIZE];           /**< parameterstructure (RAM) */
extern uint8_t userram[USERRAM_SIZE];

timer_t timer1;

void app_loop() {
    if (check_timeout(&timer1)){
        /* eine Sekunde um */
        alloc_timer(&timer1, 100);
        UART_PUTHEX(0xAB);
    }
}


uint8_t restartApplication(void) {
    UART_INIT();
    alloc_timer(&timer1, 100);
    return 1;
}



#endif /* _FB_APP_C */