#ifndef _KEYBOARD_ARRAY_HPP
#define _ORDERED_ARRAY_HPP

#include <kernel/system.h>

typedef bool (*lessthan_predicate_t)(uintptr_t, uintptr_t);

typedef struct{
	uintptr_t *array;
	uint32_t size;
	uint32_t max_size;
	lessthan_predicate_t less_than;
} ordered_array_t;

bool standard_lessthan_predicate(uintptr_t a, uintptr_t b);

ordered_array_t create_ordered_array(uint32_t max_size, lessthan_predicate_t less_than);
ordered_array_t place_ordered_array(uintptr_t *addr, uint32_t max_size, lessthan_predicate_t less_than);

void destroy_ordered_array(ordered_array_t *array);

uintptr_t get_ordered_array(uint32_t i, ordered_array_t *array);
void remove_ordered_array(uint32_t i, ordered_array_t *array);

#endif
