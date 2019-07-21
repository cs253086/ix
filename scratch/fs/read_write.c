#include <message.h>
#include <errno.h>
#include "common.h"
#include "file.h"

static int read_write(int flag)
{
	int rv, caller_pid = msg.src_pid;
	int fd = msg.integer[FD_IDX_IN_MESSAGE];
	int max_size_in_cur_block, remaining_size = msg.uinteger[SIZE_IDX_IN_MESSAGE];
	file_descriptor_t *file_desc_ptr;
	uint32_t *file_pos_p;
	inode_t *inode_p;
	char *src_dst = msg.character;

	caller_fs_proc_ptr = &fs_proc_table[caller_pid];
	file_desc_ptr = &(caller_fs_proc_ptr->file_descriptor_pool.fdp[fd]);	// get file_descritor pointer
	inode_p = file_desc_ptr->file_inode;	// get inode for the file descriptor
	file_pos_p = &(file_desc_ptr->file_pos);	// get the current file position

	if ((inode_p->mode & INODE_TYPE) == INODE_CHAR_SPECIAL)	// if it's a special file (e.g /dev/tty)
	{
		/* flush(command) it to hardware controller to get the result
		 * Note that rv is the character read from tty 
		 */ 
        	rv = dev_io(flag, inode_p->zone[0], *file_pos_p, remaining_size, caller_pid, src_dst);	// zone[0] indicates device num in case that it's a device
		if (flag == READING)
			copy(src_dst, &rv, remaining_size);
		/* TODO: asynchrous (if there is no input from tty, do other things until getting one 
		if (rv == SUSPEND_REPLY)
			suspend_reply_to(caller_pid, fd, dst, remaining_size, inode_p->zone[0]);
		*/
		dont_reply = 0;
		return remaining_size;
	
	}
	else
	{
		if (flag == READING)
		{
			while (remaining_size)
			{
				if ((inode_p->mode & INODE_TYPE) == INODE_DIRECTORY)
				{
					/* locate the current position of file */
					find_block_for_inode_with_position(inode_p, &inode_block, *file_pos_p, DATA_TYPE_DIR);
					/* copy the size from the file to dst */
					/* TODO: msg.character has limited array. this expression is only valid < BLOCK_SIZE */
					max_size_in_cur_block = BLOCK_SIZE - (*file_pos_p % BLOCK_SIZE);
					if (remaining_size < max_size_in_cur_block)
					{
						copy(src_dst, &inode_block.dir_entry[*file_pos_p / sizeof(dir_entry_t)], remaining_size); 
						*file_pos_p += remaining_size;
						remaining_size = 0;
					}
					else 
					{
						copy(src_dst, &inode_block.dir_entry[*file_pos_p / sizeof(dir_entry_t)], max_size_in_cur_block);
						remaining_size -= max_size_in_cur_block;
						src_dst = msg.character + max_size_in_cur_block; 
						*file_pos_p += max_size_in_cur_block;
					}
				
				}
				else
				{
					/* locate the current position of file */
					find_block_for_inode_with_position(inode_p, &inode_block, *file_pos_p, DATA_TYPE_RAW);
					/* copy the size from the file to dst */
					/* TODO: msg.character has limited array. this expression is only valid < BLOCK_SIZE */
					max_size_in_cur_block = BLOCK_SIZE - (*file_pos_p % BLOCK_SIZE);
					if (remaining_size <= max_size_in_cur_block)
					{
						copy(src_dst, &inode_block.data[*file_pos_p % BLOCK_SIZE], remaining_size); 
						*file_pos_p += remaining_size;
						remaining_size = 0;
					}
					else 
					{
						copy(src_dst, &inode_block.data[*file_pos_p % BLOCK_SIZE], max_size_in_cur_block);
						remaining_size -= max_size_in_cur_block;
						src_dst += max_size_in_cur_block; 
						*file_pos_p += max_size_in_cur_block;
					}
				}
			}
		
			return OK;
		}
	}	
		
}

int fs_read()
{
	return read_write(READING);
}

int fs_write()
{
	return read_write(WRITING);
}

