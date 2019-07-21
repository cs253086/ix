#ifndef	TTY_H
#define	TTY_H

#include "../h/common.h"
#include <message.h>

#define SCAN_CODE_ACK_BIT_1	0x80
#define SCAN_CODE_ACK_BIT_0	0x7F
#define TTY_MAX_CHAR_CAPACITY	10

#define PORT_A_8255	0x60
#define PORT_B_8255	0x61

#define TTY_CHAR_INT	1

#define TTY_READ	DEVICE_READ
#define TTY_WRITE	DEVICE_WRITE

int tty_task();
message_t keyboard();

#endif	/* TTY_H */
