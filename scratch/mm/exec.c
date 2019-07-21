#include <stdint.h>
#include <std.h>
#include <errno.h>
#include "../h/common.h"
#include "common.h"
#include "exec.h"

//uint32_t elf_section_header_buf[SECTION_HEADER_SIZE / NUM_OF_SECTION_HEADER_MEMBER];
int mm_exec()
{
	int fd, i;
	int caller_pid = msg.src_pid;
	int offset, text_addr, text_offset, rodata_addr, rodata_offset, data_addr, data_offset, bss_addr, bss_offset, bss_size;
	uint32_t block_data_phy_address, caller_phy_address, remaining_size;
	char *filename = msg.character;
debug_printf("mm_exec: caller_pid: %d\n", caller_pid);
	/* open the file */
	fd = open(filename, 0);

	/* access to ELF section headers and extract necessary information to load those section into memory */
	read_section_header(fd, &text_addr, &text_offset, &rodata_addr, &rodata_offset, &data_addr, &data_offset, &bss_addr, &bss_offset, &bss_size);  
	block_data_phy_address = mm_proc_table[PID_MM].begin_phy_address + ((uint32_t) block_data - PROCESS_STARTING_VIR_ADDRESS);
	/* load .text */
	caller_phy_address = mm_proc_table[caller_pid].begin_phy_address + (text_addr - PROCESS_STARTING_VIR_ADDRESS);
	for (offset = text_offset; offset < rodata_offset - MAX_MESSAGE_CHAR_SIZE; offset += MAX_MESSAGE_CHAR_SIZE)
	{
		lseek(fd, offset, SEEK_SET);
		read(fd, block_data, MAX_MESSAGE_CHAR_SIZE);
		sys_copy(block_data_phy_address, caller_phy_address, MAX_MESSAGE_CHAR_SIZE);
		caller_phy_address += MAX_MESSAGE_CHAR_SIZE;
	}
	// last read and copy for the remaining data <= MAX_MESSAGE_CHAR_SIZE
	remaining_size = rodata_offset - offset;
	if (remaining_size > 0)
	{
		lseek(fd, offset, SEEK_SET);
		read(fd, block_data, remaining_size);
		sys_copy(block_data_phy_address, caller_phy_address, remaining_size);
	}
debug_printf("mm_exec: text end: %d\n", caller_pid);
	
	/* load .rodata */
	caller_phy_address = mm_proc_table[caller_pid].begin_phy_address + (rodata_addr - PROCESS_STARTING_VIR_ADDRESS);
	for (offset = rodata_offset; offset < data_offset - MAX_MESSAGE_CHAR_SIZE; offset += MAX_MESSAGE_CHAR_SIZE)
	{
		lseek(fd, offset, SEEK_SET);
		read(fd, block_data, MAX_MESSAGE_CHAR_SIZE);
		sys_copy(block_data_phy_address, caller_phy_address, MAX_MESSAGE_CHAR_SIZE);
		caller_phy_address += MAX_MESSAGE_CHAR_SIZE;
	}
	// last read and copy for the remaining data <= MAX_MESSAGE_CHAR_SIZE
	remaining_size = data_offset - offset;
	if (remaining_size > 0)
	{
		lseek(fd, offset, SEEK_SET);
		read(fd, block_data, remaining_size);
		sys_copy(block_data_phy_address, caller_phy_address, remaining_size);
	}
debug_printf("mm_exec: rodata end: %d\n", caller_pid);

	/* load .data */
	caller_phy_address = mm_proc_table[caller_pid].begin_phy_address + (data_addr - PROCESS_STARTING_VIR_ADDRESS);
	for (offset = data_offset; offset < bss_offset - MAX_MESSAGE_CHAR_SIZE; offset += MAX_MESSAGE_CHAR_SIZE)
	{
		lseek(fd, offset, SEEK_SET);
		read(fd, block_data, MAX_MESSAGE_CHAR_SIZE);
		sys_copy(block_data_phy_address, caller_phy_address, MAX_MESSAGE_CHAR_SIZE);
		caller_phy_address += MAX_MESSAGE_CHAR_SIZE;
	}
	// last read and copy for the remaining data <= MAX_MESSAGE_CHAR_SIZE
	remaining_size = bss_offset - offset;
	if (remaining_size > 0)
	{
		lseek(fd, offset, SEEK_SET);
		read(fd, block_data, remaining_size);
		sys_copy(block_data_phy_address, caller_phy_address, remaining_size);
	}

debug_printf("mm_exec: data end: %d\n", caller_pid);
	/* load .bss */
	// set block_data to 0's for .bss
	for (i = 0; i < MAX_MESSAGE_CHAR_SIZE; i++)
		block_data[i] = 0;
	caller_phy_address = mm_proc_table[caller_pid].begin_phy_address + (bss_addr - PROCESS_STARTING_VIR_ADDRESS);
	for (offset = bss_offset; offset < bss_offset + bss_size - MAX_MESSAGE_CHAR_SIZE; offset += MAX_MESSAGE_CHAR_SIZE)
	{
		sys_copy(block_data_phy_address, caller_phy_address, MAX_MESSAGE_CHAR_SIZE);
		caller_phy_address += MAX_MESSAGE_CHAR_SIZE;
	}
	// last read and copy for the remaining data <= MAX_MESSAGE_CHAR_SIZE
	remaining_size = bss_offset + bss_size - offset;
	if (remaining_size > 0)
	{
		sys_copy(block_data_phy_address, caller_phy_address, remaining_size);
	}
	
debug_printf("mm_exec: bss end: %d\n", caller_pid);
	sys_exec(caller_pid);
	/* close 'filename' */
	close(fd);
	return OK;
	//tell_fs(FORK, parent_pid, child_pid);
	/* read header to extract the segment sizes */
	/* refresh stack and copy the execution file to destination */
	
}

void read_section_header(int fd, int *text_addr, int *text_offset, int *rodata_addr, int *rodata_offset, int *data_addr, int *data_offset, int *bss_addr, int *bss_offset, int *bss_size)
{
	/* refer to ELF_Format.pdf 
	 * .shstrtab: this section holds sectin names and followed by the actual data of a section header
	 */
	uint32_t start_of_section_headers;
	uint32_t elf_section_header_buf[SECTION_HEADER_SIZE / sizeof(uint32_t)];
	// get position of section header from ELF header	
	start_of_section_headers = read_elf_header(fd, START_OF_SECTION_HEADERS_IN_ELF_HEADER);
	// lseek(); to locate section header (text)
	lseek(fd, start_of_section_headers + (SECTION_HEADER_SIZE * TEXT_IDX_IN_SECTION_HEADER_TABLE), SEEK_SET);
	/* read the section header */
	read(fd, (char *) elf_section_header_buf, SECTION_HEADER_SIZE);
	/* extract the arguments */
	*text_addr = elf_section_header_buf[SECTION_HEADER_ADDR_IDX];
	*text_offset = elf_section_header_buf[SECTION_HEADER_OFFSET_IDX];

	// lseek(); to locate section header (rodata)
	lseek(fd, start_of_section_headers + (SECTION_HEADER_SIZE * RODATA_IDX_IN_SECTION_HEADER_TABLE), SEEK_SET);
	/* read the section header */
	read(fd, (char *) elf_section_header_buf, SECTION_HEADER_SIZE);
	/* extract the arguments */
	*rodata_addr = elf_section_header_buf[SECTION_HEADER_ADDR_IDX];
	*rodata_offset = elf_section_header_buf[SECTION_HEADER_OFFSET_IDX];

	// lseek(); to locate section header (data)
	lseek(fd, start_of_section_headers + (SECTION_HEADER_SIZE * DATA_IDX_IN_SECTION_HEADER_TABLE), SEEK_SET);
	/* read the section header */
	read(fd, (char *) elf_section_header_buf, SECTION_HEADER_SIZE);
	/* extract the arguments */
	*data_addr = elf_section_header_buf[SECTION_HEADER_ADDR_IDX];
	*data_offset = elf_section_header_buf[SECTION_HEADER_OFFSET_IDX];

	// lseek(): to locate section header (bss)	
	lseek(fd, start_of_section_headers + (SECTION_HEADER_SIZE * BSS_IDX_IN_SECTION_HEADER_TABLE), SEEK_SET);
	/* read the section header */
	read(fd, (char *) elf_section_header_buf, SECTION_HEADER_SIZE);
	/* extract the arguments */
	*bss_addr = elf_section_header_buf[SECTION_HEADER_ADDR_IDX];
	*bss_offset = elf_section_header_buf[SECTION_HEADER_OFFSET_IDX];
	*bss_size = elf_section_header_buf[SECTION_HEADER_SIZE_IDX];
}

uint32_t read_elf_header(int fd, int elf_header_entry)
{
	char elf_header_buf[ELF_HEADER_SIZE];
	read(fd, elf_header_buf, ELF_HEADER_SIZE);
	return *(uint32_t *) (elf_header_buf + elf_header_entry);
}
