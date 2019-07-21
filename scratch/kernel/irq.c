#include "../h/common.h"
#include "../h/util.h"
#include <message.h>
#include <std.h>
#include "common.h"
#include "irq.h"
#include "clock.h"
#include "tty.h"
#include "ata.h"

int cur_irq_num;	// used in wait_for_interrupt_num()

void irq_handler(int irq_num, int errcode)	// refer to 'interrupt()' and OSDI-p.99 in MINIX1
{
	message_t msg;
	int task, this_task_bit, tasks_to_check, nowait_tasks;
	message_t *task_msg[NUM_OF_TASKS];

	/* Send EOI(end of interrupt) to indicate that you have finished so that it can dispatch the next one */ 
	if (irq_num >= SLAVE_FIRST_INT)
		outb(0xA0, 0x20);	// send reset signal(EOI) to slave
	outb(0x20, 0x20);	// send reset signal(EOI) to master

	cur_irq_num = irq_num;
	/* create a message to a task */
	switch (irq_num)
	{
		case IRQ_CLOCK:
			msg.src_pid = PID_HARDWARE;
			msg.type = CLOCK_TICK;	
			task = PID_CLOCK;
			this_task_bit = 1 << PID_CLOCK;
			break;

		case IRQ_KEYBOARD:
			msg = keyboard();
			msg.src_pid = PID_HARDWARE;
			msg.type = TTY_CHAR_INT;
			task = PID_TTY;
			this_task_bit = 1 << PID_TTY;
			break;

		case IRQ_PRIMARY_ATA:
			msg.src_pid = PID_HARDWARE;
			msg.type = ATA_INT;	
			task = PID_ATA;
			this_task_bit = 1 << PID_ATA;
			break;
	}
debug_printf("irq_handler(): irq_num=%x\n", irq_num);

	/* task is not waiting for msg */
	if (kernel_send_msg(PID_HARDWARE, task, &msg) != 0)	// src for every IRQs should be HARDWARE
	{
		if (task == PID_CLOCK)
			lost_clock_ticks++;
		else
		{
			tasks_to_check = nowait_tasks;	// don't need to add 'this_task_bit' here since we know it's busy this time
			nowait_tasks |= this_task_bit;
			task_msg[task] = &msg;
		}	
		// stores msg temporaryily 
		
	}
	/* msg is successfully sent */
	else
	{
		nowait_tasks &= ~this_task_bit;
		tasks_to_check = nowait_tasks;	 
	}	

    	if ((ready_q_head[TASK_Q] != NULL || ready_q_head[SERVER_Q] != NULL || ready_q_head[INIT_USR_Q] != NULL) && cur_proc_id == PID_IDLE)
	{
        	pick_proc();
	}
debug_printf("cur_proc_id: %d\n", cur_proc_id);
}

