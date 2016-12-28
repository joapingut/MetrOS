#ifndef _TIMERS_H
#define _TIMERS_H

#include <kernel/system.h>

#define PIT_A 0x40
#define PIT_B 0x41
#define PIT_C 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SCALE 1193180
#define PIT_SET 0x36

#define TIMER_IRQ 0

#define SUBTICKS_PER_TICK 1000
#define RESYNC_TIME 1

void timer_handler(regs *r);
void timer_install();
void ticks();

#endif
