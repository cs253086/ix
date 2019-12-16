#include <message.h>
#include <std.h>
#include <stdint.h>
#include "sys.h"
#include "common.h"
#include "irq.h"

static message_t msg;

static int kernel_copy(message_t *mp)
{
debug_printf("kernel_copy: 0x%x, 0x%x, 0x%x\n", mp->uinteger[SRC_BASE_IDX_IN_MESSAGE], mp->uinteger[DST_BASE_IDX_IN_MESSAGE], mp->uinteger[SIZE_IDX_IN_MESSAGE]);
	phy_copy(mp->uinteger[SRC_BASE_IDX_IN_MESSAGE], mp->uinteger[DST_BASE_IDX_IN_MESSAGE], mp->uinteger[SIZE_IDX_IN_MESSAGE]);

	return 0;
}

static int kernel_fork(message_t *mp)
{
	int parent_pid = mp->integer[PARENT_PID_IDX_IN_MESSAGE];
	int child_pid = mp->integer[CHILD_PID_IDX_IN_MESSAGE];
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
	proc_table[child_pid].messbuf = proc_table[parent_pid].messbuf;
	//proc_table[child_pid].flags |= NO_MAP;
	// it means it's not ready to execute
	//ready(&proc_table[child_pid]);
	proc_table[child_pid].flags |= RECEIVING;	// it waits for msg from MM to wake it up 

	return 0;
}

static int kernel_exec(message_t *mp)
{
	int target_pid = mp->integer[TARGET_PID_IDX_IN_MESSAGE];

	proc_t *target_ptr = &proc_table[target_pid];
	// reset process table members	
	target_ptr->eip = PROCESS_STARTING_VIR_ADDRESS;	

	target_ptr->esp = PROCESS_STARTING_VIR_ADDRESS + (target_ptr->end_phy_address - target_ptr->begin_phy_address);

	target_ptr->flags &= ~RECEIVING;
	if (target_ptr->flags == 0)
		ready(target_ptr);
}

static int kernel_exit(message_t *mp)
{
	int parent_pid = mp->integer[PARENT_PID_IDX_IN_MESSAGE];
	int child_pid = mp->integer[CHILD_PID_IDX_IN_MESSAGE];
	proc_t *child_p;
	char *parent_cp, *child_cp;
	int size_proc_t;

debug_printf("kernel_exit: parent_pid=%d, child_pid=%d", parent_pid, child_pid);
	// reset the process table for exited process
	proc_table[child_pid].eip = 0;
	proc_table[child_pid].esp = PROCESS_STARTING_VIR_ADDRESS + (proc_table[child_pid].end_phy_address - proc_table[child_pid].begin_phy_address);
	proc_table[child_pid].esplimit = proc_table[child_pid].esp - USER_STACK_SIZE;       
	proc_table[child_pid].map[TEXT].mem_size = 0;
	
	proc_table[child_pid].map[DATA].offset = 0;
	proc_table[child_pid].map[DATA].phy_address = 0;
	proc_table[child_pid].map[DATA].mem_size = 0;
	
	proc_table[child_pid].map[STACK].offset = 0;
	proc_table[child_pid].map[STACK].phy_address = 0;
	proc_table[child_pid].map[STACK].mem_size = 0;
	proc_table[child_pid].messbuf = NULL;
	//proc_table[child_pid].flags |= NO_MAP;
	// it means it's not ready to execute
	//ready(&proc_table[child_pid]);
	proc_table[child_pid].flags &= ~RECEIVING;	// it waits for msg to wake it up from MM
	block(&proc_table[child_pid]);

	return 0;
}

int sys_task()
{
	int status;
	while (1)
	{
		receive(ANY_PROC, &msg);
		switch (msg.type)
		{
			case SYS_COPY:
				status = kernel_copy(&msg);
				break;
			case SYS_FORK:
				status = kernel_fork(&msg);
				break;
			case SYS_EXEC:
				status = kernel_exec(&msg);
				break;
			case SYS_EXIT:
				status = kernel_exit(&msg);
				break;
		}
		msg.integer[0] = status;	
		send(msg.src_pid, &msg); 	
	}

	return 0;
}


