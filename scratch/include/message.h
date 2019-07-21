#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

#define MAX_MESSAGE_CHAR_SIZE	64
#define MAX_MESSAGE_UCHAR_SIZE	64
#define MAX_MESSAGE_INT_SIZE	16
#define MAX_MESSAGE_UINT_SIZE	16

/* FS message members */
#define DEVICE_IDX_IN_MESSAGE	0
#define POS_IDX_IN_MESSAGE	0
#define BUF_ADDRESS_IDX_IN_MESSAGE	1
#define FD_IDX_IN_MESSAGE	2
#define PID_IDX_IN_MESSAGE	3

/* sys_copy message members */
#define SRC_BASE_IDX_IN_MESSAGE		0
#define DST_BASE_IDX_IN_MESSAGE		1

/* exit() message members */
#define STATUS_IDX_IN_MESSAGE		0

/* sys_fork, sys_exec, tell_fs message members */
#define PARENT_PID_IDX_IN_MESSAGE	0
#define CHILD_PID_IDX_IN_MESSAGE	1
#define TARGET_PID_IDX_IN_MESSAGE	2

/* open message members */
#define FLAGS_IDX_IN_MESSAGE		0

/* lseek message members */
#define OFFSET_IDX_IN_MESSAGE		3
#define OFFSET_TYPE_IDX_IN_MESSAGE	4

/* common message members */
#define SIZE_IDX_IN_MESSAGE		2
#define RETURN_IDX_IN_MESSAGE		0


/* Note: if a change is made, MESSAGE_SIZE in kernel/kernel_low.asm should be also changed */
typedef struct message {
	//header
        int32_t src_pid;   /* who sent the message */
        int32_t type;     /* what kind of message is it */
	//content
	char character[MAX_MESSAGE_CHAR_SIZE];
	unsigned char ucharacter[MAX_MESSAGE_UCHAR_SIZE];
	int32_t integer[MAX_MESSAGE_INT_SIZE];
	uint32_t uinteger[MAX_MESSAGE_UINT_SIZE];
	//void *buf;	/* message buffer */
	//int integer;	/* message integer */
} message_t;

#endif	/* MESSAGE_H */
