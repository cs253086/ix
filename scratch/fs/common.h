#ifndef MM_COMMON_H
#define MM_COMMON_H
#include "../h/common.h"
#include <message.h>
#include "inode.h"
#include "file.h"

/* When a block is released, the type of usage is passed to put_block(). */
//#define TYPE_WRITE_IMMEDIATE        0100 /* block should be written to disk now */
#define TYPE_INODE_MAP		3 /* inode bit map */

#define INODE_MAP_SLOTS		1 /* max # of blocks in the inode bit map */
#define ZONE_MAP_SLOTS		1 /* max # of blocks in the zone bit map */

#define LOOK_UP		/* tell search_dir() to look up 'file_name' */


#define INODES_PER_BLOCK (BLOCK_SIZE / (sizeof(inode_t) - SIZE_OF_INODE_NOT_ON_DISK))	/* # inodes/disk blk */
#define SUPER_BLOCK_BEGIN_SECTOR	2
#define INODE_MAP_BEGIN_BLOCK		((SUPER_BLOCK_BEGIN_SECTOR / 2) + 1)
#define ZONE_MAP_BEGIN_BLOCK		INODE_MAP_BEGIN_BLOCK + 1	

#define ROOT_FILESYSTEM_BEGIN_LBA		20000	// in LBA
#define ROOT_FILESYSTEM_SUPER_BLOCK_SECTOR	(ROOT_FILESYSTEM_BEGIN_LBA + SUPER_BLOCK_BEGIN_SECTOR)	// e.g root filesystem starts from sector #2048. But, minimum unit of disk is block(two sectors). Thefore, super block is located in sector #2050
#define ROOT_FILESYSTEM_BEGIN_BLOCK		(ROOT_FILESYSTEM_BEGIN_LBA / 2)	// first sector of the root filesystem / 2	
#define ROOT_FILESYSTEM_SUPER_BLOCK	(uint16_t) (ROOT_FILESYSTEM_SUPER_BLOCK_SECTOR / 2) 	// (the first sector of super block of the root filesystem / 2)
#define ROOT_FILESYSTEM_INODE_MAP_BEGIN_BLOCK		(ROOT_FILESYSTEM_SUPER_BLOCK + 1)
#define ROOT_FILESYSTEM_ZONE_MAP_BEGIN_BLOCK		(ROOT_FILESYSTEM_INODE_MAP + 1)

#define MAX_FILE_DESC_SIZE	3
#define NO_BLOCK		-1

typedef struct fproc {
	//mask_bits fp_umask;           /* mask set by umask system call */
	struct inode *workdir;     /* pointer to working directory's inode */
	struct inode *rootdir;     /* pointer to current root dir (see chroot) */
	struct _file_descriptor_pool {
		struct file_descriptor fdp[MAX_FILE_DESC_SIZE]; /* the file descriptor table */
		int cur_available_idx;
	} file_descriptor_pool;

	//uid fp_realuid;               /* real user id */
	//uid fp_effuid;                /* effective user id */
	//gid fp_realgid;               /* real group id */
	//gid fp_effgid;                /* effective group id */
	//dev_nr fs_tty;                /* major/minor of controlling tty */
	int fd;                    /* place to save fd if rd/wr can't finish */
	char *buffer;              /* place to save buffer if rd/wr can't finish */
	int  nbytes;               /* place to save bytes if rd/wr can't finish */
	char suspended_reply;            /* set to indicate process hanging */
	//char fp_revived;              /* set to indicate process being revived */
	char task;                 /* which task is proc suspended on */
} fs_proc_t;

#define MAX_NAME_SIZE	30	// change affects SIZE_OF_DIR_ENTRY in bin/ls/main.c
typedef struct {                /* directory entry */
  uint16_t inode_num;              /* inode number */
  char entry_name[MAX_NAME_SIZE];       /* character string */
} dir_entry_t;	// change affects SIZE_OF_DIR_ENTRY in bin/ls/main.c

#define MAX_DIR_ENTRIES	(BLOCK_SIZE / sizeof(dir_entry_t))
#define DATA_TYPE_RAW	0
#define DATA_TYPE_DIR	1
#define DATA_TYPE_ZONE	2
typedef struct buf {
	int block_num;
	union {
		unsigned char data[BLOCK_SIZE];
		dir_entry_t dir_entry[MAX_DIR_ENTRIES];
		uint16_t indirect_zone[NUM_OF_INDIRECT_ZONE];
	};
	uint16_t device; /* major | minor device where block resides */
} buf_t;

extern buf_t inode_block;	// real data block indicated by inode	

extern message_t msg;
extern int major_dev;
extern int msg_src_pid;
extern int minor_dev;
extern int task;
extern int dont_reply;
extern buf_t buffer;
extern fs_proc_t *caller_fs_proc_ptr;
extern fs_proc_t fs_proc_table[MAXIMUM_PROCS];
#endif	/* FS_COMMON_H */
