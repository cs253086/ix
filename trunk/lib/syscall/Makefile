libsyscall: common.o sendrec.o fork.o wait.o
	ar -cvq libsyscall.a common.o sendrec.o fork.o wait.o

common.o: common.c
	gcc -m32 -c $(CFLAGS) common.c

sendrec.o: sendrec.asm
	nasm $(ASMFLAGS) -f elf sendrec.asm

fork.o: fork.c
	gcc -m32 -c $(CFLAGS) fork.c

wait.o: wait.c
	gcc -m32 -c $(CFLAGS) wait.c

clean:
	rm -rf *.o libsyscall.a		
