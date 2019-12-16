#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

void disable_interrupts(void);	// lib/util/util.asm
void enable_interrupts(void);	// lib/util/util.asm
void restore_flags(void);	// lib/util/util.asm
void outb(uint16_t port, uint8_t value);	// lib/util/util.asm
uint8_t inb(uint16_t port);		// lib/util/util.asm
uint16_t inw(uint16_t port);		// lib/util/util.asm

#endif	/* UTIL_H */ 
