#include <stdio.h>
#include "../../h/common.h"
#include "../../h/syscall_num.h"
#include "common.h"

int wait()
{
	syscall_msg.type = WAIT;
	syscall_msg.buf = NULL;
	send_receive(PID_MM, &syscall_msg);

	return syscall_msg.integer;
}
