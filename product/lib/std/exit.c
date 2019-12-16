#include "../../h/common.h"
#include "common.h"
#include <std.h>
#include <message.h>

int exit(int status)
{
	msg.type = EXIT;
	msg.integer[STATUS_IDX_IN_MESSAGE] = status;
	/* this system call doesn't receive reply practically because all the memory mapping and info are cleared 
	if send() instead of send_receive(), it means asynchronous and returns immediately, it results in execution of subsequent instructions after exit() 
	That's why we need send_receive() */
	send_receive(PID_MM, &msg);	

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
