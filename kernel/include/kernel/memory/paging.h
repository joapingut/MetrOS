#ifndef _PAGING_H
#define _PAGING_H

#include <kernel/system.h>
#include <kernel/multiboot.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_DIRECTORIES_BASE 0xF0000000
#define KERNEL_PAGE (KERNEL_VIRTUAL_BASE >> 22)
#define PAGE_TAM 0x1000
#define PAGE_TAM_4MB 0x400000
#define PAGE_MASC (PAGE_TAM - 1)
#define PAGE_MASC_4MB (PAGE_TAM_4MB - 1)
#define PAGE_DEFAULT_VALUE 0x00000007 //todo change to 3 later
#define END_RESERVED_MEMORY 0x400000

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

typedef struct {
	uint32_t present	: 1;   // Page present in memory
	uint32_t rw		: 1;   // Read-only if clear, readwrite if set
	uint32_t user		: 1;   // Supervisor level only if clear
	uint32_t pwt		: 1;   // CPU Page write through
	uint32_t pcd		: 1;   // CPU Page cache disabled
	uint32_t accessed	: 1;   // Has the page been accessed since last refresh?
	uint32_t dirty		: 1;   // Has the page been written to since last refresh?
	uint32_t ps		: 1;   // Is 4MB page
	uint32_t unused		: 1;  // Amalgamation of unused and reserved bits
	uint32_t avail	:3; // Available for kernel
	uint32_t frame	: 20;  // Frame address (shifted right 12 bits)
} __attribute__ ((packed)) page_t;

typedef struct{
	page_t pages[1024];
} __attribute__ ((packed)) page_table_t;

typedef struct {
	uintptr_t physical_tables[1024];
	page_table_t *tables[1024];
} __attribute__ ((packed)) page_directory_t;

extern uint32_t *__KERNEL_END;
extern uint32_t *BootPageDirectory;
extern uint32_t next_freeAddress;
page_directory_t *kernel_directory;

void paging_install(multiboot_info_t *mbi);
void paging_handler(irt_regs *r);
page_t *get_page(uint32_t address, int make, page_directory_t *dir);
page_t *get_page_default(uint32_t address, int make);
static void set_frame(uint32_t frame_addr);
static void clear_frame(uint32_t frame_addr);
static uint32_t test_frame(uint32_t frame_addr);
static uint32_t first_frame();

page_directory_t *create_page_directory();
page_directory_t *copy_page_directory(page_directory_t *src);
page_table_t *copyTable(page_table_t *src, uint32_t *phy);
uint32_t memory_used(); // Used memory en bytes
#endif

