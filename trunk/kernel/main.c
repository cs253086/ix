#include <stdio.h>
#include <stdint.h>
#include "../h/util.h"
#include "../h/common.h"
#include "common.h"
#include "kernel_lib.h"
#include "clock.h"
#include "tty.h"
#include "told_kernel.h"
#include "paging.h"

extern void init_8259(void);	// defined in 8259_pic.asm

int (*tasks[NUM_OF_TASKS]) () = {clock_task, tty_task, told_kernel_task, NULL, NULL, NULL, NULL};
proc_t proc_table[MAXIMUM_PROCS];

proc_t *ready_q_head[NUM_Q];
proc_t *ready_q_tail[NUM_Q];

int cur_proc_id;
proc_t *cur_proc_ptr;
proc_t *prev_proc_ptr;
proc_t *bill_proc_ptr;

void setup_process_table_common(int proc_table_idx)
{
	proc_table[proc_table_idx].pid = proc_table_idx;
	proc_table[proc_table_idx].cs = CS_SELECTOR;
	proc_table[proc_table_idx].ds = DS_SELECTOR;
	proc_table[proc_table_idx].es = DS_SELECTOR;
	proc_table[proc_table_idx].fs = DS_SELECTOR;
	proc_table[proc_table_idx].gs = DS_SELECTOR;
	proc_table[proc_table_idx].ss = DS_SELECTOR;
	if (proc_table[proc_table_idx].eip != 0)	
		ready(&proc_table[proc_table_idx]);
	proc_table[proc_table_idx].psw = INIT_PSW;
	proc_table[proc_table_idx].flags = 0;
}

int main(int argc, char *argv[])
{
	int i, proc_table_idx = 1;      // proc_table[0] is for PID_IDLE;
	int ix_component_idx = 0;
	task_stack_t task_stacks[NUM_OF_TASKS];	

	disable_interrupts();
	init_8259();	// defined in 8259_pic.asm
	setup_paging();

	/* process process table */
	/* setup for real tasks (except HARDWARE), whose pids are from 0 to 6. 
	 * HARDWARE task means virtual process that hardware produces interrupts
	 */
        
	// IDLE process: virtual processe for 'idle' procedure in kernel/kernel_low.asm
        proc_table[0].esp = -1;	// there is no stack for 'idle'
        proc_table[0].esplimit = -1;       
	proc_table[0].page_directory = kernel_page_directory;	

	for (i = 0; i < NUM_OF_TASKS; i++, proc_table_idx++)
        {
                proc_table[proc_table_idx].eip = (int) tasks[i];
		setup_process_table_common(proc_table_idx);
                proc_table[proc_table_idx].esp = (int) task_stacks[i].stack + TASK_STACK_SIZE;
                proc_table[proc_table_idx].esplimit = (int) task_stacks[i].stack;        // array[0] is the bottom(low address) ... array[last] is the top(high address)

             
		proc_table[proc_table_idx].map[TEXT].offset = 0;
                proc_table[proc_table_idx].map[TEXT].phy_address = KERNEL_HEAD_ADDRESS;
                proc_table[proc_table_idx].map[TEXT].mem_size = sizes[ix_component_idx + TEXT];
                
                proc_table[proc_table_idx].map[DATA].offset = sizes[ix_component_idx + TEXT];
                proc_table[proc_table_idx].map[DATA].phy_address = KERNEL_HEAD_ADDRESS + sizes[ix_component_idx + TEXT];
                proc_table[proc_table_idx].map[DATA].mem_size = sizes[ix_component_idx + DATA];
                
                proc_table[proc_table_idx].map[STACK].offset = NA;	// there is no stack segment in kernel since the stack is in bss
                proc_table[proc_table_idx].map[STACK].phy_address = NA;
                proc_table[proc_table_idx].map[STACK].mem_size = NA;
		proc_table[proc_table_idx].page_directory = kernel_page_directory;	
        }
	ix_component_idx = ix_component_idx + COMPONENT_INDEX_SIZE;

	for (i = 0; i < NUM_OF_SERVER_PROCS; i++, proc_table_idx++)
	{
                proc_table[proc_table_idx].eip = PROCESS_STARTING_VIR_ADDRESS;	// starting point of text segment
		setup_process_table_common(proc_table_idx);
                proc_table[proc_table_idx].esp = (int) server_stack_address[i] + SERVER_STACK_SIZE;
                //proc_table[proc_table_idx].esp = server_stack_address[i] + SERVER_STACK_BYTES;
                proc_table[proc_table_idx].esplimit = server_stack_address[i];        // array[0] is the top ... array[last] is the bottom

		proc_table[proc_table_idx].map[TEXT].offset = 0;
                proc_table[proc_table_idx].map[TEXT].phy_address = proc_table[proc_table_idx].begin_phy_address;
                proc_table[proc_table_idx].map[TEXT].mem_size = sizes[ix_component_idx + TEXT];
                
                proc_table[proc_table_idx].map[DATA].offset = sizes[ix_component_idx + TEXT];
                proc_table[proc_table_idx].map[DATA].phy_address = proc_table[proc_table_idx].map[TEXT].phy_address + proc_table[proc_table_idx].map[TEXT].mem_size;
                proc_table[proc_table_idx].map[DATA].mem_size = sizes[ix_component_idx + DATA];
                
                proc_table[proc_table_idx].map[STACK].offset = NA;	// there is no stack segment in server processes  since the stack is in bss
                proc_table[proc_table_idx].map[STACK].phy_address = NA;
                proc_table[proc_table_idx].map[STACK].mem_size = NA;
                //proc_table[proc_table_idx].eip = proc_table[proc_table_idx].map[TEXT].phy_address;	// starting point of text segment
		if (i == 0)
			proc_table[proc_table_idx].page_directory = mm_page_directory;	
		else
			proc_table[proc_table_idx].page_directory = fs_page_directory;	
		
		ix_component_idx = ix_component_idx + COMPONENT_INDEX_SIZE;
	}

	// init process setup
	proc_table[proc_table_idx].eip = PROCESS_STARTING_VIR_ADDRESS;	// starting point of text segment
	setup_process_table_common(proc_table_idx);

	proc_table[proc_table_idx].esp = PROCESS_STARTING_VIR_ADDRESS + (proc_table[proc_table_idx].end_phy_address - proc_table[proc_table_idx].begin_phy_address);
	proc_table[proc_table_idx].esplimit = proc_table[proc_table_idx].esp - INIT_STACK_SIZE;        // array[0] is the top ... array[last] is the bottom

	proc_table[proc_table_idx].map[TEXT].offset = 0;
	proc_table[proc_table_idx].map[TEXT].phy_address = proc_table[proc_table_idx].begin_phy_address;
	proc_table[proc_table_idx].map[TEXT].mem_size = sizes[ix_component_idx + TEXT];
	
	proc_table[proc_table_idx].map[DATA].offset = sizes[ix_component_idx + TEXT];
	proc_table[proc_table_idx].map[DATA].phy_address = proc_table[proc_table_idx].map[TEXT].phy_address + proc_table[proc_table_idx].map[TEXT].mem_size;
	proc_table[proc_table_idx].map[DATA].mem_size = sizes[ix_component_idx + DATA];
	
	proc_table[proc_table_idx].map[STACK].offset = NA;	// there is no stack segment in server processes  since the stack is in bss
	proc_table[proc_table_idx].map[STACK].phy_address = NA;
	proc_table[proc_table_idx].map[STACK].mem_size = NA;
	//proc_table[proc_table_idx].eip = proc_table[proc_table_idx].map[TEXT].phy_address;	// starting point of text segment
	proc_table[proc_table_idx].page_directory = init_page_directory;	
	proc_table_idx++;

	// user processes setup
	for (i = 0; i < NUM_OF_USR_PROCS; i++, proc_table_idx++)
	{
		proc_table[proc_table_idx].eip = 0;	// starting point of text segment
		setup_process_table_common(proc_table_idx);
		// init process setup
		proc_table[proc_table_idx].esp = 0;
		//proc_table[proc_table_idx].esp = server_stack_address[i] + SERVER_STACK_BYTES;
		proc_table[proc_table_idx].esplimit = 0;        // array[0] is the top ... array[last] is the bottom

		proc_table[proc_table_idx].map[TEXT].offset = 0;
		proc_table[proc_table_idx].map[TEXT].phy_address = proc_table[proc_table_idx].begin_phy_address;
		proc_table[proc_table_idx].map[TEXT].mem_size = 0; 
		
		proc_table[proc_table_idx].map[DATA].offset = 0;
		proc_table[proc_table_idx].map[DATA].phy_address = 0;
		proc_table[proc_table_idx].map[DATA].mem_size = 0;
		
		proc_table[proc_table_idx].map[STACK].offset = 0;	// there is no stack segment in server processes  since the stack is in bss
		proc_table[proc_table_idx].map[STACK].phy_address = 0;
		proc_table[proc_table_idx].map[STACK].mem_size = 0;
		//proc_table[proc_table_idx].eip = proc_table[proc_table_idx].map[TEXT].phy_address;	// starting point of text segment
		proc_table[proc_table_idx].page_directory = user_page_directories[i];	
	}


        // HARDWARE process: virtual processe for hardware processing 
	proc_table[proc_table_idx].eip = 0;	// starting point of text segment
	setup_process_table_common(proc_table_idx);
	proc_table[proc_table_idx].esp = 0;
	proc_table[proc_table_idx].esplimit = 0;        
	proc_table[proc_table_idx].map[TEXT].offset = 0;
	proc_table[proc_table_idx].map[TEXT].phy_address = 0;
	proc_table[proc_table_idx].map[TEXT].mem_size = 0; 
	proc_table[proc_table_idx].map[DATA].offset = 0;
	proc_table[proc_table_idx].map[DATA].phy_address = 0;
	proc_table[proc_table_idx].map[DATA].mem_size = 0;
	proc_table[proc_table_idx].map[STACK].offset = 0;
	proc_table[proc_table_idx].map[STACK].phy_address = 0;
	proc_table[proc_table_idx].map[STACK].mem_size = 0;
	proc_table[proc_table_idx].page_directory = 0;	
	proc_table[proc_table_idx].begin_phy_address = proc_table[1].begin_phy_address;
	proc_table[proc_table_idx].end_phy_address = proc_table[1].end_phy_address;
	// end of process table setup

	proc_table_address = (uint32_t) proc_table;	// for mm

	pick_proc();
	//enable_interrupts();
	run_proc();

	return 0;
}
