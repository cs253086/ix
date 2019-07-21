# -fno-asynchronous-unwind-tables : removes unwind information (exception handling). 
# 		In other words, it removes .eh_frame section. Without the option, kernel would not work after bootloader 
# 		Executing a binary under operating system (e.g Linux), the option may be needed, but it is not in a new kernel.
# 		So, the option adjust the binary to make it work properly without the exception handling code.
# -fno-stack-protector : disable stack protection
export CFLAGS=-g -fno-asynchronous-unwind-tables -fno-stack-protector -DDEBUG -DTEST
export ASMFLAGS=-g
export PROCESS_STARTING_VIR_ADDRESS=0x40000
image:
	cd lib; $(MAKE)
	cd boot; $(MAKE)
	cd build; $(MAKE)
	cd kernel; $(MAKE)
	cd mm; $(MAKE) 
	cd fs; $(MAKE) 
	cd init; $(MAKE)
	build/build boot/bootloader kernel/kernel mm/mm fs/fs init/init ix.bin

clean:
	cd lib; $(MAKE) clean
	cd boot; $(MAKE) clean
	cd build; $(MAKE) clean
	cd kernel; $(MAKE) clean
	cd mm; $(MAKE) clean
	cd fs; $(MAKE) clean
	cd init; $(MAKE) clean
	rm -rf ix.bin
