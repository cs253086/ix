#include "../h/common.h"
#include "common.h"
#include "idt.h"
#include "kernel_low.h"

gidtr_t idtr;
idt_descriptor_t idt[IDT_SIZE];

void setup_idt()
{
	int i;
	// create idt descriptors (exceptions and interrupt vectors including IRQs)
	idt_gate_t idt_gates[] = {
		{exception0, DIVIDE_ERROR_VECTOR, RING0},
		{exception1, DEBUG_VECTOR, RING0},
		{exception2, NMI_VECTOR, RING0},
		{exception3, BREAKPOINT_VECTOR, RING0},
		{exception4, OVERFLOW_VECTOR, RING0},
		{exception5, BOUNDS_VECTOR, RING0},
		{exception6, INVAL_OP_VECTOR, RING0},
		{exception7, DEVICE_NOT_VECTOR, RING0},
		{exception8, DOUBLE_FAULT_VECTOR, RING0},
		{exception9, COPROC_SEG_VECTOR, RING0},
		{exception10, INVAL_TSS_VECTOR, RING0},
		{exception11, SEG_NOT_VECTOR, RING0},
		{exception12, STACK_FAULT_VECTOR, RING0},
		{exception13, PROTECTION_VECTOR, RING0},
		{irq0, 0x20, RING0},
		{irq1, 0x21, RING0},
		{irq2, 0x22, RING0},
		{irq3, 0x23, RING0},
		{irq4, 0x24, RING0},
		{irq5, 0x25, RING0},
		{irq6, 0x26, RING0},
		{irq7, 0x27, RING0},
		{irq8, 0x28, RING0},
		{irq9, 0x29, RING0},
		{irq10, 0x2a, RING0},
		{irq11, 0x2b, RING0},
		{irq12, 0x2c, RING0},
		{irq13, 0x2d, RING0},
		{irq14, 0x2e, RING0},
		{irq15, 0x2f, RING0},
		
		{sys_call_low, SYS_CALL_VECTOR, RING0}
	};

	// setup idtr
	idtr.limit = sizeof(idt) - 1;
	idtr.base = viraddr_to_physaddr(idt);

	// setup idt gates(descriptors) into IDT
	for (i = 0; i < sizeof(idt_gates) / sizeof(idt_gate_t); i++)		
		setup_gate((uint32_t) idt_gates[i].gate, idt_gates[i].vec_num, PRESENT | ((uint8_t) idt_gates[i].privilege << DPL_SHIFT) | STORAGE_SEG_INT_GATE | INT_GATE_TYPE);
}

static void setup_gate(uint32_t offset, uint8_t vec_num, uint8_t p_dpl_type)
{
	idt[vec_num].offset_low = (uint16_t) offset;
	idt[vec_num].selector = (uint16_t) CS_SELECTOR;
	idt[vec_num].pad = 0;
	idt[vec_num].p_dpl_type = p_dpl_type;
	idt[vec_num].offset_high = (uint16_t) (offset >> SHIFT_FOR_OFFSET_HIGH);
} 



