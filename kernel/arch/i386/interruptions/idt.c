#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/interruptions/idt.h>

#define ENTRY(X) (idt.entries[(X)])

static struct {
	idt_entry_t entries[256];
	idt_pointer_t pointer;
} idt __attribute__((used));

extern void idt_load(uintptr_t);

void idt_set_gate(uint8_t num, void (*base)(void), uint16_t sel, uint8_t flags) {
//	printf("IDT_GATE_SET %d", num);
	ENTRY(num).base_low = ((uintptr_t)base & 0xFFFF);
	ENTRY(num).base_high = ((uintptr_t)base >> 16) & 0xFFFF;
	ENTRY(num).sel = sel;
	ENTRY(num).zero = 0;
	ENTRY(num).flags = flags | 0x60;
}

void idt_install(void) {
	idt_pointer_t * idtp = &idt.pointer;
//	printf("\nIDT: %x:", idtp);
	idtp->limit = sizeof idt.entries - 1;
	idtp->base = (uintptr_t)&ENTRY(0);
	memset(&ENTRY(0), 0, sizeof idt.entries);

	idt_load((uintptr_t)idtp);
}
