#ifndef _LED_H_
#define _LED_H_


class CLed : public CTask
{
	public:	
		CLed ();

		void On () {PORTC &= ~_BV(PC1);};
		void Off () {PORTC |= _BV(PC1);};

	protected:
		void Execute (void);
};

extern CLed Led;

#endif //_LED_H_
