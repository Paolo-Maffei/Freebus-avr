#include "main.h"

enum TXSTATE
{
	TXISFREE,
	TXCHECKBUSFREE53,
	TXCHECKBUSFREE50,
	TXCHECKBUSFREE150,
	TXSTART,
	TXCHECKFORCOLLISION1,
	TXCHECKFORCOLLISION2,
	TXBITFINISHED,
	TXWAITFORACK,
	RXRECEIVECHAR,
	RXWAITFORNEXTOCTET
};



void ReloadTimer0 (uint8_t value, uint8_t tccr)
{
	TCCR0B &= ~0x07;      //stop timer
	TCNT0 = value;        //set start of timer, run up to 0x00
	TIMSK0 |= (1<<TOIE0); //enable overflow interrupt
	TCCR0B |= tccr;       //set mode and re-enable timer
}

uint16_t ui16Octet;
uint8_t ui8OctetPos;
uint8_t ui8TxState = TXISFREE;
CMessage* pReceivedMessage = new CMessage ();
CMessage* volatile pMessageToSend = pReceivedMessage;

ISR (INT0_vect)
{
	switch (ui8TxState)
	{
		case TXISFREE:
			//Startbit of a new message received
			
		case RXWAITFORNEXTOCTET:
			//Startbit of an octet received, start scanning each bit
			ui8TxState = RXRECEIVECHAR;
			ui8OctetPos = 0;
			ui16Octet = 0;
		
		case RXRECEIVECHAR:
		{   //we received a '0' -> resync timer
			ReloadTimer0 (0xff, 0x01);	//Scan immediately
			break;			
		}
	}
}

ISR (TIMER0_OVF_vect)
{
	switch (ui8TxState)
	{
		/////////////////////////////////////////////////////////////////////////////////////
		//functions for receiving
		case RXRECEIVECHAR:
		{

			if (PIND & 0x04) //liegt eine "1" an, dann speichern
				ui16Octet |= 0x0400;

	
			if (ui8OctetPos < 10)
			{
				ui16Octet >>=1;
				ui8OctetPos++;
				ReloadTimer0 (224, 3);	//104µs
			}
			else
			{
				ReloadTimer0(158, 0x03); //312µs -> Timeout for next octet
				ui8TxState = RXWAITFORNEXTOCTET;
				pReceivedMessage->SetOctet (pReceivedMessage->Len, ui16Octet);
				pReceivedMessage->Len++;
			}

			break;
		}

		case RXWAITFORNEXTOCTET:
		{   //no more bit received
			TCCR0B &= ~0x07;      //stop timer
	
			ui8TxState = TXISFREE;
			pReceivedMessage->State = MSGSTATE_RECEIVECOMPLETE;
			break;
		}
	}
}

