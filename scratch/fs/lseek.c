#include <message.h>
#include <std.h>
#include <errno.h>
#include "common.h"
#include "file.h"
#include "lseek.h"

int fs_lseek()
{
	/* Perform the lseek(ls_fd, offset, whence) system call. */
	int caller_pid = msg.src_pid;
	int fd = msg.integer[FD_IDX_IN_MESSAGE];
	int offset = msg.integer[OFFSET_IDX_IN_MESSAGE];
	int offset_type = msg.integer[OFFSET_TYPE_IDX_IN_MESSAGE];

	caller_fs_proc_ptr = &fs_proc_table[caller_pid];
	file_descriptor_t *file_desc_ptr = &(caller_fs_proc_ptr->file_descriptor_pool.fdp[fd]);

	/* The value of 'whence' determines the algorithm to use. */
	switch(offset_type) {
	case SEEK_SET: file_desc_ptr->file_pos = offset;   break;
	case SEEK_CUR: file_desc_ptr->file_pos += offset; break;
	case SEEK_END: file_desc_ptr->file_pos = file_desc_ptr->file_inode->size + offset; break;
	default: return E_INVAL;
	}

	return file_desc_ptr->file_pos;
}

