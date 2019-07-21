#ifndef KERNEL_COMMON_H
#define KERNEL_COMMON_H
#include <stdint.h>
#include "../h/common.h"
#include <message.h>


#define NR_REGS       11      // eax, ecx, edx, ebx, ebp, esp, esi, edi, ds, es, fs, gs, ss in order
#define ES_REG	7   // proc[i].p_reg[ESREG] is saved es 
#define DS_REG	8   // proc[i].p_reg[DSREG] is saved ds 
#define CS_REG	9   // proc[i].p_reg[CSREG] is saved cs 
#define SS_REG	10  // proc[i].p_reg[SSREG] is saved ss 

#define TASK_Q		0	// TASK ready queue index
#define SERVER_Q	1	// SERVER ready queue index
#define INIT_USR_Q	2	// INIT and USR ready queue index
#define NUM_Q		3

#define TASK_STACK_SIZE	512	// task stack size in bytes



#define INIT_ESP (int*) 0x0010	// initial sp: 3 words pushed by 'run_proc' each time cur_proc starts running 
#define INIT_PSW	0x0200	// only IF(interrupt flag) is enabled and IOPL 0

#define SENDING		004	/* set when process blocked trying to send */
#define RECEIVING	010	// set when process blocked trying to receive

/* flags for proc_t */
#define NO_MAP           002	/* keeps unmapped forked child from running */
 
extern uint32_t ds_base;	// defined in main.c

extern uint32_t sizes[BUILD_PATCH_TABLE_SIZE];	// defined in kernel_head.asm
extern uint32_t server_stack_address[NUM_OF_SERVER_PROCS];
extern uint32_t init_stack_address;
extern uint32_t kernel_directory_address;
extern uint32_t proc_table_address;

#define viraddr_to_physaddr(vir)	(uint32_t) vir	// assuming base address of segment(e.g DS) is 0 

typedef struct pc_psw {
  int (*pc)();          /* storage for program counter */
	/* Why cs is 32bit even if cs is 16bit size ?
	: because iret pops default size (32bit) */
  unsigned int  cs;       /* code segment register */
  unsigned int psw;         /* program status word */
} pc_psw_t;

#define SENDER_MAX_SIZE		3


/* process table structure */
typedef struct proc {
	int ss, gs, fs, es, ds, cs;
	int psw;
	int eip, edi, esi, ebp, esp, ebx, edx, ecx, eax;

    	int flags;    /* P_SLOT_FREE, SENDING, RECEIVING, etc. */
	uint32_t *page_directory;
	uint32_t begin_phy_address;
	uint32_t end_phy_address;
    	mem_map_t map[NR_SEGS];  /* memory map for process memory space (text, rodata, data and stack) */
	int esplimit; /* lowest legal stack value */
    	int pid;  /* process id */

	struct sender_list {
		struct proc *list[SENDER_MAX_SIZE];
		int cur_available_idx;	
	} sender;

    	//struct proc *sender_q; /* head of list of procs wishing to send to this process p.96 */
    	//struct proc *sendlink;    /* link to next proc wishing to send to this process */
    	message_t *messbuf; /* pointer to messaage buffer. It can be both to send and to receive */
    	int src;  /* If a process want to receive a message from a specific process, but no message is there yet
					 * then, the process blocks and the specific process is stored in this variable 
					 */

    	struct proc *nextready;   /* pointer to next ready process */
	int spent_cpu_time;
} proc_t;

extern int cur_proc_id;
extern proc_t *cur_proc_ptr;
extern proc_t *prev_proc_ptr;
extern proc_t *bill_proc_ptr;	// the process to be charged for CPU time
extern proc_t *ready_q_head[NUM_Q];
extern proc_t *ready_q_tail[NUM_Q];

/* task_stack is a stack for each task/driver 
 * Why is it needed ?
 * kernel binary has all the tasks/drivers 
 * so, each task doesn't have a stack segment 
 * TODO: each task/driver needs to be a separate binary/process
 */
typedef struct task_stack {
    uint32_t stack[TASK_STACK_SIZE / sizeof(uint32_t)];
} task_stack_t;

extern int (*tasks[NUM_OF_TASKS])();
extern proc_t proc_table[MAXIMUM_PROCS];

#endif	/* KERNEL_COMMON_H */
