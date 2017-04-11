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
#include <kernel/memory/paging.h>

typedef enum {STARTING, RUNNING, WAITING, STOPED} process_status;

typedef struct process_struct{
	irt_regs regs;
	uint32_t esp0;
	page_directory_t *cr3;
	uint32_t identifier;
	process_status status;
	struct process_struct* previous;
	struct process_struct* next;
} process_t;

process_t kernelProcess;

void tasking_install();
bool createTask(process_t *task, void (*main)(), page_directory_t *pagedir);
volatile void switch_task(irt_regs *regs);

#endif /* KERNEL_INCLUDE_KERNEL_TASKING_TASKING_H_ */
