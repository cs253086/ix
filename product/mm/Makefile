LDIR=-L../lib/util -L../lib/std -L../lib/debug_print -L ../lib/sys
LIB=-lstd -ldebug_print -lutil -lsys


mm: mm_head.o main.o mm_low.o alloc.o forkexit.o exec.o
	# --entry=main : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=main --section-start=.text=$(PROCESS_STARTING_VIR_ADDRESS) -o mm mm_head.o main.o mm_low.o alloc.o forkexit.o exec.o $(LDIR) $(LIB)
	#ld --verbose=5 -melf_i386 --entry=main --section-start=.text=$(starting_addr) -o mm main.o mm_low.o alloc.o forkexit.o $(LDIR) $(LIB)
   
 
mm_head.o: mm_head.asm
	nasm $(ASMFLAGS) -f elf mm_head.asm

main.o: main.c
	gcc -c $(CFLAGS) main.c

mm_low.o: mm_low.asm
	nasm $(ASMFLAGS) -f elf mm_low.asm

alloc.o: alloc.c
	gcc -c $(CFLAGS) alloc.c

forkexit.o: forkexit.c
	gcc -c $(CFLAGS) forkexit.c

exec.o: exec.c
	gcc -c $(CFLAGS) exec.c

clean: 
	rm -rf *.o mm
