.section .text
.align 0x1000

.global idt_load
.type idt_load, @function

idt_load:
    mov 4(%esp), %eax
    lidt (%eax)
    ret
