#ifndef _KHEAP_H
#define _KHEAP_H

#include <kernel/system.h>

#define KHEAP_START	0xD0000000
#define KHEAP_INITIAL_SIZE	0x100000
#define HEAP_INDEX_SIZE	0x20000
#define HEAP_MAGIC	0xABABBABA
#define HEAP_MIN_SIZE	0x80000

typedef struct{
	uint32_t magic;   // Magic number, used for error checking and identification.
	bool is_hole;   // 1 if this is a hole. 0 if this is a block.
	uint32_t size;    // size of the block, including the end footer.
} header_t;

typedef struct{
	uint32_t magic;     // Magic number, same as in header_t.
	header_t *header; // Pointer to the block header.
} footer_t;

typedef struct{
	ordered_array_t index;
	uint32_t start_address; // The start of our allocated space.
	uint32_t end_address;   // The end of our allocated space. May be expanded up to max_address.
	uint32_t max_address;   // The maximum address the heap can be expanded to.
	bool supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
	bool readonly;       // Should extra pages requested by us be mapped as read-only?
} heap_t;

/**
  Create a new heap.
**/
heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, bool supervisor, bool readonly);
/**
  Allocates a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting
  on a page boundary.
**/
void *alloc(uint32_t size, bool page_align, heap_t *heap);
/**
  Releases a block allocated with 'alloc'.
**/
void free(void *p, heap_t *heap); 

#endif