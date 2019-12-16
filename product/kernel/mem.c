#include <stdint.h>
#include "../h/common.h"
#include <message.h>
#include <std.h>
#include "mem.h"

typedef struct mem_device {
	uint32_t begin_phy_address;
	uint32_t end_phy_address;	
} mem_device_t; 

static message_t msg;
static mem_device_t mem_devices[NUM_OF_MEM_DEVICES];

static int setup_ramdisk()
{
debug_printf("setup_ramdisk: started\n");
	mem_devices[RAM_DISK].begin_phy_address = RAM_BEGIN_PHY_ADDRESS;
	mem_devices[RAM_DISK].end_phy_address = RAM_END_PHY_ADDRESS;

	return 0;
}

static int read_write_mem(message_t *msg)
{
	int32_t pos_in_byte = msg->uinteger[POS_IDX_IN_MESSAGE];
	int msg_buf_address = msg->uinteger[BUF_ADDRESS_IDX_IN_MESSAGE];
	int msg_src_pid = msg->src_pid;
	int byte_count = msg->integer[SIZE_IDX_IN_MESSAGE];

	int32_t mem_phy_address = mem_devices[RAM_DISK].begin_phy_address + pos_in_byte;
	uint32_t msg_buffer_phy_address = vir_to_phy_address(msg_buf_address, msg_src_pid);

	if (msg->type == DISK_READ)
		phy_copy(mem_phy_address, msg_buffer_phy_address, byte_count); 
	else
		phy_copy(msg_buffer_phy_address, mem_phy_address, byte_count); 
		
	return 0;
}

int mem_task()
{
	int status;
	int opcode;
	while (1)
	{
		receive(ANY_PROC, &msg);
		opcode = msg.type;
		switch (opcode)
		{
			case DISK_IOCTL:
debug_printf("mem msg received: msg.src_pid=0x%x, msg.type=0x%x\n", msg.src_pid, msg.type);
				status = setup_ramdisk();
				break;

			case DISK_READ:
			case DISK_WRITE:
				status = read_write_mem(&msg);
				break;
		}
		msg.integer[0] = status;
		send(msg.src_pid, &msg);
	}
	
	return 0;
}
