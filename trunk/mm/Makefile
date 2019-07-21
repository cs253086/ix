LDIR=-L../lib/util -L../lib/syscall -L../lib/primitive_print -L../lib/debug_print -L../lib/test_print -L ../lib/tell_kernel
LIB=-lsyscall -lprimitive_print -ldebug_print -lutil -ltest_print -ltell_kernel


mm: mm_head.o main.o mm_low.o alloc.o forkexit.o 
	# --entry=main : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=main --section-start=.text=$(PROCESS_STARTING_VIR_ADDRESS) -o mm mm_head.o main.o mm_low.o alloc.o forkexit.o $(LDIR) $(LIB)
	#ld --verbose=5 -melf_i386 --entry=main --section-start=.text=$(starting_addr) -o mm main.o mm_low.o alloc.o forkexit.o $(LDIR) $(LIB)
   
 
mm_head.o: mm_head.asm
	nasm $(ASMFLAGS) -f elf mm_head.asm

main.o: main.c
	gcc -m32 -c $(CFLAGS) main.c

mm_low.o: mm_low.asm
	nasm $(ASMFLAGS) -f elf mm_low.asm

alloc.o: alloc.c
	gcc -m32 -c $(CFLAGS) alloc.c

forkexit.o: forkexit.c
	gcc -m32 -c $(CFLAGS) forkexit.c

clean: 
	rm -rf *.o mm
