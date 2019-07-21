#include <stdint.h>
#include "../h/common.h"
#include "common.h"
#include "fs_lib.h"

void rw_block(buf_t *bp, int flag, int data_type)
{
	uint32_t pos = bp->block_num * BLOCK_SIZE;
	if (data_type == DATA_TYPE_DIR)
		dev_io(flag, bp->device, pos, BLOCK_SIZE, PID_FS, bp->dir_entry);	
	else if (data_type == DATA_TYPE_RAW)
		dev_io(flag, bp->device, pos, BLOCK_SIZE, PID_FS, bp->data);
	else if (data_type == DATA_TYPE_ZONE)
		dev_io(flag, bp->device, pos, BLOCK_SIZE, PID_FS, bp->indirect_zone);
}

void get_block(int dev, int block_num, buf_t *bp, int data_type)
{
	bp->device = dev;
	bp->block_num = block_num;
	rw_block(bp, READING, data_type);
}

void put_block(int dev, int block_num, buf_t *bp, int data_type)
{
	bp->device = dev;
	bp->block_num = block_num;
	rw_block(bp, WRITING, data_type);
}

void suspend_reply_to(int pid, int fd, char *buf, int nbytes, int task)
{
	/* for example, user calls 'getc()', but there is no input from tty yet. now fs suspends(fs doesn't reply to user yet and continue doing other things) pid until getting a message from tty */
	fs_proc_t *caller_ptr = &fs_proc_table[pid];
	caller_ptr->suspended_reply = (char) SUSPEND_REPLY;
	caller_ptr->fd = fd;
	caller_ptr->buffer = buf;
	caller_ptr->nbytes = nbytes;
	//caller_ptr->task = task;
	dont_reply = 1;            /* do not send caller a reply message now */
			
}

int fs_revive_reply()
{
	char *dst;
	int size;
	msg_src_pid = msg.integer[PID_IDX_IN_MESSAGE];
	dst = fs_proc_table[msg_src_pid].buffer;
	size = fs_proc_table[msg_src_pid].nbytes;
	copy(dst, &(msg.integer[RETURN_IDX_IN_MESSAGE]), size);
	dont_reply = 0;

	return size;
}

