#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/paging.h>
#include <kernel/alloc.h>

extern uint32_t next_freeAddress;
extern uint32_t next_virtualFreeAddress;

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phy){
	// This will eventually call malloc() on the kernel heap.
	// For now, though, we just assign memory at placement_address
	// and increment it by sz. Even when we've coded our kernel
	// heap, this will be useful for use before the heap is initialised.
	if (align == 1 && (next_virtualFreeAddress & PAGE_MASC) != 0){
		// Align the placement address;
		next_freeAddress &= 0xFFFFF000;
		next_virtualFreeAddress &= 0xFFFFF000;
		next_freeAddress += 0x1000;
		next_virtualFreeAddress += 0x1000;
	}
	uint32_t tmp = next_virtualFreeAddress;
	if(phy){
		*phy = next_freeAddress;
	}
	next_virtualFreeAddress += sz;
	next_freeAddress += sz;
	return tmp;
}

uint32_t kmalloc(uint32_t sz){
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