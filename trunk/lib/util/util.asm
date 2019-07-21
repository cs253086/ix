SECTION .text
	global inb, outb
	global disable_interrupts, enable_interrupts, restore_flags

; Assuming that int is standard 4 bytes
; int intb(int port)
inb:
	push	ebp
	mov	ebp, esp
	mov 	edx, [ebp + 8]	; port
	in	al, dx
	pop	ebp
	ret


; Assuming that int is standard 4 bytes
; outb(uint_16 port, uint_8 value)
outb:
	push	ebp
	mov	ebp, esp 
	mov 	edx, [ebp + 8]	; port	
	mov 	eax, [ebp + 8 + 4]	; value
	out	dx, al
	pop 	ebp
	ret	

; void disable_interrupts(void)
disable_interrupts:
	pushf
	cli
	pop	dword [flags]
	ret

; void enable_interrupts(void)
enable_interrupts:
	sti
	ret

; restore flags. It is usually used with 'disable_interrupts' 
restore_flags:
	push	dword [flags]
	popf
	ret

SECTION .data
flags	DD	0
