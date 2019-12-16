libprimitive_print: primitive_print.o
	ar -cvq libprimitive_print.a primitive_print.o  

primitive_print.o: primitive_print.c
	gcc -m32 -c $(CFLAGS) primitive_print.c 

clean:
	rm -rf *.o libprimitive_print.a		
