#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/memory/paging.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/alloc.h>
#include <kernel/interruptions/isrs.h>
#include <kernel/interruptions/idt.h>

uintptr_t to_physical_addr(uint32_t virtual, page_directory_t *dir);
uint32_t loadModulesFromGrub(multiboot_info_t *mbi);
page_table_t* get_table_by_self_directory(page_directory_t *dir);
//void alloc_frame_int(page_t *page, bool is_kernel, bool is_writeable, bool is_accessed, bool is_dirty, bool map_frame, uint32_t frameAddr);
void switch_4kb_pagination(uint32_t phy);
void refresh_page(uint32_t address);

page_directory_t *current_page_directory;
uint32_t next_freeAddress; // First 4MB (first page) reserved
uint32_t next_virtualFreeAddress;
uint32_t modules_firstAddress;


bool starting = true;

extern uint8_t *initrd_addr;

void paging_install(multiboot_info_t *mbi){
	// Primero creamos un nuevo directory para la paginación que además será el del kernel
	// Obtenemos un bloque de memoria libre (4kb) que es el tamaño de un directorio.
	uint32_t phy;
	uint32_t *newPD = kmalloc_ap(sizeof(page_directory_t), &phy);
	// Limpiamos el contenido de la memoria
	memset(newPD, 0, sizeof(page_directory_t));
	//El nuevo directorio será el reservado para el kernel
	kernel_directory = (page_directory_t *) newPD;
	//Mapeamos la ultima pagina del directorio a si mismo
	//newPD[1023] = phy | 0x63;
	// Para evitar reclamar memoria que ya está ocupada iniciamos las paginas del kernel en el directorio
	uint32_t kernel_space_end = next_virtualFreeAddress;
	printf("\nTo install %x : %x", KERNEL_VIRTUAL_BASE, kernel_space_end);
	for(uint32_t i = KERNEL_VIRTUAL_BASE; i <= kernel_space_end; i += PAGE_TAM){
		page_t *pg = get_page(i, 1, kernel_directory);
		free_frame(pg);
		alloc_frame_int(pg, true, true, true, true, true, i - KERNEL_VIRTUAL_BASE);
	}
	printf("\nkernel installed");
	printf("\nRecursive: %x at %x", newPD, phy);
	//kernel_directory->tables[1023] = (page_table_t *) ((uint32_t)phy | 0x63);

	starting = false;
	if(modules_firstAddress != NULL){
		uint32_t initrd = loadModulesFromGrub(mbi);
		initrd_addr = (uint8_t *)initrd;
		printf("\nInitrd module load on 0x%x", initrd);
	}else{
		initrd_addr = NULL;
	}
	//Registramos el handler para la paginacion
	register_isrs_handler(14, paging_handler);
	//Cambiamos al modo 4KB con el nuevo directorio
	switch_4kb_pagination(phy);
}

void paging_handler(irt_regs *r){
	//The address with the error is in cr2
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
	// The error code gives us details of what happened.
	int present   = !(r->err_code & 0x1);	// Page not present
	int rw = r->err_code & 0x2;		// Write operation?
	int us = r->err_code & 0x4;		// Processor was in user-mode?
	int reserved = r->err_code & 0x8;	// Overwritten CPU-reserved bits of page entry?
	int id = r->err_code & 0x10;		// Caused by an instruction fetch?
	printf("\nFaulting (p:%x, rw:%x, u:%x, r:%x, id:%x): %x\n", present, rw, us, reserved, id, faulting_address);
	if(present){
		//Obtenemos la pagina que falta o la creamos si no existe
		page_t *ptm = get_page(faulting_address, 1, current_page_directory);
		alloc_frame(ptm, true, true);
		//Actualizamos la cache de direcciones de la CPU
		refresh_page(faulting_address);
		printf("\nPtm %x : %x", faulting_address, ptm->frame);
		if(faulting_address == 0){
			printf("CE: 0x%x", *(uint32_t *)ptm);
			while(1);
		}
	}else{
		uint32_t fcr3, fcr4, fcr0;
		asm volatile("mov %%cr3, %0" : "=r" (fcr3));
		asm volatile("mov %%cr4, %0" : "=r" (fcr4));
		asm volatile("mov %%cr0, %0" : "=r" (fcr0));
		//panic_exception("Paging handler", 0x22);
		printf("\nFatal error, Faulting: %x", faulting_address);
		printf("\nCR3: %x; CR4: %x; CR0: %x", fcr3, fcr4, fcr0);
		printf("\nP: %d; RW: %d; US: %d; R: %d; ID: %d;\n", present, rw, us, reserved, id);
		printregs(r);
		fault_halt();
	}
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir){
	// Turn the address into an index.
	uint32_t table_idx = PAGE_DIRECTORY_INDEX(address);
	uint32_t page_idx = PAGE_TABLE_INDEX(address);
	printf("\nget_page: t: %d, p: %d %x, %x, %x", table_idx, page_idx,  address, make, dir->tables[table_idx]);

	uint32_t *table = (uint32_t *) &(dir->tables[table_idx]);

	bool table_exist = false;

	if(table != NULL && dir->physical[table_idx].present != 0){// If this table is already assigned *(uint32_t *)(&dir->tables[table_idx])!=0
		table_exist = true;
		page_t* page = &(dir->tables[table_idx]->pages[page_idx]);
		printf("\nTable exist: %x : %x ", dir->physical[table_idx], page);
		if (page != NULL && page->present != 0) {
			printf("\nAddress %x Already: 0x%x ", address, *page);
			return page;
		}
	}

	if(make){
		page_table_t * page_table;
		if (!table_exist) {
			// Obtenemos un bloque (4kb) para alojar una nueva tabla de páginas
			uint32_t pt_phy;
			page_table = (page_table_t *) kmalloc_ap(sizeof(page_table_t), &pt_phy);
			// Limpiamos el area que nos han dado
			memset((uint32_t *) page_table, 0, sizeof(page_table_t));
			dir->physical[table_idx].frame = pt_phy >> 12;
			dir->physical[table_idx].present = 1;
			dir->physical[table_idx].rw = 1;
			dir->physical[table_idx].user = 1;
			dir->tables[table_idx] = page_table;
			printf("\nNew Table: %x : %x", dir->physical[table_idx], dir->tables[table_idx]);
		} else {
			page_table = dir->tables[table_idx];
		}
		// Ahora creamos la pagina en si
		page_t * page = &page_table->pages[page_idx];
		alloc_frame_int(page, true, true, true, true, false, NULL);
		printf("\nReturn %x : %x : %x", page_table, page, (page->frame << 12));
		return page;
	}

	return NULL;
}

page_t *get_page_default(uint32_t address, int make){
	return get_page(address, make, kernel_directory);
}

void switch_page_directory(page_directory_t *dir){
	page_directory_t *work = current_page_directory;
	current_page_directory = dir;
	if(work != dir){
		printf("\nDir = %x old %x ker %x", (uint32_t)dir, (uint32_t)work, (uint32_t)kernel_directory);
		printf("\n PTY %x k %x", to_physical_addr((uint32_t)dir, work), to_physical_addr((uint32_t)dir, kernel_directory));
		//STOP()
	}
	asm volatile("mov %0, %%cr3":: "r"(to_physical_addr((uint32_t)dir, work)): "%eax");
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
	uintptr_t remaining = virtual % 0x1000;
	uintptr_t frame = virtual / 0x1000;
	uintptr_t table = frame / 1024;
	uintptr_t subframe = frame % 1024;
	if (dir->tables[table] != NULL) {
		page_t *p = &dir->tables[table]->pages[subframe];
		if (p != NULL){
			return (p->frame << 12) + remaining;
		}
	}
	return NULL;
}

uint32_t loadModulesFromGrub(multiboot_info_t *mbi){
	uint32_t initrd_location = *((uint32_t*)mbi->mods_addr);
	uint32_t initrd_end = *(uint32_t*)(mbi->mods_addr+4);
	uint32_t initrd_frame = initrd_location & 0xFFFFF000;
	uint32_t module_tam = initrd_end - initrd_location;
	uint32_t module_offset = initrd_location - initrd_frame;

	//Normally the modules are far ahead the kernel, we move it to the next free page at the end of the kernel
	printf("\ninitrd: 0x%x\n", initrd_location);
	for(int i= 0; i < 0x20; i++){
		printf("0x%x ", *((uint8_t *) (initrd_location + i)));
	}
	if(initrd_location > next_freeAddress){
		memcpy((uint8_t*)next_freeAddress, (uint8_t*)initrd_location, module_tam);
		//Update the vars
		initrd_location = next_freeAddress;
		next_freeAddress += module_tam;
		initrd_end = initrd_location + module_tam;
		initrd_frame = initrd_location & 0xFFFFF000;
		module_offset = initrd_location - initrd_frame;
		//Now we aling the next_freeAddress with a page boundary
		next_freeAddress &= 0xFFFFF000;
		next_freeAddress += 0x00001000;
	}
	printf("\nInitrd: 0x%x\n", initrd_location);
	printf("\nInitrd tam: 0x%x\n", module_tam);
	for(int i= 0; i < 0x20; i++){
		printf("0x%x ", *((uint8_t *) (initrd_location + i)));
	}
	if(next_virtualFreeAddress & 0x00000FFF != 0){
		next_virtualFreeAddress &= 0xFFFFF000;
		next_virtualFreeAddress += 0x00001000;
	}
	uint32_t module_virtualAddr = next_virtualFreeAddress + module_offset;
	uint32_t i = 0;
	for(i = 0; i < module_tam; i += PAGE_TAM){
		next_virtualFreeAddress += i;
		page_t *pg = get_page(next_virtualFreeAddress, 1, kernel_directory);
		free_frame(pg);
		alloc_frame_int(pg, true, true, true, true, true, initrd_frame + i);
		printf("\n%x : %x", next_virtualFreeAddress, initrd_frame );
	}
	if( (i - PAGE_TAM) == 0){
		next_virtualFreeAddress += module_tam;
	}
	printf("\nModule: 0x%x\n", module_virtualAddr);
	page_t *pg = get_page(module_virtualAddr, 0, kernel_directory);
	printf("\nFrame: 0x%x\n", pg->frame);
	for(int i= 0; i < 0x20; i++){
		printf("0x%x ", *((uint8_t *) (module_virtualAddr + i)));
	}
	printf("\nInitf: 0x%x\n", initrd_frame);
	return module_virtualAddr;
}



page_directory_t * create_page_directory(){
	page_directory_t *newPD = (page_directory_t *)kmalloc_dumb(sizeof(page_directory_t));
	memset(newPD, 0, sizeof(page_directory_t));
	for(int i = KERNEL_PAGE; i < 1024; i++){
		newPD->tables[i] = kernel_directory->tables[i];
		//newPD->physical_tables[i] = kernel_directory->physical_tables[i];
	}
	return newPD;
}

page_directory_t * copy_page_directory(page_directory_t *src){
	page_directory_t *newPD = (page_directory_t *)kmalloc(sizeof(page_directory_t));
	memset(newPD, 0, sizeof(page_directory_t));
	for(int i = 0; i < 1024; i++){
		if(i >= KERNEL_PAGE){
			newPD->tables[i] = src->tables[i];
			//newPD->physical_tables[i] = src->physical_tables[i];
		} else if (src->tables[i] != NULL){
			uint32_t phy = 0;
			newPD->tables[i] = copyTable(src->tables[i], &phy);
			//newPD->physical_tables[i] = (uintptr_t)phy;
		}
	}
	return newPD;
}

page_table_t *copyTable(page_table_t *src, uint32_t *phy){
	page_table_t *newPD = (page_directory_t *)kmalloc(sizeof(page_directory_t));
	*phy = ((uint32_t) to_physical_addr((uint32_t) &newPD, kernel_directory));
	for(int i= 0; i < 1024; i++){
		memcpy(&(newPD->pages[i]), &(src->pages[i]), sizeof(page_t));
	}
	return newPD;
}

