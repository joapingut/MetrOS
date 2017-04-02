#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/interruptions/isrs.h>
#include <kernel/memory/paging.h>
#include <kernel/screen/tty.h>

static isrs_pointers_t isrs;
static isrs_function isrs_routines[256] = {0};

void register_isrs_handler(uint32_t isrs, isrs_function handler){
	if(isrs < 256){
		isrs_routines[isrs] = handler;
	}
}

void fault_handler(regs *r){
	isrs_function handler = isrs_routines[r->int_no];
	if(handler != 0){
		handler(r);
	}else{
		panic_exception(exception_messages[r->int_no], exception_messages[r->err_code]);
	}
}

void panic_exception(char *message, uint32_t errorCode){
	set_kernel_panic_vga();
	printf("%s Exception. System Halted!\n", message);
	printf("Error code: 0x%x\n", errorCode);
	fault_halt();
}

void panic(const char *message, const char *file, uint32_t line){
	set_kernel_panic_vga();
	printf("KERNEL PANIC!!\N");
	printf("On file: %s\n", file);
	printf("At line: %d\n", line);
	printf("Message:\n%s", message);
	fault_halt();
}

void panic_assert(const char *file, uint32_t line, const char *desc){
	set_kernel_panic_vga();
	printf("ASSERT FAULT!!\N");
	printf("On file: %s\n", file);
	printf("At line: %d\n", line);
	printf("DESCRIPTION:\n%s", desc);
	fault_halt();
}

__attribute__((__noreturn__))
void fault_halt(){
	while ( 1 ) { }
	__builtin_unreachable();
}

void isrs_install(){
	idt_set_gate(0, (unsigned)_isr0, 0x08, 0x8E);
	idt_set_gate(1, (unsigned)_isr1, 0x08, 0x8E);
	idt_set_gate(2, (unsigned)_isr2, 0x08, 0x8E);
	idt_set_gate(3, (unsigned)_isr3, 0x08, 0x8E);
	idt_set_gate(4, (unsigned)_isr4, 0x08, 0x8E);
	idt_set_gate(5, (unsigned)_isr5, 0x08, 0x8E);
	idt_set_gate(6, (unsigned)_isr6, 0x08, 0x8E);
	idt_set_gate(7, (unsigned)_isr7, 0x08, 0x8E);
	idt_set_gate(8, (unsigned)_isr8, 0x08, 0x8E);
	idt_set_gate(9, (unsigned)_isr9, 0x08, 0x8E);
	idt_set_gate(10, (unsigned)_isr10, 0x08, 0x8E);
	idt_set_gate(11, (unsigned)_isr11, 0x08, 0x8E);
	idt_set_gate(12, (unsigned)_isr12, 0x08, 0x8E);
	idt_set_gate(13, (unsigned)_isr13, 0x08, 0x8E);
	idt_set_gate(14, (unsigned)_isr14, 0x08, 0x8E);
	idt_set_gate(15, (unsigned)_isr15, 0x08, 0x8E);
	idt_set_gate(16, (unsigned)_isr16, 0x08, 0x8E);
	idt_set_gate(17, (unsigned)_isr17, 0x08, 0x8E);
	idt_set_gate(18, (unsigned)_isr18, 0x08, 0x8E);
	idt_set_gate(19, (unsigned)_isr19, 0x08, 0x8E);
	idt_set_gate(20, (unsigned)_isr20, 0x08, 0x8E);
	idt_set_gate(21, (unsigned)_isr21, 0x08, 0x8E);
	idt_set_gate(22, (unsigned)_isr22, 0x08, 0x8E);
	idt_set_gate(23, (unsigned)_isr23, 0x08, 0x8E);
	idt_set_gate(24, (unsigned)_isr24, 0x08, 0x8E);
	idt_set_gate(25, (unsigned)_isr25, 0x08, 0x8E);
	idt_set_gate(26, (unsigned)_isr26, 0x08, 0x8E);
	idt_set_gate(27, (unsigned)_isr27, 0x08, 0x8E);
	idt_set_gate(28, (unsigned)_isr28, 0x08, 0x8E);
	idt_set_gate(29, (unsigned)_isr29, 0x08, 0x8E);
	idt_set_gate(30, (unsigned)_isr30, 0x08, 0x8E);
	idt_set_gate(31, (unsigned)_isr31, 0x08, 0x8E);
}
