libstd: common.o sendrec.o fork.o wait.o exit.o open.o close.o exec.o strcpy.o strcmp.o strlen.o copy.o read.o write.o lseek.o fgets.o getc.o putc.o printf.o fflush.o
	ar -cvq libstd.a common.o sendrec.o fork.o wait.o exit.o open.o close.o exec.o strcpy.o strcmp.o strlen.o copy.o read.o write.o lseek.o fgets.o getc.o putc.o printf.o fflush.o

common.o: common.c
	gcc -m32 -c $(CFLAGS) common.c

sendrec.o: sendrec.asm
	nasm $(ASMFLAGS) -f elf sendrec.asm

fork.o: fork.c
	gcc -m32 -c $(CFLAGS) fork.c

wait.o: wait.c
	gcc -m32 -c $(CFLAGS) wait.c

exit.o: exit.c
	gcc -m32 -c $(CFLAGS) exit.c

open.o: open.c
	gcc -m32 -c $(CFLAGS) open.c

close.o: close.c
	gcc -m32 -c $(CFLAGS) close.c

exec.o: exec.c
	gcc -m32 -c $(CFLAGS) exec.c

strcpy.o: strcpy.c
	gcc -m32 -c $(CFLAGS) strcpy.c

strcmp.o: strcmp.c
	gcc -m32 -c $(CFLAGS) strcmp.c

strlen.o: strlen.c
	gcc -m32 -c $(CFLAGS) strlen.c

copy.o: copy.c
	gcc -m32 -c $(CFLAGS) copy.c

read.o: read.c
	gcc -m32 -c $(CFLAGS) read.c

write.o: write.c
	gcc -m32 -c $(CFLAGS) write.c

lseek.o: lseek.c
	gcc -m32 -c $(CFLAGS) lseek.c

fgets.o: fgets.c
	gcc -m32 -c $(CFLAGS) fgets.c

getc.o: getc.c
	gcc -m32 -c $(CFLAGS) getc.c

putc.o: putc.c
	gcc -m32 -c $(CFLAGS) putc.c

printf.o: printf.c
	gcc -m32 -c $(CFLAGS) printf.c

fflush.o: fflush.c
	gcc -m32 -c $(CFLAGS) fflush.c

clean:
	rm -rf *.o libstd.a		
