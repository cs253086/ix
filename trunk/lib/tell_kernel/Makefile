tell_kernel: tell_kernel.o
	ar -cvq libtell_kernel.a tell_kernel.o

tell_kernel.o: tell_kernel.c
	gcc -m32 -c $(CFLAGS) tell_kernel.c

clean:
	rm -rf *.o libtell_kernel.a		
