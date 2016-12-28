.section .text
.align 0x1000

.global _irq0
.type _irq0, @function
	_irq0:
	cli
        push $0
        push $32
        jmp irq_common

.global _irq1
.type _irq1, @function
	_irq1:
	cli
        push $0
        push $33
        jmp irq_common

.global _irq2
.type _irq2, @function
	_irq2:
	cli
        push $0
        push $34
        jmp irq_common

.global _irq3
.type _irq3, @function
	_irq3:
	cli
        push $0
        push $35
        jmp irq_common

.global _irq4
.type _irq4, @function
	_irq4:
	cli
        push $0
        push $36
        jmp irq_common

.global _irq5
.type _irq5, @function
	_irq5:
	cli
        push $0
        push $37
        jmp irq_common

.global _irq6
.type _irq6, @function
	_irq6:
	cli
        push $0
        push $38
        jmp irq_common

.global _irq7
.type _irq7, @function
	_irq7:
	cli
        push $0
        push $39
        jmp irq_common

.global _irq8
.type _irq8, @function
	_irq8:
	cli
	push $0
        push $40
        jmp irq_common

.global _irq9
.type _irq9, @function
	_irq9:
	cli
        push $0
        push $41
        jmp irq_common

.global _irq10
.type _irq10, @function
	_irq10:
	cli
	push $0
        push $42
        jmp irq_common

.global _irq11
.type _irq11, @function
	_irq11:
	cli
	push $0
        push $43
        jmp irq_common

.global _irq12
.type _irq12, @function
	_irq12:
	cli
	push $0
        push $44
        jmp irq_common

.global _irq13
.type _irq13, @function
	_irq13:
	cli
	push $0
        push $45
        jmp irq_common

.global _irq14
.type _irq14, @function
	_irq14:
	cli
	push $0
        push $46
        jmp irq_common

.global _irq15
.type _irq15, @function
	_irq15:
	cli
        push $0
        push $47
        jmp irq_common

.extern irq_handler
.type irq_handler, @function

irq_common:
    /* Save all registers */
    pusha

    /* Save segment registers */
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Call interrupt handler */
    push %esp
    call irq_handler
    add $4, %esp

    /* Restore segment registers */
    pop %gs
    pop %fs
    pop %es
    pop %ds

    /* Restore all registers */
    popa
    /* Cleanup error code and IRQ # */
    add $8, %esp
    /* pop CS, EIP, EFLAGS, SS and ESP */
    iret
