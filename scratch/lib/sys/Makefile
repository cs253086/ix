libsys: libsys.o
	ar -cvq libsys.a libsys.o

libsys.o: libsys.c
	gcc -m32 -c $(CFLAGS) libsys.c

clean:
	rm -rf *.o libsys.a		
