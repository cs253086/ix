#ifndef IRQ_H
#define IRQ_H

/* message types */
/* IRQ numbers */
#define IRQ_CLOCK	0
#define IRQ_KEYBOARD	1
#define IRQ_PRIMARY_ATA	14
extern int lost_clock_ticks;
extern cur_irq_num;
/* miscellaneous */
#define SLAVE_FIRST_INT	0x08

#endif	/* IRQ_H */
