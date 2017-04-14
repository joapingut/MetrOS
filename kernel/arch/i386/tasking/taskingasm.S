/*
 * tasking.S
 *
 *  Created on: 2 abr. 2017
 *      Author: joaquin
 */
.section .text
.align 0x1000

.global _switchKernelTask
.type _switchKernelTask, @function
_switchKernelTask:
    add $8, %esp

    /* Restore segment registers */
    pop %gs
    pop %fs
    pop %es
    pop %ds

    /* Restore all registers */
    popa
    /* Cleanup error code and IRQ # */
    add $8, %esp
    add (%esp), %ebx //EIP
    add 4(%esp), %ecx //CS
    add 8(%esp), %eax //EFLAGS
    push %eax
    popf
    pop %eax
    add 12(%esp), %edx //ESP
    add 16(%esp), %eax //SS
    push %eax
    pop %ss
    pop %eax
    add %edx, %esp
    push %ecx
    push %ebx
    /* pop CS, EIP, EFLAGS, SS and ESP */
    ret
/*eip, cs, eflags, useresp, ss*/
