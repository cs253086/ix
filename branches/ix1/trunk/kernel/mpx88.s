| This file is part of the lowest layer of the MINIX kernel.  All processing
| switching and message handling is done here and in file "proc.c".  This file
| is entered on every transition to the kernel, both for sending/receiving
| messages and for all interrupts.  In all cases, the trap or interrupt
| routine first calls save() to store the machine state in the proc table.
| Then the stack is switched to k_stack.  Finally, the real trap or interrupt
| handler (in C) is called.  When it returns, the interrupt routine jumps to
| restart, to run the process or task whose number is in 'cur_proc'.
|
| The external entry points into this file are:
|   s_call:     process or task wants to send or receive a message
|   tty_int:    interrupt routine for each key depression and release
|   lpr_int:    interrupt routine for each line printer interrupt
|   disk_int:   disk interrupt routine
|   wini_int:   winchester interrupt routine
|   clock_int:  clock interrupt routine (HZ times per second)
|   surprise:   all other interrupts < 16 are vectored here
|   trp:        all traps with vector >= 16 are vectored here
|   divide:     divide interrupt vector (invoked by CPU when division by zero is attempted)
|   restart:    start running (proc_ptr=proc[cur_proc]) process

K_STACK_BYTES = 256
IDLE = -999

| These symbols MUST agree with the values in ../h/com.h to avoid disaster
WINCHESTER = -6
FLOPPY = -5
CLOCK = -3
DISKINT = 1
CLOCK_TICK = 2

| The following procedures are defined in this file and called from outside it
.globl _surprise, _trp, _divide, _clock_int, _disk_int, _tty_int, _restart, _s_call
|.globl _tty_int, _lpr_int, _clock_int, _disk_int, _winchester_int
|.globl _s_call, _surprise, _trp, _divide, _restart

| The following external procedures are defined outside and called in this file
.globl _main, _unexpected_int, _trap, _div_trap, _interrupt, _keyboard
|.globl _main, _unexpected_int, _panic, _unexpected_int, _trap, _div_trap 
|_sys_call, _interrupt, _keyboard, _panic, _unexpected_int, _trap
|.globl _pr_char, _div_trap

| Variables defined in this file and called outside
.globl _sizes, begtext, begdata, begbss	| this statement is necessary to make them visible outside

| Variables defined outside and called in this file
.globl _cur_proc, _proc_ptr, _k_stack, splimit, _int_mess, _scan_code

| Variables, data structures and miscellaneous
|.globl _cur_proc, _proc_ptr, _scan_code, _int_mess, _k_stack, splimit

| The following constants are offsets into the proc table.
esreg = 14
dsreg = 16
csreg = 18
ssreg = 20
SP    = 22
PC    = 24
PSW   = 28
SPLIM = 50
OFF   = 18
ROFF  = 12

.text
begtext:
|*===========================================================================*
|*                              NEURO                                        *
|*===========================================================================*
NEURO:			| this is the entry point for the NEURO kernel
	jmp setup_segs	| this instruction takes 3bytes (from 0x0060:0000 to 0002)
	.word 0, 0	| patch_kernel() in build.c puts DS value at kernel text address 0x0060:0004
			| this takes 4bytes (from 0x0060:0003 to 0006). After patch_kernel() in build.c, value becomes 0x00, (0x60, 0x00), 0x00
			| (...) is patched from patch_kernel() in build.c
setup_segs:
	cli	| disable interrupts
	mov ax, cs	| cs is 0x0060
	mov ds, ax	| ds==0x0060
	mov ax, 4	| cs:0004==0x0060:0004. Load DS value patched by patch_kernel in build.c 
	mov ds, ax	| ds now is proper value
	mov ss, ax	| ss now is proper value
	mov _scan_code, bx	| bx value(scan code for '=') from fsck1.s. scan_code is defined in tty.c
	mov sp, #_k_stack	| set sp to starting point of k_stack, which is k_stack[0]. defined in kernel/glo.h
	add sp, #K_STACK_BYTES	| set sp to point to the top of the kernel stack

	call _main	| start the main program of MINIX

|*===========================================================================*
|*                              surprise                                     *
|*===========================================================================*
_surprise:                      | This is where unexpected interrupts come.
        call save               | save the machine state
        call _unexpected_int    | go panic
        jmp _restart            | never executed


|*===========================================================================*
|*                              trp                                          *
|*===========================================================================*
_trp:                           | This is where unexpected traps come.
        call save               | save the machine state
        call _trap              | print a message
        jmp _restart            | this error is not fatal

|*===========================================================================*
|*                              divide                                       *
|*===========================================================================*
_divide:                        | This is where divide overflow traps come.
        call save               | save the machine state
        call _div_trap          | print a message
        jmp _restart            | this error is not fatal

|*===========================================================================*
|*                              clock_int                                    *
|*===========================================================================*
_clock_int:                     | Interrupt routine for the clock.
        call save               | save the machine state
        mov _int_mess+2,*CLOCK_TICK     | message.m_type = CLOCK_TICK
        mov ax,#_int_mess       | prepare to call interrupt(CLOCK, &intmess)
        push ax                 | push second parameter
        mov ax,*CLOCK           | prepare to push first parameter
        push ax                 | push first parameter
        call _interrupt         | this is the call
        jmp _restart            | continue execution

|*===========================================================================*
|*                              disk_int                                     *
|*===========================================================================*
_disk_int:                      | Interrupt routine for the floppy disk.
        call save               | save the machine state
        mov _int_mess+2,*DISKINT| build message for disk task
        mov ax,#_int_mess       | prepare to call interrupt(FLOPPY, &intmess)
        push ax                 | push second parameter
        mov ax,*FLOPPY          | prepare to push first parameter
        push ax                 | push first parameter
        call _interrupt         | this is the call
        jmp _restart            | continue execution

|*===========================================================================*
|*                              tty_int                                      *
|*===========================================================================*
_tty_int:                       | Interrupt routine for terminal input.
        call save               | save the machine state
        call _keyboard          | process a keyboard interrupt
        jmp _restart            | continue execution

|*===========================================================================*
|*                              s_call                                       *
|*===========================================================================*
_s_call:                        | System calls are vectored here.
        call save               | save the machine state
        mov bp,_proc_ptr        | use bp to access sys call parameters
        push 2(bp)              | push(pointer to user message) (was bx)
        push (bp)               | push(src/dest) (was ax)
        push _cur_proc          | push caller
        push 4(bp)              | push(SEND/RECEIVE/BOTH) (was cx)
        call _sys_call          | sys_call(function, caller, src_dest, m_ptr)
        jmp _restart            | jump to code to restart proc/task running

|*===========================================================================*
|*                              save                                         *
|*===========================================================================*
save:                           | save current process state in the proc[cur_proc]   
        push ds                 | stack: psw/cs/pc/ret addr/ds after this instruction
				| Refer to Bug4 
        push cs                 | prepare to restore ds
        pop ds                  | ds = cs. ds has now been set to cs
        mov ds,4                | word 4 in kernel text space contains ds value. Same as 'setup_segs' label
        pop ds_save             | stack: psw/cs/pc/ret addr
                                | save ds

        pop ret_save            | stack: psw/cs/pc
                                | save address of caller

        mov bx_save,bx          | save bx for later ; we need a free register
        mov bx,_proc_ptr        | start save set up; make bx point to save proc_ptr
        add bx,*OFF             | bx points to cs into proc structure - p_reg[NR_REGS] 
        pop PC-OFF(bx)          | store pc in proc table 
        pop csreg-OFF(bx)       | store cs in proc table
        pop PSW-OFF(bx)         | store psw 
        mov ssreg-OFF(bx),ss    | store ss
        mov SP-OFF(bx),sp       | sp as it was prior to interrupt
                                | save pc, cs, psw, ss, sp to proc_ptr 

| Why use stack from this point? Bug17 
        mov sp,bx               | now use sp to point into cs into proc structure
        mov bx,ds               | about to set ss
        mov ss,bx               | set ss to use stack 
        push ds_save            | start saving all the registers, sp first
        push es                 | save es between sp and bp
        mov es,bx               | es now references kernel ds too
        push bp                 | save bp
        push di                 | save di
        push si                 | save si
        push dx                 | save dx
        push cx                 | save cx
        push bx_save            | save original bx
        push ax                 | all registers now saved to restore them later

        mov sp,#_k_stack        | temporary stack for interrupts
        add sp,#K_STACK_BYTES   | set sp to top of temporary stack
        mov splimit,#_k_stack   | limit for temporary stack
        add splimit,#8          | splimit checks for stack overflow
        mov ax,ret_save         | ax = address of caller
        jmp (ax)                | return to caller

|*===========================================================================*
|*                              restart                                      *
|*===========================================================================*
_restart:                       | cur_proc is selected by pick_proc() in main.c at the first time. This routine sets up and runs a proc or task.
        cmp _cur_proc,#IDLE     | restart user; if cur_proc = IDLE, go idle
        je idle                 | no user is runnable, jump to idle routine
        cli                     | disable interrupts

| Restore registers from proc_ptr = proc[cur_proc] 
        mov sp,_proc_ptr        | return to user, fetch regs from proc table
        pop ax                  | start restoring registers
        pop bx                  | restore bx
        pop cx                  | restore cx
        pop dx                  | restore dx
        pop si                  | restore si
        pop di                  | restore di
        mov lds_low,bx          | lds_low contains bx
        mov bx,sp               | bx points to saved bp register
        mov bp,SPLIM-ROFF(bx)   | splimit = p_splimit
        mov splimit,bp          | ditto
        mov bp,dsreg-ROFF(bx)   | bp = ds
        mov lds_low+2,bp        | lds_low+2 contains ds
        pop bp                  | restore bp
        pop es                  | restore es
        mov sp,SP-ROFF(bx)      | restore sp
        mov ss,ssreg-ROFF(bx)   | restore ss using the value of ds

| Restore psw, cs, pc of user/task by pushing them. p369
        push PSW-ROFF(bx)       | push psw
        push csreg-ROFF(bx)     | push cs
        push PC-ROFF(bx)        | push pc
        | Now, interrupt bit in psw is turn off. p369
        lds bx,lds_low          | restore ds and bx in one shot
        iret                    | return to the proc[cur_proc] and continue executing 

|*===========================================================================*
|*                              idle                                         *
|*===========================================================================*
idle:                           | executed when there is no work 
        sti                     | enable interrupts
L3:     wait                    | just idle while waiting for interrupt
        jmp L3                  | loop until interrupt

|*===========================================================================*
|*                              data                                         *
|*===========================================================================*
.data
begdata:
_sizes:	.word 0x526F	| this must be the first entry of data (magic #)
	.zerow 7	| build table uses pre word and these words. so, 8 words
bx_save: .word 0                | storage for bx
ds_save: .word 0                | storage for ds
ret_save:.word 0                | storage for return address
lds_low: .word 0,0              | storage used for restoring bx

.bss
begbss:
