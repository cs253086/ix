#ifndef CLOCK_H
#define CLOCK_H

extern int lost_clock_ticks;
extern long realtime;

int clock_task();

#endif /* CLOCK_H */