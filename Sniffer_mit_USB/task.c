
#include "task.h"

static uint8_t ui8TaskCount;
static CTask* TaskList [MAX_NUM_TASKS];


CTask::CTask (void)
{
	if (ui8TaskCount < MAX_NUM_TASKS)
		TaskList[ui8TaskCount++] = this;
}

void CTask::Run (void)
{
	static uint8_t task = 0;

	TaskList[task++]->Execute();
	if (task == ui8TaskCount)
		task = 0;
}
