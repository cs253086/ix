#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

#include <stdint.h>
#include "common.h"

#define NUM_OF_SUPER_BLOCK_SLOTS	2

/* you can verify the super block information by formatting device to minix file system.
 * (e.g sudo mkfs.minix -1 /dev/mapper/loop0p1)
 */
typedef struct super_block {
	uint16_t num_of_inode;           /* # usable inodes on the minor device */
	uint16_t num_of_zone;             /* total device size, including bit maps etc */
	uint16_t num_of_block_by_inode_bit_map;        /* # of blocks used by inode bit map */
	uint16_t num_of_block_by_zone_bit_map;        /* # of blocks used by zone bit map */
	uint16_t first_data_zone;      /* number of first data zone */
	int16_t log2_zone;    /* log2 of blocks/zone */
	int32_t max_file_size;          /* maximum file size on this device */
	int16_t magic;                  /* magic number to recognize super-blocks */

	/* The following items are only used when the super_block is in memory. */
	struct buf inode_map[INODE_MAP_SLOTS]; /* pointers to the in-core inode bit map */
	struct buf zone_map[ZONE_MAP_SLOTS]; /* pointers to the in-core zone bit map */
	uint16_t dev;                 /* whose super block is this? */
	uint32_t lba;	/* staring lba sector of this file system in the disk */
	struct inode *root_dir_inode;         /* inode for root dir of mounted file sys */
	struct inode *inode_mounted_on;       /* inode mounted on */
	int32_t time;             /* time of last update */
	char is_readonly;               /* set to 1 if file sys mounted read only */
	//char s_dirt;                  /* CLEAN or DIRTY */
} super_block_t;


extern super_block_t super_block[NUM_OF_SUPER_BLOCK_SLOTS]; /* super blocks loaded from disks and currently in use */
#endif	/* SUPER_BLOCK_H */
