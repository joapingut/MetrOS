#ifndef KERNEL_INCLUDE_KERNEL_VMM_H_
#define KERNEL_INCLUDE_KERNEL_VMM_H_

#include <kernel/system.h>
#include <kernel/memory/paging.h>

#define ALLOC_VMM_START	0xD0000000
#define ALLOC_VMM_FINISH 0xEFFFF000
#define ALLOC_VMM_NUM_PAGES 0x1FFFF

void vmm_install();
static void set_page(uint32_t addr);
static void clear_page(uint32_t addr);
static uint32_t test_page(uint32_t addr);
static uint32_t find_num_pages(size_t numPages);

int   liballoc_lock();
int   liballoc_unlock();
void* liballoc_alloc(size_t);
int   liballoc_free(void*,size_t);

#endif /* KERNEL_INCLUDE_KERNEL_VMM_H_ */
