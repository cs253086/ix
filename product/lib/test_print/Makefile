libtest_print: test_print.o
	ar -cvq libtest_print.a test_print.o  

test_print.o: test_print.c
	gcc -m32 -c $(CFLAGS) test_print.c 

clean:
	rm -rf *.o libtest_print.a		
