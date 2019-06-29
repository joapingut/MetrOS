#ifndef _SERIAL_H
#define _SERIAL_H

#include <kernel/system.h>

#define COM_1 0x3f8   /* COM1 */

void serial_install();
int is_transmit_empty();
void write_serial(char a);
int serial_received();
char read_serial() ;

#endif