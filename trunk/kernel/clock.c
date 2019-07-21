#include "../h/message.h"
#include "../h/syscall.h"
#include "clock.h"
#include "common.h"
#include "irq.h"

static message_t msg;
int lost_clock_ticks;
long realtime;

static void cpu_accounting()
{
	bill_proc_ptr->spent_cpu_time++;
debug_printf("process: %x spent cpu time for %d\n", bill_proc_ptr, bill_proc_ptr->spent_cpu_time);
}

static void do_clock_tick()
{
	int t;
	/* To guard against race conditions, first copy 'lost_ticks' to a local
   	 * variable, add this to 'realtime', and then subtract it from 'lost_ticks'.
   	 */
	t = lost_clock_ticks;	// lost_clock_ticks missed interrupts
	realtime += t + 1;	// update realtime
	lost_clock_ticks -= t;	// these interrupts are no longer missed 

	// charge clock ticks to process
	cpu_accounting();
}

int clock_task()
{
	int opcode;
	while (1)
	{
		receive(ANY_PROC, &msg);	// wait for a clock interrupt
		opcode = msg.type;
		switch (opcode)
		{
			case CLOCK_TICK: 
debug_printf("clock msg received: msg.src_pid=0x%x, msg.type=0x%x\n", msg.src_pid, msg.type);
				do_clock_tick();
				break;
		}
				
	}

	return 0;
}

