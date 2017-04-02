#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/interruptions/irq.h>
#include <kernel/interruptions/idt.h>
#include <kernel/io/io.h>

static void *irq_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(regs *r)){
	SYNC_CLI();
	irq_routines[irq] = handler;
	SYNC_STI();
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq){
	SYNC_CLI();
	irq_routines[irq] = NULL;
	SYNC_STI();
}

static void irq_remap(void){
	/* Cascade initialization */
	outportb(PIC1_COMMAND, ICW1_INIT|ICW1_ICW4); PIC_WAIT();
	outportb(PIC2_COMMAND, ICW1_INIT|ICW1_ICW4); PIC_WAIT();
	/* Remap */
	outportb(PIC1_DATA, PIC1_OFFSET); PIC_WAIT();
	outportb(PIC2_DATA, PIC2_OFFSET); PIC_WAIT();

	/* Cascade identity with slave PIC at IRQ2 */
	outportb(PIC1_DATA, 0x04); PIC_WAIT();
	outportb(PIC2_DATA, 0x02); PIC_WAIT();

	/* Request 8086 mode on each PIC */
	outportb(PIC1_DATA, 0x01); PIC_WAIT();
	outportb(PIC2_DATA, 0x01); PIC_WAIT();
}

void irq_install(){
	irq_remap();
	idt_set_gate(32, (unsigned)_irq0, 0x08, 0x8E);
	idt_set_gate(33, (unsigned)_irq1, 0x08, 0x8E);
	idt_set_gate(34, (unsigned)_irq2, 0x08, 0x8E);
	idt_set_gate(35, (unsigned)_irq3, 0x08, 0x8E);
	idt_set_gate(36, (unsigned)_irq4, 0x08, 0x8E);
	idt_set_gate(37, (unsigned)_irq5, 0x08, 0x8E);
	idt_set_gate(38, (unsigned)_irq6, 0x08, 0x8E);
	idt_set_gate(39, (unsigned)_irq7, 0x08, 0x8E);
	idt_set_gate(40, (unsigned)_irq8, 0x08, 0x8E);
	idt_set_gate(41, (unsigned)_irq9, 0x08, 0x8E);
	idt_set_gate(42, (unsigned)_irq10, 0x08, 0x8E);
	idt_set_gate(43, (unsigned)_irq11, 0x08, 0x8E);
	idt_set_gate(44, (unsigned)_irq12, 0x08, 0x8E);
	idt_set_gate(45, (unsigned)_irq13, 0x08, 0x8E);
	idt_set_gate(46, (unsigned)_irq14, 0x08, 0x8E);
	idt_set_gate(47, (unsigned)_irq15, 0x08, 0x8E);
}

void irq_handler(regs *r){
	//printf("IRQ: %d", r->int_no);
	SYNC_CLI();
	/* This is a blank function pointer */
	void (*handler)(regs *r);

	/* Find out if we have a custom handler to run for this
	*  IRQ, and then finally, run it */
	handler = irq_routines[r->int_no - 32];
	if (handler){
		handler(r);
	}

	irq_ack(r->int_no - 32);
	SYNC_STI();
}

void irq_ack(size_t irq_no) {
	/* If the IDT entry that was invoked was greater than 7
	*, then we need to send an EOI to
	*  the slave controller */
	if (irq_no > 7) {
		outportb(PIC2_COMMAND, PIC_EOI);
	}
	/* In either case, we need to send an EOI to the master
	*  interrupt controller too */
	outportb(PIC1_COMMAND, PIC_EOI);
}
