#ifndef FS_LIB_H
#define FS_LIB_H

void get_block(int dev, int block_num, buf_t *bp, int data_type);
void put_block(int dev, int block_num, buf_t *bp, int data_type);
void rw_block(buf_t *bp, int flag, int data_type);
void suspend_reply_to(int pid, int fd, char *buf, int nbytes, int task);
int fs_revive_reply();
#endif /* FS_LIB_H */
