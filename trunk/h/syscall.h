#ifndef SYSCALL_H
#define SYSCALL_H

#include "message.h"

int fork();
void send(int dst, message_t *mp);
void receive(int src, message_t *mp);
void sendrec(int src_dst, message_t *mp);

#endif	/* SYSCALL_H */
