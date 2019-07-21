%define SEND		1
%define RECEIVE		2
%define SEND_RECEIVE	3

%define SYS_CALL_VECTOR	0x31	; defined in kernel/idt.h

	global send, receive, send_receive
send:
	mov	ecx, SEND
	jmp	sendrec_common

receive:
	mov	ecx, RECEIVE	
	jmp	sendrec_common

send_receive:			; send and wait for reply
	mov	ecx, SEND_RECEIVE
	jmp	sendrec_common

sendrec_common:
	push	ebp
	mov	ebp, esp
	mov	eax, [ebp + 8]	; src/dst
	mov	ebx, [ebp + 12]	; &message
	int	SYS_CALL_VECTOR
	pop	ebp
	ret
