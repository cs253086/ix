| Summary
| 1. copy bootblok from BIOSSEG to BOOTSEG
| 2. set disk parameters
| 3. load operating system to memory
| 	A. Check if the track of the staring sector to read is the same track of the last sector to read in this turn  
| 	B. Check to see if this read crosses a 64K boundary (128 sectors).


| Summary of the words patched into the boot block by build:
| Word at 504: # sectors to load
| Word at 506: # DS value for fsck
| Word at 508: # PC value for fsck
| Word at 510: # CS value for fsck
|
| This version of the boot block must be assembled without separate I & D
| space.

LOADSEG = 0x0060	| segment location boot block starts loading os at 
BIOSSEG = 0x07C0	| segment location BIOS loads bootblock. 
BOOTSEG = 0x2FE0	| location(from 192K, 512bytes) to copy bootblock loaded by BIOS to (2FE00 is about 192K which is after init). 
			| So, order in memory would be kernel->mm->fs->init->fsck->bootblok:  p92, p430
|TOKNOW: why 4?
DSKBASE = 120		| 120 = 4 * (0x1E is ptr to disk parameters): http://read.pudn.com/downloads107/doc/442250/cvery.comvc347721333/boot/bootblock.s__.htm

NWD_BOOTBLK = 256	| number of words in bootblok
ABV_VECTBL = 1024	| above vector table

final = 504	| 504 byte
fsck_ds = 506	| 506 byte
fsck_pc = 508	| 508 byte
fsck_cs = 510	| 510 byte

.globl begtext, begdata, begbss, endtext, enddata, endbss 	| asld needs these
.text
begtext:
.data
begdata:
.bss
begbss:
.text

| copy bootblock to bootseg
mov	ax, #BIOSSEG	| ax = (value of BIOSSEG)
mov	ds, ax	| ds is used by movw as src address
xor	si, si	| si set to 0x0000 and used as displacement - ds:si
mov	ax, #BOOTSEG
mov	es, ax	| es is used by movw as dest address
xor	di, di	| di set to 0x0000 and used as displacement - es:di
mov	cx, #NWD_BOOTBLK	| word is 2byte. Therefore, 512bytes = 256 words
rep	| repeat movw; cx-- until 0
movw	| copy a word from ds:si to es:di; si++; di++

| jump to bootblock copied itself
jmpi	start, BOOTSEG	| cs=BOOTSEG ip=start - pcasm-1.2.6

start:
| initialize cs, ds, es, ss
mov	dx, cs
mov	ds, dx	| now ds==cs
xor	ax, ax	| ax == 0x0000  
mov	es, ax	| es == 0x0000
mov	ss, ax	| ss == 0x0000
mov	sp, #ABV_VECTBL	| set sp to the top of vector table: p.92

| print greeting
mov	ax, #2	| set video mode: http://www.computing.dcu.ie/~ray/teaching/CA296/notes/8086_bios_and_dos_interrupts.html#int10h_00h
int 	0x10	| call bios video interrupt 
mov	ax, #0x0200	| set cursor position: http://www.computing.dcu.ie/~ray/teaching/CA296/notes/8086_bios_and_dos_interrupts.html#int10h_02h
xor	bx, bx
xor	dx, dx
int	0x10	| call bios vidoe interrupt for 'set cursor position'
mov	bx, #greet
call	print
	
|initialize disk parameters: 1. DSKBASE = disk parameters 2. DSKBASE+2 = code segment(cs) 
xor 	ax, ax	
mov 	ax, #pcpar	| #pcpar is the address of pcpar. Assume 360K diskette
seg 	es	| segment overriding
mov 	DSKBASE, ax	| (es:DSKBASE) = ax 	
seg 	es
mov 	DSKBASE+2, cs	| DSKBASE+2 = cs
int 	0x13	| diskette reset

load:
| Load the operating system from diskette
call 	setreg	| set up ah, cx, dx
mov 	bx, disksec	| bx = #next sector to read
add 	bx, #2	| first sector of os(atually, it's sector#2) goes at 1536("sector 3 in memory): p92
shl 	bx, #1	| 3 * (consecutive 5 times bit-shift-left) = 3 * 2^5 = 3 * 32 = 96 = 0060h in 16bit = 00600 in 20bit physical memory = 1536 in decimal
shl 	bx, #1
shl 	bx, #1
shl 	bx, #1
shl 	bx, #1
mov 	es, bx	| core address is es:bx (with bx=0x0000)
xor 	bx, bx	
 
add 	disksec, ax	| number of sectors that will be read until this loop. ax tells how many sectores to read for this loop
movb 	ah, #2	| opcode for read
int 	0x13	| call the BIOS for a read
jb 	error	| jump on diskette error
mov 	ax, disksec	
cmp 	ax, final	| see if we are done loading. value at 'final' address is E3h=227
jb 	load	| jump if there is more to load

| Loading done. Finihsing up.
mov	dx, #0x03F2	| disk port number =  one of registers of floppy disk contoller http://wiki.osdev.org/Floppy_Disk_Controller
mov	ax, #0x000C	| kill the motor command: 8+4+0=12: http://viralpatel.net/taj/tutorial/programming_fdc.phpa
out
cli
mov 	bx, tracksize	| fsck expects # sectors/track in bx	 
| set segment registers
mov 	ax, fsck_ds	| value at the address of fsck_ds == 1627h 
mov	ds, ax	
mov	es, ax	
mov	ss, ax	

seg 	cs
jmpi	@fsck_pc	| jump to fsck: http://wiki.answers.com/Q/Difference_between_intersegment_and_intrasegment_jump, http://minix1.woodhull.com/pub/info/pc-ix-assem.txt
			| value of address of fsck_pc is 0000 and jmp(i) is intersegment. So, it points to 1627:0000 which is the first instruction of fsck1.s
			| how does it change cs segment to 1627... because of fsck_cs ??
			| yes, cs is stored at 510 and ip is stored at 508. remember x86 is little endian

setreg:
chk_same_track:
| Check if the track of the staring sector to read is the same track of the last sector to read in this turn
| If they are in different tracks, it should be avoided. BIOS gets them wrong
mov	si, tracksize	| 9 sectors per track
mov	ax, disksec	| ax = next sector to read
xor	dx, dx	| dx:ax is 32-bit dividend
div	si	| divide first sector by track size: http://www.muslim-programers.com/1/inst.html#DIV
		| ax=(dx:ax)/si, dx=remainer. So, ax=0x0000, dx=0x0001
mov	cx, ax	| cx=#track, dx=#sector(0-origin:index from 0)
		| cx=0x0000, dx=0x0001
mov	bx, dx	| bx=sector number

mov	ax, disksec	| ax=first sector to read
add	ax, si	| ax=last sector to read + 1
dec	ax	| ax=last sector to read
xor	dx, dx	| dx:ax is 32-bit dividend
div	tracksize	| divide last sector by track size
cmpb	al, cl	| is starting track is same as ending track?
je 	chk_64k_bndry	| jump if the whole reading happens only in one track
sub	si, dx	| else, tracksize(9sectors for 360K) - overflowed sectors => si is #sectors + 1 to make first sector and last sector in the same track
dec	si	| si is #sectors to make them in the same track 

| Check to see if this read crosses a 64K boundary (128 sectors).
| Such calls must be avoided. BIOS gets them wrong
chk_64k_bndry:	
mov	ax, disksec	| ax = first sector to read
add	ax, #2	| disk sector 1 goes to in memory sector 3

mov	dx, ax	| dx = first sector to read (memory side)
add	dx, si	| dx = one sector beyond end of read (memory side)
dec	dx	| dx = last sector to read (memory side)

| 1. 7 bits represents 127 sectors. 
| 2. 127x512bytes = 64K
| So, if ax is more than 127, then ah bits would be set to 1s (e.g. ah==0x0003).
| In order to check if ax > 127, shift left by one bit and check ah to see if it's set to any 1s
shl	ax, #1	| ax=first sector to read
shl	dx, #1	| dx=last sector to read
cmpb	ah, dh	| ah != dh means it read 64K boundary crossed. ah and dh are 0x00 then it's not 64K boundary crossed
je 	setval	| jump if no boundary crossed 
| similar to setreg: consider that size is 127 now instead of 9
shrb	dl, #1	| calculate the overflowed sectors
xorb	dh, dh	| dh is 0x00
sub	si, dx	| si(from setreg) - dx
dec	si	| si is final #sectors to read

| Now, using information so far, set real register values for reading diskette 
setval:
| get dh (head)
mov	ax, si	| ax=number of sectors to read
xor	dx, dx	| dh is head, dl is drive
movb	dh, cl	| cl was the track to read
andb	dh, #0x01	| dh = head, head number alternates 0->1->0->1
| get ch (#cylinder)
movb	ch, cl	| ch is track to read, temporarily
shrb	ch, #1	| ch = cynlinder (2 tracks - front and back). since #head alternates, ch goes in this way, 0 in head0-> 0 in head1-> 1 in head0 ...
| get cl (#first sector to read) 
movb	cl, bl	| cl is #sector (0-origin)
incb	cl	| cl is #sector (1-origin)
| get dl (#drive)
xorb	dl, dl	| dl = drive number (0)
ret	| return 

| Helper procedures

error:
push	ax
mov	bx, #fderr	| #fderr is address of fderr
call 	print
xor	cx, cx
delay:	mul	0	| delay. cx is from 0, -1, -2, ..., -65536, 65535, 65334, ..., 0 (it circles around)
loop	delay
int	0x19	| system reboot

print:		| print sting (bx)
	movb	al, (bx)	| al contains a char to be printed
	testb	al, al		| test if al is null char
	jne	print_char	| no? then print the char
	ret			| otherwise, return 
print_char:
	movb	ah, *14		| 8-bits 14(0x0e) = print char, so ah contains a interrupt vector(what to do), and al contains char
	inc	bx		| increment string pointer
	push	bx		| save bx for now since we gonna use bx temporily
	movb	bl, *1		| forground color
	xorb	bh, bh		| page number == 0
	int	0x10		
	pop	bx		| restore bx
	jmp	print

disksec: .word 1	| 0 sector is boot sector
tracksize: .word 9	| we assume 360K diskette

| disk parameters: http://stanislavs.org/helppc/dbt.html
pcpar: .byte 0xDF, 0x02, 25, 2, 9, 0x2A, 0xFF, 0x50, 0xF6, 1, 3	| for PC(360k)

fderr:  .asciz  "Read error.  Automatic reboot.\r\n"
greet:	.asciz "\rBooting NEURO 1.0\r\n"	| \r: go back to first chracter location \n: new line

.text
endtext:
.data
enddata:
.bss
endbss:
