#ifndef IRQ_H
#define IRQ_H

/* message types */
#define CLOCK_TICK	0
#define TTY_CHAR_INT	1
/* IRQ numbers */
#define IRQ_CLOCK	0
#define IRQ_KEYBOARD	1

extern int lost_clock_ticks;
/* miscellaneous */
#define SLAVE_FIRST_INT	0x28

#endif	/* IRQ_H */
