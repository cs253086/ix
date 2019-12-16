ORG 0x7C00
BITS 16	; 16-bit mode

;%define BOOT_SEG_IN_MEMORY	0x2FE0	; Segment location that bootblok is copied by itself (2FE00 is about 192K which is after init)
					; So, order in memory would be kernel->mm->fs->init->fsck->bootblok:  p92, p430
;%define NUM_WORDS_BOOTBLOK	256	; 512 bytes
%define TOP_OF_VEC_TABLE	1024	; the address where the vector table ends

%define VIDEO_MODE	2	; 16 shades of gray text
;%define CURSOR_POSITION	0x0200	; ah value to set the cursor postion with int 10

%define CS_SELECTOR    0x08	; cs selector with TI:0, RPL: 0. If you change this value, also change it in $IX_SOURCE/include/common.h
%define DS_SELECTOR    0x10
%define LOCATION_TO_LOAD_OS	0x07E0	; Segment location that bootblok loads OS
;%define SECTORS_PER_TRACK	9	; currenlty set for 360K floppy geometry
;%define SECTORS_PER_DMA		128	; 64k boundry contains 128 sectors
%define DRIVE	0x80	; boot drive number
%define MAX_SECTORS_READ_PER	0x7F
%define MAX_BYTES_READ_PER	65024	; MAX_SECTORS_READ_PER * SECTOR_SIZE 
%define SECTOR_SIZE	512	; 512 bytes per sector
SECTION .text
	global _start	; _start needs to be global for 'ld' to find it and link it 

_start:
; initialize cs, ds, es, ss and sp
	mov	dx, cs
	mov	ds, dx	; ds = cs
	xor	ax, ax	; ax == 0x0000
	mov	es, ax	; es == 0x0000
	mov	ss, ax	; ss == 0x0000
	mov	sp, TOP_OF_VEC_TABLE	; set sp to the top of interrupt vectors p.92. this initialization is necessary for CPU to use BIOS interrupt calls

; print greeting
	mov	ax, VIDEO_MODE	; save the video mode to al
	int	0x10	; call bios video interrupt
	mov	ax, 0x0200	; setting ah to set cursor position
	xor	bx, bx	; page number 0
	xor	dx, dx	; dh is row, set to 0, dl is column, set to 0
	mov	bx, greeting	; bx = &greeting 
	call	print

; get total memory size
	mov	ax, 0xE801
	int 	0x15
	jc	memory_size_error	
	test	ax, ax	; if size == 0
	je	memory_size_error
	cmp	ah, 0x86	; unsupported function
	je	memory_size_error
	cmp	ah, 0x80	; invalid command
	je	memory_size_error
	mov	[mem_size], ax

; check if 'INT 0x13' extension for LBA is supported 
	mov	ah, 0x41
	mov	bx, 0x55AA
	mov	dl, DRIVE
	int	0x13
	jc 	os_load_error
; load OS from disk/diskette into memory (LBA mode)

; while (tmp > MAX_SECTORS_READ_PER)
; 	read
;	tmp = tmp - MAX_SECTORS_READ_PER	
; read
load_os:
	.read:
		mov	word [blkcnt], MAX_SECTORS_READ_PER
		mov	si, dapack
		mov	ah, 0x42
		mov	dl, DRIVE
		int	0x13
		jc 	os_load_error
	; dest_seg += MAX_BYTES_READ_PER
	mov	ax, MAX_BYTES_READ_PER
	shr	ax, 4		; remember that physical address is represented in 16bit on 8086
	add	[dest_seg], ax
	add	word [sectors_read_sofar], MAX_SECTORS_READ_PER
	add	word [lba], MAX_SECTORS_READ_PER
	mov	ax, [sectors_read_sofar]
	cmp	ax, [os_size_sector]
	jb	.read

;	mov	word [next_os_loading_addr], LOCATION_TO_LOAD_OS 
;	mov	es, [next_os_loading_addr]
;load_os:
;	xor	ax, ax
;	xor	bx, bx 
;	mov	dh, [cur_head]
;	mov	dl, DRIVE
;	mov	ch, [cur_cylinder]
;	mov	cl, [starting_sector]	; from which sector to read
;	mov	al, [sectors_to_read]
;	mov	ah, 0x02
;	int	0x13	
;	jc	os_load_error
;
;	; prepare the next read-cycle	
;	mov	[sectors_read], al
;	add	[track_bottle], al	; track_bottle = track_bottle + sectors_read
;	add	[dma_bottle], al	;dma_bottle = dma_bottle + sectors_read
;	; calculate total number of sectors read so far
;	add	[sectors_read_sofar], al
;
;	.next_sectors_by_track:
;		; if (track_bottle == SECTORS_PER_TRACK) track_bottle = 0
;		cmp	byte [track_bottle], SECTORS_PER_TRACK
;		jb	.get_sectors_by_track
;		mov	byte [track_bottle], 0
;		.get_sectors_by_track:
;		mov	al, SECTORS_PER_TRACK
;		sub	al, [track_bottle]
;
;	.next_sectors_by_dma:
;		cmp	byte [dma_bottle], SECTORS_PER_DMA
;		jb	.get_sectors_by_dma
;		mov	byte [dma_bottle], 0	
;		.get_sectors_by_dma:
;		mov	bl, SECTORS_PER_DMA
;		sub	bl, [dma_bottle]
;	.min:	; have minimum from ax and bx
;		cmp	al, bl
;		jb	.min_al
;		mov	[sectors_to_read], bl
;		jmp	.end_min
;		.min_al:
;		mov	[sectors_to_read], al
;	.end_min:
;
;	; starting_sector = track_bottle + 1
;	mov	al, [track_bottle]
;	mov	[starting_sector], al
;	inc	byte [starting_sector]	
;
;	cmp	byte [track_bottle], 0	; if (track_bottle == 0)
;	jnz	.else
;	; the order to read: (head0, cylinder0)->(head1, cylinder0)->(head0, cylinder1)...
;	xor	byte [cur_head], 0x01	; cur_head++
;	jnz	.else
;	inc	byte [cur_cylinder]
;	.else:
;	; next_os_loading_addr = next_os_loading_addr + (512 * sectors_read)
;	mov	ax, SECTOR_SIZE
;	mul	word [sectors_read]
;	shr	ax, 4		; remember that physical address is represented in 16bit on 8086
;	add	[next_os_loading_addr], ax
;	mov	es, [next_os_loading_addr]	; updated location to load os
;	mov	ax, [sectors_read_sofar]
;	cmp	ax, [os_size]
;	jb	load_os

; Prepare to enter 32-bit protected mode
; Enable A20 to access up to sixteen megabytes of memory (instead of the 1 MByte of the 8086)
seta20_1:   
	in al, 0x64     ; get value of status register of keyboard controller
	test al, 0x02   ; busy?
	jnz seta20_1    ; yes
	mov al, 0xd1    ; command: write
	out 0x64, al    ; command port of keyboard controller
seta20_2:   
	in al, 0x64     ; get status
	test al, 0x02   ; busy?
	jnz seta20_2    ; yes
	mov al, 0xdf    ; data: enable A20
	out 0x60, al    ; data port of keyboard controller 

enter_protected:
	cli ; disable hardware interrupt 
	lgdt [gdtr]   ; load Global Descriptor Table
	; set PE (Protection Enable) bit in CR0 (Control Register 0)
	mov eax, cr0    ; eax = current value of cr0
	or eax, 0x1 ; protected mode flag value
	mov cr0, eax ; enable protected mode 

	; jump to the entry point of protected mode
	jmp CS_SELECTOR : protected_entry    

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

os_load_error:
	mov	bx, os_load_error_msg
	call	print
	jmp	$	

memory_size_error:
	mov	bx, memory_size_error_msg
	call	print
	jmp	$

; entry point of protected mode
; set up segments
BITS 32
protected_entry:
	mov ax, DS_SELECTOR   
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
 
; jump to kernel entry 
	mov ax, LOCATION_TO_LOAD_OS
	shl ax, 4	; 0x07E0 need to be changed to 0x7E00 since 0x07E0 in 16-bit is actually 0x7E00
    	jmp ax

gdt_start:	
	DD 0, 0	; null descriptor
	DB 0xFF, 0xFF, 0x00, 0x00, 0x00, 10011010b, 11001111b, 0x00    ; code segment entry
	DB 0xFF, 0xFF, 0x00, 0x00, 0x00, 10010010b, 11001111b, 0x00    ; data segment entry
gdt_end:

gdtr:
    DW gdt_end - gdt_start - 1	; limit == last byte == (size) - 1 It means GDTR starts from 0 in virtual address. Think about an array.
    DD gdt_start   ; base address of GDT	

dapack:				; disk address packet
	DB	0x10	; size of packet
	DB	0
blkcnt:	DW	0	; how many sectors to read
dest_offset:	
	DW	0		; memory buffer destination address offset 
dest_seg:
	DW	LOCATION_TO_LOAD_OS		; memory buffer destination address segment
lba:	DD	1		; starting sector to read (0 based) 
	DD	0		; more storage bytes only for big lba's ( > 4 bytes )
	
greeting		DB 0Dh, "Booting IX 1.0", 0Dh, 0Ah, 0	; 0Dh:carriage return, 0Ah:line feed, end of string
os_load_error_msg	DB "OS Read error. Please, Reboot.", 0Dh, 0Ah, 0
memory_size_error_msg	DB "Memory size error. Please, Reboot.", 0Dh, 0Ah, 0
;cur_head		DB 0
;cur_cylinder		DB 0
;starting_sector		DB 2	; 1 is bootsector (index from 1)
;sectors_to_read		DB 8	; SECTORS_PER_TRACK - (starting_sector - 1)
;sectors_read		DB 0	; return value from int 0x13
;next_os_loading_addr	DW 0
sectors_read_sofar	DW 0	; total #sectors read so far
;track_bottle		DB 1	; 1(boot sector). it is used to check if a read crosses the track boundry (9 sectors/track)
;dma_bottle		DB 63	; 7e00 / 512 = 63. it is used to check if a read crosses the track boundry (64k)

; bootloader code size is 446
times 440 - ($ - $$) DB	0 
mem_size	DD 0	; total physical mem - (uma + base memory)
os_size_sector		DW 0	; # sectors to read (patched in by build)
; do nothing in partition table area
times 510 - ($ - $$) DB	0	; bootloader size should be 512 bytes

DW 0xaa55	;bootloader signature
