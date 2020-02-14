CFLAGS= -DDEBUG
obj= mpx88.s main.s glo.s proc.s system.s tty.s clock.s memory.s floppy.s dummy_task.s task_array.s klib88.s
l=/usr/lib

kernel:	$(obj) 
	asld -o kernel $(obj) ../lib/standalone1.s $l/libc.a $l/end.s 
	@echo kernel build done

main.s: main.c
	cc -c $(CFLAGS) main.c 
	@echo main.s build done

glo.s: 	glo.c
	cc -c $(CFLAGS) glo.c
	@echo glo.s build done

proc.s: proc.c
	cc -c $(CFLAGS) proc.c
	@echo proc.s build done

system.s:	system.c
		cc -c $(CFLAGS) system.c
		@echo system.s build done

tty.s:	tty.c
	cc -c $(CFLAGS) tty.c
	@echo tty.s build done

clock.s:	clock.c
		cc -c $(CFLAGS) clock.c
		@echo clock.s build done

memory.s:	memory.c
		cc -c $(CFLAGS) memory.c
		@echo memory.s build done

floppy.s:	floppy.c
		cc -c $(CFLAGS) floppy.c
		@echo floppy.s build done
	
dummy_task.s:	dummy_task.c
		cc -c $(CFLAGS) dummy_task.c
		@echo dummy_task.s build done	

task_array.s: 	task_array.c
		cc -c $(CFLAGS) task_array.c
		@echo task_array.s build done	

### make clean ###
clean:
	rm -rf main.s glo.s proc.s system.s tty.s clock.s memory.s floppy.s dummy_task.s task_array.s kernel

