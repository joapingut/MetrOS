#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/paging.h>
#include <kernel/alloc.h>
#include <kernel/isrs.h>
#include <kernel/idt.h>

uintptr_t to_physical_addr(uint32_t virtual, page_directory_t *dir);
uint32_t getMemoryAmountFromGrub(multiboot_info_t *mbi, bool setMemory);
void alloc_frame_int(page_t *page, bool is_kernel, bool is_writeable, bool is_accessed, bool is_dirty, bool map_frame, uint32_t frameAddr);
void switch_4kb_pagination(uint32_t phy);
void refresh_page(uint32_t address);

page_directory_t *kernel_directory; //The page directory created on boot.S for kernel
page_directory_t *current_page_directory;
uint32_t next_freeAddress; // First 4MB (first page) reserved
uint32_t next_virtualFreeAddress;
static uint32_t max_num_frames; //Max number of frames for installed memory
static uint32_t *frames_Array; //Pointer to array with the frames status (0 FREE, 1 USED)

void paging_install(multiboot_info_t *mbi){
	/***First create the bitmap***/
	//Leemos la cabecera que nos ha pasado GRUB para sacar el tamaño
	uint32_t memoryTam = getMemoryAmountFromGrub(mbi, false);
	max_num_frames = memoryTam / PAGE_TAM;
	//Direccion para el bitmap despues del kernel
	next_freeAddress = (uint32_t)((((uint32_t) &__KERNEL_END) + 4) - KERNEL_VIRTUAL_BASE);
	next_virtualFreeAddress =  (uint32_t)(((uint32_t) &__KERNEL_END) + 4);
	//Obtenemos un array para el bitmap
	frames_Array = (uint32_t *)kmalloc(INDEX_FROM_BIT(max_num_frames));
	//Limpiamos el contenido de la memoria
	memset(frames_Array, 0, INDEX_FROM_BIT(max_num_frames));
	//Leemos la cabecera que nos ha pasado GRUB de nuevo para marcar los lugares reservados
	getMemoryAmountFromGrub(mbi, true);
	printf("Max num frames: %d; TAM: %d bytes\n", max_num_frames, memoryTam);
	if(max_num_frames <= 4){
		PANIC("INSTALLED MEMORY BELOW 16MB");
	}
	//Creamos un nuevo directory para la paginacion que ademas sera el del kernel
	//Lo haremos en el area del kernel y no en la de los directorios
	uint32_t phy;
	uint32_t *newPD = (uint32_t *)kmalloc_ap(sizeof(page_directory_t), &phy);
	memset(newPD, 0, sizeof(page_directory_t));
	//El nuevo directorio será el reservado para el kernel
	kernel_directory = (page_directory_t *)newPD;
	//Mapeamos la ultima pagina del directorio a si mismo
	newPD[1023] = phy | 0x63;
	//Iniciamos las paginas del kernel
	for(uint32_t i = KERNEL_VIRTUAL_BASE; i < next_virtualFreeAddress; i += PAGE_TAM){
		page_t *pg = get_page(i, 1, kernel_directory);
		alloc_frame_int(pg, true, true, true, true, true, i - KERNEL_VIRTUAL_BASE);
	}
	//Registramos el handler para la paginacion
	register_isrs_handler(14, paging_handler);
	//Cambiamos al modo 4KB con el nuevo directorio
	switch_4kb_pagination(phy);
}

void paging_handler(regs *r){
	//The address with the error is in cr2
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
	// The error code gives us details of what happened.
	int present   = !(r->err_code & 0x1);	// Page not present
	int rw = r->err_code & 0x2;		// Write operation?
	int us = r->err_code & 0x4;		// Processor was in user-mode?
	int reserved = r->err_code & 0x8;	// Overwritten CPU-reserved bits of page entry?
	int id = r->err_code & 0x10;		// Caused by an instruction fetch?
	printf("\nFaulting: %x\n", faulting_address);
	if(present){
		//Obtenemos la pagina que falta o la creamos si no existe
		page_t *ptm = get_page(faulting_address, 1, current_page_directory);
		alloc_frame(ptm, true, true);
		//Actualizamos la cache de direcciones de la CPU
		refresh_page(faulting_address);
	}else{
		printf("P: %d; RW: %d; US: %d; R: %d; ID: %d;\n", present, rw, us, reserved, id);
		uint32_t fcr3, fcr4, fcr0;
		asm volatile("mov %%cr3, %0" : "=r" (fcr3));
		asm volatile("mov %%cr4, %0" : "=r" (fcr4));
		asm volatile("mov %%cr0, %0" : "=r" (fcr0));
		printf("CR3: %x; CR4: %x; CR0: %x\n", fcr3, fcr4, fcr0);
		panic_exception("Paging handler", 0x22);
	}
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir){
	// Turn the address into an index.
	uint32_t table_idx = address / PAGE_TAM_4MB;
	uint32_t page_idx = (address / PAGE_TAM) % 1024;
	if (*(uint32_t *)(&dir->tables[table_idx])!=0){// If this table is already assigned
		return &(dir->tables[table_idx]->pages[page_idx]);
	}else if(make){
		uint32_t phy;
		uint32_t *tmp = (uint32_t *)kmalloc_ap(sizeof(page_table_t), &phy);
		memset(tmp, 0, sizeof(page_table_t));
		dir->tables[table_idx] = (page_table_t *)tmp;
		dir->physical_tables[table_idx] = (uintptr_t)(phy | PAGE_DEFAULT_VALUE);
		return &(dir->tables[table_idx]->pages[page_idx]);
	}else{
		return 0;
	}
}

void switch_page_directory(page_directory_t *dir){
	current_page_directory = dir;
	asm volatile("mov %0, %%cr3":: "r"(to_physical_addr(&dir, dir)): "%eax");
}

void switch_4kb_pagination(uint32_t phy){
	current_page_directory = kernel_directory;
	uint32_t fcr4;
	asm volatile("mov %%cr4, %0" : "=r" (fcr4));
	fcr4 &= 0xFFFFFFEF;
	printf("\nCR4 %x", fcr4);
	asm volatile("mov %0, %%cr3\n\t"
				 "mov %1, %%cr4"::"r"(phy), "r"(fcr4));
}

void refresh_page(uint32_t address){
	asm volatile (
		"movl %0,%%eax\n"
		"invlpg (%%eax)\n"
		:: "r"((uintptr_t)(address & 0xFFFFF000)) : "%eax");
}

uintptr_t to_physical_addr(uint32_t virtual, page_directory_t *dir){
	uint32_t table_idx = virtual / PAGE_TAM_4MB;
	uint32_t page_idx = (virtual / PAGE_TAM) % 1024;
	/*page_table_t *table = dir->tables[table_idx];
	uint32_t tableOff = table->pages[page_idx].frame << 12;
	return (void *)((tableOff & ~(PAGE_MASC)) + ( virtual & PAGE_MASC));*/
	
	uint32_t *tableDEntry = 0xFFFFF000 + (table_idx * sizeof(uint32_t));
	//check if table really exist
	uint32_t *table =  0xFFC00000 + (table_idx * PAGE_TAM);
	//check if page exist
	return (uintptr_t)((table[page_idx] & ~PAGE_MASC) + (virtual & PAGE_MASC));
}

uint32_t getMemoryAmountFromGrub(multiboot_info_t *mbi, bool setMemory){
	uint64_t memory = 0;
	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *) (mbi->mmap_addr + KERNEL_VIRTUAL_BASE);
	while((uint32_t) mmap < (mbi->mmap_addr + mbi->mmap_length) + KERNEL_VIRTUAL_BASE) {
		printf("Size = 0x%x, ", (uint32_t) mmap->size);
		printf("base_addr = 0x%x", (uint32_t)(mmap->addr >> 32));
		printf("%x, ", (uint32_t)(mmap->addr & 0xFFFFFFFF));
		printf("length = 0x%x", (uint32_t)(mmap->len >> 32));
		printf("%x, ", (uint32_t)(mmap->len & 0xFFFFFFFF));
		printf("type = 0x%x\n", (uint32_t) mmap->type);
		if((uint32_t)mmap->type == 0x01){
			memory += (uint64_t)mmap->len;
		}else if(setMemory){
			for(uint64_t i = mmap->addr; i < (mmap->addr + mmap->len); i += PAGE_TAM){
				set_frame((uint32_t)(i & 0xFFFFFFFF));
			}
		}
		mmap = (multiboot_memory_map_t *) ((unsigned int) mmap + (unsigned int)mmap->size + sizeof(uint32_t));
	}
	return (uint32_t)(memory & 0xFFFFFFFF);
}

// Function to allocate a frame.
void alloc_frame_int(page_t *page, bool is_kernel, bool is_writeable, bool is_accessed, bool is_dirty, bool map_frame, uint32_t frameAddr){
	uint32_t idx;
	if(map_frame){
		idx = frameAddr >> 12;
	}else{
		idx = first_frame();; // idx is now the index of the first free frame.
	} 
	if (idx == (uint32_t) -1){
		PANIC("No free frames!");
	}
	set_frame(idx << 12); // this frame is now ours!
	page->ps = 0; //We are using 4KB pages
	page->present = 1; // Mark it as present.
	page->rw = (is_writeable == true)?1:0; // Should the page be writeable?
	page->user = (is_kernel == true)?0:1; // Should the page be user-mode?
	page->accessed = (is_accessed == true)?1:0;
	page->dirty = (is_dirty == true)?1:0;
	page->frame = idx;
}

void alloc_frame(page_t *page, bool is_kernel, bool is_writeable){
	alloc_frame_int(page, is_kernel, is_writeable, false, false, false, 0);
}

// Function to deallocate a frame.
void free_frame(page_t *page){
	if(page == 0){
		return;
	}
	if (!test_frame((page->frame) << 12)){
		return; // The given page didn't actually have an allocated frame!
	}else{
		clear_frame((page->frame) << 12); // Frame is now free again.
		*((uint32_t *)page) = 0;
	}
}

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr){
	uint32_t frame = frame_addr/PAGE_TAM;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames_Array[idx] |= (0x1 << off);
}


// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr){
	uint32_t frame = frame_addr/PAGE_TAM;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames_Array[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static uint32_t test_frame(uint32_t frame_addr){
	uint32_t frame = frame_addr/PAGE_TAM;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	return (frames_Array[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static uint32_t first_frame(){
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(max_num_frames); i++){
		if (frames_Array[i] != 0xFFFFFFFF){ // nothing free, exit early.
			// at least one bit is free here.
			for (j = 0; j < 32; j++){
				uint32_t toTest = 0x1 << j;
				if ( !(frames_Array[i]&toTest) ){
					return i*4*8+j;
				}
			}
		}
	}
}
