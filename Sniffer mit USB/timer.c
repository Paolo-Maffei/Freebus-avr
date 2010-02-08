#include "main.h"

static CTimer *timer_vector[MAX_NUM_TIMERS];
static uint8_t timer_vector_cnt;

CTimer::CTimer(void) : flag (false)
{
	if (timer_vector_cnt == 0)
	{
		//initialize timer for Timer objects
		TCCR1A = _BV(WGM01); //CTC-Mode
		OCR1A = 250;  //Comparewert
		TIMSK1 |= _BV(OCIE0A);
		TCCR1B=0x03; //Start timer 
	}

	timer_vector[timer_vector_cnt++] = this;
}

void CTimer::SetTime(uint16_t time)
{
	TIMSK1 &= ~_BV(OCIE1A);
	cnt = time;

	flag = 0;

	TIMSK1 |= _BV(OCIE1A);
}


ISR(TIMER1_COMPA_vect)
{
	for (uint8_t i=0; i<timer_vector_cnt; i++)
	{
		if (timer_vector[i]->cnt)
		{
			if (--timer_vector[i]->cnt == 0)
				timer_vector[i]->flag = -1;
		}
	}
}
