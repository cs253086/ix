LDIR=-L../lib/util -L../lib/syscall -L../lib/primitive_print -L../lib/debug_print -L../lib/test_print -L../lib/tell_kernel
LIB=-lsyscall -lprimitive_print -ldebug_print -lutil -ltest_print -ltell_kernel


fs: fs_head.o main.o
	# --entry=main : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=main --section-start=.text=$(PROCESS_STARTING_VIR_ADDRESS) -o fs fs_head.o main.o $(LDIR) $(LIB)
   
 
fs_head.o: fs_head.asm
	nasm $(ASMFLAGS) -f elf fs_head.asm

main.o: main.c
	gcc -m32 -c $(CFLAGS) main.c

clean: 
	rm -rf *.o fs
