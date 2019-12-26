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
#include <filesystem/vfs.h>
#include <kernel/memory/paging.h>

typedef enum {STARTING, RUNNING, WAITING, STOPED} process_status;

typedef struct process_struct{
	irt_regs regs;
	uint32_t esp0;
	uint32_t cr3;
	uint32_t identifier;
	process_status status;
	struct process_struct* previous;
	struct process_struct* next;
}  process_t;

process_t kernelProcess;

void tasking_install();
bool createTask(process_t *task, void (*main)(), uint32_t stack_bottom, uint32_t stack_size, uint32_t pagedir);
void executeFile(fs_node_t *file);
volatile void switch_task(irt_regs *regs);
extern void _switchKernelTask();
void switchSchedulerState();
void enter_user_mode();

#endif /* KERNEL_INCLUDE_KERNEL_TASKING_TASKING_H_ */
