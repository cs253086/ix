SECTION .bss
	global fs_stack

fs_stack:
	RESB 4096	; if the value changes, SERVER_STACK_SIZE should be also changed accordingly
