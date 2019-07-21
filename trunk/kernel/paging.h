#ifndef PAGING_H
#define PAGING_H

#define NOT_PRESENT	0
#define PRESENT		1
#define READ_WRITE	2

#define NUM_PAGING_ENTRIES	1024

extern uint32_t *kernel_page_directory;
extern uint32_t *mm_page_directory;
extern uint32_t *fs_page_directory;
extern uint32_t *init_page_directory;
extern uint32_t *user_page_directories[NUM_OF_USR_PROCS];	// excludes init
void setup_paging();

#endif /* PAGING_H */
