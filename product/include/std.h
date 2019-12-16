#ifndef STD_H
#define STD_H

#include <message.h>

#if !defined(NULL)
    #define NULL ((void*)0)
#endif

/* msg types for system calls */
#define EXIT		1
#define FORK		2
#define READ		3
#define WRITE		4
#define OPEN		5
#define CLOSE		6
#define WAIT		7
#define LSEEK		19	
	#define SEEK_SET	0
	#define SEEK_CUR	1
	#define SEEK_END	2
#define EXEC	59	

/* others msg types */
#define DISK_IOCTL	5
#define REPLY		9	// general reply
#define FORK_REPLY_TO_PARENT		10
#define FORK_REPLY_TO_CHILD	11

#define EOF     (-1)

#define NFILES  20
#define READMODE     1
#define WRITEMODE    2
#define UNBUFF       4
#define _EOF         8
#define _ERR        16
#define IOMYBUF     32
#define PERPRINTF   64
#define STRINGS    128


struct _io_buf {
    int     _fd;	// file descriptor 
    int     _count;	// number of characters available in buffer
    int     _flags;	// the state of the FILE
    char   *_buf;	// buffer
    char   *_ptr;	// next character from/to here in buffer
};

extern struct _io_buf *_io_table[NFILES];
#define FILE struct _io_buf
#define STD_IN_OUT_BUF_SIZE	MAX_MESSAGE_CHAR_SIZE

#define stdin  (_io_table[0])   
#define stdout  (_io_table[1])
#define stderr  (_io_table[2])
#define testflag(p,x)           ((p)->_flags & (x))

/* used in getc.c */
#define CMASK   0377

/* system calls */
int fork();
int open(const char *pathname, int flags);
int exec(const char *pathname);
int read(int fd, char *buf, int size);
//int write(int fd, char *buf, int size);

void send(int dst, message_t *mp);
void receive(int src, message_t *mp);
void sendrec(int src_dst, message_t *mp);

/* standard utility calls */
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strlen(const char *s);    
char *fgets(char *s, int size, FILE *stream);
int putc(char ch, FILE *stream);
int printf(const char *s, ...);

#endif	/* STD_H */
