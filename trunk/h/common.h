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

#define KERNEL_STACK_SIZE    	4096	// bytes
#define SERVER_STACK_SIZE	4096
#define INIT_STACK_SIZE		4096

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
#define PID_TOLD_KERNEL	3
#define PID_MM		8
#define PID_FS		9
#define PID_INIT	10
#define PID_IDLE	0	// it's not an actual process, but it executes 'idle' procedure in kernel/kernel_low.asm
#define CS_SELECTOR    0x08	// cs selector with TI:0, RPL: 0
#define DS_SELECTOR    0x10

/* tell kernel calls */
#define KERNEL_FORK	4
#define KERNEL_COPY	6

#define SEND		1
#define RECEIVE		2

#define RING0	0

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
