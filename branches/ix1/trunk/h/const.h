#define TRUE               1    /* used for turning integers into Booleans */
#define FALSE              0    /* used for turning integers into Booleans */
#define MAX_LONG  2147483647	/* maximum positive long, i.e. 2^31 - 1 */

#define CLICK_SHIFT	4	/* 4 bits shift related to CLICK */
#define KERNEL_BEGIN	1536	/* starting point of kernel in physical address */

#define NR_SERVER_INIT_PROC	3	/* mm, fs, init */
#define	NR_SEGS		3	/* # segments per process */
#define NR_TASKS	8	/* # tasks */
#define NR_PROCS	16	/* # slots in proc table */
#define T                  0    /* proc[i].mem_map[T] is for text */
#define D                  1    /* proc[i].mem_map[D] is for data */
#define S                  2    /* proc[i].mem_map[S] is for stack */

#define INIT_PROC_NR       2    /* init process number -- the process that goes multiuser */

/* Process numbers of some important processes */
#define MM_PROC_NR         0    /* process number of memory manager */
#define FS_PROC_NR         1    /* process number of file system */
#define INIT_PROC_NR       2    /* init -- the process that goes multiuser */

#define LOW_USER           2   	/* first user not part of operating system */

#define CLOCK_HZ                60    /* clock freq (software settable on IBM-PC) */
#define BYTE_MASK           0377    /* mask for 8 bits. 0xFF */
#define NOT_NUM           0x8000 /* used as numerical argument to panic() in kernel/main.c */
#define BLOCK_SIZE      1024	/* # bytes in a disk block */
