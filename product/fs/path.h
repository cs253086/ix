#ifndef PATH_H
#define PATH_H

inode_t *get_inode_from_path(char *path);
void find_block_for_inode_with_position(inode_t *inode_p, buf_t *inode_block, uint32_t position, int data_type);
int search_dir(char *file_name, buf_t *inode_block);

#endif	/* PATH_H */
