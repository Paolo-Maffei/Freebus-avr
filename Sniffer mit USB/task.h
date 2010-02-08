
#ifndef _TASK_H_
#define _TASK_H_

#include "inttypes.h"

#define MAX_NUM_TASKS 5

class CTask
{
	public:
		CTask(void);
		virtual void Execute(void) = 0;
		static void Run (void);
};

//Makros zur Ablaufsteuerung
#define THREAD_BEGIN()		 \
  static uint8_t __id; \
  switch (__id)				 \
  {							 \
  	  case 0:				 \



#define THREAD_END()	\
	__id = -1;				\
	}					\

#define THREAD_WAIT_WHILE(cond)		\
  __id = __LINE__; case __LINE__:	\
    if (cond) return;	\

#define THREAD_WAIT_UNTIL(cond)  THREAD_WAIT_WHILE(!(cond))

#define THREAD_RESTART() {__id = 0; return;}

#define THREAD_YIELD()	\
  __id = __LINE__; return; case __LINE__:

#define THREAD_EXIT()  \
	__id = -1; return;

#endif //_TASK_H_
