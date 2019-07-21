#include <stdio.h>
#include "../../h/common.h"
#include "../../h/syscall_num.h"
#include "common.h"

int fork()
{
	syscall_msg.type = FORK;
	syscall_msg.buf = NULL;
	send_receive(PID_MM, &syscall_msg);

	return syscall_msg.integer;
}
