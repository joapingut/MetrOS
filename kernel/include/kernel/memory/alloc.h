#ifndef _ALLOC_H
#define _ALLOC_H

#include <kernel/system.h>
#include <kernel/memory/boundaries.h>

uint32_t kdmalloc_init(uint32_t start, uint32_t base);
void kdmalloc_stop();

uint32_t kdmalloc_a(uint32_t sz);
uint32_t kdmalloc_p(uint32_t sz, uint32_t *phy);
uint32_t kdmalloc_ap(uint32_t sz, uint32_t *phy);
uint32_t kdmalloc(uint32_t sz);

#endif
