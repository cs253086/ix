#include "../h/const.h"
#include "../h/type.h"
#include "const.h"
#include "type.h"

/* process table structure. Actual definition is in proc.c */
struct proc {
	int p_reg[NR_REGS];	/* process's registers */
	int *p_sp;	/* stack pointer */
	struct pc_psw p_pcpsw;	/* pc and psw as pushed by interrupt p.369 */
	int p_flags; 	/* P_SLOT_FREE, SENDING, RECEIVING, etc. */
	struct mem_map p_map[NR_SEGS];	/* memory map */
	int *p_splimit;	/* lowest legal stack value */
	int p_pid;	/* process id passed in from MM */	
	
	real_time user_time;	/* user time in ticks */
	real_time sys_time;	/* sys time in ticks */
	real_time child_utime;	/* cumulative user time of children */
	real_time child_stime;	/* cumulative sys time of children */
	real_time p_alarm; 	/* time of next alarm in ticks, or 0 */

	struct proc *p_callerq;	/* head of list of procs wishing to send to this process p.96 */
	struct proc *p_sendlink;	/* link to next proc wishing to send to this process */
	message *p_messbuf;	/* pointer to messaage buffer */
	int p_getfrom;	/* from whom does this process want to receive? */

	struct proc *p_nextready;	/* pointer to next ready process */
	int p_pending;	/* bit map for pending signals 1-16 */
};

/* Bits for p_flags in proc[]. A process is runnable iff p_flags == 0 */ 
#define P_SLOT_FREE	001	/* set when slot is not in use */
#define NO_MAP	002	/* keeps unmapped forked child from running */
#define SENDING	004	/* set when process blocked trying to send */
#define RECEIVING 010	/* set when process blocked trying to receive */

#define proc_addr(n)	&proc[NR_TASKS + n]
#define NIL_PROC	(struct proc *) 0

extern struct proc proc[NR_TASKS + NR_PROCS];
extern struct proc *proc_ptr;	/* &proc[cur_proc] */
extern struct proc *bill_ptr;	/* ptr to process who calls system_calls or tasks */
extern struct proc *rdy_head[NQ];	/* pointers to ready list headers */
extern struct proc *rdy_tail[NQ];	/* pointers to ready list tails */

extern unsigned busy_map;	/* bit map of busy tasks */
extern message *task_mess[NR_TASKS + 1];	/* ptrs to messages for busy tasks */
					/* Why NR_TASKS+1 ? Bug18 */
extern pick_proc();
extern unready();
