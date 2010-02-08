#include "main.h"

static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);
static FILE* uart_str = fdevopen (uart_putchar, uart_getchar);


#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1)


#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif


static volatile uint8_t ui8RxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t ui8TxBuffer[UART_TX_BUFFER_SIZE];
static volatile uint8_t ui8RxBufferHead;
static volatile uint8_t ui8RxBufferReadPos;
static volatile uint8_t ui8TxBufferHead;
static volatile uint8_t ui8TxBufferReadPos;
static uint8_t tmp;

void InitUART (void)
{
	//Set stdin and stdout to our functions
	stdout = stdin = uart_str;

	// USART initialization
	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
	UCSR0C = _BV (USBS0) | _BV (UCSZ01) | _BV(UCSZ00);

	//Enable receiver, transmitter and the receive-interrupt
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);


	//Baudrate: 500000
	UCSR0A = (1<<U2X0);
	UBRR0H=0x00;
	UBRR0L=0x04;
}


static int uart_putchar (char c, FILE *stream)
{
	//Replace newline with carrige return
	if (c == '\n')
		c = '\r';

	tmp = (ui8TxBufferHead + 1) & UART_TX_BUFFER_MASK;
	while (tmp == ui8TxBufferReadPos); /* wait for free space in buffer */

	ui8TxBuffer[tmp] = c;
	ui8TxBufferHead = tmp;
	UCSR0B |= _BV(UDRIE0);
	return c;
}


static int uart_getchar (FILE *stream)
{
	while (ui8RxBufferReadPos == ui8RxBufferHead);

	ui8RxBufferReadPos = (ui8RxBufferReadPos +1 ) & UART_RX_BUFFER_MASK;
	return ui8RxBuffer[ui8RxBufferReadPos];
}


uint8_t uart_getchar ()
{
	ui8RxBufferReadPos = (ui8RxBufferReadPos +1 ) & UART_RX_BUFFER_MASK;
	return ui8RxBuffer[ui8RxBufferReadPos];
}


uint8_t Uart_IsDataAvailible ()
{
	return (ui8RxBufferReadPos != ui8RxBufferHead);
}


//Interrupthandler für Data register empty
ISR (USART_UDRE_vect)
{
	if (ui8TxBufferReadPos != ui8TxBufferHead)
	{
		ui8TxBufferReadPos = (ui8TxBufferReadPos +1) & UART_TX_BUFFER_MASK;
		UDR0 = ui8TxBuffer[ui8TxBufferReadPos];
	}
	else
		UCSR0B &= ~_BV(UDRIE0);
}


//Interrupthandler für Receive complete
ISR (USART_RX_vect)
{
	ui8RxBufferHead = (ui8RxBufferHead + 1) & UART_RX_BUFFER_MASK; //ui8BufferPos läuft nur im Bereich von 0 bis UART_RX_BUFFER_SIZE-1!
	ui8RxBuffer[ui8RxBufferHead] = UDR0;
}
