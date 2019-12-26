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

uint32_t nextIdentifier;

process_t* processList;
process_t* currentProcess;

bool schedulerOn = false;

void tasking_install(){

    kernelProcess.cr3 = (uint32_t) kernel_directory;
    kernelProcess.identifier = 0;
    kernelProcess.next = NULL;
    kernelProcess.previous = NULL;
    kernelProcess.status = RUNNING;

    processList = NULL;
    currentProcess = &kernelProcess;
    nextIdentifier = 1;
}

void switchSchedulerState(){
	schedulerOn = !schedulerOn;
}

bool createTask(process_t *task, void (*main)(), uint32_t stack_bottom, uint32_t stack_size, uint32_t pagedir){
	printf("\nNew Process");
	if(stack_bottom == NULL){
		stack_bottom = (uint32_t) kmalloc_a(0x1000);
		stack_size = 0x1000;
		printf("\nAsigned Stack: %x", stack_bottom);
	}
	uint32_t nKesp = (uint32_t) kmalloc_a(0x1000);
	memset((uint32_t *) nKesp, 0, 0x1000);
	printf("\nKesp: %x", nKesp);
	task->identifier = nextIdentifier;
	task->status = STARTING;
	task->next = task;
	task->previous = task;
	task->cr3 = pagedir;
	task->regs.eip = (uint32_t) main;
	task->regs.esp = 0;
	task->esp0 = nKesp + 0x1000;
	task->regs.cs = 0x1B;
	task->regs.ss = 0x23;
	task->regs.eflags = 0x200;
	task->regs.useresp = stack_bottom + stack_size;

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
	task->previous = NULL;
	if(processList != NULL){
		task->next = processList;
		processList->previous = task;
	}else{
		task->next = NULL;
	}
	processList = task;
	printf("\nProcess number: %d created", task->identifier);
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
		if (currentProcess->next != NULL){
			process_t * nextProcess = currentProcess->next;
			printf("\nCurrent id: %x", (currentProcess->identifier));
			printf("\nNext id: %x %x", nextProcess->identifier, currentProcess->regs.eip);
			printf("\nStatus: %x", currentProcess->status);
			if(currentProcess->identifier != nextProcess->identifier){
				currentProcess = nextProcess;
			}else{
				currentProcess = &kernelProcess;
			}
		}else{
			if (processList != NULL){
				currentProcess = processList;
			} else {
				currentProcess = &kernelProcess;
			}
		}
	}

	printf("\nNprc %x : %x", currentProcess, currentProcess->regs.eip);
	currentProcess->status = RUNNING;
	/* now hack the registers! */
	memcpy(regs, &(currentProcess->regs), sizeof(irt_regs));
	printf("Kstack: %x", currentProcess->esp0);
	set_kernel_stack(currentProcess->esp0);
	switch_page_directory(currentProcess->cr3);

	printregs(&currentProcess->regs);
	printf("\nBYE %x", currentProcess->identifier);
};

__attribute__((__noreturn__))
void enter_user_mode(){
	process_t * cur = processList;
	do {
		printf("\nid: %d", cur->identifier);
		cur = cur->next;
	} while (cur != NULL);

	uint32_t esp;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	kernelProcess.esp0 = esp;
	kernelProcess.regs.useresp = esp;
	schedulerOn = true;
	HALT();
}

void printregs(irt_regs *regs){
	printf("\nGS %x fs %x es %x ds %x", regs->gs, regs->fs, regs->es, regs->ds);
	printf("\nebp %x esp %x edi %x esi %x", regs->ebp, regs->esp, regs->edi, regs->esi);
	printf("\nEIP %x CS %x EFLAGS %x USERESP %x SS %x", regs->eip, regs->cs, regs->eflags, regs->useresp, regs->ss);
}
