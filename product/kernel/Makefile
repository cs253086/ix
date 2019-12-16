LDIR=-L../lib/util -L../lib/std -L ../lib/primitive_print -L../lib/debug_print 
LIB=-lstd -lutil -lprimitive_print -ldebug_print


kernel: kernel_head.o kernel_low.o 8259_pic.o kernel_lib.o idt.o exceptions.o irq.o paging.o clock.o tty.o mem.o sys.o ata.o main.o
	# --entry=ix : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=ix --section-start=.text=0x00007E00 -o kernel kernel_head.o kernel_low.o 8259_pic.o kernel_lib.o idt.o paging.o exceptions.o irq.o clock.o tty.o mem.o sys.o ata.o main.o $(LDIR) $(LIB)
    
kernel_head.o: kernel_head.asm
	nasm $(ASMFLAGS) -f elf kernel_head.asm

kernel_low.o: kernel_low.asm
	nasm $(ASMFLAGS) -f elf kernel_low.asm

8259_pic.o: 8259_pic.asm
	nasm $(ASMFLAGS) -f elf 8259_pic.asm

kernel_lib.o: kernel_lib.c
	gcc -c $(CFLAGS) kernel_lib.c

idt.o: idt.c
	gcc -c $(CFLAGS) idt.c

exceptions.o: exceptions.c
	gcc -c $(CFLAGS) exceptions.c

irq.o: irq.c
	gcc -c $(CFLAGS) irq.c

paging.o: paging.c
	gcc -c $(CFLAGS) paging.c

clock.o: clock.c
	gcc -c $(CFLAGS) clock.c

tty.o: tty.c
	gcc -c $(CFLAGS) tty.c

mem.o: mem.c
	gcc -c $(CFLAGS) mem.c

sys.o: sys.c
	gcc -c $(CFLAGS) sys.c

ata.o: ata.c
	gcc -c $(CFLAGS) ata.c

main.o: main.c
	gcc -c $(CFLAGS) main.c

clean: 
	rm -rf *.o kernel
