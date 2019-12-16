# -fno-asynchronous-unwind-tables : removes unwind information (exception handling). 
# 		In other words, it removes .eh_frame section. Without the option, kernel would not work after bootloader 
# 		Executing a binary under operating system (e.g Linux), the option may be needed, but it is not in a new kernel.
# 		So, the option adjust the binary to make it work properly without the exception handling code.
# -fno-stack-protector : disable stack protection
# -fno-builtin : disable built-in functions (e.g printf) 
# -mtune=i486 : generated code is optimized for i486 (e.g 'cmovs' instruction doesn't exist on i486). To reproduce the issue, without this option, try 'int32_t tmp = 1024; tmp = tmp / 1024. http://en.wikipedia.org/wiki/X86_instruction_listings

IX_SRC_ROOT=$(shell pwd)

# include path includes heade files to be used by users(application side)
INCLUDE_PATH=$(IX_SRC_ROOT)/include
export CFLAGS=-m32 -mtune=i486 -g -O0 -fno-asynchronous-unwind-tables -fno-stack-protector -fno-builtin -DDEBUG -DTEST -I $(INCLUDE_PATH)
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
	cd sh; $(MAKE)
	cd bin; $(MAKE)
	build/build boot/bootloader kernel/kernel mm/mm fs/fs init/init ix.bin

clean:
	cd lib; $(MAKE) clean
	cd boot; $(MAKE) clean
	cd build; $(MAKE) clean
	cd kernel; $(MAKE) clean
	cd mm; $(MAKE) clean
	cd fs; $(MAKE) clean
	cd init; $(MAKE) clean
	cd sh; $(MAKE) clean
	cd bin; $(MAKE) clean
	rm -rf ix.bin
