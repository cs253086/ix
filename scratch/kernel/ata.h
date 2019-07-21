#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include "../h/common.h"

#define ATA_INT		0

#define ATA_PRIMARY_IO		(uint16_t) 0x1f0
#define ATA_SECONDARY_IO	(uint16_t)	0x170
#define ATA_MASTER		0
#define ATA_SLAVE		1

#define PRIMARY_IO_MASTER_ATA	0
#define PRIMARY_IO_SLAVE_ATA	1
#define SECONDARY_IO_MASTER_ATA	2
#define SECONDARY_IO_SLAVE_ATA	3

/* 
 * I/O Ports(I/O base address) used by ata(ide) disk controller for the primary bus 
 * http://wiki.osdev.org/ATA_PIO_Mode#Absolute.2FRelative_LBA 
 */

#define ATA_REG1       (uint16_t) 0x0	// data port
#define ATA_REG2       (uint16_t) 0x1	// features / error information
#define ATA_REG3       (uint16_t) 0x2 // sector count
#define ATA_REG4       (uint16_t) 0x3 // sector number-LBA low
#define ATA_REG5       (uint16_t) 0x4 // sector number-LBA mid
#define ATA_REG6       (uint16_t) 0x5 // sector number-LBA high
#define ATA_REG7       (uint16_t) 0x6 // drive/head port
#define ATA_REG8       (uint16_t) 0x7 // command port / reading status port

#define ATA_PRIMARY_DEV_CONTROL_REG       (uint16_t) 0x3F6	// device control register/alternative status port for primary bus

#define ATA_COMMAND_SELECT_MASTER_DRIVE	(uint8_t) 0xA0
#define ATA_COMMAND_SELECT_SLAVE_DRIVE	(uint8_t) 0xB0
#define ATA_COMMAND_IDENTIFY_DRIVE	(uint8_t) 0xEC
#define ATA_COMMAND_READ	(uint8_t) 0x20

#define ATA_BUSY_FLAG	(uint16_t) 0x80
#define ATA_DRQ_FLAG	(uint16_t) 0x8
#define ATA_ERR_FLAG	(uint16_t) 0x1

#define ATA_BUFFER_SIZE_IN_16BIT	256

#define DISK_READ	DEVICE_READ
#define DISK_WRITE	DEVICE_WRITE

int ata_task();

#endif /* ATA_H */
