/*
 * syscalls.c
 *
 *  Created on: 1 may. 2017
 *      Author: joaquin
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/syscalls/syscalls.h>

void syscalls_install(){
	//Registramos el handler para la paginacion
	register_isrs_handler(0x80, syscalls_handler);
	idt_set_gate(0x80, (unsigned)_syscallCommon, 0x08, 0x8E);

}

void syscalls_handler(irt_regs *r){
	uint32_t code = r->eax;
	printf("\nThis is syscall handler 0x%x", code);
	if (code == 69){
		HALT();
	}
}
