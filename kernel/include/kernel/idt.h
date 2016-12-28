#ifndef _IDT_H
#define _IDT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint16_t base_low;
	uint16_t sel;
	uint8_t zero;
	uint8_t flags;
	uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t limit;
	uintptr_t base;
} __attribute__((packed)) idt_pointer_t;

void idt_set_gate(uint8_t num, void (*base)(void), uint16_t sel, uint8_t flags);
void idt_install(void);

#endif
