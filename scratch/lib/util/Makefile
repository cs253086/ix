libutil: util.o 
	ar -cvq libutil.a util.o

util.o: util.asm
	nasm $(ASMFLAGS) -f elf util.asm

clean:
	rm -rf *.o libutil.a		
