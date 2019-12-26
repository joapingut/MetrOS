#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/memory/pmm.h>
#include <kernel/memory/alloc.h>

extern uint32_t next_freeAddress;
extern uint32_t next_virtualFreeAddress;

void validate_actual_frame();

void validate_crossing_frame(uint32_t size) {
	uint32_t actual_frame = (next_freeAddress) & ~PAGE_MASC;
	uint32_t dir_frame = (next_freeAddress + size) & ~PAGE_MASC;
	printf("\nAlloc f: %x : %x", actual_frame, dir_frame);
	if (dir_frame != actual_frame){
		next_freeAddress = first_frame();
		printf(" n: %x", next_freeAddress >> 12);
		next_virtualFreeAddress = (next_virtualFreeAddress & ~PAGE_MASC) + PAGE_TAM;
	}
}

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phy){
	// This is dumb malloc
	if (align == 1 && (next_virtualFreeAddress & PAGE_MASC) != 0) {
		// Align the placement address;
		next_freeAddress &= 0xFFFFF000;
		next_virtualFreeAddress &= 0xFFFFF000;
		next_freeAddress += 0x1000;
		next_virtualFreeAddress += 0x1000;
	}
	//validate_crossing_frame(sz);
	set_frame(next_freeAddress);
	uint32_t tmp = next_virtualFreeAddress;
	if (phy) {
		*phy = next_freeAddress;
	}
	next_virtualFreeAddress += sz;
	next_freeAddress += sz;
	return tmp;
}

uint32_t kmalloc_dumb(uint32_t sz){
	return kmalloc_int(sz, 0, 0);
}

uint32_t kmalloc_a(uint32_t sz){
	return kmalloc_int(sz, 1, 0);
}

uint32_t kmalloc_p(uint32_t sz, uint32_t *phy){
	return kmalloc_int(sz, 0, phy);
}

uint32_t kmalloc_ap(uint32_t sz, uint32_t *phy){
	return kmalloc_int(sz, 1, phy);
}

void kfree_dumb(void *p){
	//NOTHING
}
