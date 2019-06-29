#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/io/serial.h>
#include <kernel/interruptions/irq.h>
#include <kernel/io/io.h>

void serial_handler(struct irt_regs *r) {
    printf("Serial input!");
    char character = read_serial();
    printf("Serial input: %s\n", character);
}

void serial_install() {
    outportb(COM_1 + 1, 0x00);    // Disable all interrupts
    irq_install_handler(4, serial_handler);
    outportb(COM_1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outportb(COM_1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outportb(COM_1 + 1, 0x00);    //                  (hi byte)
    outportb(COM_1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outportb(COM_1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outportb(COM_1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty() {
   return inportb(COM_1 + 5) & 0x20;
}
 
void write_serial(char a) {
   while (is_transmit_empty() == 0);
   outportb(COM_1,a);
}

int serial_received() {
   return inportb(COM_1 + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return inportb(COM_1);
}