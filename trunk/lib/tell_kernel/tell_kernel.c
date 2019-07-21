#include "../../h/tell_kernel.h"
#include "../../h/message.h"
#include "../../h/common.h"
#include "../../h/syscall.h"

/*
 * calls to told_kernel task
 */

static message_t msg;

int tell_kernel_copy(uint32_t src_base, uint32_t dst_base, uint32_t size)
{
	uint32_t base_address[3] = {src_base, dst_base, size};
	msg.type = KERNEL_COPY;
	msg.buf = (void *)base_address;	
	// send and wait for receive msg
	/* if it only sends without 'receive'?
	 * then, we're not sure if kernel_copy() is finished 
         * and mm_fork() executes the next instruction. 
         * It causes an unexpected behavior
	 */
	send_receive(PID_TOLD_KERNEL, &msg);
	
	return msg.type;
}

int tell_kernel_fork(int parent_pid, int child_pid)
{
	int pid[2] = {parent_pid, child_pid};
	msg.type = KERNEL_FORK;
	msg.buf = (void *)pid;
	send_receive(PID_TOLD_KERNEL, &msg);

	return msg.type;
}

