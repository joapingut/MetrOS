#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_scroll();
void terminal_movecursor(size_t x, size_t y);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void set_kernel_panic_vga();

#endif
