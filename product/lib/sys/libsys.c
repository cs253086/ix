#include <message.h>
#include "../../h/common.h"
#include <std.h>
#include "libsys.h"

/*
 * calls to told_kernel task
 */

static message_t msg;

/* copy call to kernel sys task */
int sys_copy(uint32_t src_base, uint32_t dst_base, uint32_t size)
{
	msg.type = SYS_COPY;
	msg.uinteger[SRC_BASE_IDX_IN_MESSAGE] = src_base;
	msg.uinteger[DST_BASE_IDX_IN_MESSAGE] = dst_base;
	msg.uinteger[SIZE_IDX_IN_MESSAGE] = size;

	// send and wait for receive msg
	/* if it only sends without 'receive'?
	 * then, we're not sure if kernel_copy() is finished 
         * and mm_fork() executes the next instruction. 
         * It causes an unexpected behavior
	 */
	send_receive(PID_SYS, &msg);
	
	return msg.integer[SRC_BASE_IDX_IN_MESSAGE];
}

/* fork call to kernel sys task */
int sys_fork(int parent_pid, int child_pid)
{
	msg.type = SYS_FORK;
	msg.integer[PARENT_PID_IDX_IN_MESSAGE] = parent_pid;
	msg.integer[CHILD_PID_IDX_IN_MESSAGE] = child_pid;

	send_receive(PID_SYS, &msg);

	return msg.integer[PARENT_PID_IDX_IN_MESSAGE];
}

/* exec call to kernel sys task */
int sys_exec(int caller_pid)
{
	msg.type = SYS_EXEC;
	msg.integer[TARGET_PID_IDX_IN_MESSAGE] = caller_pid;

	send_receive(PID_SYS, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}

/* exit call to kernel sys task */
int sys_exit(int parent_pid, int child_pid)
{
	msg.type = SYS_EXIT;
	msg.integer[PARENT_PID_IDX_IN_MESSAGE] = parent_pid;
	msg.integer[CHILD_PID_IDX_IN_MESSAGE] = child_pid;

	send_receive(PID_SYS, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}

int tell_fs(int type, int parent_pid, int child_pid)
{
	msg.type = type;
	msg.integer[PARENT_PID_IDX_IN_MESSAGE] = parent_pid;
	msg.integer[CHILD_PID_IDX_IN_MESSAGE] = child_pid;

	send_receive(PID_FS, &msg);

	return msg.integer[PARENT_PID_IDX_IN_MESSAGE];
}

