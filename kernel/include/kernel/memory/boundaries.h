#ifndef _MEMORY_BOUNDARIES_H
#define _MEMORY_BOUNDARIES_H

#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_DIRECTORIES_BASE 0xF0000000
#define KERNEL_PAGE (KERNEL_VIRTUAL_BASE >> 22)
#define PAGE_TAM 0x1000
#define PAGE_TAM_4MB 0x400000
#define PAGE_MASC (PAGE_TAM - 1)
#define PAGE_MASC_4MB (PAGE_TAM_4MB - 1)
#define PAGE_DEFAULT_VALUE 0x00000007 //todo change to 3 later
#define END_RESERVED_MEMORY 0x400000

extern uint32_t *__KERNEL_END;
extern uint32_t *BootPageDirectory;
extern uint32_t kernel_next_freeAddress;
extern uint32_t next_virtualFreeAddress;

#endif