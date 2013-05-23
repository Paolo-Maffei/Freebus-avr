
#ifndef _UART_H_
#define _UART_H_

#include "inttypes.h"

#define UART_RX_BUFFER_SIZE 8
#define UART_TX_BUFFER_SIZE 64

void InitUART (void);
uint8_t Uart_IsDataAvailible (void);
uint8_t uart_getchar (void);

#endif //_UART_H_
