/*
 * elf.c
 *
 *  Created on: 1 may. 2017
 *      Author: joaquin
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <utils/elf.h>
#include <kernel/memory/paging.h>


bool elf_check_file(Elf32_Header *header){
	if(!header)
		return false;
	if(header->e_ident[EI_MAG0] != ELFMAG0) {
		printf("ELF Header EI_MAG0 incorrect.\n");
		return false;
	}
	if(header->e_ident[EI_MAG1] != ELFMAG1) {
		printf("ELF Header EI_MAG1 incorrect.\n");
		return false;
	}
	if(header->e_ident[EI_MAG2] != ELFMAG2) {
		printf("ELF Header EI_MAG2 incorrect.\n");
		return false;
	}
	if(header->e_ident[EI_MAG3] != ELFMAG3) {
		printf("ELF Header EI_MAG3 incorrect.\n");
		return false;
	}
	return true;
}

bool elf_check_supported(Elf32_Header *hdr) {
	if(!elf_check_file(hdr)) {
		printf("Invalid ELF File.\n");
		return false;
	}
	if(hdr->e_ident[EI_CLASS] != ELFCLASS32) {
		printf("Unsupported ELF File Class.\n");
		return false;
	}
	if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
		printf("Unsupported ELF File byte order.\n");
		return false;
	}
	if(hdr->e_machine != EM_386) {
		printf("Unsupported ELF File target.\n");
		return false;
	}
	if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
		printf("Unsupported ELF File version.\n");
		return false;
	}
	if(hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
		printf("Unsupported ELF File type.\n");
		return false;
	}
	return true;
}

int exec_elf(fs_node_t * file, process_t *elf_process){
	Elf32_Header header;
	printf("\nHEllo, it's me marioelf");
	if(read_fs(file, 0, sizeof(Elf32_Header), (uint8_t *)&header) == NULL){
		printf("\nCannot read the elf file!");
	}
	if(!elf_check_file(&header)){
		return -1;
	}
	if(!elf_check_supported(&header)){
		return -1;
	}

	STOP()
	uintptr_t entry = (uintptr_t)header.e_entry;
	uintptr_t base_addr = 0xFFFFFFFF;
	uintptr_t end_addr  = 0x0;
	printf("\nTrying to load elf at %x", entry);

	for (uintptr_t x = 0; x < (uint32_t)header.e_phentsize * header.e_phnum; x += header.e_phentsize) {
		Elf32_Phdr phdr;
		read_fs(file, header.e_phoff + x, sizeof(Elf32_Phdr), (uint8_t *)&phdr);
		if (phdr.p_type == PT_LOAD) {
			if (phdr.p_vaddr < base_addr) {
				base_addr = phdr.p_vaddr;
			}
			if (phdr.p_memsz + phdr.p_vaddr > end_addr) {
				end_addr = phdr.p_memsz + phdr.p_vaddr;
			}
		}
	}
	printf("\nBase addr %x end Addr %x", base_addr, end_addr);
	page_directory_t *newDirectory = create_page_directory();
	createTask(elf_process, header.e_entry, USER_STACK_BOTTOM, USER_STACK_TOP - USER_STACK_BOTTOM, newDirectory);
	printf("\nElf task created at %x", header.e_entry);
	for (uintptr_t x = 0; x < (uint32_t)header.e_phentsize * header.e_phnum; x += header.e_phentsize) {
		Elf32_Phdr phdr;
		read_fs(file, header.e_phoff + x, sizeof(Elf32_Phdr), (uint8_t *)&phdr);
		if (phdr.p_type == PT_LOAD) {
			for (uintptr_t i = phdr.p_vaddr; i < phdr.p_vaddr + phdr.p_memsz; i += 0x1000) {
				alloc_frame(get_page(i, true, newDirectory), false, true);
				/* This doesn't care if we already allocated this page */
				/*alloc_frame(get_page(i, 1, newDirectory), 0, 1);
				invalidate_tables_at(i);*/
			}
			read_fs(file, phdr.p_offset, phdr.p_filesz, (uint8_t *)phdr.p_vaddr);
			size_t r = phdr.p_filesz;
			while (r < phdr.p_memsz) {
				*(char *)(phdr.p_vaddr + r) = 0;
				r++;
			}
		}
	}
	close_fs(file);
	for (uintptr_t stack_pointer = USER_STACK_BOTTOM; stack_pointer < USER_STACK_TOP; stack_pointer += 0x1000) {
		alloc_frame(get_page(stack_pointer, true, newDirectory), false, true);
	}
	return 1;
}
