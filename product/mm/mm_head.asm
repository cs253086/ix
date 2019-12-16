SECTION .bss
	global mm_stack

mm_stack:
	RESB 4096	; if the value changes, SERVER_STACK_SIZE should be also changed accordingly
