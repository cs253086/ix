#ifndef MEM_H
#define MEM_H

#include "../h/common.h"

#define NUM_OF_MEM_DEVICES	4

#define RAM_DISK	0
#define RAM_BEGIN_PHY_ADDRESS	0x70000
#define RAM_END_PHY_ADDRESS	0x8FFFF

#define DISK_READ	DEVICE_READ
#define DISK_WRITE	DEVICE_WRITE

int mem_task();

#endif	/* MEM_H */
