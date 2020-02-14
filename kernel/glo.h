/* clock and timers */
extern real_time realtime;      /* real time clock */
extern int lost_ticks;

extern int prev_proc;		/* previous process */
extern int cur_proc;	/* current process #  */
extern message int_mess;
extern int sig_procs;           /* number of procs with p_pending != 0 */

extern int pc_at;               /*  PC-AT type diskette drives (360K/1.2M) */
extern char k_stack[K_STACK_BYTES];     /* The kernel stack. */

/* The kernel and task stacks. */
struct task_stack {
  int stk[TASK_STACK_BYTES / sizeof(int)];
};

extern struct task_stack task_stack[NR_TASKS - 1];
