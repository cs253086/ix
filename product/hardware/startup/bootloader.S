#define BIOS_INT_VEC_TABLE_TOP	1024 /* top of the bios interrupt vector table */
#define VIDEO_MODE	2	/* 16 shades of gray text */

.org 0x7C00	/* BIOS loads a valid boot sector to linear addres 0x7c00 */

/* It starts with real mode */
.text

.global _start	/* _stat needs to be global for 'ld' to find it and link it */

_start:
	/* initialize cs == ds, es == 0x0000, ss == 0x0000 and sp = BIOS_INT_VEC_TABLE_TOP */
	mov %cs, %dx
	mov %dx, %ds
	xor %ax, %ax
	mov %ax, %es
	mov %ax, %ss
	mov BIOS_INT_VEC_TABLE_TOP, %sp	/* set sp to the top of the bios interrupt vector table so that CPU can use them */

	/* print greeting */
	mov VIDEO_MODE, %ax	/* select a video mode */
	int 0x10	/* initialize the video mode */

	mov 0x0200, %ax	/* set the cursor position */
	xor %bx, %bx	/* set page number to 0 */
	xor %dx, %dx	/* set row and column to 0 */
	mov greeting, %bx	/* set a message to print */
	call print

greeting_msg:
	db "Booting IX", 0x00	
