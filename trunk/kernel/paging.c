#include <stdint.h>
#include "../h/common.h"
#include "common.h"
#include "paging.h"

#define KERNEL	0
#define MM	1
#define FS	2
#define INIT	3
#define USR	4

uint32_t *kernel_page_directory;	// initial page directory
uint32_t *kernel_page_table;

uint32_t *mm_page_directory;
uint32_t *mm_page_table;

uint32_t *fs_page_directory;
uint32_t *fs_page_table;

uint32_t *init_page_directory;
uint32_t *init_page_table;

uint32_t *user_page_directories[NUM_OF_USR_PROCS];	// excludes init
uint32_t *user_page_tables[NUM_OF_USR_PROCS];

static void setup_paging_kernel(uint32_t *page_table, int component)
{
	int i = 0;
	uint32_t kernel_phy_address = 0;

	if (component == 0)	// if it's kernel compoent, kernel has the whole physical memory access
	{
		for (i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++)
		{
			page_table[i] = kernel_phy_address | READ_WRITE | PRESENT;	
			kernel_phy_address += PAGE_SIZE;
		}
	}
	else
	{
		for (i = 0; i < KERNEL_MAX_IN_PAGE_SIZE; i++)
		{
			page_table[i] = kernel_phy_address | READ_WRITE | PRESENT;	
			kernel_phy_address += PAGE_SIZE;
		}
	}
}

static void setup_paging_system_memory(uint32_t *page_table, int *i)
{
	uint32_t phy_address = BEGIN_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY;
	for (; *i < END_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY / PAGE_SIZE; (*i)++)
	{
		page_table[*i] = phy_address | READ_WRITE | PRESENT;	
		phy_address += PAGE_SIZE;
	}
		
}

static void setup_paging_component(uint32_t *page_directory, uint32_t *page_table, int component)
{
	int i;
	static uint32_t server_phy_address = KERNEL_MAX_IN_PAGE_SIZE * PAGE_SIZE;
	static uint32_t usr_phy_address = 0x100000;
	static int proc_table_idx = PID_INIT + 1;

	//total_kernel_size = sizes[0 + TEXT] + sizes[0 + DATA] + KERNEL_HEAD_ADDRESS;
	//total_kernel_pages = total_kernel_size % PAGE_SIZE ? (total_kernel_size / PAGE_SIZE) + 1 : total_kernel_size / PAGE_SIZE;

	for (i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++)
		page_directory[i] = READ_WRITE | NOT_PRESENT;
	/* every component needs memory spacing for kernel since, for example, switch_paging is inside kernel
	 * set aside the maximum memory space for kernel
	 */
	if (component == 0)	//kernel
	{
		i = 0;
		setup_paging_kernel(page_table, component);

		/*for (; i < BEGIN_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY / PAGE_SIZE; i++)
			page_table[i] = READ_WRITE | NOT_PRESENT;
		setup_paging_system_memory(page_table, &i);
		for (; i < PAGE_SIZE / sizeof(uint32_t); i++)
			page_table[i] = READ_WRITE | NOT_PRESENT;*/
		page_directory[0] = (uint32_t) page_table;
		page_directory[0] |= PRESENT;

		for (i = 1; i < NUM_OF_TASKS + 1; i++)	// 1 is for idle process
		{
			proc_table[i].begin_phy_address = 0;	
			proc_table[i].end_phy_address = PAGE_SIZE * (PAGE_SIZE / sizeof(uint32_t));	
		}
debug_printf("page frame from %x to %x, ", proc_table[1].begin_phy_address, proc_table[1].end_phy_address);
	}
	else if (0 < component && component < USR)	//mm, fs, init
	{
		uint32_t total_component_pages, total_component_size;

		i = 0;
		total_component_size = sizes[(component * 2) + TEXT] + sizes[(component * 2) + DATA];
		total_component_pages = total_component_size % PAGE_SIZE ? (total_component_size / PAGE_SIZE) + 1 : total_component_size / PAGE_SIZE;

		setup_paging_kernel(page_table, component);
		proc_table[NUM_OF_TASKS + component].begin_phy_address = server_phy_address;

		for (i = KERNEL_MAX_IN_PAGE_SIZE; i < total_component_pages + KERNEL_MAX_IN_PAGE_SIZE; i++)
		{
			page_table[i] = server_phy_address | READ_WRITE | PRESENT;	
			server_phy_address += PAGE_SIZE;
		}
		for (; i < BEGIN_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY / PAGE_SIZE; i++)
			page_table[i] = READ_WRITE | NOT_PRESENT;
		proc_table[NUM_OF_TASKS + component].end_phy_address = server_phy_address;

		setup_paging_system_memory(page_table, &i);
		for (;i < PAGE_SIZE / sizeof(uint32_t); i++)
        		page_table[i] = READ_WRITE | NOT_PRESENT;
debug_printf("page frame from %x to %x, ", proc_table[NUM_OF_TASKS + component].begin_phy_address, proc_table[NUM_OF_TASKS + component].end_phy_address);
	}
	else
	{
		i = 0;
		setup_paging_kernel(page_table, component);
		proc_table[proc_table_idx].begin_phy_address = usr_phy_address;

		for (i = KERNEL_MAX_IN_PAGE_SIZE; i < USR_MAX_IN_PAGE_SIZE + KERNEL_MAX_IN_PAGE_SIZE; i++)
		{
			page_table[i] = usr_phy_address | READ_WRITE | PRESENT;	
			usr_phy_address += PAGE_SIZE;
		}
		proc_table[proc_table_idx].end_phy_address = usr_phy_address;

		for (; i < BEGIN_ADDRESS_OF_REAL_MODE_SYSTEM_MEMORY / PAGE_SIZE; i++)
			page_table[i] = READ_WRITE | NOT_PRESENT;

		setup_paging_system_memory(page_table, &i);
		for (;i < PAGE_SIZE / sizeof(uint32_t); i++)
        		page_table[i] = READ_WRITE | NOT_PRESENT;
debug_printf("page frame from %x to %x, ", proc_table[proc_table_idx].begin_phy_address, proc_table[proc_table_idx].end_phy_address);
		proc_table_idx++;
	}
debug_printf("component idx:%d page directory address=0x%x, page table address=0x%x\n", component, page_directory, page_table);
	page_directory[0] = (uint32_t) page_table;
	page_directory[0] |= PRESENT;
}

void setup_paging()
{
	int i;
	// kernel paging setup
	kernel_page_directory = (uint32_t *) kernel_directory_address;
	kernel_page_table = kernel_page_directory + NUM_PAGING_ENTRIES;
	setup_paging_component(kernel_page_directory, kernel_page_table, KERNEL);
	// mm paging setup	
	mm_page_directory = (uint32_t *) kernel_page_table + NUM_PAGING_ENTRIES;
	mm_page_table = mm_page_directory + NUM_PAGING_ENTRIES;
	setup_paging_component(mm_page_directory, mm_page_table, MM);

	// fs paging setup
	fs_page_directory = (uint32_t *) mm_page_table + NUM_PAGING_ENTRIES;
	fs_page_table = fs_page_directory + NUM_PAGING_ENTRIES;
	setup_paging_component(fs_page_directory, fs_page_table, FS);

	// init paging setup
	init_page_directory = (uint32_t *) fs_page_table + NUM_PAGING_ENTRIES;
	init_page_table = init_page_directory + NUM_PAGING_ENTRIES;
	setup_paging_component(init_page_directory, init_page_table, INIT);

	user_page_directories[0] = (uint32_t *) init_page_table + NUM_PAGING_ENTRIES;
	user_page_tables[0] = user_page_directories[0] + NUM_PAGING_ENTRIES;
	setup_paging_component(user_page_directories[0], user_page_tables[0], USR);

	for (i = 1; i < NUM_OF_USR_PROCS; i++)
	{
		user_page_directories[i] = (uint32_t *) user_page_tables[i - 1] + NUM_PAGING_ENTRIES;
		user_page_tables[i] = user_page_directories[i] + NUM_PAGING_ENTRIES;
		setup_paging_component(user_page_directories[i], user_page_tables[i], USR);
	}
	switch_paging(kernel_page_directory);
}



