.align 0x1000
.section .paging
.type paging_switch, @function
.global paging_switch
paging_switch:
	mov 4(%esp), %ecx
	mov %ecx, %cr3
 	movl %cr4, %ecx
	orl $0x00000010, %ecx #Activamos las paginas de 4MB en vez de las de 4KB
	movl %ecx, %cr4
	movl %cr0, %ecx
	orl $0x80000000, %ecx #Activamos la paginacion
	movl %ecx, %cr0
	lea (paging_ret), %ecx
	jmp *%ecx

.section .pagingret
.align 0x1000
.global paging_ret
paging_ret:
	ret

.type paging_flush, @function
.global paging_flush
paging_flush:
	mov 4(%esp), %eax
	invlpg (%eax)
	ret
