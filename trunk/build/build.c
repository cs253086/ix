#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <stdint.h>
#include "../h/common.h"

#define KERNEL_COMMAND_IDX	2
#define MM_COMMAND_IDX	3
#define BOOTBLOCK_SIZE	512
#define SECTOR_SIZE	512

int image;  // disk file descriptor

// These sizes mean actual size + padding for alignment
uint32_t text_size;	// text section + rodata section
//int rodata_size;
uint32_t data_size;	// data section + bss section
//int bss_size;

uint32_t server_stack_address[NUM_OF_SERVER_PROCS];
//uint32_t init_stack_address;
uint32_t kernel_directory_address;
uint32_t cur_component_offset_in_memory = KERNEL_HEAD_ADDRESS;
uint32_t mm_gap_to_page;
uint32_t total_gap_to_pages;

void create_image(char *file)
{
    image = creat(file, 0666);	
}

void copy_bootblock(char *bootblock)
{
	char *buf;
	ssize_t fd;
	int bytes_read;
	off_t bootblock_size;
	struct stat st;

	// retrieve the size of bootblock
	stat(bootblock, &st);
	bootblock_size = st.st_size;

	buf = (char *) malloc(bootblock_size);

	// open bootblock
	if ((fd = open(bootblock, O_RDONLY)) < 0)
		err(EXIT_FAILURE, "Can't open %s", bootblock);

	// read bootblock and put into buf and write to image
	bytes_read = read(fd, buf, bootblock_size);
	if (bytes_read < 0)
		err(EXIT_FAILURE, "read error on %s", bootblock);
	else
		write(image, buf, bytes_read);
	close(fd); 
}

/*
 * perform page alignment up to 'align_phy_address'
 * if 'align_phy_address' == 0, then align to the next page
 */
uint32_t align_paging(uint32_t align_phy_address)
{
	/* it measure the gap to the next page and fill them with 0s */
	uint8_t *buf;
	uint32_t gap_to_page;
	if (align_phy_address == 0)
		gap_to_page = PAGE_SIZE - (cur_component_offset_in_memory % PAGE_SIZE);
	else
	{
		if (align_phy_address % PAGE_SIZE != 0)
			return -1;
		else
			gap_to_page = align_phy_address - cur_component_offset_in_memory;
			
	}
	buf = (uint8_t *) calloc(gap_to_page, sizeof(uint8_t));
	write(image, buf, gap_to_page);
	cur_component_offset_in_memory += gap_to_page;	
	return gap_to_page;
}
void write_section(int fd, Elf *elf_handle, Elf_Scn *scn_cur, GElf_Shdr shdr_cur, char* section_name, int *section_size)
{
	uint8_t *buf;
	off_t offset;
	Elf_Scn *scn_next;
	GElf_Shdr shdr_next;
	int elf_section_size = 0;
	size_t section_idx;

	section_idx = elf_ndxscn(scn_cur);	
	gelf_getshdr(scn_cur, &shdr_cur);
	scn_next = elf_getscn(elf_handle, section_idx + 1);
	gelf_getshdr(scn_next, &shdr_next);
	if (shdr_next.sh_addr != 0)	// if scn_cur is the last one containing content
		elf_section_size = shdr_next.sh_addr- shdr_cur.sh_addr;	// we consider not only size itself also alignment size
		// please note that (.bss).sh_addr - (.data).sh_addr != (.bss).sh_offset - (.data).sh_offset
	else
		elf_section_size = shdr_cur.sh_size;
	*section_size += elf_section_size;	 
	offset = lseek(fd, 0, SEEK_SET);        // reposition the file pointer to offset 0
	if (offset != 0)
		err(EXIT_FAILURE, "Return value(offset) of lseek() is NOT correct.");
	 
	offset = lseek(fd, shdr_cur.sh_offset, SEEK_SET);       // seek the section
	if (offset != shdr_cur.sh_offset)
		err(EXIT_FAILURE, "Return value(offset) of lseek() is NOT correct.");

	// read the section and copy into buf
	buf = (uint8_t *) calloc(elf_section_size, sizeof(uint8_t));
	if (strcmp(section_name, ".bss") != 0)
		read(fd, buf, elf_section_size);
	// write the section to OS image
	write(image, buf, elf_section_size);
	printf("%s: size(0x%x)\n", section_name, elf_section_size);
}

/*
 *  Copy OS components to OS image.
 *  All components are expected to be ELF file format
 */

void copy_component(char *file)
{
	/* ELF library initialization */
	ssize_t fd;
    	char *name;
    	Elf *elf_handle;
	Elf_Scn *scn_cur = NULL;
	GElf_Shdr shdr_cur;
    	size_t shstrndx;
	char *cur_section_name;
	off_t offset;

    	if (elf_version(EV_CURRENT) == EV_NONE)
        	err(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));

    	if ((fd = open(file, O_RDONLY)) < 0)
        	err(EXIT_FAILURE, "Can't open %s", file);

    	if ((elf_handle = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
        	err(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));

    	if (elf_kind(elf_handle) != ELF_K_ELF)
        	err(EXIT_FAILURE, "%s is not ELF object.", file);

    	if (elf_getshdrstrndx (elf_handle, &shstrndx) != 0) // section index of .shstrtab, which contains section names
        	err(EXIT_FAILURE , " elf_getshdrstrndx ()  failed : %s.", elf_errmsg ( -1));

	while ((scn_cur = elf_nextscn(elf_handle, scn_cur)) != NULL)
	{
		gelf_getshdr(scn_cur, &shdr_cur);
		cur_section_name = elf_strptr(elf_handle, shstrndx, shdr_cur.sh_name); 
		if (strcmp(cur_section_name, ".text") == 0)
		{
			if (strcmp(file, "kernel/kernel") != 0)
			{
				if (strcmp(file, "mm/mm") == 0)
				{
					mm_gap_to_page = align_paging(PROCESS_STARTING_VIR_ADDRESS);
					total_gap_to_pages += mm_gap_to_page; 
				}
				else
					total_gap_to_pages += align_paging(0);
				printf("cur offset in memory: 0x%x\n", cur_component_offset_in_memory);
			}
			/* Get .text offset and size and write .text to OS image */
			write_section(fd, elf_handle, scn_cur, shdr_cur, ".text", &text_size);
		}
		else if (strcmp(cur_section_name, ".rodata") == 0)
		{
			/* Get .rodata offset and size and write .rodata to OS image */
			write_section(fd, elf_handle, scn_cur, shdr_cur, ".rodata", &text_size);

		}
		else if (strcmp(cur_section_name, ".data") == 0)
		{
			/* Get .data offset and size and write .data to OS image */
			write_section(fd, elf_handle, scn_cur, shdr_cur, ".data", &data_size);
		}
		else if (strcmp(cur_section_name, ".bss") == 0)
		{
			/* Get .bss size, we don't need to write bss section since it's not present in executable file */
			write_section(fd, elf_handle, scn_cur, shdr_cur, ".bss", &data_size);
			if (strcmp(file, "mm/mm") == 0)
				server_stack_address[0] = (uint32_t) shdr_cur.sh_addr;
			else if (strcmp(file, "fs/fs") == 0)
				server_stack_address[1] = (uint32_t) shdr_cur.sh_addr;
			//else if (strcmp(file, "init/init") == 0)
				//init_stack_address = (uint32_t) shdr_cur.sh_addr;
		}

	}
	close(fd);

}

void patch_os_size_to_bootblock(uint16_t *os_size_sector)
{
	off_t os_size_offset = (off_t) (BOOTBLOCK_SIZE - 4);	// 4 means 0xaa55, os_size_sector
	off_t offset = lseek(image, os_size_offset, SEEK_SET);
	write(image, os_size_sector, sizeof(uint16_t)); 
	
}
/*
 * patch text and data size of each program at the beginning address of kernel .data section
 */

void patch_to_kernel(uint32_t *sizes, int array_size)
{
	int i;
	off_t kernel_data_offset = (off_t) (BOOTBLOCK_SIZE + sizes[KERNEL_COMMAND_IDX - 2]);	// 2 means build and bootloader
	off_t offset = lseek(image, kernel_data_offset, SEEK_SET);
	for (i = 0; i < array_size; i++)
		write(image, (sizes + i), sizeof(uint32_t));
	write(image, server_stack_address, sizeof(server_stack_address));
	//write(image, &init_stack_address, sizeof(uint32_t));
	write(image, &kernel_directory_address, sizeof(uint32_t));
}

void patch_sizes_to_mm(uint32_t *sizes, int array_size)
{
	int i;
	off_t mm_data_offset = (off_t) (BOOTBLOCK_SIZE + sizes[KERNEL_COMMAND_IDX - 2] + sizes[KERNEL_COMMAND_IDX - 2 + 1] + mm_gap_to_page + sizes[(MM_COMMAND_IDX - 2) * 2]);	// 2 means build and bootloader
	off_t offset = lseek(image, mm_data_offset, SEEK_SET);
	for (i = 0; i < array_size; i++)
		write(image, (sizes + i), sizeof(uint32_t));
}

void close_image(void)
{
	close(image);	
}

int main(int argc, char *argv[])
{
	int i; 
	uint32_t os_size_byte = 0;
	uint16_t os_size_sector = 0;	// total os size (# sectors) except bootblock
	uint32_t section_sizes[((argc - 1) - KERNEL_COMMAND_IDX) * 2];	// *2 because text_size and data_size

	/* TODO: create usage message with --help */
	/* create the output file  */
	create_image(argv[argc - 1]);
	
	/* copy the bootblok and os components to the os image file */
	copy_bootblock(argv[1]);
	for (i = KERNEL_COMMAND_IDX; i < (argc - 1); i++)
	{
		printf("[%s]\n", argv[i]);
		copy_component(argv[i]);
		section_sizes[(i - KERNEL_COMMAND_IDX) * 2] = text_size;
		section_sizes[(i - KERNEL_COMMAND_IDX) * 2 + 1] = data_size;
		cur_component_offset_in_memory += (text_size + data_size);
		text_size = 0;
		data_size = 0;
	}
		
	/* copy text and data sizes to kernel so that it can use them for process table setup */
	for (i = 0; i < sizeof(section_sizes) / sizeof(uint32_t); i++)
		os_size_byte += section_sizes[i];
	
	printf("Total gap: 0x%x\n", total_gap_to_pages);
	os_size_byte += total_gap_to_pages;
	printf("Total OS size in byte(s): 0x%x\n", os_size_byte);
	kernel_directory_address = (KERNEL_HEAD_ADDRESS + os_size_byte + PAGE_SIZE) & PAGE_ALIGN_FLAG;
	printf("Page directory address: 0x%x\n", kernel_directory_address);
	os_size_sector = (os_size_byte / SECTOR_SIZE) + 1;
	printf("Total OS size in sector(s): 0x%x\n", os_size_sector);

	printf("mm_stack_address = 0x%x (in virtual address)\n", server_stack_address[0]);
	printf("fs_stack_address = 0x%x (in virtual address)\n", server_stack_address[1]);
	//printf("init_stack_address = 0x%x (in virtual address)\n", init_stack_address);

	patch_os_size_to_bootblock(&os_size_sector);
	patch_to_kernel(section_sizes, sizeof(section_sizes) / sizeof(uint32_t));
	patch_sizes_to_mm(section_sizes, sizeof(section_sizes) / sizeof(uint32_t));

	close_image();   
 
    return EXIT_SUCCESS;
}

