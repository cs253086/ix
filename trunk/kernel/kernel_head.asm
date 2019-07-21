; This is the starting point of kernel

SECTION .text
	global ix	; needs to let linker know 
	global sizes, server_stack_address, init_stack_address, kernel_directory_address, proc_table_address
    	extern idtr, main, setup_idt, kernel_stack_saved
; This is the entry point for IX kernel 
ix:
	mov	esp, kernel_stack_end	; set stack pointer for kernel
	mov	[kernel_stack_saved], esp	; initialize kernel_stack_saved
	call	setup_idt	; defined in idt.c
	lidt	[idtr]	; defined in idt.c
	call	main 
	;jmp	$	; without this, kernel goes into undefined instruction

SECTION .data
sizes	TIMES 8 DD 0 	; two double words(text, data) are needed for each os component(kernel, mm, fs, init)
server_stack_address	TIMES 2 DD 0	; starting address of MM and FS stacks. they are patched in build
;init_stack_address	DD 0
kernel_directory_address	DD 0
proc_table_address		DD 0	; used by mm
SECTION .bss
kernel_stack_begin:
	RESB 4096	; if the value changes, KERNEL_STACK_SIZE should be also changed in kernel/common.h
kernel_stack_end:
