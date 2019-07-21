#include <stdio.h>
#include <stdint.h>
#include "../h/errno.h"
#include "../h/common.h"
#include "../h/tell_kernel.h"
#include "../h/syscall_num.h"
#include "common.h"
#include "alloc.h"

#define FIRST_USER_PID	(PID_INIT + 1)

int mm_fork()
{
debug_printf("mm_fork(): started\n");
	int parent_pid = msg_src_pid, child_pid, ret;
	uint32_t total_size_of_parent;
	uint32_t parent_base, child_base;
	mm_proc_t *child_mm_proc;
	// allocate memory for child
	if (procs_in_use == MAXIMUM_PROCS)
		return EAGAIN;
	child_mm_proc = allocate_process_hole();
	//if ((child_mm_proc = allocate_process_hole()) == NULL)
		//return EAGAIN;
	
	// clone the parent in physical address
	/*why can't we  copy the memory block in MM, not only in kernel?
	1. copying memory is too critical to have it in MM.  
	why told_kernel task deals with it?
	1. because kernel_copy is a systemc call for MM or FS, we need to have a layer to commnicate them with kernel
	*/
	parent_base = mm_proc_table[parent_pid].begin_phy_address;
	child_base = child_mm_proc->begin_phy_address;
	total_size_of_parent = mm_proc_table[parent_pid].end_phy_address - mm_proc_table[parent_pid].begin_phy_address;
	tell_kernel_copy(parent_base, child_base, total_size_of_parent);	

	// assign proper proc table fields to child proc
	child_pid = child_mm_proc->pid;
	mm_proc_table[child_pid].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[child_pid].parent_pid = parent_pid;

	// assign child pid to the parent process
	mm_proc_table[parent_pid].child_pid = child_pid;
	
	// report it to kernel and FS so that kernel and FS can change proc tables accordingly
	ret = tell_kernel_fork(parent_pid, child_pid);
	//tell_fs(FORK, parent_pid, child_pid, 0);

	// reply to child to wake it up and ready to run: this is the reply to fork()
	msg.type = FORK_REPLY_TO_CHILD;
	msg.integer = 0;
	send(child_pid, &msg);
	return child_pid;
	
}

int mm_wait()
{
	int parent_pid = msg_src_pid;
	if (mm_proc_table[parent_pid].child_pid != 0)
	{
		mm_proc_table[parent_pid].flags != WAITING;
		dont_reply = 1;
		return mm_proc_table[parent_pid].child_pid;
	}
	else
		return 0;
}

int mm_exit()
{

}
