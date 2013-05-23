#include "main.h"

CLed Led;
CTimer timer1;

CLed::CLed (void)
{
	DDRC |= _BV(PC1);
}


void CLed::Execute (void)
{
	THREAD_BEGIN ();
	timer1.SetTime(500);

	THREAD_WAIT_UNTIL (timer1.IsFlagged());
	PORTC |= _BV(PC1);
	timer1.SetTime(500);

	THREAD_WAIT_UNTIL (timer1.IsFlagged());
	PORTC &= ~_BV(PC1);
	THREAD_RESTART ();
	THREAD_END ();
}
