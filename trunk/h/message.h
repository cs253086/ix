#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

/* Note: if a change is made, MESSAGE_SIZE in kernel/kernel_low.asm should be also changed */
typedef struct message {
	//header
        int32_t src_pid;   /* who sent the message */
	//content
        int32_t type;     /* what kind of message is it */
	void *buf;	/* message buffer */
	int integer;	/* message integer */
} message_t;

#endif	/* MESSAGE_H */
