#ifndef _PHYSICAL_MEMORY_MANAGER_H
#define _PHYSICAL_MEMORY_MANAGER_H

#include <kernel/system.h>
#include <stdbool.h>
#include <kernel/memory/boundaries.h>
#include <kernel/multiboot.h>
#include <kernel/memory/paging.h>

void pmm_install(multiboot_info_t *mbi);

void set_frame(uint32_t frame_addr);
void clear_frame(uint32_t frame_addr);
bool test_frame(uint32_t frame_addr);
uint32_t first_frame();

uint32_t memory_used();

void alloc_frame_int(page_t *page, bool is_kernel, bool is_writeable, bool is_accessed, bool is_dirty, bool map_frame, uint32_t frameAddr);
void alloc_frame(page_t *page, bool is_kernel, bool is_writeable);

extern uint32_t modules_firstAddress;
extern uint32_t next_freeAddress;


#endif