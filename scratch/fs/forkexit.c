#include "common.h"
#include "fs_lib.h"
#include "forkexit.h"

int fs_fork()
{
	int parent_pid = msg.integer[PARENT_PID_IDX_IN_MESSAGE];
	int child_pid = msg.integer[CHILD_PID_IDX_IN_MESSAGE];

	fs_proc_t *parent_fs_proc = &fs_proc_table[parent_pid];
	fs_proc_t *child_fs_proc = &fs_proc_table[child_pid];	

	/* Copy the parent's fproc struct to the child. */
	copy(child_fs_proc, parent_fs_proc, sizeof(fs_proc_t));

	// reset file descriptor pool
	child_fs_proc->file_descriptor_pool.cur_available_idx = 0;

	return 0;
}

int fs_exit()
{
	int child_pid = msg.integer[CHILD_PID_IDX_IN_MESSAGE];

	fs_proc_t *child_fs_proc = &fs_proc_table[child_pid];	

	child_fs_proc->workdir = &(inode_slots.inode[ROOT_DEVICE_MAJOR][ROOT_INODE_IDX]);
	child_fs_proc->rootdir = &(inode_slots.inode[ROOT_DEVICE_MAJOR][ROOT_INODE_IDX]);
	
	child_fs_proc->file_descriptor_pool.cur_available_idx = 0;
	
	return 0;
}
