#include "../../h/common.h"
#include "common.h"
#include <std.h>

int lseek(int fd, int offset, int offset_type)
{
	msg.type = LSEEK;
	msg.integer[FD_IDX_IN_MESSAGE] = fd;
	msg.integer[OFFSET_IDX_IN_MESSAGE] = offset;
	msg.integer[OFFSET_TYPE_IDX_IN_MESSAGE] = offset_type;
	send_receive(PID_FS, &msg);
 
	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
