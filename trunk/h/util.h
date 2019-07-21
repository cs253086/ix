#ifndef UTIL_H
#define UTIL_H

void disable_interrupts(void);	// lib/util/util.asm
void enable_interrupts(void);	// lib/util/util.asm
void restore_flags(void);	// lib/util/util.asm
void outb(int port, int value);	// lib/util/util.asm
unsigned char inb(int port);		// lib/util/util.asm

#endif	/* UTIL_H */ 
