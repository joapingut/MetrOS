#ifndef _GDT_H
#define _GDT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	/* Limits */
	uint16_t limit_low;
	/* Segment address */
	uint16_t base_low;
	uint8_t base_middle;
	/* Access modes */
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t limit;
	uintptr_t base;
} __attribute__((packed)) gdt_pointer_t;

void gdt_set_gate(uint8_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran);
static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0);
void gdt_install(void);


#endif
