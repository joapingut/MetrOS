#ifndef _KERNEL_IO_H
#define _KERNEL_IO_H

#include <stddef.h>

static inline void outportb(uint16_t _port, uint8_t _data)
{
	__asm__ __volatile__ ("outb %1, %0"
		: :
		"dN" (_port),
		"a" (_data));
}

static inline uint8_t inportb(uint16_t _port){
	uint8_t ret;
	__asm__ __volatile__ ("inb %1, %0"
		: "=a" (ret)
		: "dN" (_port));
	return ret;
}

#endif
