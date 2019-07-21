#ifndef KERNEL_LOW_H
#define KERNEL_LOW_H

#include <stdint.h>

void exception0 ();
void exception1 ();
void exception2 ();
void exception3 ();
void exception4 ();
void exception5 ();
void exception6 ();
void exception7 ();
void exception8 ();
void exception9 ();
void exception10();
void exception11();
void exception12();
void exception13();

void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();

void sys_call_low();
void run_proc();
void cp_msg_low(message_t *src_mgs, message_t *dst_msg);
void phy_copy(uint32_t src_address, uint32_t dst_address, uint32_t size_bytes);
void switch_paging(uint32_t *page_directory);
void wait_for_interrupt();
#endif /* KERNEL_LOW_H */
