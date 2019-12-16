#include "../../h/common.h"
#include "common.h"
#include <std.h>
#include <message.h>

int fork()
{
	msg.type = FORK;
	send_receive(PID_MM, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}
