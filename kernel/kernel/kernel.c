#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isrs.h>
#include <kernel/irq.h>
#include <kernel/paging.h>
#include <kernel/timers.h>
#include <kernel/keyboard.h>
#include <kernel/alloc.h>

extern uint32_t end;
uint32_t getMemoryAmountFromGrub(multiboot_info_t *mbi);

void kernel_early(multiboot_info_t* mbi, unsigned int magic){
	terminal_initialize();
	//printf ("mbi_addr = 0x%x, mbi_length = 0x%x\n",(unsigned int) mbi->mmap_addr, (unsigned int) mbi->mmap_length);
	if(magic != MULTIBOOT_BOOTLOADER_MAGIC){
		printf("Magic number error!!");
	}else{
		printf("Magic number has arrived!!");
		if((mbi->flags & (1<<6)) == 0){
			printf("No memory info passed!!");
		}
	}
	printf("kernel end: 0x%x; tam: 0x%x\n", &__KERNEL_END, &__KERNEL_END - KERNEL_VIRTUAL_BASE);
	printf("Page directory: 0x%x\n", &BootPageDirectory);
	printf("Kernel END: 0x%x\n", &end);
	//printf("Installing GDT\n");
	gdt_install();
	printf("Installed GDT\n");
	//printf("Installing IDT\n");
	idt_install();
	printf("Installed IDT\n");
	//printf("Installing ISRS\n");
	isrs_install();
	printf("Installed ISRS\n");
	//printf("Installing IRQ\n");
	irq_install();
	printf("Installed IRQ\n");
	//printf("Installing PAGING\n");
	paging_install(mbi);
	printf("Installed PAGING\n");
	//printf("Installing timers\n");
	timer_install();
	printf("Installed timers\n");
	//printf("Installing keyboard\n");
	keyboard_install();
	printf("Installed keyboard\n");
}

void kernel_main(void){
	int a = 0;
	//printf("Memoria disponible: %d\n", mmap->size);
	printf("Kernel\n");
	printf("Hello, kernel World!\n");
	printf("Prueba de la segunda linea!\n");
	//printf("Prueba de printf: %d\n String: %s", 200, " String !!! ñ");
	printf("Varios saltos\n\n\n");
	printf("FIN!\n");
	printf("Divison por 0!\n");
	a = 20 / 1;
	printf("Resultado: %d!\n", a);
	ticks();
	uint32_t dirr = kmalloc(sizeof(uint32_t));
	printf("Asigned: 0x%x\n", dirr);
	uint32_t *ptr = dirr;
	printf("PAGE A %x\n", ptr);
	uint32_t do_page_fault = *ptr;
	printf("PAGE F %d", do_page_fault);
	ptr = (uintptr_t)0xA0100032;
	printf("\nPAGE B %x\n", ptr);
	*ptr = 20;
	do_page_fault = *ptr;	
	printf("\nPAGE F %d\n", do_page_fault);
	printf("\n***END**");
	while(1);
}

