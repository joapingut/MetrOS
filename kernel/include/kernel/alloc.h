#ifndef _ALLOC_H
#define _ALLOC_H

#include <kernel/system.h>

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phy);
uint32_t kmalloc_a(uint32_t sz);
uint32_t kmalloc_p(uint32_t sz, uint32_t *phy);
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phy);
uint32_t kmalloc(uint32_t sz);

#endif
