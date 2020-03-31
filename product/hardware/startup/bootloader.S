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
	mov greeting_msg, %bx	/* set a message to print */
	call print

	/* get total memory size */
	mov 0xE801, ax
	int 0x15
	jc memory_size_fail
	test ax, ax	/* if size == 0 */
	je memory_size_fail
	cmp 0x86, ah	/* unspported function */
	je memory_size_fail
	cmp 0x80, ah	/* invalid command */
	je memory_size_fail
	mov ax, [mem_size]

	/* check if 'INT 0x13' extension for LBA is supported */
	mov 0x41, ah
	mov 0x55AA, bx
	mov DRIVE, dl
	int 0x13
	jc os_load_fail

	/* load OS from disk/diskette into memory (LBA mode)
	 * while (tmp > MAX_SECTORS_PER_READ) {
	 * 		read
	 *		tmp = tmp - MAX_SECTORS_PER_READ
	 *		read
	 * }
	 */
load_os:
.Lread:
	mov MAX_SECTORS_PER_READ
	mov dapack, si
	mov 0x42, ah
	mov DRIVE, dl
	int 0x13
	jc os_load_fail
	mov MAX_BYTES_PER_READ	/* dst_seg += MAX_BYTES_PER_READ */
	shr 4, ax /* remember that physical address is represented in 16bit on 8086 */
	add ax, [dst_seg]
	add MAX_SECTORS_PER_READ, [sectors_read_sofar]
	add MAX_SECTORS_PER_READ, [lba]
	mov [sectors_read_sofar], ax
	cmp [os_size_sector], ax
	jb .Lread







greeting_msg:
	db "Booting IX", 0x00

