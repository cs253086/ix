LDIR=-L../lib/util -L../lib/std -L ../lib/primitive_print -L../lib/debug_print -L../lib/sys
LIB=-lstd -lprimitive_print -ldebug_print -lutil -lsys


fs: fs_head.o main.o fs_lib.o device.o inode.o super_block.o forkexit.o open.o close.o path.o read_write.o lseek.o
	# --entry=main : sets the entry point of binary to 'ix'. Default is '_start'
	# --section-start=.text=0x00007E00 : sets the loading address of .text to 0x00007E00
	ld -melf_i386 --entry=main --section-start=.text=$(PROCESS_STARTING_VIR_ADDRESS) -o fs fs_head.o main.o fs_lib.o device.o inode.o super_block.o forkexit.o open.o close.o path.o read_write.o lseek.o $(LDIR) $(LIB)
   
fs_head.o: fs_head.asm
	nasm $(ASMFLAGS) -f elf fs_head.asm

main.o: main.c
	gcc -c $(CFLAGS) main.c

fs_lib.o: fs_lib.c
	gcc -c $(CFLAGS) fs_lib.c

device.o: device.c
	gcc -c $(CFLAGS) device.c

inode.o: inode.c
	gcc -c $(CFLAGS) inode.c

super_block.o: super_block.c
	gcc -c $(CFLAGS) super_block.c

forkexit.o: forkexit.c
	gcc -c $(CFLAGS) forkexit.c

close.o: close.c
	gcc -c $(CFLAGS) close.c

open.o: open.c
	gcc -c $(CFLAGS) open.c

path.o: path.c
	gcc -c $(CFLAGS) path.c

read_write.o: read_write.c
	gcc -c $(CFLAGS) read_write.c

lseek.o: lseek.c
	gcc -c $(CFLAGS) lseek.c

clean: 
	rm -rf *.o fs
