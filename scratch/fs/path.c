#include "../include/std.h"
#include "../include/errno.h"
#include "super_block.h"
#include "common.h"
#include "inode.h"
#include "path.h"

#define NUM_OF_DIRECT_ZONE		7

static char *parse_one_file(char *path, char *file_name)
{
	int c;
	char *path_cur_pos;
	path_cur_pos = path;
	while ((c = *path_cur_pos) == '/') path_cur_pos++;	// skip leading slashes

	/* copy the first file name to 'file_name' */
	while (c != '/' && c != '\0') 
	{
		*file_name++ = c;
		c = *++path_cur_pos;		
	}
	*file_name = '\0';

	while ((c = *path_cur_pos) == '/') path_cur_pos++;	// skip leading slashes

	return path_cur_pos;
}

/* -refer to p263 Fig. 5-11 for algorithm for this function 
 * -refer to p302 - p303 for the detail about zone 
 */
inode_t *get_inode_from_path(char *path)
{
	int inode_num;
	char file_name[MAX_NAME_SIZE];	// note that directory is also considered as a file
	uint16_t position;
	// set the initial inode
	inode_t *inode_p = (*path == '/' ? caller_fs_proc_ptr->rootdir : caller_fs_proc_ptr->workdir);	

	while (*path != '\0' && inode_p != NULL)	// loop until the end of path or there is no inode for file_name
	{
		for (position = 0; position < inode_p->size; position += BLOCK_SIZE)
		{
			find_block_for_inode_with_position(inode_p, &inode_block, position, DATA_TYPE_DIR);	// find the block numbers associated with the inode
			path = parse_one_file(path, file_name);
			inode_num = search_dir(file_name, &inode_block);
			if (inode_num > 0)
				break;
		}
		if (inode_num == E_INVAL)
			return NULL;

		// find a free inode slot
		inode_p = &(inode_slots.inode[ROOT_DEVICE_MAJOR][++(inode_slots.cur_available_idx[ROOT_DEVICE_MAJOR])]);
		// get the inode for the file
		get_inode(ROOT_DEVICE, inode_num, inode_p);
	}

	return inode_p;
}

void find_block_for_inode_with_position(inode_t *inode_p, buf_t *inode_block, uint32_t position, int data_type)
{
	int scale, relative_block_num_in_file, relative_zone_num, relative_block_num_within_zone, excess, root_indirect_block_num;
	uint16_t absolute_block_num, absolute_zone_num, root_indirect_zone_num;

	// the scale factor used for converting blocks to zones
	scale = super_block[(inode_p->dev >> MAJOR_DEV_FLAG) && 0xff].log2_zone;
	relative_block_num_in_file = position / BLOCK_SIZE;      /* relative blk # in file */
	relative_zone_num = relative_block_num_in_file >> scale;    /* relative zone # in the inode */
	relative_block_num_within_zone = relative_block_num_in_file - (relative_zone_num << scale);   /* relative blk # within zone */

	/* Is 'position' to be found in the inode itself? (direct) */  
	if (relative_zone_num < NUM_OF_DIRECT_ZONE) 
	{
		if ((absolute_zone_num = inode_p->zone[relative_zone_num]) == NO_ZONE) 
			inode_block->block_num = NO_BLOCK;

		absolute_block_num = (absolute_zone_num << scale) + relative_block_num_within_zone;
		get_block(inode_p->dev, absolute_block_num, inode_block, data_type); 
	}
	else	/* indirect */
	{
		//TODO: consider double indirect zone as well
		excess = relative_zone_num - NUM_OF_DIRECT_ZONE;
		if (excess < NUM_OF_INDIRECT_ZONE)
			root_indirect_zone_num = inode_p->zone[SINGLE_INDRECT_ZONE_IDX];	

		// get the block containing indirect zone information
		root_indirect_block_num = root_indirect_zone_num << scale;		
		get_block(inode_p->dev, root_indirect_block_num, inode_block, DATA_TYPE_ZONE);	 
		// get the absolute block in the block containing indirect zone information
		absolute_zone_num = inode_block->indirect_zone[excess];
		absolute_block_num = (absolute_zone_num << scale) + relative_block_num_within_zone;
		get_block(inode_p->dev, absolute_block_num, inode_block, data_type); 
		
	}
}

/*inode_t *get_inode_from_name(inode_t *inode_p, char *file_name)
{
	int inode_num;

	if (file_name[0] == '\0')
		return inode_p;

	if (search_dir(inode_p, file_name, &inode_num, LOOK_UP) != OK)
		return NULL;

	return get_inode(inode_p->dev, inode_num);
}*/

int search_dir(char *file_name, buf_t *inode_block)
{
	int i = 0;
	/* loop if block num is valid */
	if (inode_block->block_num != 0)	// note that block num is 1 based 
	{
		while (inode_block->dir_entry[i].inode_num != 0)	// note that inode num is 1 based
		{
			if (strcmp(inode_block->dir_entry[i].entry_name, file_name) == 0)
				return inode_block->dir_entry[i].inode_num;
			i++;
		}
	}

	return E_INVAL;
}
