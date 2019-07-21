#include <stdio.h>
#include <stdint.h>
#include "../h/message.h"
#include "../h/syscall.h"
#include "../h/util.h"
#include "../h/errno.h"
#include "../h/common.h"

static message_t msg;

int main(int argc, char *argv[])
{
	int ret, status;
	if(fork())
	{
		//if it's parent
		wait(&status);
debug_printf("init parent: you should not see this\n");
	}
	else
	{
		//if it's child
debug_printf("init child\n");
	}
	// this is temporary
	while (1)
	{
		receive(ANY_PROC, &msg);
	}
}
