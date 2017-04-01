/*
 * initrd.h
 *
 *  Created on: 25 mar. 2017
 *      Author: joaquin
 */

#ifndef KERNEL_FILESYSTEM_INITRD_H_
#define KERNEL_FILESYSTEM_INITRD_H_

#include <kernel/system.h>
#include <filesystem/vfs.h>

#define HEADER_MAGIC 0XFA
#define DIR_MAGIC 0XFE
#define FILE_MAGIC 0XF1
#define isDir(x)(( x & 0xFF) == DIR_MAGIC)

typedef struct{
	uint32_t magic;
	uint8_t flags;
	uint32_t firstNode; //offset of first ramdisk node
}__attribute__((packed)) initrd_header_t;

typedef struct{
	uint8_t magic; //offset of first ramdisk node
	uint8_t flags; // flags
	char name[64];  // Node name
	uint32_t parent; //offset of parent
	uint32_t next; //offset of next child of parent
	uint32_t child; //offset of first child if directory or offset of content
	uint32_t length;   // Length of the content if file.
}__attribute__((packed)) initrd_node_t;

typedef struct{
	uint8_t magic;     // Magic number, for error checking.
	uint8_t flags;   // Offset in the initrd that the file starts.
	uint8_t name[64];  // Filename.
	uint32_t length;   // Length of the file.
} initrd_file_header_t;

fs_node_t *initialise_initrd();

#endif /* KERNEL_FILESYSTEM_INITRD_H_ */
