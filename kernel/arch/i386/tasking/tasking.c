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

//void copyRegs(irt_regs *src, irt_regs *dst);

uint32_t nextIdentifier;

process_t *processList;
process_t *currentProcess;

bool schedulerOn = false;

void tasking_install(){

    kernelProcess.cr3 = kernel_directory;
    kernelProcess.identifier = 0;
    kernelProcess.next = NULL;
    kernelProcess.previous = NULL;

    processList = NULL;
    currentProcess = &kernelProcess;
    nextIdentifier = 1;
    schedulerOn = true;
}

bool createTask(process_t *task, void (*main)(), page_directory_t *pagedir){
	task->identifier = nextIdentifier;
	task->next = task;
	task->previous = task;
	task->cr3 = pagedir;
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

volatile void switch_task(irt_regs *regs){
	printf("\nScheduler!!");
	if(!schedulerOn)
		return;
	/* copy the saved registers into the current_proc structure */
	/*printf("\nCurr regs %x", &currentProcess->regs);
	printf("\nregs %x", &regs);*/
	memcpy(&(currentProcess->regs), regs, sizeof(irt_regs));
	/*printf("\nCr3 %x", currentProcess->cr3);
	printf("\neip %x", currentProcess->regs.eip);
	printf("\nesp %x", currentProcess->regs.esp);
	printf("\nUSERSSP %x", currentProcess->regs.useresp);
	printf("\nss %x", currentProcess->regs.ss);
	printf("\nEIP: %x ESP: %x ERR: %x IN: %x ", regs->eip, regs->esp, regs->err_code, regs->int_no);
	printf("\nCS: %x SS: %x USP: %x", regs->cs, regs->ss, regs->useresp);*/

	/* now go onto the next task - if there isn't one, go back to the start of the queue. */
	if(currentProcess->identifier == 0){
		if(processList != NULL){
			currentProcess = processList;
		}else{
			return;
		}
	}else{
		if (currentProcess->next != NULL){
			currentProcess = currentProcess->next;
		}else{
			currentProcess = &kernelProcess;
		}
	}

	uint32_t eip, esp;

	if(currentProcess->status == STARTING){
		eip = currentProcess->regs.eip;
		esp = currentProcess->regs.esp;
	}

	if(currentProcess->status == STARTING){
		memcpy(&(currentProcess->regs), regs, sizeof(irt_regs));
		currentProcess->regs.eip = eip;
		currentProcess->regs.esp = esp;
		currentProcess->status = RUNNING;
	}
	/* now hack the registers! */
	memcpy(regs, &(currentProcess->regs), sizeof(irt_regs));
	/*printf("\nCr3 %x", currentProcess->cr3);
	printf("\neip %x", currentProcess->regs.eip);
	printf("\nUSERSSP %x", currentProcess->regs.useresp);
	printf("\nesp %x", currentProcess->regs.esp);*/
	//set_kernel_stack(kernelProcess.regs.esp);
	switch_page_directory(currentProcess->cr3);
	/*printf("\nEIP: %x ESP: %x ERR: %x IN: %x ", regs->eip, regs->esp, regs->err_code, regs->int_no);
	printf("\nCS: %x SS: %x USP: %x", regs->cs, regs->ss, regs->useresp);
	printf("\nBYE");*/
};


/*void copyRegs(irt_regs *src, irt_regs *dst){
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
}*/
