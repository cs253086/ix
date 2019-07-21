#include "../../h/common.h"
#include "common.h"
#include <std.h>

int exec(const char *pathname)
{
	msg.type = EXEC;
	strcpy(msg.character, pathname);
	msg.uinteger[SIZE_IDX_IN_MESSAGE] = strlen(pathname);
	send_receive(PID_MM, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
