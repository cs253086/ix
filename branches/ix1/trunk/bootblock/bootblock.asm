BITS 16	; 16-bit mode

%define BIOS_LOAD_BOOT_SEG	0x07C0	; Segment location that BIOS loads the bootblok
%define BOOT_SEG_IN_MEMORY	0x2FE0	; Segment location that bootblok is copied by itself (2FE00 is about 192K which is after init)
					; So, order in memory would be kernel->mm->fs->init->fsck->bootblok:  p92, p430
%define NUM_WORDS_BOOTBLOK	256	; 512 bytes
%define TOP_OF_VEC_TABLE	1024	; the address where the vector table ends

%define VIDEO_MODE	2	; 16 shades of gray text
%define CURSOR_POSITION	0x0200	; ah value to set the cursor postion with int 10

%define LOCATION_TO_LOAD_OS	0x0060	; Segment location that bootblok loads OS
%define SECTORS_PER_TRACK	9
%define FRONT_HEAD	0x00
%define	BACK_HEAD	0x01
%define DRIVE	0x00	; boot drive number

%define SECTOR_SIZE	512	; 512 bytes per sector
segment .text
	global _start	; _start needs to be global for 'ld' to find it and link it 

; copy bootblok to BOOT_SEG_IN_MEMORY
; because memory in BIOS_LOAD_BOOT_SEG needs to be used
	mov	ax, BIOS_LOAD_BOOT_SEG	
	mov	ds, ax	; ds is used by movsw as src segment
	xor	si, si	; si is set to zero and used as displacement of ds, ds:si
	mov	ax, BOOT_SEG_IN_MEMORY
	mov	es, ax	; es is used by movsw as dest segment
	xor	di, di	; di is set to zero and used as displacement of es, es:di
	mov	cx, NUM_WORDS_BOOTBLOK
	rep	movsw	; repeat movsw. cx-- until 0 
			; copy a word from ds:si to es:di, si++, di++
	jmp	BOOT_SEG_IN_MEMORY:_start	; cs=BOOT_SEG_IN_MEMORY ip=start

_start:
; initialize cs, ds, es, ss and sp
	mov	dx, cs
	mov	ds, dx	; ds = cs
	xor	ax, ax	; ax == 0x0000
	mov	es, ax	; es == 0x0000
	mov	ss, ax	; ss == 0x0000
	mov	sp, TOP_OF_VEC_TABLE	; set sp to the top of interrupt vectors p.92

; print greeting
	mov	ax, VIDEO_MODE	; save the video mode to al
	int	0x10	; call bios video interrupt
	mov	ax, 0x0200	; setting ah to set cursor position
	xor	bx, bx	; page number 0
	xor	dx, dx	; dh is row, set to 0, dl is column, set to 0
	mov	bx, greeting	; bx = &greeting 
	call	print

; load OS from disk/diskette into memory
	mov	bx, LOCATION_TO_LOAD_OS
	mov	es, bx
load_os:
	xor	bx, bx 
	mov	dh, [cur_head]
	mov	dl, DRIVE
	mov	ch, [cur_cylinder]
	mov	cl, [starting_sector]	; from which sector to read
	mov	al, [sectors_to_read]
	mov	ah, 0x02
	int	0x13	
	jc	error
	
	; sectors_read_sofar  = sectors_read_read_sofar + (sectors_per_track - (starting_sector - 1))
	dec	byte [starting_sector]	
	mov	al, SECTORS_PER_TRACK
	sub	al, [starting_sector]
	add	[sectors_read_sofar], al	
	
	xor	byte [cur_head], 0x01		
	jnz	set_starting_sector
	inc	byte [cur_cylinder]
set_starting_sector:
	mov	byte [starting_sector], 0x01
	; location_to_load_os(es) = location_to_load_os(es) + (512 * sectors_to_read)
	mov	ax, SECTOR_SIZE
	mul	word [sectors_read_sofar]
	mov	es, ax
	mov	ax, [sectors_read_sofar]
	cmp	ax, [os_size]
	jb	load_os
	mov	bx, welcome
	call	print
	jmp	LOCATION_TO_LOAD_OS:0x0000

; print procedure: print string in bx
print:	
	mov	al, [bx]	; al = *bx, which is a byte
	cmp	al, 0
	jne	print_char
	ret
print_char:
	inc	bx	; increment string pointeri (bx++)
	push	bx	; save bx for now since we going to use bx temporily
	mov	ah, 0x0E	; teletype output
	mov	bl, 0x01	; foreground color
	int	0x10
	pop	bx	; restore bx
	jmp	print

error:
	mov	bx, error_msg
	call	print
	jmp	$	
	
greeting DB	0Dh, "Booting IX 1.0", 0Dh, 0Ah, 0	; 0Dh:carriage return, 0Ah:line feed, end of string
error_msg DB	"Read error. Please, Reboot.", 0Dh, 0Ah, 0
welcome DB	"Welcome...", 0Dh, 0Ah, 0

cur_head	DB	FRONT_HEAD
cur_cylinder	DB	0
starting_sector	DB	2	; 1 is bootsector (index from 1)
sectors_to_read	DB	8	; SECTORS_PER_TRACK - (starting_sector - 1)
sectors_read_sofar DW	0	; total #sectors read so far

os_size	DW	227	; # sectors to read (patched in by build)
times 510 - ($ - $$) DB	0 
DW	0xaa5
