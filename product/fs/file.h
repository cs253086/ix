#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include "inode.h"

typedef struct file_descriptor {
  uint8_t filp_mode;          /* RW bits, telling how file is opened */
  int filp_count;               /* how many file descriptors share this slot? */
  struct inode *file_inode;       /* pointer to the inode */
  uint32_t file_pos;            /* file position */
} file_descriptor_t;

#endif	/* FILE_H */

