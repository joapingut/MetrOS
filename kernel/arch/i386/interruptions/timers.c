#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/timers.h>
#include <kernel/irq.h>
#include <kernel/io.h>

long timer_ticks = 0;
int timer_seconds = 0;

void timer_phase(int hz) {
	int divisor = PIT_SCALE / hz;
	outportb(PIT_CONTROL, PIT_SET);
	outportb(PIT_A, divisor & PIT_MASK);
	outportb(PIT_A, (divisor >> 8) & PIT_MASK);
}

void timer_handler(regs *r){
    /* Increment our 'tick count' */
    timer_ticks++;

    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    if (timer_ticks % 1000 == 0){
	timer_seconds++;
        //printf("%d seconds has passed\n", timer_seconds);
    }
}

void timer_install(){
	/* Installs 'timer_handler' to IRQ0 */
	irq_install_handler(0, timer_handler);
	timer_phase(1000);
}

void ticks(){
	printf("%d\n", timer_ticks);
}
