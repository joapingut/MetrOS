/*
 * vmm.c
 *
 *  Created on: 5 feb. 2017
 *      Author: joaquin
 */


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/memory/vmm.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/alloc.h>

static uint32_t *pages_Array; //Pointer to array with the heap pages status (0 FREE, 1 USED)

void vmm_install(){
	uint32_t tam = INDEX_FROM_BIT(ALLOC_VMM_NUM_PAGES);
	if(ALLOC_VMM_NUM_PAGES % 32 != 0){
		tam += 1;
	}
	pages_Array = (uint32_t *)kdmalloc(sizeof(uint32_t) * tam); // Reemplazar
	//Limpiamos el contenido de la memoria
	memset(pages_Array, 0, sizeof(uint32_t) * tam);
}

// Static function to set a bit in the frames bitset
static void set_page(uint32_t index){
	uint32_t off = index % sizeof(uint32_t);
	pages_Array[index] |= (0x1 << off);
}


// Static function to clear a bit in the frames bitset
static void clear_page(uint32_t index){
	uint32_t off = index % sizeof(uint32_t);
	pages_Array[index] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static uint32_t test_page(uint32_t index){
	uint32_t off = index % sizeof(uint32_t);
	return (pages_Array[index] & (0x1 << off));
}


static uint32_t find_num_pages(size_t numPages){
	uint32_t i, j, foundAt, page, frameNum, having;
	foundAt = NULL;
	page = NULL;
	frameNum = 0;
	having = 0;
	for (i = 0; i < INDEX_FROM_BIT(ALLOC_VMM_NUM_PAGES); i++) {
		if (having >= numPages) {
			break;
		}
		if (pages_Array[i] != 0xFFFFFFFF) { // nothing free, exit early.
			// at least one bit is free here.
			for (j = 0; j < 32; j++) {
				uint32_t toTest = 0x1 << j;
				if (!(pages_Array[i] & toTest)) {
					if (foundAt == NULL) {
						page = frameNum;
						foundAt = frameNum;
						having += 1;
						if (having >= numPages) {
							return page;
						}
					} else if ((frameNum - 1) == foundAt) {
						foundAt = frameNum;
						having += 1;
						if (having >= numPages) {
							return page;
						}
					} else {
						foundAt = NULL;
						page = NULL;
						having = 0;
					}
				}
				frameNum += 1;
			}
		} else {
			frameNum += 32;
		}
	}
	return page;
}

int   liballoc_lock(){
	return 0;
}

int   liballoc_unlock(){
	return 0;
}

void* liballoc_alloc(size_t num_pages){
	uint32_t freePage = find_num_pages(num_pages);
	printf("\nLIBALLOC REQ: 0x%x", num_pages);
	printf("\nLIBALLOC: 0x%x", freePage);
	if (freePage == NULL){
		return NULL;
	}
	uint32_t addr = ALLOC_VMM_START + (PAGE_TAM * freePage);
	for(uint32_t i = 0; i < num_pages; i++){
		printf("\nLIBALLOC ADDR: 0x%x - ", addr + (PAGE_TAM * i));
		set_page(freePage + i);
		page_t *pg = get_page_default(addr + (PAGE_TAM * i), 1);
		//alloc_frame_int(pg, true, true, true, true, false, NULL);
		//Actualizamos la cache de direcciones de la CPU
		refresh_page(addr + (PAGE_TAM * i));
		printf("\nLIBALLOC PAGE: 0x%x - ", *((uint32_t *)pg));
	}
	return addr;
}

int liballoc_free(void* addr, size_t num_pages) {
	uint32_t page = (((uint32_t)addr) - ALLOC_VMM_START) / PAGE_TAM;
	for (uint32_t i = 0; i < num_pages; i++) {
		clear_page(page + i);
		page_t *pg = get_page_default(addr + (PAGE_TAM * i), 1);
		free_frame(pg);
	}
	return 0;
}
