#include <stdint.h>
#include <message.h>
#include "common.h"
#include "device.h"

/* The order of the entries here determines the mapping between major device
 * numbers and tasks.  The first entry (major device 0) is not used.  The
 * next entry is major device 1, etc.  Character and block devices can be
 * intermixed at random.  If this ordering is changed, BOOT_DEV and ROOT_DEV
 * must be changed to correspond to the new values.
 */
dev_map_t dev_maps[] = {
/*  	Open       Read/Write   Close       Task #      Device  File
    	----       ----------   -----       -------     ------  ----      */
	no_call,   rw_dev,      no_call,    PID_ATA,  	/* 0 = /dev/sda  */
	no_call,   rw_dev,      no_call,    PID_MEM,    /* 1 = /dev/ram  */
	no_call,   rw_dev,      no_call,    PID_TTY,        /* 2 = /dev/tty */
	//no_call,   rw_dev2,     no_call,    PID_TTY,        /* 4 = /dev/tty  */
    //no_call,   rw_dev,      no_call,    FLOPPY,      	/* 2 = /dev/fd0  */
};

static get_major_minor_dev(int dev)
{
	major_dev = (dev >> MAJOR_DEV_FLAG) & 0xFF;
	minor_dev = (dev >> MINOR_DEV_FLAG) & 0xFF;
	task = dev_maps[major_dev].task;
}

int dev_open(int dev)
{
	get_major_minor_dev(dev);
	(*dev_maps[major_dev].open)(task, &msg);
	return msg.integer[RETURN_IDX_IN_MESSAGE];
}

int dev_io(int flag, int dev, uint32_t pos, int size_in_byte, int pid, char *buf)
{
	get_major_minor_dev(dev);
	msg.type = (flag == READING ? DEVICE_READ : DEVICE_WRITE);
	msg.integer[DEVICE_IDX_IN_MESSAGE] = minor_dev;
	msg.uinteger[POS_IDX_IN_MESSAGE] = pos;
	msg.uinteger[BUF_ADDRESS_IDX_IN_MESSAGE] = (uint32_t) buf;
	msg.integer[SIZE_IDX_IN_MESSAGE] = size_in_byte;
	msg.integer[PID_IDX_IN_MESSAGE] = pid;
	(*dev_maps[major_dev].rw)(task, &msg);

	return msg.integer[RETURN_IDX_IN_MESSAGE];
}

int rw_dev(int task, message_t *msg)
{
	send_receive(task, msg);
	return 0;
}

int no_call(int task, message_t *msg)
{
	/* Null operation always succeeds */
	msg->integer[RETURN_IDX_IN_MESSAGE] = 0;	
	return 0;
}

