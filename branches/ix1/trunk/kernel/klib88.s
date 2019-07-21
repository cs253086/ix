| This file contains a number of assembly code utility routines needed by the
| kernel.  They are:
|
|   phys_copy:  copies data from anywhere to anywhere in memory
|   lock:       disable interrupts
|   unlock:     enable interrupts
|   restore:    restore interrupts (enable/disabled) as they were before lock()
|   get_chrome: returns 0 is display is monochrome, 1 if it is color
|   get_byte:   reads a byte from a user program and returns it as value
|   port_in:    inputs data from an I/O port
|   port_out:   give command data to hardware controller
|   reboot:     reboot for CTRL-ALT-DEL
|   wreboot:    wait for character then reboot 
|
| Note
| 1. underscore variable is used to communicate between C and assembly.
|    variables in C/assembly should be with underscore prefix to communicate with assembly/C

| The following procedures are defined in this file and called from outside it.
.globl _phys_copy, _lock, _unlock, _restore, _get_chrome, _get_byte, _port_in, _port_out, _wreboot, _reboot

| The following variables and data structures are defined in this file and used outside
.globl _vec_table
|.globl splimit

|*===========================================================================*
|*                              phys_copy                                    *
|*===========================================================================*
| This routine copies a block of physical memory.  It is called by:
|    phys_copy( (long) source, (long) destination, (long) bytecount)

_phys_copy:
        pushf                   | save flags
        cli                     | disable interrupts. Why cli? Refer to http://en.wikipedia.org/wiki/Interrupt_flag
        push bp                 | save the registers
        push ax                 | save ax
        push bx                 | save bx
        push cx                 | save cx
        push dx                 | save dx
        push si                 | save si
        push di                 | save di
        push ds                 | save ds
        push es                 | save es
        mov bp,sp               | set bp to point to saved es

  L0:   mov ax,28(bp)           | ax = high-order word of 32-bit destination
        mov di,26(bp)           | di = low-order word of 32-bit destination
        mov cx,*4               | start extracting click number from dest
  L1:   rcr ax,*1               | click number is destination address / 16
        rcr di,*1               | it is used in segment register for copy
        loop L1                 | 4 bits of high-order word are used
        mov es,di               | es = destination click

        mov ax,24(bp)           | ax = high-order word of 32-bit source
        mov si,22(bp)           | si = low-order word of 32-bit source
        mov cx,*4               | start extracting click number from source
  L2:   rcr ax,*1               | click number is source address / 16
        rcr si,*1               | it is used in segment register for copy
        loop L2                 | 4 bits of high-order word are used
        mov ds,si               | ds = source click

        mov di,26(bp)           | di = low-order word of dest address
        and di,*0x000F          | di = offset from paragraph # in es
        mov si,22(bp)           | si = low-order word of source address
        and si,*0x000F          | si = offset from paragraph # in ds

        mov dx,32(bp)           | dx = high-order word of byte count
        mov cx,30(bp)           | cx = low-order word of byte count

        test cx,#0x8000         | if bytes >= 32768, only do 32768 
        jnz L3                  | per iteration
        test dx,#0xFFFF         | check high-order 17 bits to see if bytes
        jnz L3                  | if bytes >= 32768 then go to L3
        jmp L4                  | if bytes < 32768 then go to L4
  L3:   mov cx,#0x8000          | 0x8000 is unsigned 32768
  L4:   mov ax,cx               | save actual count used in ax; needed later

        test cx,*0x0001         | should we copy a byte or a word at a time?
        jz L5                   | jump if even
        rep                     | copy 1 byte at a time
        movb                    | byte copy
        jmp L6                  | check for more bytes

  L5:   shr cx,*1               | word copy
        rep                     | copy 1 word at a time
        movw                    | word copy

  L6:   mov dx,32(bp)           | decr count, incr src & dst, iterate if needed
        mov cx,30(bp)           | dx || cx is 32-bit byte count
        xor bx,bx               | bx || ax is 32-bit actual count used
        sub cx,ax               | compute bytes - actual count
        sbb dx,bx               | dx || cx is # bytes not yet processed
        or cx,cx                | see if it is 0
        jnz L7                  | if more bytes then go to L7
        or dx,dx                | keep testing
        jnz L7                  | if loop done, fall through

        pop es                  | restore all the saved registers
        pop ds                  | restore ds
        pop di                  | restore di
        pop si                  | restore si
        pop dx                  | restore dx
        pop cx                  | restore cx
        pop bx                  | restore bx
        pop ax                  | restore ax
        pop bp                  | restore bp
        popf                    | restore flags
        ret                     | return to caller

L7:     mov 32(bp),dx           | store decremented byte count back in mem
        mov 30(bp),cx           | as a long
        add 26(bp),ax           | increment destination
        adc 28(bp),bx           | carry from low-order word
        add 22(bp),ax           | increment source
        adc 24(bp),bx           | carry from low-order word
        jmp L0                  | start next iteration

|*===========================================================================*
|*                              cp_mess                                      *
|*===========================================================================*
| This routine is makes a fast copy of a message from anywhere in the address
| space to anywhere else.  It also copies the source address provided as a
| parameter to the call into the first word of the destination message.
| It is called by:
|    cp_mess(src, src_clicks, src_offset, dst_clicks, dst_offset)
| where all 5 parameters are shorts (16-bits).
|
| Note that the message size, 'Msize' is in WORDS (not bytes) and must be set
| correctly.  Changing the definition of message the type file and not changing
| it here will lead to total disaster.
| This routine destroys ax.  It preserves the other registers.

Msize = 12                      | size of a message in 16-bit words
_cp_mess:
        push bp                 | save bp
        push es                 | save es
        push ds                 | save ds
        mov bp,sp               | index off bp because machine can't use sp
        pushf                   | save flags
        cli                     | disable interrupts
        push cx                 | save cx
        push si                 | save si
        push di                 | save di

        mov ax,8(bp)            | ax = process number of sender
        mov di,16(bp)           | di = offset of destination buffer
        mov es,14(bp)           | es = clicks of destination
        mov si,12(bp)           | si = offset of source message
        mov ds,10(bp)           | ds = clicks of source message
        seg es                  | segment override prefix
        mov (di),ax             | copy sender's process number to dest message
        add si,*2               | don't copy first word
        add di,*2               | don't copy first word
        mov cx,*Msize-1         | remember, first word doesn't count
        rep                     | iterate cx times to copy 11 words
        movw                    | copy the message

        pop di                  | restore di
        pop si                  | restore si
        pop cx                  | restore cs
        popf                    | restore flags
        pop ds                  | restore ds
        pop es                  | restore es
        pop bp                  | restore bp
        ret                     | that's all folks!

|*===========================================================================*
|*                              lock                                         *
|*===========================================================================*
| Disable CPU interrupts.
_lock:
        pushf                   | save flags(psw) on stack. http://www.muslim-programers.com/1/inst.html#PUSHF
        cli                     | disable interrupts
        pop lockvar             | save flags for possible restoration later
        ret                     | return to caller

|*===========================================================================*
|*                              unlock                                       *
|*===========================================================================*
| Enable CPU interrupts.
_unlock:
        sti                     | enable interrupts
        ret                     | return to caller

|*===========================================================================*
|*                              restore                                      *
|*===========================================================================*
| Restore enable/disable bit to the value it had before last lock.
_restore:
        push lockvar            | push flags as they were before previous lock
        popf                    | restore flags
        ret                     | return to caller

|*===========================================================================*
|*                              get_chrome                                   *
|*===========================================================================*
| This routine calls the BIOS to find out if the display is monochrome or 
| color.  The drivers are different, as are the video ram addresses, so we
| need to know.
_get_chrome:
        int 0x11                | call the BIOS to get equipment type
        andb al,#0x30           | isolate color/mono field
        cmpb al,*0x30           | 0x30 is monochrome
        je getchr1              | if monochrome then go to getchr1
        mov ax,#1               | color = 1
        ret                     | color return
getchr1: xor ax,ax              | mono = 0
        ret                     | monochrome return

|*===========================================================================*
|*                              get_byte                                     *
|*===========================================================================*
| This routine is used to fetch a byte from anywhere in memory.
| The call is:
|     c = get_byte(seg, off)
| where
|     'seg' is the value to put in es
|     'off' is the offset from the es value
_get_byte:
        push bp                 | save bp
        mov bp,sp               | we need to access parameters
        push es                 | save es
        mov es,4(bp)            | load es with segment value
        mov bx,6(bp)            | load bx with offset from segment
        seg es                  | go get the byte
        movb al,(bx)            | al = byte
        xorb ah,ah              | ax = byte
        pop es                  | restore es
        pop bp                  | restore bp
        ret                     | return to caller

|*===========================================================================*
|*                              port_in                                      *
|*===========================================================================*
| port_in(port, &value) reads from port 'port' and puts the result in 'value'.
_port_in:
        push bx                 | save bx
        mov bx,sp               | index off bx
        push ax                 | save ax
        push dx                 | save dx
        mov dx,4(bx)            | dx = port
        in                      | input 1 byte
        xorb ah,ah              | clear ah
        mov bx,6(bx)            | fetch address where byte is to go
        mov (bx),ax             | return byte to caller in param
        pop dx                  | restore dx
        pop ax                  | restore ax
        pop bx                  | restore bx
        ret                     | return to caller

|*===========================================================================*
|*                              port_out                                     *
|*===========================================================================*
| port_out(port, value) writes 'value' on the I/O port 'port'.

_port_out:
        push bx                 | save bx
        mov bx,sp               | index off bx
        push ax                 | save ax
        push dx                 | save dx
        mov dx,4(bx)            | dx = port
        mov ax,6(bx)            | ax = value
        out                     | output 1 byte
        pop dx                  | restore dx
        pop ax                  | restore ax
        pop bx                  | restore bx
        ret                     | return to caller

|*===========================================================================*
|*                              reboot & wreboot                             *
|*===========================================================================*
| This code reboots the PC

_reboot:
        cli                     | disable interrupts
        mov ax,#0x20            | re-enable interrupt controller
        out 0x20
        call resvec             | restore the vectors in low core
        int 0x19                | reboot the PC

_wreboot:
        cli                     | disable interrupts
        mov ax,#0x20            | re-enable interrupt controller
        out 0x20
        call resvec             | restore the vectors in low core
        xor ax,ax               | wait for character before continuing
        int 0x16                | get char
        int 0x19                | reboot the PC

| Restore the interrupt vectors in low core.
resvec: cld
        mov cx,#2*71
        mov si,#_vec_table
        xor di,di
        mov es,di
        rep
        movw
        ret

| Some library routines use exit, so this label is needed.
| Actual calls to exit cannot occur in the kernel.
.globl _exit
_exit:  sti
        jmp _exit

.data
lockvar:	.word 0         | place to store flags for lock()/restore()
_vec_table:     .zerow 142      | storage for interrupt vectors
splimit:        .word 0         | stack limit for current task (kernel only)
