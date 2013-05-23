#ifndef _LED_H_
#define _LED_H_

/** 
* @todo add documentation
* 
*/
class CLed : public CTask
{
	public:	
		CLed ();

		/** @todo add documentation */
		void On () {PORTC &= ~_BV(PC1);};
		/** @todo add documentation */
		void Off () {PORTC |= _BV(PC1);};

	protected:
		/** @todo add documentation */
		void Execute (void);
};

extern CLed Led;

#endif //_LED_H_
