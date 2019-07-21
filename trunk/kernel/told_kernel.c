#include "../h/message.h"
#include "../h/syscall.h"
#include <stdint.h>
#include "told_kernel.h"
#include "common.h"
#include "irq.h"

static message_t msg;

static int kernel_copy(message_t *mp)
{
debug_printf("kernel_copy: 0x%x, 0x%x, 0x%x\n", ((uint32_t *)(mp->buf))[0], ((uint32_t *)(mp->buf))[1], ((uint32_t *)(mp->buf))[2]);
	phy_copy(((uint32_t *)(mp->buf))[0], ((uint32_t *)(mp->buf))[1], ((uint32_t *)(mp->buf))[2]);

	return 0;
}

static int kernel_fork(message_t *mp)
{
	int parent_pid = ((int *)(mp->buf))[0];
	int child_pid = ((int *)(mp->buf))[1];
	proc_t *child_p;
	char *parent_cp, *child_cp;
	int size_proc_t;

debug_printf("kernel_fork: parent_pid=%d, child_pid=%d", parent_pid, child_pid);
	// clone parent's process structure to child
	proc_table[child_pid].eip = proc_table[parent_pid].eip;
	proc_table[child_pid].esp = proc_table[parent_pid].esp;
	proc_table[child_pid].esplimit = proc_table[parent_pid].esplimit; 
	proc_table[child_pid].map[TEXT].mem_size = proc_table[parent_pid].map[TEXT].mem_size;
	
	proc_table[child_pid].map[DATA].offset = proc_table[parent_pid].map[DATA].offset;
	proc_table[child_pid].map[DATA].phy_address = proc_table[child_pid].map[TEXT].phy_address + proc_table[child_pid].map[TEXT].mem_size;
	proc_table[child_pid].map[DATA].mem_size = proc_table[parent_pid].map[DATA].mem_size;
	
	proc_table[child_pid].map[STACK].offset = NA;
	proc_table[child_pid].map[STACK].phy_address = NA;
	proc_table[child_pid].map[STACK].mem_size = NA;
	//proc_table[child_pid].flags |= NO_MAP;
	// it means it's not ready to execute
	//ready(&proc_table[child_pid]);
	proc_table[child_pid].flags |= RECEIVING;	// it waits for msg to wake it up from MM

	return 0;
}

int told_kernel_task()
{
	int status;
	while (1)
	{
		receive(ANY_PROC, &msg);
		switch (msg.type)
		{
			case KERNEL_COPY:
				status = kernel_copy(&msg);
				break;
			case KERNEL_FORK:
				status = kernel_fork(&msg);
				break;
		}
		msg.type = status;	
		send(msg.src_pid, &msg); 	
	}
}


