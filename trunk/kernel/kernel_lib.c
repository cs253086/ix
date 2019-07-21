#include <stdio.h>
#include "../h/errno.h"
#include "../h/common.h"
#include "common.h"
#include "kernel_lib.h"
#include "paging.h"

void debug_print_ready_queue()
{
	proc_t *q_head;
	proc_t *tmp_proc;

	q_head = ready_q_head[TASK_Q];	
	if (q_head == NULL)
		debug_printf("Queue is empty\n");
	else
	{
		tmp_proc = ready_q_head[TASK_Q];			
		debug_printf("Queue: ");
		while (tmp_proc != NULL)
		{
			debug_printf("%x->", tmp_proc);
			tmp_proc = tmp_proc->nextready;
		}
		debug_printf("NULL\n");
	}
}

uint32_t vir_to_phy_address(uint32_t vir_address, int pid)
{
	if (pid == PID_HARDWARE || pid < PID_MM)
		return (proc_table[pid].begin_phy_address + vir_address);
	else if (PID_IDLE < pid && pid < PID_HARDWARE) 
		return (proc_table[pid].begin_phy_address + (vir_address - (KERNEL_MAX_IN_PAGE_SIZE * PAGE_SIZE)));
	else
		return 0;
}

/*
 * every system call goes through this function in kernel
*/
void system_call(int function, int caller_pid, int src_dst, message_t *msg)
{
	int ret;
debug_printf("system_call(): function#=%x\n", function);
	msg->src_pid = caller_pid;
debug_printf("system_call(): msg-src_pid: %d\n", msg->src_pid);

	if (function & SEND)
		ret = kernel_send_msg(caller_pid, src_dst, msg);

	if (function & RECEIVE)
		ret = kernel_receive_msg(caller_pid, src_dst, msg);

	proc_table[caller_pid].eax = ret; // it is the return value of send, receive, send_receive	
}

void ready(proc_t *proc_p)
{
	/* Why needs to disable interrupts? it means this function is atomic
	 * , meaning that it is never interrupted since it has critical access somehow 
	 * You don't want any other process to access and potentially modify that variable while you're reading it.
	 */

	int q;

	disable_interrupts();	 

	q = TASK_Q; 
	if (ready_q_head[TASK_Q] == NULL)
		ready_q_head[TASK_Q] = proc_p;
	else
		ready_q_tail[TASK_Q]->nextready = proc_p;
	ready_q_tail[TASK_Q] = proc_p;
	proc_p->nextready = NULL;
	
debug_printf("ready(): &proc_p=%x\n", proc_p);
	debug_print_ready_queue();
	
	restore_flags();
}

void block(proc_t *proc_p)
{
	int proc_num;
	proc_t * tmp_p;

	disable_interrupts();	 

	proc_num = proc_p - proc_table;
	tmp_p = ready_q_head[TASK_Q];
	if (tmp_p != NULL)
	{
		// is proc_p the head?
		if (proc_p == ready_q_head[TASK_Q])
		{
			// remove the head
			ready_q_head[TASK_Q] = tmp_p->nextready;
			// was the removed one also the tail?
			if (tmp_p->nextready == NULL)
				ready_q_tail[TASK_Q] = NULL;
debug_printf("block(): %x is blocked\n", proc_p);
debug_print_ready_queue();
			pick_proc();	// since head is removed, we need to update the current process to run
		}
		else
		{
			// try to find the process if it's there in the queue after the head
			while (tmp_p->nextready != proc_p && tmp_p->nextready != NULL)
				tmp_p = tmp_p->nextready;
			
			if (tmp_p->nextready == NULL)	// couldn't find it
			{
				primitive_printf("FATAL ERROR: cannot find a process in ready queue\n");
			}
			else	// found it
			{
				tmp_p->nextready = tmp_p->nextready->nextready;
				if (tmp_p->nextready == NULL)
					ready_q_tail[TASK_Q] = tmp_p;
debug_printf("block(): %x is blocked\n", proc_p);
debug_print_ready_queue();
			}
		}
	}
	restore_flags();
}

void pick_proc()
{
	proc_t *prev_proc_ptr = cur_proc_ptr;
	if (ready_q_head[TASK_Q] != NULL)
	{
		cur_proc_id = ready_q_head[TASK_Q] - proc_table;	/* cur_proc_id = index of current process in proc_table */
		cur_proc_ptr = ready_q_head[TASK_Q];
	}
	else
	{
		cur_proc_id = PID_IDLE;
		cur_proc_ptr = &proc_table[PID_IDLE];
	}

	if (cur_proc_id != PID_CLOCK)
		bill_proc_ptr = cur_proc_ptr;
	else
		bill_proc_ptr = prev_proc_ptr;
	
debug_printf("pick_proc(): cur_proc_id=%d, cur_proc_ptr=%x\n", cur_proc_id, cur_proc_ptr);
}

/*proc_t * get_proc_addr(int proc_id)
{
	if (PID_HARDWARE <= proc_id &&  proc_id <= MAXIMUM_PROCS - 1)
			return (proc_id == PID_HARDWARE ? HARDWARE_PROC_ADDR : &proc_table[proc_id]);
	else
		return NULL;
}*/

// copy message in physical memory spacing
void cp_msg(message_t *src, message_t *dst)
{
	// switch it to physical memroy spacing	
	switch_paging(kernel_page_directory);
	cp_msg_low(src, dst);
	// restore to the current memroy spacing
	switch_paging(proc_table[cur_proc_id].page_directory);
}

int kernel_send_msg(int caller_pid, int dst_pid, message_t *msg)
{
	proc_t *dst_proc_addr;
	message_t *src_msg_in_phy_address, *dst_msg_in_phy_address;

	dst_proc_addr = &proc_table[dst_pid];	

	src_msg_in_phy_address = (message_t *)vir_to_phy_address((uint32_t) msg, caller_pid);
	dst_msg_in_phy_address = (message_t *)vir_to_phy_address((uint32_t) proc_table[dst_pid].messbuf, dst_pid);
//debug_printf("src_msg_phy_address=%x,%d, dst_msg_phy_address=%x,%d\n", src_msg_in_phy_address, caller_pid, dst_msg_in_phy_address, dst_pid);
//debug_printf("msg address=%x, dst_proc_addr->messbuf=%x\n", msg, dst_proc_addr->messbuf);

	if (dst_proc_addr->flags & RECEIVING)
	{
		// copy the message to dst process table

		cp_msg(src_msg_in_phy_address, dst_msg_in_phy_address);
		//cp_msg(msg, dst_proc_addr->messbuf);
		dst_proc_addr->flags &= ~RECEIVING;
		if (dst_proc_addr->flags == 0)	// not waiting to receive or send
			ready(dst_proc_addr);

		return OK;
	}	
	else
		return EINVAL;
}

int kernel_receive_msg(int caller_pid, int src_pid, message_t *msg)
{
/* A process or task wants to get a message.  If one is already queued,
 * acquire it and deblock the sender.  If no message from the desired source
 * is available, block the caller.  
 */
	proc_t *caller_ptr = &proc_table[caller_pid];
	while (caller_ptr->sender_q != NULL)
	{
		//while (1)
		//	primitive_print_str("msg is received\n");	
	}

	caller_ptr->src = src_pid;
	caller_ptr->messbuf = msg;
	caller_ptr->flags |= RECEIVING;
	block(caller_ptr);	

	return OK;
}

