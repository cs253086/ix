/* This file contains the main program of NEURO. The routine main()
 * initialize the system and starts by setting up the proc table,
 * interrupt vectors, and scheduling each task to run to initialize
 * itself
 *
 * The entries at this file are:
 * main:        NEURO main program
 * panic:             abort MINIX due to a fatal error
 */

#include "../lib/standalone.c"  /* Debugging */

#include "../h/com.h"
#include "proc.h"
#include "glo.h"

#define VERY_BIG       39328    /* must be bigger than kernel size (clicks) */
#define KERNEL_PATCH_TABLE_SIZE         8
#define STACK_OVERFLOW_SAFETY   8       /* margin of safety for stack overflow in int */

#define KERNEL_BEGIN    1536    /* address where MINIX starts in memory - p.92 */
#define CPU_TY_SEG       0xFFFF    /* BIOS segment that tells CPU type */
#define CPU_TY_OFFSET       0x000E    /* BIOS offset that tells CPU type */
#define PC_AT           0xFC    /* IBM code for PC-AT (in BIOS at 0xFFFFE) */

main()
{
        extern int color; /* defined in tty.c */
        extern int vec_table[], (*task[])();
        extern int get_chrome(), get_byte();
        extern int surprise(), trp(), restart(), divide(), clock_int(), s_call(), dummy_int();
        extern phys_bytes umap();
        extern unsigned sizes[KERNEL_PATCH_TABLE_SIZE]; /* this table is filled by build */

        register struct proc *rp;
        register int tmp;
        vir_clicks size;
        phys_clicks kernel_begin_click, mm_begin_click, pre_begin_click;
        phys_bytes phys_b;

        lock(); /* interrupt handling is not possible yet
                 * because interrupt handling routines are not set yet
                 */

        kernel_begin_click = KERNEL_BEGIN >> CLICK_SHIFT;       /* how many clicks */

        size = sizes[0] + sizes[1];     /* kernel text size + data size in clicks */
        mm_begin_click = kernel_begin_click + size;
       
        /* setup proc table for tasks(note that actual number of tasks  are 7) and MM, FS and INIT */
        for (rp = &proc[0]; rp <= &proc[(NR_TASKS - 1) + NR_SERVER_INIT_PROC]; rp++) {
                for (tmp=0; tmp < NR_REGS; tmp++) rp->p_reg[tmp] = 0100 * tmp;       /* for  debugging. Bug6 */
                tmp = rp - &proc[0] - NR_TASKS; /* NR_TASKS is 8. Refer to h/com.h, but hardware -1 never runs. So, actual number of tasks is 7
                                                 * Therefore, tmp should be -8 to -2 for actual tasks
                                                 */
                /* Set up proc table entry for user processes.  Be very careful about
                 * sp, since the 3 words prior to it will be clobbered when the kernel pushes
                 * pc, cs, and psw onto the USER's stack when starting the user the first
                 * time.  This means that with initial sp = 0x10, user programs must leave
                 * the words at 0x000A(2bytes), 0x000C(2bytes), and 0x000E(2bytes) free. Refer to struct pc_psw in type.h
                 * Note that stack grows top to down. So, word at 0x00E is the last one in the stack
                 * Also, the reason that initial sp = 0x10 is each segment should be a multiple of 16 bytes.
                 */
                rp->p_sp = (rp < &proc[NR_TASKS] ? task_stack[NR_TASKS + tmp].stk : INIT_SP);   /* if rp points to a task, assign a task stack,
                                                                                                 * otherwise, assign INIT_SP    
                                                                                                 */
                rp->p_splimit = rp->p_sp;       /* lowest limit (when it's full) for user process */
                if (rp->p_splimit != INIT_SP)
                        rp->p_splimit -= (TASK_STACK_BYTES - STACK_OVERFLOW_SAFETY) / sizeof(int);      /* lowest limit for task */
                rp->p_pcpsw.pc = task[tmp + NR_TASKS];
                if (rp->p_pcpsw.pc != 0 || tmp >= 0) ready(rp); /* if it's a task or mm,fs,init, it goes to ready queue. Only tmp = -1(hardware) doesn't go to ready(rp) */
                rp->p_pcpsw.psw = INIT_PSW;
                rp->p_flags = 0;

                /* Set up memory map for tasks(-8 ~ -1) and MM, FS, INIT. Refer to Bug8 in detail
                 * Note that each task is not a independent process by itself, in strictly speaking
                 * since each doesn't have main(). They are part of kernel. That's why they share p_map[]
                 */
                if (tmp < 0) {
                        /* I/O tasks. */
                        rp->p_map[T].mem_len  = VERY_BIG;
                        rp->p_map[T].mem_phys = kernel_begin_click;
                        rp->p_map[D].mem_len  = VERY_BIG;
                        rp->p_map[D].mem_phys = kernel_begin_click + sizes[0];
                        rp->p_map[S].mem_len  = VERY_BIG;
                        rp->p_map[S].mem_phys = kernel_begin_click + sizes[0] + sizes[1];
                        rp->p_map[S].mem_vir = sizes[0] + sizes[1];
                } else {
                        /* MM, FS, and INIT. */
                        pre_begin_click = proc[NR_TASKS + tmp - 1].p_map[S].mem_phys;
                        rp->p_map[T].mem_len  = sizes[2 * tmp + 2];
                        rp->p_map[T].mem_phys = (tmp == 0 ? mm_begin_click : pre_begin_click);
                        rp->p_map[D].mem_len  = sizes[2 * tmp + 3];
                        rp->p_map[D].mem_phys = rp->p_map[T].mem_phys + sizes[2 * tmp + 2];
                        rp->p_map[S].mem_vir  = sizes[2 * tmp + 3];
                        rp->p_map[S].mem_phys = rp->p_map[D].mem_phys + sizes[2 * tmp + 3];
                }

                /* Segment assembly register setup for each process */
                rp->p_reg[CS_REG] = rp->p_map[T].mem_phys;
                rp->p_reg[DS_REG] = rp->p_map[D].mem_phys;
                rp->p_reg[SS_REG] = rp->p_map[D].mem_phys;
                rp->p_reg[ES_REG] = rp->p_map[D].mem_phys;
        }
        /* Interrupt source task -1 and set up the temporary stack for it. Refer to mpx88.s Bug9 */
        proc[NR_TASKS+(HARDWARE)].p_sp = (int *) k_stack;
        proc[NR_TASKS+(HARDWARE)].p_sp += K_STACK_BYTES / 2;
        proc[NR_TASKS+(HARDWARE)].p_splimit = (int *) k_stack;
        proc[NR_TASKS+(HARDWARE)].p_splimit += STACK_OVERFLOW_SAFETY / 2;

        /* Now, simple setup for user processes */
        for (rp = proc_addr(LOW_USER + 1); rp < proc_addr(NR_PROCS); rp++)
                rp->p_flags = P_SLOT_FREE;
       
        /* Determine if display is color or monochrome and CPU type (from BIOS). */
        color = get_chrome();         /* 0 = mono, 1 = color */
        tmp = get_byte(CPU_TY_SEG, CPU_TY_OFFSET);       /* is this PC, XT, AT ... ?. Bug10*/
        if (tmp == PC_AT) pc_at = TRUE;

        /* Save the old interrupt vectors. */
        phys_b = umap(proc_addr(HARDWARE), D, (vir_bytes) vec_table, VECTOR_BYTES); /* virtual address of vec_table in kernel -> physical address. umap() is defined in system.c */
        phys_copy(0L, phys_b, (long) VECTOR_BYTES);   /* save all the vectors */

        /* Initialize the new interrupt vectors in the place where the original ones were located, 0L */       
        /* for (tmp = 0; tmp < 16; tmp++) set_vec(tmp, surprise, kernel_begin_click);
        for (tmp = 16; tmp < 256; tmp++) set_vec(tmp, trp, kernel_begin_click); */
        /* Set up interrupt vectors that NEURO uses. Refer to Bug14 */
        set_vec(DIVIDE_VECTOR, divide, kernel_begin_click);
        set_vec(CLOCK_VECTOR, clock_int, kernel_begin_click);
        set_vec(SYS_VECTOR, s_call, kernel_begin_click);
        set_vec(KEYBOARD_VECTOR, tty_int, kernel_begin_click);
        set_vec(FLOPPY_VECTOR, disk_int, kernel_begin_click);
        /* set_vec(PRINTER_VECTOR, dummy_int, kernel_begin_click);
        if (pc_at)
                set_vec(AT_WINI_VECTOR, dummy_int, kernel_begin_click);
        else
                set_vec(XT_WINI_VECTOR, dummy_int, kernel_begin_click); */

        /* Put proc table ptr right below kernel so it can be found in /dev/mem Bug11 */
        /* set_vec( (KERNEL_BEGIN - 4)/4, proc, (phys_clicks) 0); */
         bill_ptr = proc_addr(HARDWARE);       /* it has to point somewhere, initializing bill_ptr */
         pick_proc();   /* Choose the next process to run */
       
        /* Now go to the assembly code to start running the current process. Bug12*/
        port_out(INT_CTLMASK, 0);     /* do not mask out(disable) any interrupts in 8259A. masked means 'ignored' */
        port_out(INT2_MASK, 0);       /* same for second interrupt controller */
        restart();      /* Run the picked process by pick_proc() */
}

/*===========================================================================*
 *                                   unexpected_int                          *
 *===========================================================================*/
unexpected_int()
{
/* A trap or interrupt has occurred that was not expected. */

  printf("Unexpected trap: vector < 16\n");
  printf("pc = 0x%x    text+data+bss = 0x%x\n",proc_ptr->p_pcpsw.pc,
                                        proc_ptr->p_map[D].mem_len << 4);
}


/*===========================================================================*
 *                                   trap                                    *
 *===========================================================================*/
trap()
{
/* A trap (vector >= 16) has occurred.  It was not expected. */

  printf("\nUnexpected trap: vector >= 16 ");
  printf("This may be due to accidentally including\n");
  printf("a non-MINIX library routine that is trying to make a system call.\n");
  printf("pc = 0x%x    text+data+bss = 0x%x\n",proc_ptr->p_pcpsw.pc,
                                        proc_ptr->p_map[D].mem_len << 4);
}

/*===========================================================================*
 *                                   div_trap                                *
 *===========================================================================*/
div_trap()
{
/* The divide intruction traps to vector 0 upon overflow. */

  printf("Trap to vector 0: divide overflow.  ");
  printf("pc = 0x%x    text+data+bss = 0x%x\n",proc_ptr->p_pcpsw.pc,
                                        proc_ptr->p_map[D].mem_len<<4);
}

/*===========================================================================*
 *                                   panic                                   *
 *===========================================================================*/
panic(s,n)
char *s;
int n;
{
/* The system has run aground of a fatal error.  Terminate execution.
 * If the panic originated in MM or FS, the string will be empty and the
 * file system already syncked.  If the panic originates in the kernel, we are
 * kind of stuck.
 */

  if (*s != 0) {
        printf("\nKernel panic: %s",s);
        if (n != NOT_NUM) printf(" %d", n);
        printf("\n");
  }
  printf("\nType space to reboot\n");
  wreboot();

}

/*===========================================================================*
 *                                   set_vec                                 *
 *===========================================================================*/
static set_vec(vec_nr, addr, base_click)
int vec_nr;                     /* which vector */
int (*addr)();                  /* where to start */
phys_clicks base_click;         /* click where kernel sits in memory */
{
/* Set up an interrupt vector. */

  unsigned vec[2];
  unsigned u;
  phys_bytes phys_b;
  extern unsigned sizes[KERNEL_PATCH_TABLE_SIZE];

  /* Build the vector in the array 'vec'.It has two elements. This is why it's called vector p.369 */
  vec[0] = (unsigned) addr;
  vec[1] = (unsigned) base_click;
  u = (unsigned) vec;

  /* Copy the vector into place. */
  phys_b = ((phys_bytes) base_click + (phys_bytes) sizes[0]) << CLICK_SHIFT;    /* phys_b = starting point of Data area */
  phys_b += u;
  phys_copy(phys_b, (phys_bytes) 4 * vec_nr, (phys_bytes) 4);   /* Copy vec[] to physical vector location (from 0L) */
}

