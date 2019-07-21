; This file is for low level kernel functions
; - handling exception and interrupts
; - message passing

%define PID_IDLE	0	; h/common.h
%define MESSAGE_SIZE	16

; offsets in proc_t in kernel/common.h
%define	CS_OFFSET	20
%define PSW_OFFSET	24
%define	PC_OFFSET	28	
%define ESP_OFFSET	44
%define EBX_OFFSET	48
%define ECX_OFFSET	56
%define	EAX_OFFSET	60
%define FLAGS_OFFSET	64
%define PAGE_DIRECTORY_OFFSET	68

%define PAGING_BIT    0x80000000

SECTION .text
	global sys_call_low, run_proc, cp_msg_low, phy_copy, kernel_stack_saved, switch_paging
	extern exception_handler ; defined in exceptions.c
	extern irq_handler; defined in irq.c
	extern cur_proc_id, cur_proc_ptr	; defined in main.c
	extern system_call	; defined in /kernel/kernel_lib.c

%macro EXCEPTION_NOERRCODE	1
	global exception%1
	exception%1:
	cli
	push	0	; push dummy error code
	push	%1	; push exception number
	jmp	exception_common 
%endmacro

%macro EXCEPTION_ERRCODE	1
	global exception%1
	exception%1:
	cli
	push	%1
	jmp	exception_common
%endmacro

%macro IRQ	1
	global irq%1
	irq%1:
	;cli
	call	save_status
	;add	esp, 12	; temporary one before 'save' implementation
	push	0	; push dummy error code
	push	%1
	jmp	irq_common

%endmacro

EXCEPTION_NOERRCODE	0
EXCEPTION_NOERRCODE	1
EXCEPTION_NOERRCODE	2
EXCEPTION_NOERRCODE	3
EXCEPTION_NOERRCODE	4
EXCEPTION_NOERRCODE	5
EXCEPTION_NOERRCODE	6
EXCEPTION_NOERRCODE	7
EXCEPTION_ERRCODE	8
EXCEPTION_NOERRCODE	9
EXCEPTION_ERRCODE	10
EXCEPTION_ERRCODE	11
EXCEPTION_ERRCODE	12
EXCEPTION_ERRCODE	13
EXCEPTION_ERRCODE	14
EXCEPTION_NOERRCODE	15
EXCEPTION_NOERRCODE	16
EXCEPTION_ERRCODE	17
EXCEPTION_NOERRCODE	18
EXCEPTION_NOERRCODE	19

IRQ	0
IRQ	1
IRQ	2
IRQ	3
IRQ	4
IRQ	5

exception_common:
	call	exception_handler
	add	esp, 8	; clean up for exception number and error code
	sti
	iret

irq_common:
	call	irq_handler
	add	esp, 8	; clean up for interrupt number and dummy error code
	jmp    run_proc

sys_call_low:
	call	save_status
	mov	ebp, [cur_proc_ptr]
	push	dword [ebp + EBX_OFFSET]	;ebx
	push	dword [ebp + EAX_OFFSET]	;eax
	push	dword [cur_proc_id]
	push	dword [ebp + ECX_OFFSET]	;ecx	
	call	system_call	; system_call (function, caller, src_dst, m_ptr)
	add	esp, 16
	jmp	run_proc

; restore the status and run the cur_proc
run_proc:
	cmp	dword [cur_proc_id], PID_IDLE
	je	idle
	cli

	mov	[kernel_stack_saved], esp	; save kernel_stack current status for coming back to kernel
	
	; restore status
	mov	eax, [cur_proc_ptr]	; eax = cur_proc_ptr
	add	eax, ESP_OFFSET		; eax = cur_proc_ptr + ESP_OFFSET
	mov	ebx, [eax]		; ebx = *(cur_proc_ptr + ESP_OFFSET)
	mov	[tmp_esp], ebx		; tmp_esp = *(cur_proc_ptr + ESP_OFFSET)	
		
	; call switch paging with page direcotry of the current process
	mov	eax, [cur_proc_ptr]
	add	eax, PAGE_DIRECTORY_OFFSET
	mov	ebx, [eax]
	push	ebx
	call	switch_paging
	add	esp, 4	

	mov	esp, [cur_proc_ptr]
	pop	ss
	pop	gs
	pop	fs
	pop	es
	pop	ds
	pop	dword [tmp_cs]
	pop	dword [tmp_psw]
	pop	dword [tmp_eip]
	popad	;esp value is ignored

	; prepare to return to cur_proc
	mov	esp, [tmp_esp]	; restore esp manually
	push	dword [tmp_psw]
	push	dword [tmp_cs]
	push	dword [tmp_eip]
	iret
idle:
	sti
wait_for_interrupt:	
	jmp 	wait_for_interrupt

save_status:
	; pop return address to ret_addr 
	pop	dword [ret_addr]

	pop	dword [tmp_eip]
	pop	dword [tmp_cs]
	pop	dword [tmp_psw]	
	mov	[tmp_esp], esp
	mov	[tmp_eax], eax
	; get cur_proc_ptr 
	mov	eax, [cur_proc_ptr]	; cur_proc_ptr
	add	eax, FLAGS_OFFSET	
	; set esp to point at the end of register variables 
	mov	esp, eax
	
	; now, save the registers
	push	dword [tmp_eax]
	push	ecx
	push	edx
	push	ebx
	push	dword [tmp_esp]
	push	ebp
	push	esi
	push	edi
	push 	dword [tmp_eip]
	push 	dword [tmp_psw]
	push 	dword [tmp_cs]
	push 	ds
	push 	es
	push 	fs
	push 	gs
	push 	ss
	
	; restore esp to kernel_stack
	mov	esp, [kernel_stack_saved]
	; jmp to the ret_addr
	jmp [ret_addr]
	
; cp_message is to pass message from src process to dst process
; Passing message is actually accomplished by copying a message to the address space of dst process
; cp_msg_low(src_msg_addr, dst_msg_addr)
cp_msg_low:
    	; typical prodecure of atomic function
    	pushf   
    	cli
	; typical procedure for a C function
	push	ebp
	mov	ebp, esp
	; we need to use ds, es to repeat copying. So we save old values
	push	ds
	push	es
    	push    esi
    	push    edi
    	push    ecx
	; we don't need to set ds and es since they are both 0x10
	mov	esi, [ebp + 12]	; src_msg_addr
	mov	edi, [ebp + 16]	; dst_msg_addr
	mov	ecx, MESSAGE_SIZE
	rep	movsb		; repeat movsb, cx-- untill 0
	
	; restore registers
    	pop 	ecx
    	pop	edi
    	pop 	esi
	pop	es
	pop	ds
	pop	ebp
    	popf
	ret

; it copies a block of physical memory.
; phy_copy(uint32_t src, uint32_t dst, uint32_t size_bytes)
phy_copy:
    	; typical prodecure of atomic function
    	pushf   
    	cli
	; typical procedure for a C function
	push	ebp
	mov	ebp, esp
	; we need to use ds, es to repeat copying. So we save old values
	push	ds
	push	es
    	push    esi
    	push    edi
    	push    ecx
	
	; we don't need to set ds and es since they are both 0x10
	mov	esi, [ebp + 12]	; src
	mov	edi, [ebp + 16]	; dst
	mov	ecx, [ebp + 20]	; size_bytes
	rep	movsb		; repeat movsb, cx-- untill 0
	
	; restore registers
    	pop 	ecx
    	pop	edi
    	pop 	esi
	pop	es
	pop	ds
	pop	ebp
    	popf
	ret
	
switch_paging:
	pushf
	cli
	push	ebp	
	mov	ebp, esp
	mov    	eax, [ebp + 12]	; page_directory
	mov    	cr3, eax    ; point page_directory to cr3
	mov    	eax, cr0    ; set paging enable bit
	or     	eax, PAGING_BIT
	mov    	cr0, eax
	pop	ebp
	popf
	ret

SECTION .data
tmp_esp	DD 0
tmp_eax	DD 0
tmp_eip	DD 0
tmp_cs	DD 0
tmp_psw	DD 0
tmp_var	DD 0
ret_addr	DD 0
kernel_stack_saved	DD 0
