#include <stdint.h>
#include <stdlib.h>
#include "../h/common.h"
#include "common.h"
#include "alloc.h"

static struct process_hole {
	mm_proc_t *mm_proc;
	struct process_hole *next;
} process_holes[NUM_OF_USR_PROCS];

/* memory allocation of a process is achieved by linked list */
struct process_hole *head_process_hole = NULL;	// first available process hole can be allocated for a process	
struct process_hole *tail_process_hole = NULL;	// you can attach the next of taile process when you free a process 	

void debug_print_process_holes()
{
	struct process_hole *process_hole_p = head_process_hole;	
	debug_printf("process holes in order: ");
	while (process_hole_p != NULL)
	{
		debug_printf("[%d](0x%x:0x%x)->", process_hole_p->mm_proc->pid, process_hole_p->mm_proc->begin_phy_address, process_hole_p->mm_proc->end_phy_address);
		process_hole_p = process_hole_p->next;
		
	}
	debug_printf("null\n");	
}

struct process_hole *del_first()
{
	struct process_hole *ret = NULL;
	if (head_process_hole != NULL)
	{
		ret = head_process_hole;
		head_process_hole = head_process_hole->next;
		if (head_process_hole == NULL) // if there is no hole
			tail_process_hole = NULL;
	}
	return ret;
}

void add_last(struct process_hole *hole_p)
{
	if (head_process_hole == NULL)
	{
		head_process_hole = hole_p;
		tail_process_hole = hole_p;
	}
	else
	{
		tail_process_hole->next = hole_p;
		tail_process_hole = hole_p;
	}	
	tail_process_hole->next = NULL;
}

mm_proc_t *allocate_process_hole()
{
	struct process_hole *hole_p;
	if ((hole_p = del_first()) != NULL)
		return hole_p->mm_proc;	
	else
		return NULL;
}

void init_mem(uint32_t mem_size)
{
	int i;

	for (i = 0; i < NUM_OF_USR_PROCS; i++)
	{
		process_holes[i].mm_proc = &mm_proc_table[PID_INIT + 1 + i];
		add_last(&process_holes[i]);
	}
	
debug_print_process_holes();
}
