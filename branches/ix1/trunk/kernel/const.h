/* General constants used by the kernel */
#define NR_REGS	11	/* ax, bx, cx, dx, si, di, bp, es, ds, cs, ss in that order */
#define INIT_PSW      0x0200    /* initial psw */
#define INIT_SP (int*)0x0010    /* initial sp: 3 words pushed by kernel */
#define VECTOR_BYTES     284    /* bytes of interrupt vectors to save. Refer to _vec_table in klib88.s */
#define MEM_BYTES    655360L	/* memory size for /dev/mem, which is the whole memory size, 640K */
#define K_STACK_BYTES    256    /* how many bytes for the kernel stack */
#define TASK_STACK_BYTES 256	/* how many bytes for each task stack */

/* The following values are used in the assembly code.  Do not change the
 * values of 'ES_REG', 'DS_REG', 'CS_REG', or 'SS_REG' without making the 
 * corresponding changes in the assembly code.
 */
#define ES_REG             7    /* proc[i].p_reg[ESREG] is saved es */
#define DS_REG             8    /* proc[i].p_reg[DSREG] is saved ds */
#define CS_REG             9    /* proc[i].p_reg[CSREG] is saved cs */
#define SS_REG            10    /* proc[i].p_reg[SSREG] is saved ss */

/* Interrupt vectors: number represents IRQ(interrupt request) interrupt vectors are located in the order */
#define DIVIDE_VECTOR      0    /* divide interrupt vector */
#define CLOCK_VECTOR       8    /* clock interrupt vector */
#define KEYBOARD_VECTOR    9    /* keyboard interrupt vector */
#define XT_WINI_VECTOR    13    /* xt winchester interrupt vector */
#define FLOPPY_VECTOR     14    /* floppy disk interrupt vector */
#define PRINTER_VECTOR    15    /* line printer interrupt vector */
#define SYS_VECTOR        32    /* system calls are made with int SYSVEC */
#define AT_WINI_VECTOR   118    /* at winchester interrupt vector */

/* The 8259A interrupt controller has to be re-enabled after each interrupt. */
#define INT_CTL         0x20    /* I/O port for interrupt controller */
#define INT_CTLMASK     0x21    /* setting bits in this port disables ints */
#define INT2_CTL        0xA0    /* I/O port for second interrupt controller */
#define INT2_MASK       0xA1    /* setting bits in this port disables ints */
#define ENABLE          0x20    /* code used to re-enable after an interrupt */

#define RET_REG            0	/* system call return code index in p_reg */
#define IDLE            -999    /* 'cur_proc' = IDLE means nobody is running */

/* The following items pertain to the 3 scheduling queues. */
#define NQ                 3    /* # of scheduling queues */
#define TASK_Q             0    /* ready tasks are scheduled via queue 0 */
#define SERVER_Q           1    /* ready servers are scheduled via queue 1 */
#define USER_Q             2    /* ready users are scheduled via queue 2 */
