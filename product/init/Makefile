LDIR=-L../lib/util -L../lib/std -L../lib/debug_print 
LIB=-lstd -ldebug_print -lutil


init: init_head.o main.o 
	# --entry=main : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=main --section-start=.text=$(PROCESS_STARTING_VIR_ADDRESS) -o init init_head.o main.o $(LDIR) $(LIB)

init_head.o: init_head.asm
	nasm $(ASMFLAGS) -f elf init_head.asm

main.o: main.c
	gcc -c $(CFLAGS) main.c

clean: 
	rm -rf *.o init
