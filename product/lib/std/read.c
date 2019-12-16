#include "../../h/common.h"
#include "common.h"
#include <std.h>

int read(int fd, char *buf, int size)
{
	msg.type = READ;
	msg.integer[FD_IDX_IN_MESSAGE] = fd;
	msg.uinteger[SIZE_IDX_IN_MESSAGE] = size;
	send_receive(PID_FS, &msg);
	copy(buf, msg.character, size);
 
	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
