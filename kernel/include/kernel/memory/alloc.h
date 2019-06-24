#ifndef _ALLOC_H
#define _ALLOC_H

#include <kernel/system.h>
#include <liballoc/liballoc.h>

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phy);
uint32_t kmalloc_a(uint32_t sz);
uint32_t kmalloc_p(uint32_t sz, uint32_t *phy);
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phy);
uint32_t kmalloc_dumb(uint32_t sz);
void kfree_dumb(void *p);

#endif
