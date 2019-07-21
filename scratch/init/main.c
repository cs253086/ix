#include <stdint.h>
#include <message.h>
#include <std.h>
#include "../h/util.h"
#include <errno.h>
#include "../h/common.h"

static message_t msg;

int main(int argc, char *argv[])
{
	int ret, status;
	if(fork())
	{
		//if it's parent
		wait(status);
	}
	else
	{
		ret = open("/dev/tty", 1);	// stdin
		ret = open("/dev/tty", 1);	// stdout
		ret = open("/dev/tty", 1);	// stderr
		exec("/bin/sh");	// note that 'stdin', 'stdout' 'stderr' would be inherited to all the children processes since the FS process tables are inherited 
	}
}
