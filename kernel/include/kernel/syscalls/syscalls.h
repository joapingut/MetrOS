/*
 * syscalls.h
 *
 *  Created on: 1 may. 2017
 *      Author: joaquin
 */

#ifndef KERNEL_INCLUDE_SYSCALLS_SYSCALLS_H_
#define KERNEL_INCLUDE_SYSCALLS_SYSCALLS_H_

#include <kernel/interruptions/isrs.h>
#include <kernel/interruptions/idt.h>

void syscalls_install();
void syscalls_handler(irt_regs *r);

extern void _syscallCommon();

#endif /* KERNEL_INCLUDE_SYSCALLS_SYSCALLS_H_ */
