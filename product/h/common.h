#ifndef COMMON_H
#define COMMON_H

#define NA	0	// not applicable
#define COMPONENT_INDEX_SIZE	2

#define BUILD_PATCH_TABLE_SIZE	8	//two double words(text, data) are needed for each os component(kernel, mm, fs, init)

#define BEGIN_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY	0xA0000	// look for wiki-kerenl
#define END_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY	0xFFFFF	// look for wiki-kerenl
#define VIDEO_MEMORY	(uint16_t *)0xB8000;
#define NR_SEGS	3	// text, rodata, data(including bss) and stack 
#define TEXT	0	// index of text segment in p_map[]
#define DATA	1	// index of data segment in p_map[]
#define STACK	2	// index of stack segment in p_map[]

#define KERNEL_IDX	0
#define MM_IDX		1 
#define FS_IDX		2
#define INIT_IDX	3

#define READING		0	/* read data and pass it to user */
#define WRITING		1	/* write data from user */

#define ELF_HEADER_SIZE	52
#define SECTION_HEADER_SIZE	40

#define SECTOR_SIZE	512
#define BLOCK_SIZE	1024	/* block size for filesystem. two consecutive sectors */

#define NUM_OF_ATA_DEVICES	4	// primary bus(master, slave) and secondry bus(master, slave)
#define NUM_OF_BLOCK_DEVICES	5	// primary bus(master, slave) and secondry bus(master, slave) and /dev/ram

/* device number of root (RAM) and boot (ata0) devices */
#define NO_DEV		~0
#define BOOT_DEVICE	0	/* ata0 ( /dev/sda) */
#define ROOT_DEVICE	256	/* ram_disk( /dev/ram) */
#define TTY_DEVICE	512	/* tty( /dev/tty) */
#define MAJOR_DEV_FLAG	8	/* major device = (dev>>MAJOR_DEV_FLAG) & 0xff */
#define MINOR_DEV_FLAG	0	/* minor device = (dev>>MINOR_DEV_FLAG) & 0xff */
#define BOOT_DEVICE_MAJOR	((BOOT_DEVICE >> MAJOR_DEV_FLAG) && 0xff)
#define ROOT_DEVICE_MAJOR	((ROOT_DEVICE >> MAJOR_DEV_FLAG) && 0xff)

/* Flag bits for i_mode in the inode. */
#define INODE_TYPE          0170000 /* this field gives inode type */
#define INODE_REGULAR       0100000 /* regular file, not dir or special */
#define INODE_BLOCK_SPECIAL 0060000 /* block special file */
#define INODE_DIRECTORY     0040000 /* file is a directory */
#define INODE_CHAR_SPECIAL  0020000 /* character special file (e.g dev file) */
#define INODE_SET_UID_BIT   0004000 /* set effective uid on exec */
#define INODE_SET_GID_BIT   0002000 /* set effective gid on exec */
#define ALL_MODES       0006777 /* all bits for user, group and others */
#define RWX_MODES       0000777 /* mode bits for RWX only */
#define R_BIT           0000004 /* Rwx protection bit */
#define W_BIT           0000002 /* rWx protection bit */
#define X_BIT           0000001 /* rwX protection bit */
#define I_NOT_ALLOC     0000000 /* this inode is free */

#define USER_PROCESS_SIZE	0x10000	// bytes
#define KERNEL_STACK_SIZE    	4096	// bytes
#define SERVER_STACK_SIZE	4096
#define INIT_STACK_SIZE		4096
#define USER_STACK_SIZE		4096

#define NUM_OF_HARDWARE_PROC	1
#define NUM_OF_IDLE           	1
#define NUM_OF_TASKS		7 
#define NUM_OF_SERVER_PROCS	2	// MM, FS
#define NUM_OF_INIT		1
#define NUM_OF_USR_PROCS	13
#define MAXIMUM_PROCS         	NUM_OF_IDLE + NUM_OF_TASKS + NUM_OF_SERVER_PROCS + NUM_OF_INIT + NUM_OF_USR_PROCS + NUM_OF_HARDWARE_PROC

#define ANY_PROC		MAXIMUM_PROCS + 100	// means any process

/* process id: pid is equal to the index in proc table, in IX */
#define PID_HARDWARE	(MAXIMUM_PROCS - 1) 	// virtual process for hardware processing
#define PID_CLOCK	1	
#define PID_TTY		2
#define PID_MEM		3
#define PID_SYS	4
#define PID_ATA		5
#define PID_MM		8
#define PID_FS		9
#define PID_INIT	10
#define PID_IDLE	0	// it's not an actual process, but it executes 'idle' procedure in kernel/kernel_low.asm
#define CS_SELECTOR    0x08	// cs selector with TI:0, RPL: 0
#define DS_SELECTOR    0x10

#define DEVICE_READ	3 /* function code for disk read  TTY_READ, DISK_READ should be same value */
#define DEVICE_WRITE	4 /* function code for disk write TTY_WRITE, DISK_WRITE should be same value */

/* system calls */
#define SYS_FORK	4
#define SYS_COPY	6
#define SYS_EXEC	7
#define SYS_EXIT	8

#define SEND		1
#define RECEIVE		2

#define RING0	0

/* message from tty */
#define SUSPEND_REPLY		-998	/* used when tty has no data */
#define REVIVE_REPLY		6

#define NO_CHILD		0
#define NO_PARENT		0

#define KERNEL_MAX_IN_PAGE_SIZE	64
#define USR_MAX_IN_PAGE_SIZE	16
#define MM_BEGIN_PHY_ADDRESS	(KERNEL_MAX_IN_PAGE_SIZE * PAGE_SIZE)	
#define PROCESS_STARTING_VIR_ADDRESS	MM_BEGIN_PHY_ADDRESS	
#define KERNEL_HEAD_ADDRESS	0x00007E00
#define PAGE_SIZE		0x1000
#define PAGE_ALIGN_FLAG		0xFFFFF000

#define MAX_PROCESS_SIZE	0x10000	// 64kb
typedef struct mem_map {
    unsigned int offset; // offset from a base (e.g. base address of a process)
    unsigned int phy_address; // physical address
    unsigned int mem_size; // memory size
} mem_map_t;

#endif	/* COMMON_H */
