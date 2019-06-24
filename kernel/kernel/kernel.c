#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/screen/tty.h>
#include <kernel/multiboot.h>
#include <kernel/memory/gdt.h>
#include <kernel/interruptions/idt.h>
#include <kernel/interruptions/isrs.h>
#include <kernel/interruptions/irq.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/vmm.h>
#include <kernel/interruptions/timers.h>
#include <kernel/io/keyboard.h>
#include <filesystem/initrd.h>
#include <liballoc/liballoc.h>
#include <kernel/tasking/tasking.h>
#include <kernel/syscalls/syscalls.h>

extern uint32_t end;
fs_node_t *initrdNode;
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
	vmm_install();
	printf("Installed PAGING\n");
	//printf("Installing timers\n");
	timer_install();
	printf("Installed timers\n");
	//printf("Installing keyboard\n");
	keyboard_install();
	printf("Installed keyboard\n");
	//initrdNode = initialise_initrd();
	printf("Installed initrd\n");
	tasking_install();
	printf("Installed tasking\n");
	syscalls_install();
	printf("Installed syscalls\n");
}

static void otherMain() {
	printf("\nHello multitasking world! 1");
	while(1){
		printf("\n SECOND");
	}
	STOP()
}

static void otherMain3() {
	printf("\nHello multitasking world! 2");
	while(1){
		printf("\n THREE");
		__asm__ (
				"int $0x80;"
		);
	}
	STOP()
}

static void otherMain4() {
	printf("\nHello multitasking world! 3");
	while(1){
		printf("\n FOUR");
	}
	STOP()
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
	//while(1);
	uint32_t *ptr = dirr;
	printf("PAGE A %x\n", ptr);
	uint32_t do_page_fault = *ptr;
	printf("PAGE F %d", do_page_fault);
	kfree(ptr);
	ptr = kmalloc(sizeof(uint32_t));
	printf("PAGE A %x\n", ptr);
	//ptr = (uintptr_t)0xA0100032;
	printf("\nPAGE B %x\n", ptr);
	*ptr = 20;
	do_page_fault = *ptr;	
	printf("\nPAGE F %d\n", do_page_fault);

	uint32_t *ptra = kmalloc(sizeof(uint32_t));
	uint32_t *ptrb = kmalloc(sizeof(uint32_t));
	printf("PAGE A %x\n", ptra);
	printf("PAGE B %x\n", ptrb);
	kfree(ptra);
	uint32_t *ptrc = kmalloc(sizeof(uint32_t));
	printf("PAGE B %x\n", ptrb);
	printf("PAGE C %x\n", ptrc);
	if(initrdNode != NULL){
		testinitrdFilesystem();
		fs_node_t *fsnode = finddir_fs(initrdNode, "dumb.o");
		if(fsnode != NULL){
			printf("\nNode Found! Executing..");
			/*process_t *elfProcess = kmalloc(sizeof(process_t));
			page_directory_t *newDirectory = create_page_directory();
			createTask(elfProcess, otherMain4, NULL, 0, newDirectory);
			int exec_elf(fsnode, process_t *elf_process);*/
			printf(" Fsnode %s", fsnode->name);
			executeFile(fsnode);
			STOP()
		}
	}
	switchSchedulerState();
	printf("\nEND");
	STOP();
	process_t *nprocess = kmalloc(sizeof(process_t));
	createTask(nprocess, otherMain, NULL, 0, kernelProcess.cr3);
	process_t *nprocess2 = kmalloc(sizeof(process_t));
	createTask(nprocess2, otherMain3, NULL, 0, kernelProcess.cr3);
	process_t *nprocess3 = kmalloc(sizeof(process_t));
	page_directory_t *newDirectory = create_page_directory();
	createTask(nprocess3, otherMain4, NULL, 0, newDirectory);
	printf("\n***END**");
	while(1){
		printf("\n first");
	}
	while(1);
}

void testinitrdFilesystem(){
	int i = 0;
	struct dirent *node = 0;
	while ( (node = readdir_fs(initrdNode, i)) != 0){
		printf("Found file ");
		printf(node->name);
		fs_node_t *fsnode = finddir_fs(initrdNode, node->name);

		if ((fsnode->flags&0x7) == FS_DIRECTORY)
			printf("\n\t(directory)\n");
		else{
			printf("\n\t contents: \"");
			char buf[256];
			uint32_t sz = read_fs(fsnode, 0, 256, buf);
			int j;
			for (j = 0; j < sz; j++)
				printf("%c", buf[j]);
			printf("\"\n");
		}
		i++;
	}
}
