#ifndef LIBSYS_H
#define LIBSYS_H

#include <message.h>

int sys_copy(uint32_t source_base, uint32_t dst_base, uint32_t size);
int sys_fork(int parent_pid, int child_pid);

#endif	/* LIBSYS_H */
