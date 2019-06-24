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
#include <utils/elf.h>

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
    kernelProcess.status = RUNNING;

    processList = NULL;
    currentProcess = &kernelProcess;
    nextIdentifier = 1;
    //schedulerOn = true;
}

void switchSchedulerState(){
	schedulerOn = !schedulerOn;
}

bool createTask(process_t *task, void (*main)(), uint32_t stack_bottom, uint32_t stack_size, page_directory_t *pagedir){
	if(stack_bottom == NULL){
		stack_bottom = (uint32_t) kmalloc(0x1000);
		stack_size = 0x1000;
	}
	uint32_t nKesp = (uint32_t) kmalloc(0x1000);
	task->identifier = nextIdentifier;
	task->status = STARTING;
	task->next = task;
	task->previous = task;
	task->cr3 = pagedir;
	task->regs.eip = (uint32_t) main;
	task->regs.esp = stack_bottom + stack_size;
	task->esp0 = nKesp + 0x1000;
	task->regs.cs = 0x1B;
	task->regs.ss = 0x23;
	task->regs.eflags = 0x200;
	task->regs.useresp = task->regs.esp;

	task->regs.gs = task->regs.ss;
	task->regs.fs = task->regs.ss;
	task->regs.es = task->regs.ss;
	task->regs.ds = task->regs.ss;

	task->regs.edi = 0;
	task->regs.esi = 0;
	task->regs.ebp = 0;
	task->regs.ebx = 0;
	task->regs.edx = 0;
	task->regs.ecx = 0;
	task->regs.eax = 0;
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

void executeFile(fs_node_t *file){
	process_t *nprocess = kmalloc(sizeof(process_t));
	printf("\nExecuting");
	if(exec_elf(file, nprocess) != 1){
		printf("\nError loading elf file.");
	}else{
		printf("\nElf file %s loaded.", file->name);
	}
}

volatile void switch_task(irt_regs *regs){
	if(!schedulerOn)
		return;
	printf("\nScheduler!! %x:%x", currentProcess->identifier, currentProcess->status);
	/* copy the saved registers into the current_proc structure */
	memcpy(&(currentProcess->regs), regs, sizeof(irt_regs));
	printregs(&currentProcess->regs);

	page_directory_t *oldC = currentProcess->cr3;

	/* now go onto the next task - if there isn't one, go back to the start of the queue. */
	currentProcess->status = WAITING;
	printf("\nOprc %x", currentProcess);
	if(currentProcess->identifier == 0){
		currentProcess->regs.useresp = regs->useresp;
		currentProcess->esp0 = regs->esp;
		if(processList != NULL){
			currentProcess = processList;
		}else{
			return;
		}
	}else{
		printf("\nJE %x", nextIdentifier);
		if (currentProcess->next != NULL){
			process_t * nextProcess = currentProcess->next;
			printf("\nCiden: %x", (uint32_t)(currentProcess->identifier));
			printf("\nAdd: %x %x", nextProcess, currentProcess->regs.eip);
			printf("\nEsta: %x", currentProcess->status);
			printf("\nIden %x", nextProcess->identifier);
			if(currentProcess->identifier != nextProcess->identifier){
				currentProcess = nextProcess;
			}else{
				currentProcess = &kernelProcess;
			}
		}else{
			currentProcess = &kernelProcess;
		}
	}

	printf("\nNprc %x", currentProcess);
	currentProcess->status = RUNNING;
	/* now hack the registers! */
	memcpy(regs, &(currentProcess->regs), sizeof(irt_regs));
	set_kernel_stack(currentProcess->esp0);
	switch_page_directory(currentProcess->cr3);

	if(oldC != currentProcess->cr3){
		//STOP()
	}
	printregs(&currentProcess->regs);
	printf("\nBYE %x", currentProcess->identifier);

	/*if(currentProcess->identifier == 0){
		_switchKernelTask();
	}*/
};

void printregs(irt_regs *regs){
	printf("\nGS %x fs %x es %x ds %x", regs->gs, regs->fs, regs->es, regs->ds);
	printf("\nebp %x esp %x edi %x esi %x", regs->ebp, regs->esp, regs->edi, regs->esi);
	printf("\nEIP %x CS %x EFLAGS %x USERESP %x SS %x", regs->eip, regs->cs, regs->eflags, regs->useresp, regs->ss);
}
/*
void copyRegs(irt_regs *src, irt_regs *dst){
	dst->gs = src->gs;
	dst->fs = src->fs;
	dst->es = src->es;
	dst->ds = src->ds;

	//dst->cs = src->cs;
	dst->eflags = src->eflags;
	//dst->useresp = 0;
	//dst->ss = 0;

	dst->edi = 0;
	dst->esi = 0;
	dst->ebp = 0;
	dst->ebx = 0;
	dst->edx = 0;
	dst->ecx = 0;
	dst->eax = 0;
	/*dst->edi = src->edi;
	dst->esi = src->esi;
	dst->ebp = src->ebp;
	dst->esp = src->esp;
	dst->ebx = src->ebx;
	dst->edx = src->edx;
	dst->ecx = src->ecx;
	dst->eax = src->eax;
}*/
