| When the PC is powered on, it reads the first block from the floppy
| disk into address 0x7C00 and jumps to it.  This boot block must contain
| the boot program in this file.  The boot program first copies itself to
| address 192K - 512 (to get itself out of the way).  Then it loads the 
| operating system from the boot diskette into memory, and then jumps to fsck.
| Loading is not trivial because the PC is unable to read a track into
| memory across a 64K boundary, so the positioning of everything is critical.
| The number of sectors to load is contained at address 504 of this block.
| The value is put there by the build program after it has discovered how
| big the operating system is.  When the bootblok program is finished loading,
| it jumps indirectly to the program (fsck) which address is given by the
| last two words in the boot block.(p.414~415) 
|
| Summary of the words patched into the boot block by build:
| Word at 504: # sectors to load
| Word at 506: # DS value for fsck
| Word at 508: # PC value for fsck
| Word at 510: # CS value for fsck
|
| This version of the boot block must be assembled without separate I & D
| space.  

        LOADSEG = 0x0060         | here the boot block will start loading
        BIOSSEG = 0x07C0         | here the boot block itself is loaded
        BOOTSEG = 0x2FE0         | here it will copy itself (192K-512b)
        DSKBASE = 120            | 120 = 4 * 0x1E = ptr to disk parameters

final   = 504
fsck_ds = 506
fsck_pc = 508
fsck_cs = 510


.globl begtext, begdata, begbss, endtext, enddata, endbss  | asld needs these
.text
begtext:
.data
begdata:
.bss
begbss:
.text

| copy bootblock to bootseg
        mov     ax,#BIOSSEG	| move the starting address of boot block to ax 
        mov     ds,ax		| mov ax to ds because movw uses ds
        xor     si,si           | si is 0x0000 ds:si - original block
        mov     ax,#BOOTSEG
        mov     es,ax		| mov ax to es because movw uses es
        xor     di,di           | di is 0x0000 es:di - new block
        mov     cx,#256         | word is 2byte so, 256*2 512bytes to copy
        rep    			| repeat movw; cx--  until 0 
	movw 		        | copy word: es:[di]=ds:[si]; si++; di++ 

| start boot procedure
        jmpi    start,BOOTSEG   | cs=BOOTSEG ip=start For more, pcasm-1.2.6 

start:
        mov     dx,cs
        mov     ds,dx           | set ds to cs
        xor     ax,ax
        mov     es,ax           | set es to 0
        mov     ss,ax           | set ss to 0
        mov     sp,#1024        | initialize sp (top of vector table): p.92
				| http://en.wikipedia.org/wiki/Interrupt_Descriptor_Table

| initialize disk parameters
	mov	ax,#atpar	| move the address of variable atpar. tenatively assume 1.2M diskette
	seg	es		| segment overriding
	mov	DSKBASE,ax	| (es:DSKBASE) = ax
	seg	es
	mov	DSKBASE+2,dx

| print greeting
	mov 	ax,#2		| reset video
	int  	0x10		| http://en.wikipedia.org/wiki/BIOS_interrupt_call, http://www.computing.dcu.ie/~ray/teaching/CA296/notes/8086_bios_and_dos_interrupts.html
        mov     ax,#0x0200	| BIOS call in put cursor in ul corner, 02 is stored at ah, and 00 is stored at al
        xor     bx,bx
        xor     dx,dx
        int     0x10
        mov     bx,#greet
        call    print

| Determine if this is a 1.2M diskette by trying to read sector 15.
	xor	ax,ax		| reset disk drives (0x00)
	int	0x13		| Low level disk interrupt
	xor	ax,ax
	mov	es,ax
	mov	ax,#0x0201
	mov	bx,#0x0600
	mov	cx,#0x000F
	mov	dx,#0x0000
	int	0x13
	jnb	L1
| Error.  It wasn't 1.2M.  Now set up for 360K.
	mov	tracksiz,#9	| 360K uses 9 sectors/track
	xor	ax,ax
	mov	es,ax
	mov	ax,#pcpar
	seg	es		| specify es register for next instruction
	mov	DSKBASE,ax
	int	0x13		| diskette reset
L1:

| Load the operating system from diskette.
load:
	call	setreg		| set up ah, cx, dx - call setreg procedure
	mov	bx,disksec	| bx = number of next sector to read
	add	bx,#2		| diskette sector 1 goes at 1536 ("sector" 3): p.92
	shl	bx,#1		| multiply sector number by 32
	shl	bx,#1		| ditto
	shl	bx,#1		| ditto
	shl	bx,#1		| ditto
	shl	bx,#1		| ditto
	mov	es,bx		| core address is es:bx (with bx = 0)
	xor	bx,bx		| see above
	add	disksec,ax	| ax tells how many sectors to read
	movb	ah,#2		| opcode for read
	int	0x13		| call the BIOS for a read
	jb	error		| jump on diskette error
	mov	ax,disksec	| see if we are done loading
	cmp	ax,final	| ditto
	jb	load		| jump if there is more to load

| Loading done.  Finish up.
        mov     dx,#0x03F2      | kill the motor
        mov     ax,#0x000C
        out
        cli
	mov	bx,tracksiz	| fsck expects # sectors/track in bx
        mov     ax,fsck_ds      | set segment registers
        mov     ds,ax           | when sep I&D DS != CS
        mov     es,ax           | otherwise they are the same.
        mov     ss,ax           | words 504 - 510 are patched by build

	seg cs
	jmpi	@fsck_pc	| jmp to fsck

| Given the number of the next disk block to read, disksec, compute the
| cylinder, sector, head, and number of sectors to read as follows:
| ah = # sectors to read;  cl = sector #;  ch = cyl;  dh = head; dl = 0
setreg:	
	mov	si,tracksiz	| 9 (PC) or 15 (AT) sectors per track
	mov 	ax,disksec	| ax = next sector to read
	xor	dx,dx		| dx:ax = 32-bit dividend
	div	si		| divide sector # by track size
	mov	cx,ax		| cx = track #; dx = sector (0-origin)
	mov	bx,dx		| bx = sector number (0-origin)
	mov	ax,disksec	| ax = next sector to read
	add	ax,si		| ax = last sector to read + 1
	dec	ax		| ax = last sector to read
	xor	dx,dx		| dx:ax = 32-bit dividend
	div	tracksiz	| divide last sector by track size
	cmpb	al,cl		| is starting track = ending track
	je	set1		| jump if whole read on 1 cylinder
	sub	si,dx		| compute lower sector count
	dec	si		| si = # sectors to read

| Check to see if this read crosses a 64K boundary (128 sectors).
| Such calls must be avoided.  The BIOS gets them wrong.
set1:	mov	ax,disksec	| ax = next sector to read
	add	ax,#2		| disk sector 1 goes in core sector 3
	mov	dx,ax		| dx = next sector to read
	add	dx,si		| dx = one sector beyond end of read
	dec	dx		| dx = last sector to read
	shl	ax,#1		| ah = which 64K bank does read start at
	shl	dx,#1		| dh = which 64K bank foes read end in
	cmpb	ah,dh		| ah != dh means read crosses 64K boundary
	je	set2		| jump if no boundary crossed
	shrb	dl,#1		| dl = excess beyond 64K boundary
	xorb	dh,dh		| dx = excess beyond 64K boundary
	sub	si,dx		| adjust si
	dec	si		| si = number of sectors to read

set2:	mov	ax,si		| ax = number of sectors to read
	xor	dx,dx		| dh = head, dl = drive
	movb	dh,cl		| dh = track
	andb	dh,#0x01	| dh = head
	movb	ch,cl		| ch = track to read
	shrb	ch,#1		| ch = cylinder
	movb	cl,bl		| cl = sector number (0-origin)
	incb	cl		| cl = sector number (1-origin)
	xorb	dl,dl		| dl = drive number (0)
	ret			| return values in ax, cx, dx


|-------------------------------+
|    error & print routines     |
|-------------------------------+

error:
        push    ax
        mov     bx,#fderr
        call    print           | print msg
	xor	cx,cx
err1:	mul	0		| delay
	loop	err1
	int	0x19


print:                          | print string (bx)
        movb	al,(bx)	        | al contains char to be printed
        testb   al,al           | null char? http://www.muslim-programers.com/1/inst.html#TEST
        jne     prt1            | no
        ret                     | else return
prt1:   movb    ah,*14          | 14(0x0a) = print char, so ah contains a interrupt vector(what to do), and al contains char 
        inc     bx              | increment string pointer
        push    bx              | save bx
        movb    bl,*1           | foreground color
	xorb	bh,bh		| page 0
        int     0x10            | call BIOS VIDEO_IO
        pop     bx              | restore bx
        jmp     print           | next character



disksec:.word 1
tracksiz:	.word 15	| changed to 9 for 360K diskettes
pcpar:	.byte	0xDF, 0x02, 25, 2, 9, 0x2A, 0xFF, 0x50, 0xF6, 1, 3   | for PC
atpar:	.byte	0xDF, 0x02, 25, 2,15, 0x1B, 0xFF, 0x54, 0xF6, 1, 8   | for AT

fderr:	.asciz	"Read error.  Automatic reboot.\r\n"
greet:	.asciz "\rBooting MINIX 1.1\r\n"	| http://linux.web.cern.ch/linux/scientific4/docs/rhel-as-en-4/ascii.html



| Don't forget that words 504 - 510 are filled in by build.  The regular
| code had better not get that far.
.text
endtext:
.data
enddata:
.bss
endbss:
