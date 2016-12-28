#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <utils/orderedArray.h>
#include <kernel/kheap.h>
#include <kernel/alloc.h>
#include <kernel/paging.h>

extern page_directory_t *kernel_directory;

static bool header_t_less_than(header_t * a, header_t * b){
	return (a->size < b->size) ? true : false;
}


static int32_t find_smallest_hole(uint32_t size, bool page_align, heap_t *heap){
	// Find the smallest hole that will fit.
	int32_t iterator = 0;
	while (iterator < heap->index.size){
		header_t *header = (header_t *) get_ordered_array(iterator, &heap->index);
		// If the user has requested the memory be page-aligned
		if (page_align == true){
			// Page-align the starting point of this header.
			uint32_t location = header;
			uint32_t offset = 0;
			if ((location + sizeof(header_t) & 0xFFFFF000) != 0)
				offset = PAGE_TAM  - (location + sizeof(header_t)) % PAGE_TAM;
			uint32_t hole_size = header->size - offset;
			// Can we fit now?
			if (hole_size >= size)
				break;
		}else if (header->size >= size)
			break;
		iterator++;
	}
	// Why did the loop exit?
	if (iterator == heap->index.size)
		return -1; // We got to the end and didn't find anything.
	else
		return iterator;
}


heap_t *create_heap(uint32_t start, uint32_t end_addr, uint32_t max, bool supervisor, bool readonly){
	// All our assumptions are made on startAddress and endAddress being page-aligned.
	ASSERT((start % PAGE_TAM) == 0);
	ASSERT((end_addr % PAGE_TAM) == 0);
	
	heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));
	// Initialise the index.
	heap->index = place_ordered_array( (uintptr_t)start, HEAP_INDEX_SIZE, &header_t_less_than);

	// Shift the start address forward to resemble where we can start putting data.
	start += sizeof(uintptr_t)*HEAP_INDEX_SIZE;

	// Make sure the start address is page-aligned.
	if (start & 0xFFFFF000 != 0){
		start &= 0xFFFFF000;
		start += PAGE_TAM;
	}
	// Write the start, end and max addresses into the heap structure.
	heap->start_address = start;
	heap->end_address = end_addr;
	heap->max_address = max;
	heap->supervisor = supervisor;
	heap->readonly = readonly;

	// We start off with one large hole in the index.
	header_t *hole = (header_t *)start;
	hole->size = end_addr-start;
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;
	insert_ordered_array((uintptr_t)hole, &heap->index);

	return heap;
}

static void expand(uint32_t new_size, heap_t *heap){
	// Sanity check.
	ASSERT(new_size > (heap->end_address - heap->start_address));
	// Get the nearest following page boundary.
	if (new_size & PAGE_MASC != 0){
		new_size &= 0xFFFFF000;
		new_size += PAGE_TAM;
	}
	// Make sure we are not overreaching ourselves.
	ASSERT((heap->start_address + new_size) <= heap->max_address);
	
	// This should always be on a page boundary.
	uint32_t old_size = heap->end_address-heap->start_address;
	uint32_t i = old_size;
	while (i < new_size){
		alloc_frame( get_page(heap->start_address + i, 1, kernel_directory), (heap->supervisor)?1:0, (heap->readonly)?0:1);
		i += PAGE_TAM;
	}
	heap->end_address = heap->start_address+new_size;
}

static uint32_t contract(uint32_t new_size, heap_t *heap){
	// Sanity check.
	ASSERT(new_size < (heap->end_address - heap->start_address));
	// Get the nearest following page boundary.
	if (new_size & PAGE_MASC != 0){
		new_size &= 0xFFFFF000;
		new_size += PAGE_TAM;
	}
	// Don't contract too far!
	if (new_size < HEAP_MIN_SIZE)
		new_size = HEAP_MIN_SIZE;

	uint32_t old_size = heap->end_address - heap->start_address;
	uint32_t i = old_size - PAGE_TAM;
	while (i > new_size){
		free_frame(get_page(heap->start_address+i, 0, kernel_directory));
		i -= PAGE_TAM;
	}
	heap->end_address = heap->start_address + new_size;
	return new_size;
}

void *alloc(uint32_t size, bool page_align, heap_t *heap){
	// Make sure we take the size of header/footer into account.
	uint32_t new_size = size + sizeof(header_t) + sizeof(footer_t);
	// Find the smallest hole that will fit.
	int32_t iterator = find_smallest_hole(new_size, page_align, heap);
	if(iterator != -1){
		// Save some previous data.
		uint32_t old_length = heap->end_address - heap->start_address;
		uint32_t old_end_address = heap->end_address;

		// We need to allocate some more space.
		expand(old_length + new_size, heap);
		uint32_t new_length = heap->end_address - heap->start_address;

		// Find the endmost header. (Not endmost in size, but in location).
		iterator = 0;
		// Vars to hold the index of, and value of, the endmost header found so far.
		uint32_t idx = -1; uint32_t value = 0x0;
		while (iterator < heap->index.size){
			uint32_t tmp = (uint32_t)get_ordered_array(iterator, &heap->index);
			if (tmp > value){
				value = tmp;
				idx = iterator;
			}
			iterator++;
		}

		// If we didn't find ANY headers, we need to add one.
		if (idx == -1){
			header_t *header = (header_t *)old_end_address;
			header->magic = HEAP_MAGIC;
			header->size = new_length - old_length;
			header->is_hole = 1;
			footer_t *footer = (footer_t *)(old_end_address + header->size - sizeof(footer_t));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
			insert_ordered_array((void*)header, &heap->index);
       }else{
			// The last header needs adjusting.
			header_t *header = get_ordered_array(idx, &heap->index);
			header->size += new_length - old_length;
			// Rewrite the footer.
			footer_t *footer = (footer_t *)((uint32_t)header + header->size - sizeof(footer_t));
			footer->header = header;
			footer->magic = HEAP_MAGIC;
		}
		// We now have enough space. Recurse, and call the function again.
		return alloc(size, page_align, heap);
	}
	header_t *orig_hole_header = (header_t *)get_ordered_array(iterator, &heap->index);
	uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
	uint32_t orig_hole_size = orig_hole_header->size;
	// Here we work out if we should split the hole we found into two parts.
	// Is the (original hole size - requested hole size) less than the overhead for adding a new hole?
	if ((orig_hole_size - new_size) < (sizeof(header_t) + sizeof(footer_t))){
		// Then just increase the requested size to the size of the hole we found.
		size += (orig_hole_size - new_size);
		new_size = orig_hole_size;
	}
	// If we need to page-align the data, do it now and make a new hole in front of our block.
	if (page_align && (orig_hole_pos & 0xFFFFF000) != 0){
		uint32_t new_location = orig_hole_pos + PAGE_TAM - (orig_hole_pos & PAGE_MASC) - sizeof(header_t);
		header_t *hole_header = (header_t *)orig_hole_pos;
		hole_header->size = PAGE_TAM - (orig_hole_pos & PAGE_MASC) - sizeof(header_t);
		hole_header->magic = HEAP_MAGIC;
		hole_header->is_hole = 1;
		footer_t *hole_footer = (footer_t *)((uint32_t)new_location - sizeof(footer_t));
		hole_footer->magic = HEAP_MAGIC;
		hole_footer->header = hole_header;
		orig_hole_pos = new_location;
		orig_hole_size = orig_hole_size - hole_header->size;
	}else{
		// Else we don't need this hole any more, delete it from the index.
		remove_ordered_array(iterator, &heap->index);
	}
	// Overwrite the original header...
	header_t *block_header = (header_t *)orig_hole_pos;
	block_header->magic = HEAP_MAGIC;
	block_header->is_hole = 0;
	block_header->size = new_size;
	// ...And the footer
	footer_t *block_footer = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
	block_footer->magic = HEAP_MAGIC;
	block_footer->header = block_header;
	// We may need to write a new hole after the allocated block.
	// We do this only if the new hole would have positive size...
	if (orig_hole_size - new_size > 0){
		header_t *hole_header = (header_t *)(orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
		hole_header->magic = HEAP_MAGIC;
		hole_header->is_hole = 1;
		hole_header->size = orig_hole_size - new_size;
		footer_t *hole_footer = (footer_t *)((uint32_t)hole_header + orig_hole_size - new_size - sizeof(footer_t));
		if ((uint32_t)hole_footer < heap->end_address){
			hole_footer->magic = HEAP_MAGIC;
			hole_footer->header = hole_header;
		}
		// Put the new hole in the index;
		insert_ordered_array((void*)hole_header, &heap->index);
	}
	// ...And we're done!
	return (void *)((uint32_t)block_header + sizeof(header_t));
}