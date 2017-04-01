#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <utils/orderedArray.h>

bool standard_lessthan_predicate(uintptr_t a, uintptr_t b){
	return (a<b)?true:false;
}

ordered_array_t create_ordered_array(uint32_t max_size, lessthan_predicate_t less_than){
	ordered_array_t to_ret;
	to_ret.array = (void *)kmalloc_dumb(max_size * sizeof(uintptr_t));
	memset(to_ret.array, 0, max_size * sizeof(uintptr_t));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;
	return to_ret;
}

ordered_array_t place_ordered_array(uintptr_t *addr, uint32_t max_size, lessthan_predicate_t less_than){
	ordered_array_t to_ret;
	to_ret.array = (uintptr_t *)addr;
	memset(to_ret.array, 0, max_size /* sizeof(uintptr_t)*/);
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;
	return to_ret;
}

void destroy_ordered_array(ordered_array_t *array){
	//kfree(array->array);
}

void insert_ordered_array(uintptr_t item, ordered_array_t *array){
	ASSERT(array->less_than);
	uint32_t iterator = 0;
	while (iterator < array->size && array->less_than(array->array[iterator], item))
		iterator +=1;
	if (iterator == array->size){ // just add at the end of the array.
		array->array[array->size] = item;
		array->size += 1;
	}else{
		uintptr_t tmp = array->array[iterator];
		array->array[iterator] = item;
		while (iterator < array->size){
			iterator +=1;
			uintptr_t tmp2 = array->array[iterator];
			array->array[iterator] = tmp;
			tmp = tmp2;
		}
		array->size += 1;
	}
}

uintptr_t get_ordered_array(uint32_t i, ordered_array_t *array){
	ASSERT(i < array->size);
	return array->array[i];
}

void remove_ordered_array(uint32_t i, ordered_array_t *array){
	while (i < array->size){
		array->array[i] = array->array[i+1];
		i +=1;
	}
	array->size -=1;
}

