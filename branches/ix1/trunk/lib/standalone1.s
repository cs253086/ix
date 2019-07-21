| This file is helper of standalone.c

.globl _main, _putc, _getc, csv, cret, _exit

.text
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
	xorb	ah, ah
	int	0x16
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
