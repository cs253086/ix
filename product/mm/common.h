#ifndef MM_COMMON_H
#define MM_COMMON_H

#include <stdint.h>
#include "../h/common.h"
#include <message.h>

#define IN_USE	1
#define WAITING	2
#define NO_MEM	0

#define KERNEL_PROC_TABLE_ADDRESS_OFFSET	(11 * 4)
typedef struct mproc {
	mem_map_t map[NR_SEGS];       /* points to text, data, stack */
	char exitstatus;           /* storage for status when process exits */
	char sigstatus;            /* storage for signal # for killed processes */
	int pid;                   /* process id */
	int parent_pid;                /* index of parent process */
	int child_pid;			/* index of child process */ 
	int32_t procgrp;               /* process group (used for signals) */
	uint32_t *page_directory;
	uint32_t begin_phy_address;
	uint32_t end_phy_address;

	/* Real and effective uids and gids. */
	//uid mp_realuid;               /* process' real uid */
	//uid mp_effuid;                /* process' effective uid */
	//gid mp_realgid;               /* process' real gid */
	//gid mp_effgid;                /* process' effective gid */

	/* Bit maps for signals. */
	uint16_t ignore;            /* 1 means ignore the signal, 0 means don't */
	uint16_t catch;             /* 1 means catch the signal, 0 means don't */
	int32_t (*mp_func)();             /* all signals vectored to a single user fcn */

	int flags;            /* flag bits */
} mm_proc_t;

extern message_t msg;
extern int msg_src_pid;
extern int dont_reply;
extern int status;
extern uint32_t total_mem;
extern mm_proc_t mm_proc_table[MAXIMUM_PROCS];

extern int procs_in_use;
extern int (*call_vec[])();
extern char block_data[MAX_MESSAGE_CHAR_SIZE];

#endif	/* MM_COMMON_H */
