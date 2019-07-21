#include "../h/common.h"
#include "super_block.h"
#include "inode.h"

inode_slots_t inode_slots;

/* you can verify if an inode has correct information by the linux command 'stat [file]' inside  * the minix file system.
 * for example, 
 * # stat /mnt/ix_root

  File: ‘/mnt/ix_root/’
  Size: 128       	Blocks: 2          IO Block: 1024   directory
  Device: fc00h/64512d	Inode: 1           Links: 4
  Access: (0755/drwxr-xr-x)  Uid: (    0/    root)   Gid: (    0/    root)
  Access: 2015-05-14 00:54:08.000000000 -0400
  Modify: 2015-05-14 00:45:34.000000000 -0400
  Change: 2015-05-14 00:45:34.000000000 -0400
  Birth: -

  - actual info from gdb
  $1 = {mode = 16877, uid = 0, size = 128, modtime = 1431578734, gid = 1024, num_of_links = 5 '\005', zone = {0, 0, 0, 0, 0, 0, 0, 0, 60672}, dev = 1, num = 0, dirt = 0 '\000', pipe = 0 '\000', 
  mount = 0 '\000', seek = 0 '\000'}
*/

int get_inode(int dev, int num, inode_t * inode_p)
{
	inode_p->dev = dev;
	inode_p->num = num;
	if (dev != NO_DEV) rw_inode(inode_p, READING);     /* get inode from disk */

	return 0;
}

int rw_inode(inode_t *inode_p, int flag)
{
	super_block_t *sp;
	buf_t block_buf;
	int inode_block_position, inode_num_in_the_block, inode_position_in_the_block;
	uint16_t inode_num_zero_based = inode_p->num - 1;
	sp = &super_block[(inode_p->dev >> MAJOR_DEV_FLAG) && 0xff];
	/* Get the block where the inode resides. */
	inode_block_position = (inode_num_zero_based / INODES_PER_BLOCK) + sp->num_of_block_by_inode_bit_map + sp->num_of_block_by_zone_bit_map + 2; // 2 means (bootblock + super block)
	inode_num_in_the_block = inode_num_zero_based % INODES_PER_BLOCK;
	get_block(inode_p->dev, inode_block_position, &block_buf, DATA_TYPE_RAW);
	if (flag == READING)
		copy(inode_p, &block_buf.data[inode_num_in_the_block * (sizeof(inode_t) - SIZE_OF_INODE_NOT_ON_DISK)], sizeof(inode_t) - SIZE_OF_INODE_NOT_ON_DISK);
	else
		copy(&block_buf.data[inode_num_in_the_block * (sizeof(inode_t) - SIZE_OF_INODE_NOT_ON_DISK)], inode_p, sizeof(inode_t) - SIZE_OF_INODE_NOT_ON_DISK);

	return 0;
}

