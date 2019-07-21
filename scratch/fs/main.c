#include <stdint.h>
#include <message.h>
#include <std.h>
#include "../h/util.h"
#include <errno.h>
#include "../h/common.h"
#include "super_block.h"
#include "fs_lib.h"
#include "common.h"
#include "inode.h"
#include "forkexit.h"
#include "open.h"
#include "close.h"
#include "read_write.h"
#include "lseek.h"

#define NUM_OF_FSCALLS	69
//uint32_t fs_stack[SERVER_STACK_SIZE / sizeof(uint32_t)];
message_t msg;
int msg_src_pid;
int dont_reply = 0;
int major_dev;
int minor_dev;
int task;
int status;
buf_t buffer_for_super_block;
buf_t buffer_tmp1, buffer_tmp2;
fs_proc_t *caller_fs_proc_ptr;
fs_proc_t fs_proc_table[MAXIMUM_PROCS];
/* moved this array from local to global since it may smash stack due to its big size */
buf_t inode_block;	// real data block indicated by inode	

static void init_fs();

int (*call_vec[NUM_OF_FSCALLS])() = {
	NULL,
	fs_exit,	/* 1 = exit */
	fs_fork,	/* 2 = fork */
	fs_read,	/* 3 = read */
	fs_write,	/* 4 = write */
	fs_open,	/* 5 = open */	
	fs_close,	/* 6 = close */	
	NULL,		/* 7 */	
	NULL,		/* 8 */	
	NULL,		/* 9 */	
	NULL,		/* 10 */	
	NULL,		/* 11 */	
	NULL,		/* 12 */	
	NULL,		/* 13 */	
	NULL,		/* 14 */	
	NULL,		/* 15 */	
	NULL,		/* 16 */	
	NULL,		/* 17 */	
	NULL,		/* 18 */	
	fs_lseek,	/* 19 = lseek */
};
// since process table for fs points to address 0x0, main function should be the first in text of mm
int main(int argc, char *argv[])
{
	int fs_call;

	init_fs();
	while (1)
	{
		receive(ANY_PROC, &msg);

		fs_call = msg.type;
		msg_src_pid = msg.src_pid;
		dont_reply = 0;
		status = 0;
	
		if (fs_call < 0 || fs_call >= NUM_OF_FSCALLS)
			status = E_BAD_CALL;
		else
			status = (*call_vec[fs_call])();

		if (dont_reply) continue;

		msg.integer[RETURN_IDX_IN_MESSAGE] = status;
		send(msg_src_pid, &msg);			
	}

	return 0;
}

static void load_ramdisk()
{
	int i, count;
	uint32_t loaded_in_kbyte;

	super_block_t *sp;
	/* setup ramdisk */
	msg.src_pid = PID_FS;
	msg.type = DISK_IOCTL;
	
	send_receive(PID_MEM, &msg);
	if (msg.integer[0] != 0)
		primitive_printf("Can't initialize /dev/ram\n");

	/* get the super block from root disk and get number of blocks of the root file system */
	get_block(BOOT_DEVICE, ROOT_FILESYSTEM_SUPER_BLOCK, &buffer_for_super_block, DATA_TYPE_RAW);  
	copy(&super_block[BOOT_DEVICE_MAJOR], buffer_for_super_block.data, sizeof(struct super_block));
	sp = &super_block[BOOT_DEVICE_MAJOR];
	sp->lba = ROOT_FILESYSTEM_BEGIN_LBA;
	count = sp->num_of_zone << sp->log2_zone;	/* # blocks on root dev */
	/* copy root disk blocks to ram disk */
	primitive_printf("Loading RAM disk from root diskette.      Loaded:   0K ");
	super_block[ROOT_DEVICE_MAJOR] = super_block[BOOT_DEVICE_MAJOR];
	for (i = 0; i < count; i++) 
	{
		get_block(BOOT_DEVICE, (uint16_t) (ROOT_FILESYSTEM_BEGIN_BLOCK + i), &buffer_tmp1, DATA_TYPE_RAW);
		//get_block(ROOT_DEVICE, i, &buffer_tmp2);
		copy(buffer_tmp2.data, buffer_tmp1.data, BLOCK_SIZE);
		//bp1->dirt = DIRTY;
		put_block(ROOT_DEVICE, i, &buffer_tmp2, DATA_TYPE_RAW);
		//put_block(bp1, TYPE_INODE_MAP);
		loaded_in_kbyte = (i * BLOCK_SIZE) / 1024L;	/* loaded in kbyte so far */
		if (loaded_in_kbyte % 5 == 0) primitive_printf("\b\b\b\b%3dK", loaded_in_kbyte);
	}
	primitive_printf("\b\b\b\bRAM disk loaded\n\n");
	primitive_clear();
}

static void init_superblock()
{
	super_block_t *sp = &super_block[ROOT_DEVICE_MAJOR];
	inode_t root_device_root_inode;
	// get root inode of root device
	get_inode(ROOT_DEVICE, ROOT_INODE_NUM, &(inode_slots.inode[ROOT_DEVICE_MAJOR][inode_slots.cur_available_idx[ROOT_DEVICE_MAJOR]++]));
	// set up a few members of super_block with the root inode of root device
	sp->inode_mounted_on = &(inode_slots.inode[ROOT_DEVICE_MAJOR][inode_slots.cur_available_idx[ROOT_DEVICE_MAJOR] - 1]);
	sp->root_dir_inode = &(inode_slots.inode[ROOT_DEVICE_MAJOR][inode_slots.cur_available_idx[ROOT_DEVICE_MAJOR] - 1]);
	sp->lba = 0;	// since it's memory, lba is meaningless
	sp->is_readonly = 0;
	if (load_bit_maps(ROOT_DEVICE))
		primitive_printf("init: can't load root bit maps\n");
	
}

static init_inode_slots()
{
	int i, j;
	for (i = 0; i < NUM_OF_BLOCK_DEVICES; i++)
	{
		for (j = 0; j < NUM_OF_INODES_PER_DEVICE; j++)
		{
			inode_slots.inode[i][j].dev = (i << MAJOR_DEV_FLAG);
		}
	}	
}

static void init_fs()
{
	int i;
	fs_proc_t *fs_proc_p;

	load_ramdisk();	/* load RAM disk from root disk */
	init_superblock();	/* initialize super block of root disk */

	// initialize workdir and rootdir for from fs_proc table[PID_MM] to fs_proc table[PID_INIT] 
	for (i = PID_MM; i <= PID_INIT; i++)
	{
		fs_proc_p = &fs_proc_table[i];
		fs_proc_p->workdir = &(inode_slots.inode[ROOT_DEVICE_MAJOR][ROOT_INODE_IDX]);
		fs_proc_p->rootdir = &(inode_slots.inode[ROOT_DEVICE_MAJOR][ROOT_INODE_IDX]);
		
	}
}
