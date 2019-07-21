#include <std.h>

#define BLOCK_SIZE	1024
#define MAX_NAME_SIZE	30	// this should be same as one in fs/common.h
// this struct must be same as one in fs/common.h
typedef struct {                /* directory entry */
  uint16_t inode_num;              /* inode number */
  char entry_name[MAX_NAME_SIZE];       /* character string */
} dir_entry_t;


#define SIZE_OF_DIR_ENTRY	32	// dir_entry_t in fs
#define	MAX_DIR_ENTRIES	(BLOCK_SIZE / sizeof(dir_entry_t))

dir_entry_t dir_entry[MAX_DIR_ENTRIES];

int main()
{
	int i = 0, fd;
	fd = open("", 0);	// if you pass "", it opens the current working directory
	lseek(fd, 0, SEEK_SET);	// reset the position of file descriptor

	do {
		read(fd, (char *) &dir_entry[i], sizeof(dir_entry_t));
		// TODO: use fprintf(stdout, ...)
		printf("%s\n", dir_entry[i].entry_name);
	} while (dir_entry[i++].inode_num != 0);
	// extract all the dir element names and print them at once using on printf()
	exit(0);
}
