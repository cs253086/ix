%define	PRIMARY_COMMAND_REGISTER	0x20
%define	SLAVE_COMMAND_REGISTER		0xA0
%define	PRIMARY_DATA_REGISTER		0x21	
%define	SLAVE_DATA_REGISTER		0xA1

SECTION .text
	global	init_8259

; init_8259()
init_8259:
	; ICW1: Initialize PICs
	mov	al, 0x11	; Refer to PIC command register in wiki
	out PRIMARY_COMMAND_REGISTER, al	; Primary PIC command register
	out SLAVE_COMMAND_REGISTER, al	; Slave PIC command register

	; ICW2: Tell PICs the base address of IRQ's
	mov	al, 0x20	; Primary PIC handled IRQ 0...7 IRQ 0 is now mapped to interrupt number 0x20
	out PRIMARY_DATA_REGISTER, al	; Primary PIC data register
	mov al, 0x28	; Slave PIC handled IRQ 8...15 IRQ 8 is now mapped to interrupt number 0x28
	out SLAVE_DATA_REGISTER, al	; Slave PIC data register

	; ICW3: Tell PICs which IRQ line is used to communicate each other(Primary and Slave)
	mov	al, 0x04	; 0x04 == 00000100 (IR line 2)	
	out	PRIMARY_DATA_REGISTER, al
	mov al, 0x02	; 0x02 means IR Line 2
	out SLAVE_DATA_REGISTER, al

	; ICW4: This controls how everything operates
	mov	al, 0x01	; bit 0 enables 80x86 mode
	out	PRIMARY_DATA_REGISTER, al
	out SLAVE_DATA_REGISTER, al

	; Do not mask out any interrupts in 8259
	mov	al, 0
	out	PRIMARY_DATA_REGISTER, al
	out SLAVE_DATA_REGISTER, al

	ret
