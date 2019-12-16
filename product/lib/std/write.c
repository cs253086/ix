#include "../../h/common.h"
#include "common.h"
#include <std.h>

int write(int fd, char *buf, int size)
{
	msg.type = WRITE;
	msg.integer[FD_IDX_IN_MESSAGE] = fd;
	msg.uinteger[SIZE_IDX_IN_MESSAGE] = size;
	copy(msg.character, buf, size);
	send_receive(PID_FS, &msg);
 
	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
