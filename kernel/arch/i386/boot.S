# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Constants for pagin
.set KERNEL_VIRTUAL_BASE, 0xC0000000
.set KERNEL_PAGE_NUMBER, (KERNEL_VIRTUAL_BASE >> 22)

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.align 0x1000

# Reserve a stack for the initial thread.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Seccion para la paginacion
.section .data
.align 0x1000
.global BootPageDirectory
BootPageDirectory:
	.long 0x00000083 #Pagina 0, necesaria para que no pete al activar la paginacion
	.fill (KERNEL_PAGE_NUMBER - 1), 4, 0x00000000 #Paginas vacias
	.long 0x00000083 #Pagina del kernel
	.long 0x00400083
	.fill (1024 - KERNEL_PAGE_NUMBER - 2), 4, 0x00000000 #Paginas vacias

# The kernel entry point. Paging activation
.section .setup, "aw", @progbits
.global _loader
_loader:
	lea (BootPageDirectory - KERNEL_VIRTUAL_BASE), %ecx
	mov %ecx, %cr3
 	movl %cr4, %ecx
	orl $0x00000010, %ecx #Activamos las paginas de 4MB en vez de las de 4KB
	movl %ecx, %cr4
	movl %cr0, %ecx
	orl $0x80000000, %ecx #Activamos la paginacion
	movl %ecx, %cr0
	lea (_start), %ecx
	jmp *%ecx

.section .text
.global _start
.type _start, @function
_start:
	#movl $0x00000000, (BootPageDirectory)
	#invlpg (0)
	#Iniciamos la stack del kernel
	movl $stack_top, %esp
	#Pasamos el numero magico del bootloader a la stack
	push %eax
	#La cabecera multiboot esta en ebx pero en una direccion fisica, le añadimos el offset para corregirlo y la pasamos a la stack
	add $KERNEL_VIRTUAL_BASE, %ebx
	push %ebx
	# Initialize the core kernel before running the global constructors.
	call kernel_early

	# Call the global constructors.
	call _init
	
	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
.Lhang:
	hlt
	jmp .Lhang
