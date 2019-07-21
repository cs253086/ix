#include "super_block.h"
#include "../h/common.h"

super_block_t super_block[NUM_OF_SUPER_BLOCK_SLOTS]; /* super blocks loaded from disks and currently in use */

int load_bit_maps(int dev)
{
/* Load the bit map from superblock. */

	int i, zone_base_block;
	super_block_t *sp;
	buf_t block_buf;
	int dev_major = (dev >> MAJOR_DEV_FLAG) && 0xff;

	sp = &super_block[dev_major];          /* get the superblock pointer */

	/* Load the inode map from the disk. */
	for (i = 0; i < sp->num_of_block_by_inode_bit_map; i++)
		get_block(dev, INODE_MAP_BEGIN_BLOCK + i, &(sp->inode_map[i]), DATA_TYPE_RAW);

	/* Load the zone map from the disk. */
	for (i = 0; i < sp->num_of_block_by_zone_bit_map; i++)
		get_block(dev, ZONE_MAP_BEGIN_BLOCK + i, &(sp->zone_map[i]), DATA_TYPE_RAW);

	/* inodes 0 and 1, and zone 0 are never allocated.  Mark them as busy. */
	//sp->s_imap[0]->b_int[0] |= 3; /* inodes 0, 1 busy */
	//sp->s_zmap[0]->b_int[0] |= 1; /* zone 0 busy */

	return 0;
}

