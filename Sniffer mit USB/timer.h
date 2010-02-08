
#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/interrupt.h>

#define MAX_NUM_TIMERS 10

ISR(TIMER1_COMPA_vect);

class CTimer
{
	friend void TIMER1_COMPA_vect (void);
	private:
		uint16_t cnt;
		bool flag;

	public:
		CTimer(void);
		
		/*inline*/bool IsFlagged(void) {return flag;};
		void SetTime(uint16_t time);
};

#endif //_TIMER_H_
