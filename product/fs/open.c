#include <message.h>
#include <errno.h>
#include <std.h>
#include "common.h"
#include "inode.h"
#include "file.h"
#include "path.h"
#include "open.h"

int fs_open()
{
	int free_file_descriptor_idx;
	inode_t *inode_p;
	file_descriptor_t *file_desc_ptr;
	int caller_pid = msg.src_pid;
	char *path = msg.character;
	int flags = msg.integer[FLAGS_IDX_IN_MESSAGE];	

	caller_fs_proc_ptr = &fs_proc_table[caller_pid];
	// get a free file descriptor
	if ((free_file_descriptor_idx = (caller_fs_proc_ptr->file_descriptor_pool.cur_available_idx)++) >= MAX_FILE_DESC_SIZE) 	
		return E_AGAIN;
	if ((inode_p = get_inode_from_path(path)) == NULL)
		return E_AGAIN;	

	switch (inode_p->mode & INODE_TYPE)
	{
		case INODE_CHAR_SPECIAL:
			dev_open(inode_p->zone[0]);	// zone[0] indicates device num in case that it's a device
			break;
		case INODE_DIRECTORY:
			break;
	}
	/* fill the free file descriptor */
	file_desc_ptr = &(caller_fs_proc_ptr->file_descriptor_pool.fdp[free_file_descriptor_idx]);
	file_desc_ptr->file_inode = inode_p;

	return free_file_descriptor_idx;
}
