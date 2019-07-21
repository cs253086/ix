#ifndef	TTY_H
#define	TTY_H

#include "../h/message.h"

#define SCAN_CODE_ACK_BIT_1	0x80
#define SCAN_CODE_ACK_BIT_0	0x7F
#define TTY_MAX_CHAR_CAPACITY	10

#define PORT_A_8255	0x60
#define PORT_B_8255	0x61

int tty_task();
message_t keyboard();

#endif	/* TTY_H */
