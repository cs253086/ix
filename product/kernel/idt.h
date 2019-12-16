#ifndef IDT_H
#define IDT_H
#include <stdint.h>

#define NUM_IRQ_PIC	8
#define IRQ8_INT_NUM	0x28
#define EXTRA_VECTORS	0x10	// for extra vectors (e.g. SYS_CALL_VECTOR)
#define IDT_SIZE	IRQ8_INT_NUM + NUM_IRQ_PIC + EXTRA_VECTORS
#define PRESENT		0b10000000	/* set for descriptor present */
#define STORAGE_SEG_INT_GATE	0b00000000
#define INT_GATE_TYPE	(INT_286_GATE | DESC_386_BIT)	/* http://wiki.osdev.org/Interrupt_Descriptor_Table */
#define INT_286_GATE         0b00000110	/* interrupt gate, used for all vectors */
#define DESC_386_BIT  0b00001000 /* 386 types are obtained by ORing with this */

#define DPL_SHIFT            5
#define SHIFT_FOR_OFFSET_HIGH   16 /* shift constant for offset_high in idt_descriptor */

#define DIVIDE_ERROR_VECTOR	0
#define DEBUG_VECTOR		1
#define NMI_VECTOR		2
#define BREAKPOINT_VECTOR	3
#define	OVERFLOW_VECTOR		4
#define BOUNDS_VECTOR        	5  /* bounds check failed */                                                             
#define INVAL_OP_VECTOR      	6  /* invalid opcode */
#define DEVICE_NOT_VECTOR    	7  /* coprocessor not available */                                                       
#define DOUBLE_FAULT_VECTOR  	8
#define COPROC_SEG_VECTOR    	9  /* coprocessor segment overrun */                                                     
#define INVAL_TSS_VECTOR    	10  /* invalid TSS */
#define SEG_NOT_VECTOR      	11  /* segment not present */                                                             
#define STACK_FAULT_VECTOR  	12  /* stack exception */
#define PROTECTION_VECTOR   	13  /* general protection */          
#define SYS_CALL_VECTOR		0x31

typedef struct idt_descriptr {
   uint16_t offset_low; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t pad;      // unused, set to 0
   uint8_t p_dpl_type; // |P|DPL|0|TYPE| 
   uint16_t offset_high; // offset bits 16..31
} idt_descriptor_t;

/* Why need __attribute__((packed)) ?
 * : Remeber default alignment for 32bit and size of gidtr is 
 * not multiple of 32
 * So, there is some padding between each member of struct gidtr 
*/
typedef struct __attribute__((packed)) gidtr {
  uint16_t limit;
  uint32_t base; 
} gidtr_t;

typedef struct idt_gate {
	void (*gate) (void);
	uint8_t vec_num;
	uint8_t privilege;	
} idt_gate_t;

static void setup_gate(uint32_t offset, uint8_t vec_num, uint8_t p_dpl_type);


#endif /* IDT_H */
