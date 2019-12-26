#ifndef _PAGING_H
#define _PAGING_H

#include <kernel/system.h>
#include <kernel/multiboot.h>
#include <kernel/memory/boundaries.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_FRAME_ADDRESS(x) (x & ~0xfff)

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
	uint32_t present	: 1;   // Page present in memory
	uint32_t rw		: 1;   // Read-only if clear, readwrite if set
	uint32_t user		: 1;   // Supervisor level only if clear
	uint32_t pwt		: 1;   // CPU Page write through
	uint32_t pcd		: 1;   // CPU Page cache disabled
	uint32_t accessed	: 1;   // Has the page been accessed since last refresh?
	uint32_t dirty		: 1;   // Has the page been written to since last refresh?
	uint32_t ps		: 1;   // Is 4MB page
	uint32_t global		: 1;  // Global Page (Ignored)
	uint32_t avail	:3; // Available for kernel
	uint32_t frame	: 20;  // Frame address (shifted right 12 bits)
} __attribute__ ((packed)) page_dte_t;

typedef struct {
	page_dte_t physical[1024];
	page_table_t* tables[1024];
} __attribute__ ((packed)) page_directory_t;

page_directory_t *kernel_directory;

void paging_install(multiboot_info_t *mbi);
void paging_handler(irt_regs *r);
page_t *get_page(uint32_t address, int make, page_directory_t *dir);
page_t *get_page_default(uint32_t address, int make);

page_directory_t *create_page_directory();
page_directory_t *copy_page_directory(page_directory_t *src);
page_table_t *copyTable(page_table_t *src, uint32_t *phy);
uint32_t memory_used(); // Used memory en bytes
#endif

