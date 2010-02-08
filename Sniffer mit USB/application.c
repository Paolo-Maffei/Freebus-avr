#include "main.h"


//http://bash-hackers.org/wiki/doku.php/scripting/terminalcodes
//http://pegasus.cs.csubak.edu/Tables_Charts/VT100_Escape_Codes.html


CApplication theApp;

CApplication::CApplication (void) : bRunApplication (true)
{
}


extern CMessage* pReceivedMessage;
extern CMessage* volatile pMessageToSend;

void CApplication::Init (void)
{
	//Start watchdog with 500ms timeout
	//wdt_enable (WDTO_500MS);

	InitUART ();


	///////////////////////
	//setup the interrupts
	
	//External interrupt 0
	EICRA = _BV(ISC01);  //The falling edge of INT0 generates an interrupt request
	EIMSK |= _BV(INT0); //Enable INT0-Interrupt


	//Init Port for Switch
	DDRB &= ~_BV(PB2);
	PORTB |= _BV(PB2);

	//Enable global interrupt
	sei ();

	
	//Show welcome message
	puts_P (PSTR ("\033[2J\033[H\033[34m    __________  ________________  __  _______"));
	puts_P (PSTR ("   / ____/ __ \\/ ____/ ____/ __ )/ / / / ___/"));
	puts_P (PSTR ("  / /_  / /_/ / __/ / __/ / __  / / / /\\__ \\"));
	puts_P (PSTR (" / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /"));
	puts_P (PSTR ("/_/   /_/ |_/_____/_____/_____/\\____//____/"));
	puts_P (PSTR ("\nFREEBUS-Sniffer V1.0\nCopyright (c) 2009 Thomas Weyhrauch <thomas@weyhrauch.de>\n"));


	/////////////
	DDRD |= 0x80;
	
	PORTC = 0x02;
	_delay_us (10);
	PORTC = 0x00;


	_delay_us (10);


	PORTC = 0x02;
	_delay_us (10);
	PORTC = 0x00;


	while (1);

	
	PORTD |= 0x80;
	_delay_us (10);
	PORTD &= ~0x80;


	_delay_us (10);


	PORTD |= 0x80;
	_delay_us (10);
	PORTD &= ~0x80;

	while (1);


};

void CApplication::OnShutdown (void)
{
}


void CApplication::Execute(void)
{

	if (pReceivedMessage->State == MSGSTATE_RECEIVECOMPLETE)
	{
		pReceivedMessage->Next = new CMessage ();
		pReceivedMessage = pReceivedMessage->Next;
			
	}	

	if (pMessageToSend != pReceivedMessage)
	{
		pMessageToSend->Dump (PINB & 0x04);

		CMessage* pTmp = pMessageToSend;
		pMessageToSend = pMessageToSend->Next;
		delete pTmp;
	}
}


