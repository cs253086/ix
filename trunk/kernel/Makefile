LDIR=-L../lib/util -L../lib/syscall -L../lib/primitive_print -L../lib/debug_print -L../lib/test_print
LIB=-lsyscall -lprimitive_print -ldebug_print -lutil -ltest_print


kernel: kernel_head.o kernel_low.o 8259_pic.o kernel_lib.o idt.o exceptions.o irq.o paging.o clock.o tty.o told_kernel.o main.o test.o
	# --entry=ix : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=ix --section-start=.text=0x00007E00 -o kernel kernel_head.o kernel_low.o 8259_pic.o kernel_lib.o idt.o paging.o exceptions.o irq.o clock.o tty.o told_kernel.o main.o test.o $(LDIR) $(LIB)
    
kernel_head.o: kernel_head.asm
	nasm $(ASMFLAGS) -f elf kernel_head.asm

kernel_low.o: kernel_low.asm
	nasm $(ASMFLAGS) -f elf kernel_low.asm

8259_pic.o: 8259_pic.asm
	nasm $(ASMFLAGS) -f elf 8259_pic.asm

kernel_lib.o: kernel_lib.c
	gcc -m32 -c $(CFLAGS) kernel_lib.c

idt.o: idt.c
	gcc -m32 -c $(CFLAGS) idt.c

exceptions.o: exceptions.c
	gcc -m32 -c $(CFLAGS) exceptions.c

irq.o: irq.c
	gcc -m32 -c $(CFLAGS) irq.c

paging.o: paging.c
	gcc -m32 -c $(CFLAGS) paging.c

clock.o: clock.c
	gcc -m32 -c $(CFLAGS) clock.c

tty.o: tty.c
	gcc -m32 -c $(CFLAGS) tty.c

told_kernel.o: told_kernel.c
	gcc -m32 -c $(CFLAGS) told_kernel.c

main.o: main.c
	gcc -m32 -c $(CFLAGS) main.c

test.o: test.c
	gcc -m32 -c $(CFLAGS) test.c
clean: 
	rm -rf *.o kernel
