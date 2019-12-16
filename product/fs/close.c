#include <message.h>
#include <errno.h>
#include <std.h>
#include "common.h"
#include "inode.h"
#include "file.h"
#include "path.h"
#include "open.h"

int fs_close()
{
	file_descriptor_t *file_desc_ptr;
	int caller_pid = msg.src_pid;
	int fd = msg.integer[FD_IDX_IN_MESSAGE];	

	caller_fs_proc_ptr = &fs_proc_table[caller_pid];

	/* reset file descriptor */
	file_desc_ptr = &(caller_fs_proc_ptr->file_descriptor_pool.fdp[fd]);
	file_desc_ptr->file_inode = NULL;
	file_desc_ptr->file_pos = 0;

	// free the file descriptor
	caller_fs_proc_ptr->file_descriptor_pool.cur_available_idx--; 	

	return 0;
}
