#include <stdio.h>
#include <stdint.h>
#include "../h/message.h"
#include "../h/syscall.h"
#include "../h/util.h"
#include "../h/errno.h"
#include "../h/common.h"

//uint32_t fs_stack[SERVER_STACK_SIZE / sizeof(uint32_t)];
static message_t msg;

int main(int argc, char *argv[])
{
	while (1)
	{
		receive(ANY_PROC, &msg);
	}
}
