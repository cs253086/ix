#include <errno.h>
#include <std.h>
#include "../h/common.h"
#include "common.h"
#include "kernel_lib.h"
#include "paging.h"
#include "irq.h"

void debug_print_ready_queue()
{
	int i;
	proc_t *q_head;
	proc_t *tmp_proc;

	for (i = 0; i <= INIT_USR_Q; i++)
	{	
		q_head = ready_q_head[i];
		
		if (q_head == NULL)
			debug_printf("{%d Queue is empty}\n", i);
		else
		{
			tmp_proc = ready_q_head[i];			
			debug_printf("{%d Queue: ", i);
			while (tmp_proc != NULL)
			{
				debug_printf("%d->", tmp_proc->pid);
				tmp_proc = tmp_proc->nextready;
			}
			debug_printf("NULL} ");
		}
	}
	debug_printf("\n");
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
	int ret = 0;
debug_printf("system_call(): function#=%x\n", function);
	msg->src_pid = caller_pid;
debug_printf("system_call(): caller_pid: %d\n", caller_pid);
debug_printf("system_call(): src_dst: %d\n", src_dst);

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

	int q, pid = proc_p->pid;

	disable_interrupts();	 

	q = (pid <= NUM_OF_TASKS ? TASK_Q : (pid <= (NUM_OF_TASKS + NUM_OF_SERVER_PROCS) ? SERVER_Q : INIT_USR_Q)); 
	if (ready_q_head[q] == NULL)
		ready_q_head[q] = proc_p;
	else
		ready_q_tail[q]->nextready = proc_p;
	ready_q_tail[q] = proc_p;
	proc_p->nextready = NULL;
	
debug_printf("ready(): &proc_p=%d\n", proc_p->pid);
	debug_print_ready_queue();
	
	restore_flags();
}

void block(proc_t *proc_p)
{
	int pid, q;
	proc_t * tmp_p;

	disable_interrupts();	 

	pid = proc_p->pid;
	q = (pid <= NUM_OF_TASKS ? TASK_Q : (pid <= NUM_OF_TASKS + NUM_OF_SERVER_PROCS ? SERVER_Q : INIT_USR_Q)); 
	tmp_p = ready_q_head[q];
	if (tmp_p != NULL)
	{
		// is proc_p the head?
		if (proc_p == ready_q_head[q])
		{
			// remove the head
			ready_q_head[q] = tmp_p->nextready;
			// was the removed one also the tail?
			if (tmp_p->nextready == NULL)
				ready_q_tail[q] = NULL;
debug_printf("block(): %d is blocked\n", proc_p->pid);
debug_print_ready_queue();
			pick_proc();	// since head is removed, we need to update the current process to run
		}
		else
		{
			// try to find the process if it's there in the queue after the head
			while (tmp_p->nextready != proc_p && tmp_p->nextready != NULL)
				tmp_p = tmp_p->nextready;
			
			if (tmp_p->nextready != NULL)	// found it!	
			{
				tmp_p->nextready = tmp_p->nextready->nextready;
				if (tmp_p->nextready == NULL)
					ready_q_tail[q] = tmp_p;
debug_printf("block(): %x is blocked\n", proc_p);
debug_print_ready_queue();
			}
			/*else
				couldn't find it! do nothing. It may be blocked already */
		}
	}
	restore_flags();
}

void pick_proc()
{
	int q = (ready_q_head[TASK_Q] != NULL) ? TASK_Q : (ready_q_head[SERVER_Q] != NULL) ? SERVER_Q : INIT_USR_Q;

	proc_t *prev_proc_ptr = cur_proc_ptr;
	if (ready_q_head[q] != NULL)
	{
		cur_proc_id = ready_q_head[q] - proc_table;	/* cur_proc_id = index of current process in proc_table */
		cur_proc_ptr = ready_q_head[q];
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
	proc_t *caller_ptr, *dst_ptr;
	message_t *src_msg_in_phy_address, *dst_msg_in_phy_address;

	caller_ptr = &proc_table[caller_pid];
	dst_ptr = &proc_table[dst_pid];	

	src_msg_in_phy_address = (message_t *)vir_to_phy_address((uint32_t) msg, caller_pid);
	dst_msg_in_phy_address = (message_t *)vir_to_phy_address((uint32_t) proc_table[dst_pid].messbuf, dst_pid);
debug_printf("kernel_send_msg: caller_pid=%d, dst_pid=%d\n", caller_pid, dst_pid);
//debug_printf("src_msg_phy_address=%x,%d, dst_msg_phy_address=%x,%d\n", src_msg_in_phy_address, caller_pid, dst_msg_in_phy_address, dst_pid);
//debug_printf("msg address=%x, dst_ptr->messbuf=%x\n", msg, dst_ptr->messbuf);

	// if dst_pid is waiting for the msg
	if (dst_ptr->flags & RECEIVING)
	{
debug_printf("kernel_send_msg: %d is waiting for msg\n", dst_pid);
		// copy the message to dst process table

		cp_msg(src_msg_in_phy_address, dst_msg_in_phy_address);
		//cp_msg(msg, dst_ptr->messbuf);
		dst_ptr->flags &= ~RECEIVING;
		if (dst_ptr->flags == 0)	// not waiting to receive or send
			ready(dst_ptr);

	}	
	else
	{
debug_printf("kernel_send_msg: %d is NOT waiting for msg\n", dst_pid);
		if (caller_pid == PID_HARDWARE)
		{
			return E_OVERRUN;
		}
		caller_ptr->messbuf = msg;
		caller_ptr->flags |= SENDING;
		block(caller_ptr);

		//put 'caller_ptr' in the dst->sender
		dst_ptr->sender.list[dst_ptr->sender.cur_available_idx] = caller_ptr;
		dst_ptr->sender.cur_available_idx++;
	}

	return OK;
		
}

int kernel_receive_msg(int caller_pid, int src_pid, message_t *msg)
{
/* A process or task wants to get a message.  If one is already queued,
 * acquire it and deblock the sender.  If no message from the desired source
 * is available, block the caller.  
 */
	int i;
	proc_t *caller_ptr = &proc_table[caller_pid];
	proc_t *cur_sender;
	message_t *src_msg_in_phy_address, *dst_msg_in_phy_address;
debug_printf("kernel_receive_msg: caller_pid=%d, src_pid=%d\n", caller_pid, src_pid);
	// Is there any message sent to this process ?
	for (i = 0; i < caller_ptr->sender.cur_available_idx; i++)
	{
		if (src_pid == ANY_PROC || src_pid == caller_ptr->sender.list[i]->pid)
		{
			cur_sender = caller_ptr->sender.list[i];
			// copy the message to caller
			src_msg_in_phy_address = (message_t *)vir_to_phy_address((uint32_t) cur_sender->messbuf, cur_sender->pid);
			dst_msg_in_phy_address = (message_t *)vir_to_phy_address((uint32_t) msg, caller_pid);
			cp_msg(src_msg_in_phy_address, dst_msg_in_phy_address);
			// remove SENDING flag
			cur_sender->flags &= ~SENDING;
			// remove the cur_sender in the sender list
			caller_ptr->sender.cur_available_idx--;
			// enqueue cur_sender in ready queue
			if ((proc_table[cur_sender->pid].flags & RECEIVING) == 0)
				ready(&proc_table[cur_sender->pid]);
			return OK;
		}
	}

debug_printf("kernel_receive_msg: no message available\n");
	// no message available sent to this process
	caller_ptr->src = src_pid;
	caller_ptr->messbuf = msg;
	caller_ptr->flags |= RECEIVING;
	block(caller_ptr);	

	return OK;
}
