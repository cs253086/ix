#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include "../h/common.h"

/* Inode table.  This table holds inodes that are currently in use. For indirect zone, refer to p301~303 and p304~305 and p260*/

#define NUM_OF_ZONE			9
#define NUM_OF_INDIRECT_ZONE		(BLOCK_SIZE / sizeof(uint16_t))	// block_size / sizeof(zone)
#define SINGLE_INDRECT_ZONE_IDX		7
#define NUM_OF_INODES_PER_DEVICE	8 	
#define ROOT_INODE_NUM			1	/* inode is 1 based */
#define NO_ZONE   			0	/* indicates the absence of a zone number */
#define SIZE_OF_INODE_NOT_ON_DISK	8	// bytes
#define INODE_TYPE          		0170000	/* this field gives inode type */
#define ROOT_INODE_IDX			0

/* Why need ___attribute__((packed)) ?
 * inode_t is not multiplied by base size(32bit)
 * So, sizeof(inode_t) would be 44 instead of 41 
 * we need to use __attribute__((packed)) for the accuracy
 */
typedef struct __attribute__((packed)) inode {
  uint16_t mode;               /* file type, protection, etc. */
  int16_t uid;                    /* user id of the file's owner */
  int32_t size;              /* current file size in bytes */
  int32_t modtime;          /* when was file data last changed */
  char gid;                    /* group number */
  char num_of_links;               /* how many hard links to this file */
  uint16_t zone[NUM_OF_ZONE]; /* zone numbers for direct, ind, and dbl ind */

  /* The following items are not present on the disk. */
  uint16_t dev;                 /* which device is the inode on */
  uint16_t num;               /* inode number on its (minor) device (zero based) */
  //int16_t count;            /* # times inode used; 0 means slot is free */
  char dirt;                  /* CLEAN or DIRTY */
  char pipe;                  /* set to I_PIPE if pipe */
  char mount;                 /* this bit is set if file mounted on */
  char seek;                  /* set on LSEEK, cleared on READING/WRITING */
} inode_t;

#define INODE_SIZE		sizeof(inode_t)

typedef struct _inode_slots_t {
	inode_t inode[NUM_OF_BLOCK_DEVICES][NUM_OF_INODES_PER_DEVICE];
	int cur_available_idx[NUM_OF_BLOCK_DEVICES];
} inode_slots_t;

extern inode_slots_t inode_slots;

int get_inode(int dev, int num, inode_t * inode_p);
int rw_inode(inode_t *inode_p, int flag);
#endif	/* INODE_H */
