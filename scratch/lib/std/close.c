#include "../../h/common.h"
#include "common.h"
#include <std.h>

int close(int fd)
{
	msg.type = CLOSE;
	msg.integer[FD_IDX_IN_MESSAGE] = fd;
	send_receive(PID_FS, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
