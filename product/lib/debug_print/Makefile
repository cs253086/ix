libdebug_print: debug_print.o
	ar -cvq libdebug_print.a debug_print.o  

debug_print.o: debug_print.c
	gcc -m32 -c $(CFLAGS) debug_print.c 

clean:
	rm -rf *.o libdebug_print.a		
