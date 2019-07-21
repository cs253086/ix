#include "../../h/common.h"
#include "common.h"
#include <std.h>

int wait(int status)
{
	msg.type = WAIT;
	send_receive(PID_MM, &msg);

	return msg.integer[0];
}
