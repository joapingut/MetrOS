#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/memory/pmm.h>
#include <kernel/memory/alloc.h>

uint32_t getMemoryAmountFromGrub(multiboot_info_t *mbi, bool setMemory);

uint32_t first_free_frame_index();

static uint32_t max_frames = NULL; //Max number of frames for installed memory
static uint32_t *frames_Array; //Pointer to array with the frames status (0 FREE, 1 USED)

void pmm_install(multiboot_info_t *mbi) {
	// Leemos la cabecera que nos ha pasado GRUB para sacar el tamaño
	uint32_t mem_tam = getMemoryAmountFromGrub(mbi, false);
	max_frames = mem_tam / PAGE_TAM;
	//Direccion para el bitmap despues del kernel
	uint32_t next_v_addr = (uint32_t)(((uint32_t) &__KERNEL_END) + 4);
	frames_Array = next_v_addr;
	//Obtenemos el tamaño del array
	uint32_t array_tam = INDEX_FROM_BIT(max_frames);
	if(array_tam % 32 != 0){
		array_tam += 1;
	}
	//Limpiamos el contenido de la memoria
	memset(frames_Array, 0, (sizeof(uint32_t) * array_tam));
	// Iniciamos el kd
	kdmalloc_init(next_v_addr, KERNEL_VIRTUAL_BASE);
	// Le pedimos una direccion, esta garantizado que reserva el espacio del array
	uint32_t array_reserved = (uint32_t *) kdmalloc (sizeof(uint32_t) * array_tam);
	printf("\nFrames Array: %x : %x\n", frames_Array, array_reserved);
	//Leemos la cabecera que nos ha pasado GRUB de nuevo para marcar los lugares reservados
	getMemoryAmountFromGrub(mbi, true);
	printf("\nMax num frames: %d; TAM: %d bytes\n", max_frames, mem_tam);
	if(max_frames <= 4){
		PANIC("INSTALLED MEMORY BELOW 16MB");
	}
}

// Function to allocate a frame.
void alloc_frame_int(page_t *page, bool is_kernel, bool is_writeable, bool is_accessed, bool is_dirty, bool map_frame, uint32_t frameAddr){
	uint32_t idx;
	if(map_frame){
		idx = frameAddr >> 12;
	}else if(page->frame == 0){
		idx = first_free_frame_index(); // idx is now the index of the first free frame.
	} else {
		idx = page->frame;
	}
	
	set_frame(idx << 12); // this frame is now ours!
	page->ps = 0; //We are using 4KB pages
	page->present = 1; // Mark it as present.
	is_writeable = true;
	page->rw = (is_writeable == true) ? 1 : 0; // Should the page be writeable?
	is_kernel = false;
	page->user = (is_kernel == true) ? 0 : 1; // Should the page be user-mode?
	page->accessed = (is_accessed == true)?1:0;
	page->dirty = (is_dirty == true)?1:0;
	page->frame = idx;
}

void alloc_frame(page_t *page, bool is_kernel, bool is_writeable){
	alloc_frame_int(page, is_kernel, is_writeable, false, false, false, NULL);
}

// Function to deallocate a frame.
void free_frame(page_t *page){
	if(page == NULL){
		return;
	}
	if (!test_frame((page->frame) << 12)){
		return; // The given page didn't actually have an allocated frame!
	}else{
		clear_frame((page->frame) << 12); // Frame is now free again.
		*((uint32_t *)page) = 0;
	}
}

// Static function to set a bit in the frames bitset
void set_frame(uint32_t frame_addr){
	uint32_t frame = frame_addr/PAGE_TAM;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames_Array[idx] |= (0x1 << off);
}


// Static function to clear a bit in the frames bitset
void clear_frame(uint32_t frame_addr){
	uint32_t frame = frame_addr/PAGE_TAM;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames_Array[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
bool test_frame(uint32_t frame_addr){
	uint32_t frame = frame_addr/PAGE_TAM;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	return (frames_Array[idx] & (0x1 << off)) != 0;
}

// Static function to find the first free frame.
uint32_t first_free_frame_index(){
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(max_frames); i++){
		if (frames_Array[i] != 0xFFFFFFFF){ // nothing free, exit early.
			// at least one bit is free here.
			for (j = 0; j < 32; j++){
				uint32_t toTest = 0x1 << j;
				if ( !(frames_Array[i]&toTest) ){
					return j+(i*8*4);
				}
			}
		}
	}
	PANIC("No free frames!");
}

uint32_t first_frame() {
    uint32_t idx = first_free_frame_index(); // idx is now the index of the first free frame.
    set_frame(idx << 12);
    return (idx << 12);
}

uint32_t getMemoryAmountFromGrub(multiboot_info_t *mbi, bool setMemory){
	uint64_t memory = 0;
	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *) (mbi->mmap_addr + KERNEL_VIRTUAL_BASE);
	while((uint32_t) mmap < (mbi->mmap_addr + mbi->mmap_length) + KERNEL_VIRTUAL_BASE) {
		printf("Size = 0x%x, ", (uint32_t) mmap->size);
		printf("base_addr = 0x%x", (uint32_t)(mmap->addr >> 32));
		printf("%x, ", (uint32_t)(mmap->addr & 0xFFFFFFFF));
		printf("length = 0x%x", (uint32_t)(mmap->len >> 32));
		printf("%x, ", (uint32_t)(mmap->len & 0xFFFFFFFF));
		printf("type = 0x%x\n", (uint32_t) mmap->type);
		if((uint32_t)mmap->type == 0x01){
			memory += (uint64_t)mmap->len;
		}else if(setMemory){
			for(uint64_t i = mmap->addr; i < (mmap->addr + mmap->len); i += PAGE_TAM){
				set_frame((uint32_t)(i & 0xFFFFFFFF));
			}
		}
		mmap = (multiboot_memory_map_t *) ((unsigned int) mmap + (unsigned int)mmap->size + sizeof(uint32_t));
	}
	uint32_t initrd_location = *((uint32_t*)mbi->mods_addr);
	uint32_t initrd_end = *(uint32_t*)(mbi->mods_addr+4);
	if(initrd_location == initrd_end){
		printf("\nNo GRUB modules found");
		initrd_location = NULL;
	}else{
		printf("\nGRUB modules found!!");
		printf("\nFirst module from 0x%x to 0x%x", initrd_location, initrd_end);
		modules_firstAddress = initrd_location;
	}
	return (uint32_t)(memory & 0xFFFFFFFF);
}

uint32_t memory_used(){
	uint32_t i, j, used = 0;
	for (i = 0; i < INDEX_FROM_BIT(max_frames); i++){
		if (frames_Array[i] == 0x0){
			continue;
		}
		if (frames_Array[i] != 0xFFFFFFFF){
			// at least one bit is free here.
			for (j = 0; j < 32; j++){
				uint32_t toTest = 0x1 << j;
				if ((frames_Array[i]&toTest)){
					used += 1;
				}
			}
		} else {
			//Nothing free
			used += 32;
		}
	}
	return used * PAGE_TAM;
}