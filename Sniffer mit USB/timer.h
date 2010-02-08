
#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/interrupt.h>

#define MAX_NUM_TIMERS 10

ISR(TIMER1_COMPA_vect);

/** 
* @todo add documentation
* 
*/
class CTimer
{
	friend void TIMER1_COMPA_vect (void);
	private:
		uint16_t cnt;
		bool flag;

	public:
		CTimer(void);
		
          /** @todo add documentation */
		/*inline*/bool IsFlagged(void) {return flag;};
		void SetTime(uint16_t time); ///< @todo add documentation
};

#endif //_TIMER_H_
