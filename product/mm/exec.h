#ifndef MM_EXEC_H
#define MM_EXEC_H

#define START_OF_SECTION_HEADERS_IN_ELF_HEADER	0x20	
	#define TEXT_IDX_IN_SECTION_HEADER_TABLE	1
	#define RODATA_IDX_IN_SECTION_HEADER_TABLE	2
	#define DATA_IDX_IN_SECTION_HEADER_TABLE	3
	#define BSS_IDX_IN_SECTION_HEADER_TABLE		4
#define SIZE_OF_SECTIOIN_HEADERS_IN_ELF_HEADER	0x2E

#define SECTION_HEADER_ADDR_IDX		3
#define SECTION_HEADER_OFFSET_IDX	4
#define SECTION_HEADER_SIZE_IDX		5

int mm_exec();
void read_section_header(int fd, int *text_addr, int *text_offset, int *rodata_addr, int *rodata_offset, int *data_addr, int *data_offset, int *bss_addr, int *bss_offset, int *bss_size);
uint32_t read_elf_header(int fd, int elf_header_entry);

#endif	/* MM_EXEC_H */
