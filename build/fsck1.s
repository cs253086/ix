STACKSIZE = 8192

.globl _main, _exit, _edata, _end, _putc, _getc 
.globl csv, cret, begtext, begdata, begbss
.globl _cylsiz, _tracksiz 

.text
begtext:
start:
	mov	dx, bx	| jmpi @fsck_pc from bootblok.s points to this instruction. bootblok puts #sectors per track in bx
	xor	ax, ax	
	mov	bx, #_edata	| refer to lib/end.s. it's to prepare to clear bss
	mov	cx, #_end	| refer to lib/end.s
	sub	cx, bx	| extract the address of .bss 
	sar	cx, *1	| shift-arithmetic-right: http://en.wikipedia.org/wiki/Bitwise_operation#Arithmetic_shift
			| shfit right is same effect as division by 2. The reason of division by 2 is because we increment pointer bx by two bytes
clear_bss:	
	mov	(bx), ax	| clear data at pointer bx
	add	bx, #2
	loop	clear_bss

	mov	_tracksiz, dx	| dx (was bx) is #sectors per track
	add	dx, dx
	mov	_cylsiz, dx	| #sectors per cylinder = 2 * (#sectors per track)
	mov	sp, #kerstack+STACKSIZE	| TOKNOW: why this statement is needed?
	call	_main
	mov	bx, ax 	| put scan code for '=' in bx: http://en.wikipedia.org/wiki/Scancode. scan code table: http://www.philipstorr.id.au/pcbook/book3/scancode.htm
			| '=' returned by _main is stored at ax
	cli
	mov	dx, #0x60
	mov	ds, dx
	mov	es, dx
	mov	ss, dx
	jmpi	0, 0x60	| 0: 16bit displacement 0x60: destination. Therefore, it jumps to 0x0060: 0000, which is kernel (MINIX label in kernel/mpx88.s) and cs=0x0060 	

| these procedures are same ones as in libc.a, but not related to kernel
_exit:  mov     bx, _tracksiz
        jmp     start

_putc:
        xor     ax, ax
        call    csv
        movb    al, 4(bp)        | al contains char to be printed
        movb    ah, #14          | 14 = print char
        movb    bl, *1           | foreground color
        push    bp              | not preserved
        int     0x10            | call BIOS VIDEO_IO
        pop     bp
        jmp     cret

_getc:
        xorb    ah, ah
        int     0x16
        ret

csv:
        pop     bx
        push    bp
        mov     bp, sp
        push    di
        push    si
        sub     sp, ax
        jmp     (bx)

cret:
        lea     sp, *-4(bp)
        pop     si
        pop     di
        pop     bp
        ret

.data
begdata:
tmp:	.word 0
tmp1:	.word 0
tmp2:	.word 0
.bss
begbss:
kerstack:	.zerow STACKSIZE/2	| kernel stack
