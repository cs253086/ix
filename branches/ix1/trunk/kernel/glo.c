#include "../h/const.h"
#include "../h/type.h"
#include "const.h"
#include "glo.h"

/* clock and timers */
real_time realtime;
int lost_ticks;

int prev_proc;		/* previous process */
int cur_proc;	/* current process # */
message int_mess; /* interrupt message */
int sig_procs;           /* number of procs with p_pending != 0 */

int pc_at;               /*  PC-AT type diskette drives (360K/1.2M) */

char k_stack[K_STACK_BYTES];     /* The kernel stack. */
struct task_stack task_stack[NR_TASKS - 1];        /* task stacks; task = -1 never really runs */
