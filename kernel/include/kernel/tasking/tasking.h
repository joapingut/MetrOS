/*
 * tasking.h
 *
 *  Created on: 2 abr. 2017
 *      Author: joaquin
 */

#ifndef KERNEL_INCLUDE_KERNEL_TASKING_TASKING_H_
#define KERNEL_INCLUDE_KERNEL_TASKING_TASKING_H_

#include <stdbool.h>
#include <kernel/system.h>

typedef struct {
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t  eip, eflags, cr3;
} __attribute__((packed)) registers;

typedef struct process_struct{
	registers regs;
	uint32_t esp0;
	uint32_t identifier;
	struct process_struct* previous;
	struct process_struct* next;
} process_t;

void tasking_install();
bool createTask(process_t *task, void (*main)(), uint32_t flags, page_directory_t *pagedir);
extern volatile void switch_task(registers *from, registers *to);
extern registers* saveRegs();

#endif /* KERNEL_INCLUDE_KERNEL_TASKING_TASKING_H_ */
