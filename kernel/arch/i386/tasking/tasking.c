/*
 * tasking.c
 *
 *  Created on: 2 abr. 2017
 *      Author: joaquin
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tasking/tasking.h>

void copyRegs(irt_regs *src, irt_regs *dst);

process_t kernelProcess;
uint32_t nextIdentifier;

process_t *processList;

void tasking_install(){
	uint32_t cr3;
	uint32_t eflags;

    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
    //asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(eflags)::"%eax");

    kernelProcess.regs.cr3 = cr3;
    kernelProcess.identifier = 0;
    kernelProcess.next = NULL;
    kernelProcess.previous = NULL;

    processList = NULL;

    nextIdentifier = 1;
}

bool createTask(process_t *task, void (*main)(), uint32_t flags, page_directory_t *pagedir){
	task->identifier = nextIdentifier;
	task->next = task;
	task->previous = task;
	task->regs.cr3 = (uint32_t) pagedir;
	task->regs.eip = (uint32_t) main;
	task->regs.esp = (uint32_t) kmalloc(0x1000);
	nextIdentifier += 1;
	if(processList != NULL){
		task->previous = processList->previous;
		task->next = processList;
		processList->previous->next = task;
		processList->previous = task;
	}else{
		processList = task;
	}
	return true;
}


void copyRegs(registers *src, registers *dst){
	dst->eflags = src->eflags;
	dst->cr3 = src->cr3;

	dst->edi = src->edi;
	dst->esi = src->esi;
	dst->ebp = src->ebp;
	dst->esp = src->esp;
	dst->ebx = src->ebx;
	dst->edx = src->edx;
	dst->ecx = src->ecx;
	dst->eax = src->eax;
}
