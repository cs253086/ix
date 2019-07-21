#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

typedef struct dev_map {
	int (*open)();
	int (*rw)();
	int (*close)();
	int task;
} dev_map_t;

int dev_io(int flag, int dev, uint32_t pos, int size_in_byte, int pid, char *buf);
int rw_dev(int task, message_t *msg);
int no_call(int task, message_t *msg);

#endif /* DEVICE_H */
