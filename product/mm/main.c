#include <stdint.h>
#include <message.h>
#include <std.h>
#include "../h/util.h"
#include <errno.h>
#include "../h/common.h"
#include "common.h"
#include "forkexit.h"
#include "exec.h"

#define MEM_SIZE_OFFSET_IN_BOOTLOADER	440	
#define	BOOTLOADER_LOAD_ADDR	0x7c00

#define KBYTES		1024
#define KERNEL_PROC_SIZE	156	// bytes
#define NUM_OF_MMCALLS	69

#define MAP_OFFSET	80
#define PID_OFFSET	MAP_OFFSET + (sizeof(mem_map_t) * NR_SEGS) + 4
#define PAGE_DIRECTORY_OFFSET	68
#define BEGIN_PHY_ADDRESS_OFFSET	72
#define END_PHY_ADDRESS_OFFSET		76
	
uint32_t sizes[BUILD_PATCH_TABLE_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1};	// if it's initialized as 0's, it goes to .bss instead of .data
//uint32_t mm_stack[SERVER_STACK_SIZE / sizeof(uint32_t)];
message_t msg;
int msg_src_pid;
int dont_reply = 0;
int status;
uint32_t total_mem;	// total available memory in kb
uint32_t kernel_proc_table_address;
mm_proc_t mm_proc_table[MAXIMUM_PROCS];
int procs_in_use; 
char block_data[MAX_MESSAGE_CHAR_SIZE];	// block_data is limited to the message char size

static void init_mm();

int (*call_vec[NUM_OF_MMCALLS])() = {
	NULL,
	mm_exit,	/* 1  = exit */
	mm_fork,	/* 2 = fork */
	NULL,		/* 3 */
	NULL,		/* 4 */
	NULL,		/* 5 */
	NULL,		/* 6 */
	mm_wait,	/* 7 = wait */
	NULL,		/* 8 */
	NULL,		/* 9 */
	NULL,		/* 10 */
	NULL,		/* 11 */
	NULL,		/* 12 */
	NULL,		/* 13 */
	NULL,		/* 14 */
	NULL,		/* 15 */
	NULL,		/* 16 */
	NULL,		/* 17 */
	NULL,		/* 18 */
	NULL,		/* 19 */
	NULL,		/* 20 */
	NULL,		/* 21 */
	NULL,		/* 22 */
	NULL,		/* 23 */
	NULL,		/* 24 */
	NULL,		/* 25 */
	NULL,		/* 26 */
	NULL,		/* 27 */
	NULL,		/* 28 */
	NULL,		/* 29 */
	NULL,		/* 30 */
	NULL,		/* 31 */
	NULL,		/* 32 */
	NULL,		/* 33 */
	NULL,		/* 34 */
	NULL,		/* 35 */
	NULL,		/* 36 */
	NULL,		/* 37 */
	NULL,		/* 38 */
	NULL,		/* 39 */
	NULL,		/* 40 */
	NULL,		/* 41 */
	NULL,		/* 42 */
	NULL,		/* 43 */
	NULL,		/* 44 */
	NULL,		/* 45 */
	NULL,		/* 46 */
	NULL,		/* 47 */
	NULL,		/* 48 */
	NULL,		/* 49 */
	NULL,		/* 50 */
	NULL,		/* 51 */
	NULL,		/* 52 */
	NULL,		/* 53 */
	NULL,		/* 54 */
	NULL,		/* 55 */
	NULL,		/* 56 */
	NULL,		/* 57 */
	NULL,		/* 58 */
	mm_exec,	/* 59 = exec */
};

// since process table for mm points to address 0x0, main function should be the first in text of mm
int main(int argc, char *argv[])
{
	int mm_call;

	init_mm();
	while (1)
	{
		receive(ANY_PROC, &msg);
		mm_call = msg.type;
debug_printf("mm: msg received mm_call: %d\n", mm_call);
		msg_src_pid = msg.src_pid;

		dont_reply = 0;
		status = 0;

		if (mm_call < 0 || mm_call >= NUM_OF_MMCALLS)
			status = E_BAD_CALL;
		else
			status = (*call_vec[mm_call])();

		if (dont_reply) continue;
		if (mm_call == EXEC && status == OK) continue;	// if it's an error, it needs to reply with the error msg, if it's ok that measn the caller has been replaced with complete new program, so don't need to reply
		msg.integer[0] = status;
		send(msg_src_pid, &msg);			
	}
	
	return 0;
}

void setup_mm_proc_table_from_kernel_proc_table()
{
	int i, j;
	char *kernel_proc_table = (char *) kernel_proc_table_address;
	for (i = 0; i < MAXIMUM_PROCS; i++)
	{
		char *src_proc_table;;
		char *dst_proc_table;
		// clone map[NR_SEGS]
		src_proc_table = kernel_proc_table + MAP_OFFSET;
		dst_proc_table = (char *) &mm_proc_table[i].map;
		for (j = 0; j < (sizeof(mem_map_t) * NR_SEGS); j++)
			*(dst_proc_table++) = *(src_proc_table++);

		// clone pid
		src_proc_table = kernel_proc_table + PID_OFFSET;
		dst_proc_table =(char *) &mm_proc_table[i].pid;
		for (j = 0; j < sizeof(int); j++)
			*(dst_proc_table++) = *(src_proc_table++);
		
		// clone page directory
		src_proc_table = kernel_proc_table + PAGE_DIRECTORY_OFFSET;
		dst_proc_table =(char *) &mm_proc_table[i].page_directory;
		for (j = 0; j < sizeof(uint32_t); j++)
			*(dst_proc_table++) = *(src_proc_table++);

		// begin_phy_address
		src_proc_table = kernel_proc_table + BEGIN_PHY_ADDRESS_OFFSET;
		dst_proc_table =(char *) &mm_proc_table[i].begin_phy_address;
		for (j = 0; j < sizeof(uint32_t); j++)
			*(dst_proc_table++) = *(src_proc_table++);

		// end_phy_address
		src_proc_table = kernel_proc_table + END_PHY_ADDRESS_OFFSET;
		dst_proc_table =(char *) &mm_proc_table[i].end_phy_address;
		for (j = 0; j < sizeof(uint32_t); j++)
			*(dst_proc_table++) = *(src_proc_table++);
		
		kernel_proc_table += KERNEL_PROC_SIZE;	// next kernel process table
	}
}

static void init_mm()
{
	int i;
	//get the toal memory size patched from bootloader 
	uint32_t *mem_size_addr = (uint32_t *) (BOOTLOADER_LOAD_ADDR + MEM_SIZE_OFFSET_IN_BOOTLOADER);
	total_mem = *(mem_size_addr) * KBYTES;
debug_printf("total memory size: 0x%x\n", total_mem);

	// Get process table info from kernel and set up mm proc table based on it
	kernel_proc_table_address = *((uint32_t *) (KERNEL_HEAD_ADDRESS + sizes[KERNEL_IDX * 2 + TEXT] + KERNEL_PROC_TABLE_ADDRESS_OFFSET));
	setup_mm_proc_table_from_kernel_proc_table();

	mm_proc_table[PID_IDLE].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[PID_CLOCK].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[PID_TTY].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[PID_SYS].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[PID_MM].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[PID_FS].flags |= IN_USE;
	procs_in_use++;
	mm_proc_table[PID_INIT].flags |= IN_USE;
	procs_in_use++;

	// initialize memory suitable for memory management
	init_mem(total_mem);
}


