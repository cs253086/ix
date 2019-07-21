#ifndef TELL_KERNEL_H
#define TELL_KERNEL_H

#include "message.h"

int tell_kernel_copy(uint32_t source_base, uint32_t dst_base, uint32_t size);
int tell_kernel_fork(int parent_pid, int child_pid);

#endif	/* TELL_KERNEL_H */
