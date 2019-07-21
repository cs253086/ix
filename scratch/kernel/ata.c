#include <message.h>
#include <errno.h>
#include <std.h>
#include "../h/util.h"
#include "common.h"
#include "kernel_lib.h"
#include "irq.h"
#include "ata.h"

static struct ata {
	int io_address;
	int slave;	// slave = 1, master = 0
	int present;

	// members for ata request
	int opcode;
	uint8_t sector_count;	
	uint8_t lba_low;	// 0 based
	uint8_t lba_mid;	// 0 based
	uint8_t lba_high;	// 0 based
	
} ata[NUM_OF_ATA_DEVICES];

uint16_t ata_buffer[ATA_BUFFER_SIZE_IN_16BIT];
static message_t msg;

static void select_ide_drive(uint16_t bus_io, uint8_t selec_drive_command)
{
	outb(bus_io + ATA_REG7, selec_drive_command);
}

static void read_from_ata_controller(uint16_t bus_io)
{
	int i;
	for (i = 0; i < ATA_BUFFER_SIZE_IN_16BIT; i++)
		ata_buffer[i] = inw(bus_io + ATA_REG1);
}

// check if the drive exists
static uint8_t identify_ide(int ata_idx)
{
	uint8_t status = 0;
	int i = 0;
	uint16_t bus_io = ata[ata_idx].io_address;
	uint8_t select_drive_command = ata[ata_idx].slave ? ATA_COMMAND_SELECT_SLAVE_DRIVE : ATA_COMMAND_SELECT_MASTER_DRIVE;

	select_ide_drive(bus_io, select_drive_command);
	outb(bus_io + ATA_REG3, 0);
	outb(bus_io + ATA_REG4, 0);
	outb(bus_io + ATA_REG5, 0);
	outb(bus_io + ATA_REG6, 0);
	outb(bus_io + ATA_REG8, ATA_COMMAND_IDENTIFY_DRIVE);
	status = inb(bus_io + ATA_REG8);
	if (status)
	{
		ata[ata_idx].present = 1;
		/* now, poll until 'BUSY' is clear */
		while (inb(bus_io + ATA_REG8) & ATA_BUSY_FLAG != 0);
		if (inb(bus_io + ATA_REG5) != 0 || inb(bus_io + ATA_REG6) != 0)
		{
			primitive_printf("error: drive port(%x) is not ATA", bus_io);
			//TODO: need handler for critical error
			while (1);
		}

		while ((status = inb(bus_io + ATA_REG8)) & (ATA_DRQ_FLAG | ATA_ERR_FLAG) == 0);
		if (status & ATA_ERR_FLAG)
		{
			primitive_printf("error: drive port(%x) has unknown error", bus_io);
			//TODO: need handler for critical error
			while (1);
		}
		read_from_ata_controller(ATA_PRIMARY_IO);
	}
	else
		ata[ata_idx].present = 0;	
	return 0;
}

static void init_ata()
{
	int i;

	ata[0].io_address = ATA_PRIMARY_IO;
	ata[0].slave = 0;

	ata[1].io_address = ATA_PRIMARY_IO;
	ata[1].slave = 1;	

	ata[2].io_address = ATA_SECONDARY_IO;
	ata[2].slave = 0;

	ata[3].io_address = ATA_SECONDARY_IO;
	ata[3].slave = 1;	

	for (i = 0; i < NUM_OF_ATA_DEVICES; i++)
	{
		if(identify_ide(i) == 0)
		{
			//setup struct ata members 
			//TODO: add more members for ata_buffer
		}
	}
}

static int read_write_ata(message_t *msg)
{
	struct ata *target_ata;
	message_t dummy_msg;
	int i, j;
	uint8_t tmp;
	int32_t pos_in_sector = msg->uinteger[POS_IDX_IN_MESSAGE] / SECTOR_SIZE;
	int msg_src_pid = msg->src_pid;
	int msg_buf_address = msg->uinteger[BUF_ADDRESS_IDX_IN_MESSAGE];
	int count = msg->integer[SIZE_IDX_IN_MESSAGE] / SECTOR_SIZE; 
	uint32_t msg_buffer_phy_address = vir_to_phy_address(msg_buf_address, msg_src_pid);

	if (msg->integer[DEVICE_IDX_IN_MESSAGE] == 0)
		target_ata = &ata[PRIMARY_IO_MASTER_ATA];
	else
		target_ata = &ata[PRIMARY_IO_SLAVE_ATA];

	for (i = 0; i < count; i++, pos_in_sector += 1)
	{
		target_ata->opcode = msg->type;
		target_ata->sector_count = 1;
		target_ata->lba_low = pos_in_sector;
		target_ata->lba_mid = pos_in_sector >> 8;
		target_ata->lba_high = pos_in_sector >> 16;
		
		// send a command for read (28bit pio)
		disable_interrupts();
	debug_printf("read_write_ata: disabled interrupt\n");
		outb(target_ata->io_address + ATA_REG7, 0xE0 | ((pos_in_sector >> 24) & 0x0F));
		outb(target_ata->io_address + ATA_REG2, 0x00);
		outb(target_ata->io_address + ATA_REG3, target_ata->sector_count);
		outb(target_ata->io_address + ATA_REG4, target_ata->lba_low);
		outb(target_ata->io_address + ATA_REG5, target_ata->lba_mid);
		outb(target_ata->io_address + ATA_REG6, target_ata->lba_high);
		
		outb(target_ata->io_address + ATA_REG8, ATA_COMMAND_READ);
	debug_printf("read_write_ata: about to enable interrupt\n");
		enable_interrupts();
		// wait for an interrupt from ata controller
		//TODO: use 'wait_for_interrupt(INT_NUM)
		wait_for_interrupt();	// synchronous
	debug_printf("read_write_ata: received IRQ_PRIMARY_ATA\n");
		while (inb(target_ata->io_address + ATA_REG8) & (ATA_DRQ_FLAG | ATA_BUSY_FLAG) == 0);
		// read data/status from data port
		read_from_ata_controller(ATA_PRIMARY_IO);	
		// copy the data to buffer from caller
		phy_copy(ata_buffer, msg_buffer_phy_address + (i * ATA_BUFFER_SIZE_IN_16BIT * 2), ATA_BUFFER_SIZE_IN_16BIT * 2); 
	}
	return 0;				
}

int ata_task()
{
	int status;

	init_ata();
	while (1)
	{
		int caller;
		receive(ANY_PROC, &msg);
		caller = msg.src_pid;
		switch (msg.type)
		{
			case DISK_READ:
			case DISK_WRITE:
				status = read_write_ata(&msg);
		}
		msg.integer[RETURN_IDX_IN_MESSAGE] = status;
		send(caller, &msg);
	}

	return 0;
}

