#include "../../h/common.h"
#include "common.h"
#include <std.h>

int open(const char *pathname, int flags)
{
	msg.type = OPEN;
	strcpy(msg.character, pathname);
	msg.integer[FLAGS_IDX_IN_MESSAGE] = flags;
	send_receive(PID_FS, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
