#include "../h/const.h"

extern int clock_task(), mem_task(), floppy_task(), dummy_task();

/* NR_TASK is -8 to -1. Refer to /h/com.h, and mm, fs, init. In other words, four 0's are for -1(task), mm, fs and init */
int (*task[NR_TASKS + NR_SERVER_INIT_PROC])() = {
	dummy_task, tty_task, dummy_task, floppy_task, mem_task,
	clock_task, dummy_task, 0, 0, 0, 0 
};

